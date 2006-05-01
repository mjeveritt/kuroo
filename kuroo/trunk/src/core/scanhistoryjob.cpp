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
#include <qtextstream.h>

/**
 * @class ScanHistoryJob
 * @short Thread for parsing emerge/unmerge entries found in emerge.log.
 */
ScanHistoryJob::ScanHistoryJob( QObject* parent, const QStringList& logLines )
	: ThreadWeaver::DependentJob( parent, "DBJob" ),
	m_db( KurooDBSingleton::Instance()->getStaticDbConnection() ), m_logLines( logLines )
{}

ScanHistoryJob::~ScanHistoryJob()
{
	KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );

	if ( isAborted() )
		SignalistSingleton::Instance()->scanAborted();
}

/**
 * Inform gui thread that scan is completed.
 */
void ScanHistoryJob::completeJob()
{
	SignalistSingleton::Instance()->scanHistoryComplete();
}

/** 
 * If new entries are found in emerge.log insert them into history and,
 * add emerge duration and increment emerge counts.
 * @return success
 */
bool ScanHistoryJob::doJob()
{
	if ( !m_db->isConnected() ) {
		kdError(0) << "Parsing emerge.log. Can not connect to database" << LINE_INFO;
		return false;
	}
	
	EmergeTimeMap emergeTimeMap( HistorySingleton::Instance()->getStatisticsMap() );
	KurooDBSingleton::Instance()->query( "BEGIN TRANSACTION;", m_db );
	
	// Parse emerge.log lines
	QString timeStamp, syncTimeStamp;
	QRegExp rxTimeStamp( "\\d+:\\s" );
	QRegExp rxPackage( "(\\s+)(\\S+/\\S+)" );
	static QMap<QString, uint> logMap;
	bool isStatisticUpdated( false );
	
	foreach ( m_logLines ) {
		
		// Abort the scan
		if ( isAborted() ) {
			kdWarning(0) << "Parsing emerge.log. History scan aborted" << LINE_INFO;
			KurooDBSingleton::Instance()->query( "ROLLBACK TRANSACTION;", m_db );
			return false;
		}
		
		QString emergeLine = QString(*it).section( rxTimeStamp, 1, 1 );
		timeStamp = QString(*it).section( ": " + emergeLine, 0, 0 );
		
		if ( emergeLine.contains( ">>> emerge" ) ) {
			uint emergeStart = timeStamp.toUInt();
			
			// Collect package and timestamp in map to match for completion
			QString package;
			if ( rxPackage.search( emergeLine ) > -1 ) {
				package = rxPackage.cap(2);
				logMap[ package ] = emergeStart;
			}
			else
				kdWarning(0) << "Parsing emerge.log. No package found!" << LINE_INFO;
		}
		else
			if ( emergeLine.contains( "::: completed emerge " ) ) {
				QString package;
				if ( rxPackage.search( emergeLine ) > -1 ) {
					package = rxPackage.cap(2);
				
					// Find matching package emerge start entry in map and calculate the emerge duration
					QMap<QString, uint>::iterator itLogMap = logMap.find( package );
					if ( itLogMap != logMap.end() ) {
						isStatisticUpdated = true;
						int secTime = timeStamp.toUInt() - itLogMap.data();
						logMap.erase( itLogMap );
						
						QStringList parts = GlobalSingleton::Instance()->parsePackage( package );
						if ( !parts.isEmpty() ) {
							QString categoryNameString = parts[0] + "/" + parts[1];

							// Update emerge time and increment count for packageNoVersion
							EmergeTimeMap::iterator itMap = emergeTimeMap.find( categoryNameString );
							if ( itMap == emergeTimeMap.end() ) {
								PackageEmergeTime pItem( secTime, 1 );
								emergeTimeMap.insert( categoryNameString, pItem );
							}
							else {
								itMap.data().add( secTime );
								itMap.data().inc();
							}
							
							QString einfo = EmergeSingleton::Instance()->packageMessage().utf8();
							KurooDBSingleton::Instance()->insert( QString( 
								"INSERT INTO history (package, timestamp, time, einfo, emerge) "
								"VALUES ('%1', '%2', '%3', '%4','true');" )
							    .arg( package ).arg( timeStamp ).arg( QString::number( secTime ) ).arg( escapeString( einfo ) ), m_db );
						}
						else
							kdWarning(0) << "Parsing emerge.log. Can not parse: " << package << LINE_INFO;
					}
				}
				else
					kdWarning(0) << "Parsing emerge.log. No package found!" << LINE_INFO;
			}
			else {
				if ( emergeLine.contains( ">>> unmerge success" ) ) {
					QString package = emergeLine.section( ">>> unmerge success: ", 1, 1 );
					KurooDBSingleton::Instance()->insert( QString( "INSERT INTO history (package, timestamp, emerge) VALUES ('%1', '%2', 'false');" )
					    .arg( package ).arg( timeStamp ), m_db );
				}
				else
					if ( emergeLine.contains( "=== Sync completed" ) )
						syncTimeStamp = timeStamp;
			}
	}
	KurooDBSingleton::Instance()->query( "COMMIT TRANSACTION;", m_db );
	
	if ( isStatisticUpdated )
		HistorySingleton::Instance()->setStatisticsMap( emergeTimeMap );
	
	if ( !syncTimeStamp.isEmpty() )
		KurooDBSingleton::Instance()->query( QString("UPDATE dbInfo SET data = '%1' WHERE meta = 'syncTimeStamp';").arg( syncTimeStamp ), m_db );
	
	return true;
}

#include "scanhistoryjob.moc"
