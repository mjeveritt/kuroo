/***************************************************************************
*   Copyright (C) 2004 by karye                                           *
*   karye@users.sourceforge.net                                           *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#include "common.h"
#include "packageemergetime.h"
#include "scanhistoryjob.h"

#include <qdir.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qheader.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qtextstream.h>

/**
 * Thread for parsing emerge/unmerge entries found in emerge.log.
 */
ScanHistoryJob::ScanHistoryJob( QObject* parent, const QStringList& logLines )
	: ThreadWeaver::DependentJob( parent, "DBJob" ),
	m_db( KurooDBSingleton::Instance()->getStaticDbConnection() ), m_logLines( logLines ), aborted(true)
{
}

ScanHistoryJob::~ScanHistoryJob()
{
	KurooDBSingleton::Instance()->returnStaticDbConnection(m_db);
	if ( aborted )
		SignalistSingleton::Instance()->scanAborted();
}

/**
 * Inform gui thread that scan is completed.
 */
void ScanHistoryJob::completeJob()
{
	kdDebug() << "ScanHistoryJob::completeJob" << endl;
	SignalistSingleton::Instance()->scanHistoryComplete();
	aborted = false;
}

/** 
 * If new entries are found in emerge.log insert them into history and 
 * add emerge time and increment how many time package have been installed
 * @return success
 */
bool ScanHistoryJob::doJob()
{
	kdDebug() << "ScanHistoryJob::doJob" << endl;
	
	if ( !m_db->isConnected() ) {
		kdDebug() << i18n("Can not connect to database") << endl;
		return false;
	}
	
	EmergeTimeMap emergeTimeMap(HistorySingleton::Instance()->getStatisticsMap());
	KurooDBSingleton::Instance()->query( "BEGIN TRANSACTION;", m_db );
	
	// Parse the output from genlog for new emerges, and unmerges
	static uint emergeStart(0);
	static QString emergeDate("");
	QString timeStamp;
	
	foreach ( m_logLines ) {
		
		// Abort the scan
		if ( isAborted() ) {
			kdDebug() << i18n("History scan aborted") << endl;
			KurooDBSingleton::Instance()->query("ROLLBACK TRANSACTION;", m_db);
			return false;
		}
		
		QRegExp rx("\\d+:\\s");
		QString package;
		QString emergeLine = QString(*it).section(rx, 1, 1);
		timeStamp = QString(*it).section(": " + emergeLine, 0, 0);
		QDateTime t;
		t.setTime_t(timeStamp.toUInt());
// 		kdDebug() << "timeStamp=" << timeStamp << ". " << t.toString("yyyy MM dd hh:mm") << endl;
		emergeDate = t.toString("MMM dd yyyy");
		
		if ( emergeLine.contains(">>> emerge") ) {
			emergeStart = timeStamp.toUInt();
		}
		else
			if ( emergeLine.contains("::: completed emerge ") ) {
				rx.setPattern("\\s\\S+/\\S+\\s");
				if (rx.search(emergeLine) > -1) {
					package = rx.cap(0).stripWhiteSpace();
				}
				
				QString packageNoVersion = package.section(pv, 0, 0);
				int secTime = timeStamp.toUInt() - emergeStart;
				QTime duration(0, 0, 0);
				duration = duration.addSecs(secTime);
				
				// Update emerge time and increment count for packageNoVersion
				EmergeTimeMap::iterator itMap = emergeTimeMap.find(packageNoVersion);
				if ( itMap == emergeTimeMap.end() ) {
					PackageEmergeTime pItem( secTime, 1 );
					emergeTimeMap.insert(packageNoVersion, pItem);
				}
				else {
					itMap.data().add(secTime);
					itMap.data().inc();
				}
				
				KurooDBSingleton::Instance()->insert( QString( "INSERT INTO history (package, date, timestamp, time, emerge) VALUES ('%1', '%2', '%3', '%4', 'true');" ).arg( package ).arg( emergeDate ).arg( timeStamp ).arg( duration.toString(Qt::TextDate) ), m_db );
			}
			else
				if ( emergeLine.contains(">>> unmerge success") ) {
					package = emergeLine.section(">>> unmerge success: ", 1, 1);
					KurooDBSingleton::Instance()->insert( QString( "INSERT INTO history (package, date, timestamp, emerge) VALUES ('%1', '%2', '%3', 'false');" ).arg( package ).arg( emergeDate ).arg( timeStamp ), m_db );
				}
				else
					if ( emergeLine.contains("=== Sync completed") )
						KurooDBSingleton::Instance()->insert( QString( "INSERT INTO history (package, date, timestamp, emerge) VALUES ('', '%1', '%2', 'false');" ).arg( emergeDate ).arg( timeStamp ), m_db );
	}
	KurooDBSingleton::Instance()->query( "COMMIT TRANSACTION;", m_db );

	KurooDBSingleton::Instance()->query( "DELETE FROM statistic;", m_db );
	KurooDBSingleton::Instance()->query( "BEGIN TRANSACTION;", m_db );
	EmergeTimeMap::iterator itMapEnd = emergeTimeMap.end();
	for ( EmergeTimeMap::iterator itMap = emergeTimeMap.begin(); itMap != itMapEnd; itMap++ ) {
		
		// Abort the scan
		if ( isAborted() ) {
			kdDebug() << i18n("History scan aborted") << endl;
			KurooDBSingleton::Instance()->query("ROLLBACK TRANSACTION;", m_db);
			return false;
		}
		
		KurooDBSingleton::Instance()->insert( QString( "INSERT INTO statistic (time, count, package) VALUES ('%1', '%2', '%3');" ).arg( itMap.data().emergeTime() ).arg( itMap.data().count() ).arg( itMap.key() ), m_db );
	}
	KurooDBSingleton::Instance()->query( "COMMIT TRANSACTION;", m_db );
	
	HistorySingleton::Instance()->setStatisticsMap( emergeTimeMap );	
	KurooConfig::setScanHistoryDate( timeStamp );
	KurooConfig::writeConfig();

	return true;
}

#include "scanhistoryjob.moc"

