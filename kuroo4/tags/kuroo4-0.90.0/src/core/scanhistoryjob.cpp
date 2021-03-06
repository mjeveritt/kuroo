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
#include <qfileinfo.h>

/**
 * @class ScanHistoryJob
 * @short Thread for parsing emerge/unmerge/sync entries found in emerge.log.
 */
ScanHistoryJob::ScanHistoryJob( QObject* parent, const QStringList& logLines )
    : ThreadWeaver::Job( parent ),
	m_db( KurooDBSingleton::Instance()->getStaticDbConnection() ), m_logLines( logLines )
{}

ScanHistoryJob::~ScanHistoryJob()
{
	KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );

    /*if ( isAborted() )
        SignalistSingleton::Instance()->scanAborted();*/
}

/**
 * If new entries are found in emerge.log insert them into history and,
 * add emerge duration and increment emerge counts.
 * @return success
 */
void ScanHistoryJob::run()
{
	if ( !m_db->isConnected() ) {
		kError(0) << "Parsing emerge.log. Can not connect to database" << LINE_INFO;
        return;
	}

	EmergeTimeMap emergeTimeMap( HistorySingleton::Instance()->getStatisticsMap() );

	KurooDBSingleton::Instance()->singleQuery( "BEGIN TRANSACTION;", m_db );

	// Parse emerge.log lines
	QString timeStamp, syncTimeStamp;
	QRegExp rxTimeStamp( "\\d+:\\s" );
	QRegExp rxPackage( "(\\s+)(\\S+/\\S+)" );
	static QMap<QString, uint> logMap;
	bool isStatisticUpdated( false );
	eLog elog;
	QString einfo;

    foreach ( QString line, m_logLines ) {

		// Abort the scan
        /*if ( isAborted() ) {
			kWarning(0) << "Parsing emerge.log. History scan aborted" << LINE_INFO;
			KurooDBSingleton::Instance()->singleQuery( "ROLLBACK TRANSACTION;", m_db );
            return;
        }*/

        QString emergeLine = QString( line ).section( rxTimeStamp, 1, 1 );
        timeStamp = QString( line ).section( ": " + emergeLine, 0, 0 );

		if ( emergeLine.contains( ">>> emerge" ) ) {
			uint emergeStart = timeStamp.toUInt();

			// Collect package and timestamp in map to match for completion
			QString package;
            if ( rxPackage.indexIn( emergeLine ) > -1 ) {
				package = rxPackage.cap(2);
				logMap[ package ] = emergeStart;
			}
			else
				kWarning(0) << "Parsing emerge.log. No package found!" << LINE_INFO;
		}
		else
			if ( emergeLine.contains( "::: completed emerge " ) ) {
				QString package;
                if ( rxPackage.indexIn( emergeLine ) > -1 ) {
					package = rxPackage.cap(2);

					// Find matching package emerge start entry in map and calculate the emerge duration
					QMap<QString, uint>::iterator itLogMap = logMap.find( package );
					if ( itLogMap != logMap.end() ) {
						isStatisticUpdated = true;
                        uint emergeStart = itLogMap.value();
						uint emergeCompleted = timeStamp.toUInt();
						int secTime = emergeCompleted - emergeStart;
						logMap.erase( itLogMap );

						// Find matching elog file
						eLogVector eLogs = HistorySingleton::Instance()->getELogs();
						if ( eLogs.size() > 0 ) {

							elog = eLogs.last();
							while ( eLogs.size() > 0 && elog.timestamp < emergeStart ) {
								elog = eLogs.last();
								eLogs.pop_back();
							}

							if ( elog.timestamp >= emergeStart && elog.timestamp <= emergeCompleted )
								einfo = elog.package;
							else
								einfo = "";

// 							kDebug() << "elog.package=" << elog.package;
// 							kDebug() << "elog.timestamp=" << elog.timestamp << LINE_INFO;
// 							kDebug() << "emergeStart=" << emergeStart << LINE_INFO;
// 							kDebug() << "emergeCompleted=" << emergeCompleted << LINE_INFO;
						}

                        QStringList parts = parsePackage( package );
						if ( !parts.isEmpty() ) {
							QString categoryNameString = parts[0] + "/" + parts[1];

							// Update emerge time and increment count for packageNoVersion
							EmergeTimeMap::iterator itMap = emergeTimeMap.find( categoryNameString );
							if ( itMap == emergeTimeMap.end() ) {
								PackageEmergeTime pItem( secTime, 1 );
								emergeTimeMap.insert( categoryNameString, pItem );
							}
							else {
                                itMap.value().add( secTime );
                                itMap.value().inc();
							}

// 							QString einfo = EmergeSingleton::Instance()->packageMessage().utf8();
							KurooDBSingleton::Instance()->insert( QString(
								"INSERT INTO history (package, timestamp, time, einfo, emerge) "
								"VALUES ('%1', '%2', '%3', '%4','true');" )
							    .arg( package ).arg( timeStamp ).arg( QString::number( secTime ) ).arg( escapeString( einfo ) ), m_db );
						}
						else
							kWarning(0) << "Parsing emerge.log. Can not parse: " << package << LINE_INFO;
					}
				}
				else
					kWarning(0) << "Parsing emerge.log. No package found!" << LINE_INFO;
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
	KurooDBSingleton::Instance()->singleQuery( "COMMIT TRANSACTION;", m_db );

	if ( isStatisticUpdated )
		HistorySingleton::Instance()->setStatisticsMap( emergeTimeMap );

	if ( !syncTimeStamp.isEmpty() )
		KurooDBSingleton::Instance()->singleQuery( QString("UPDATE dbInfo SET data = '%1' WHERE meta = 'syncTimeStamp';").arg( syncTimeStamp ), m_db );

	SignalistSingleton::Instance()->scanHistoryComplete();
}

QString ScanHistoryJob::escapeString(const QString& str) const {
	QString result=str;
	return result.replace('\'', "''");
}

#include "scanhistoryjob.moc"
