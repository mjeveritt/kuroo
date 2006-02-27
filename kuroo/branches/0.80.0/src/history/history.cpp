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
#include "cacheportagejob.h"

#include <qdatetime.h>

#include <kprocio.h>
#include <kdirwatch.h>

class UpdateStatisticsJob : public ThreadWeaver::DependentJob
{
public:
UpdateStatisticsJob( QObject *dependent ) : DependentJob( dependent, "DBJob" ) {}
	
	virtual bool doJob() {
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->query( "DELETE FROM statistic;", m_db );
		KurooDBSingleton::Instance()->query( "BEGIN TRANSACTION;", m_db );
		
		EmergeTimeMap emergeTimeMap( HistorySingleton::Instance()->getStatisticsMap() );
		EmergeTimeMap::iterator itMapEnd = emergeTimeMap.end();
		for ( EmergeTimeMap::iterator itMap = emergeTimeMap.begin(); itMap != itMapEnd; itMap++ ) {
			KurooDBSingleton::Instance()->insert( QString( "INSERT INTO statistic (time, count, package) VALUES ('%1', '%2', '%3');" )
			                                      .arg(itMap.data().emergeTime()).arg(itMap.data().count()).arg(itMap.key()), m_db );
		}
		
		KurooDBSingleton::Instance()->query( "COMMIT TRANSACTION;", m_db );
		KurooDBSingleton::Instance()->returnStaticDbConnection(m_db);
		return true;
	}
	
};

/**
 * @class History
 * @short Object for the emerge history and statistics.
 * History watches for changes in emerge.log and parses new entries to register emerges and unmerges of packages in database.
 */
History::History( QObject *m_parent )
	: QObject( m_parent ), userSync( false ), isEmerging( false ), logWatcher( 0 )
{
	slotInit();
}

History::~History()
{
	log.close();
	
	delete logWatcher;
	logWatcher = 0;
}

void History::init( QObject *parent )
{
	m_parent = parent;
	if ( !log.open(IO_ReadOnly) )
		kdDebug() << i18n("Error reading /var/log/emerge.log") << endl;
	else
		stream.setDevice( &log );
}

/**
 * Load emerge statistics if any.
 * And start a watch on the emerge.log for changes.
 */
void History::slotInit()
{
	log.setName( "/var/log/emerge.log" );
	loadTimeStatistics();
	
	connect( SignalistSingleton::Instance(), SIGNAL( signalScanHistoryComplete() ), this, SLOT( slotScanHistoryCompleted() ) );
	
	logWatcher = new KDirWatch(this);
	logWatcher->addFile( "/var/log/emerge.log" );
	connect( logWatcher, SIGNAL( dirty( const QString& ) ), this, SLOT( slotParse() ) );
}

void History::slotScanHistoryCompleted()
{
	emit signalScanHistoryCompleted();
}

/**
 * Check for new entries in emerge.log similar to a "tail".
 * @return false if emerge log shows changes.
 */
bool History::slotRefresh()
{
	kdDebug() << "History::slotRefresh" << endl;
	
	QString lastDate = KurooDBSingleton::Instance()->lastHistoryEntry();
	if ( lastDate.isEmpty() )
		lastDate = "0";
	
	// Collect all recent entries in emerge log
	QStringList emergeLines;
	QRegExp rx( "\\d+" );
	while ( !stream.atEnd() ) {
		QString line = stream.readLine();
		if ( rx.search(line) > -1 )
			if ( rx.cap(0) > lastDate )
				if ( line.contains( QRegExp("(>>> emerge)|(=== Sync completed)|(::: completed emerge)|(>>> unmerge success)") ) )
					emergeLines += line + " ";
	}

	// Check only for successfull emerge/unmerges or sync outside kuroo
	if ( !emergeLines.grep(QRegExp("(=== Sync completed)|(::: completed emerge)|(>>> unmerge success)")).isEmpty() ) {
		slotScanHistory( emergeLines );
		return false;
	}

	slotScanHistoryCompleted();
	return true;
}

/**
 * Load the history map with emerge times statistics from database.
 */
void History::loadTimeStatistics()
{
	m_statisticsMap.clear();
	const QStringList timePackageList = KurooDBSingleton::Instance()->allStatistic();
	foreach ( timePackageList ) {
		QString package = *it++;
		QString time = *it++;
		QString count = *it;
		PackageEmergeTime p( time.toInt(), count.toInt() );
		m_statisticsMap.insert( package, p );
	}
}

/**
 * Return statistics map.
 * @return m_statisticsMap
 */
EmergeTimeMap History::getStatisticsMap()
{
	return m_statisticsMap;
}

/**
 * Set statistics map.
 * @param timeMap
 */
void History::setStatisticsMap( const EmergeTimeMap& statisticsMap )
{
	m_statisticsMap = statisticsMap;
}

/**
 * Get emerge time for this package.
 * @param package
 * @return emergeTime		time or na
 */
QString History::packageTime( const QString& packageNoversion )
{
	EmergeTimeMap::iterator itMap = m_statisticsMap.find( packageNoversion );
	if ( itMap != m_statisticsMap.end() )
		return QString::number( itMap.data().emergeTime() / itMap.data().count() );
	else
		return QString::null;
}

/**
 * Launch scan to load into db.
 * @param emergeLines
 */
void History::slotScanHistory( const QStringList& lines )
{
	SignalistSingleton::Instance()->scanStarted();
	ThreadWeaver::instance()->queueJob( new ScanHistoryJob( this, lines ) );
}

/**
 * Parse emerge.log when it has been changed, eg after, emerge, unmerge, sync...
 * Allow for translation of the output.
 */
void History::slotParse()
{
	static bool syncDone( false );
	QStringList emergeLines;
	QRegExp rxTimeStamp( "\\d+:\\s" );
	QRegExp rxPackage( "(\\()(\\d+)(\\s+of\\s+)(\\d+)(\\)\\s+)(\\S+/\\S+)" );
	
	while ( !stream.atEnd() )
		emergeLines += stream.readLine() + " ";
	
	foreach ( emergeLines ) {
		QString line = *it;
		
		if ( !line.isEmpty() ) {
			QString emergeLine = line.section( rxTimeStamp, 1, 1 );
			emergeLine = emergeLine.section( QRegExp( "(!!! )|(>>> )|(=== )|(\\*\\*\\* )|(::: )" ), 1, 1 );
			
			// For translation
			emergeLine.replace( " to ", i18n(" to ") );
			emergeLine.replace( " of ", i18n(" of ") );
			
			// Is it a regular emerge or something else
			if ( QRegExp("(\\s|\\S)*(\\*\\*\\* emerge)(\\s|\\S)*" ).exactMatch( line ) ) {
				isEmerging = true;
				
				// Not emerging - just downloading the packages
				if ( line.contains( QRegExp("fetch-all-uri") ) )
					isEmerging = false;
			}

			// Parse out nice statusbar text
			if ( line.contains( QRegExp("(\\) )(Cleaning)|(Compiling/Merging)|(Post-Build Cleaning)") ) ) {
				QString logLine = "(" + emergeLine.section( "::", 0, 0 ).remove( "(" );
				
				logLine.replace( "Compiling/Merging", i18n( "Compiling/Merging" ) );
				logLine.replace( "Post-Build Cleaning", i18n( "Post-Build Cleaning" ) );
				logLine.replace( "Cleaning", i18n( "Cleaning" ) );
				
				KurooStatusBar::instance()->setProgressStatus( QString::null, logLine );
				LogSingleton::Instance()->writeLog( logLine, EMERGELOG );
			}
			else
			if ( line.contains( "Started emerge on" ) ) {
				line.replace( "Started emerge on", i18n( "Started emerge on" ) );
				LogSingleton::Instance()->writeLog( line.section( rxTimeStamp, 1, 1 ), EMERGELOG );
			}
			else // Emerge has started, signal queue to launch progressbar for this package
			if ( line.contains( ">>> emerge" ) && isEmerging ) {
				if ( rxPackage.search( line ) > -1 ) {
					int order = rxPackage.cap(2).toInt();
					int total = rxPackage.cap(4).toInt();
					QString package = rxPackage.cap(6);
					QueueSingleton::Instance()->emergePackageStart( package, order, total );
				}
				else
					kdDebug() << i18n("Can not parse package emerge start in /var/log/emerge.log: %1").arg( line ) << endl;
			}
			else // Emerge has completed, signal queue to mark package as installed
			if ( line.contains( "::: completed emerge " ) && isEmerging ) {
				if ( rxPackage.search( line ) > -1 ) {
					int order = rxPackage.cap(2).toInt();
					int total = rxPackage.cap(4).toInt();
					QString package = rxPackage.cap(6);
					QueueSingleton::Instance()->emergePackageComplete( package, order, total );
					InstalledSingleton::Instance()->addPackage( package );
					UpdatesSingleton::Instance()->removePackage( package );
					emit signalHistoryChanged();
				}
				else
					kdDebug() << i18n("Can not parse package emerge complete in /var/log/emerge.log: %1").arg( line ) << endl;
				
				emergeLine.replace( "completed emerge", i18n( "completed emerge" ) );
				LogSingleton::Instance()->writeLog( emergeLine, EMERGELOG );
			}
			else
			if ( emergeLine.contains("unmerge success") ) {
				QString package = emergeLine.section( "unmerge success: ", 1, 1 );
				InstalledSingleton::Instance()->removePackage( package );
				emergeLine.replace( "unmerge success", i18n( "unmerge success" ) );
				LogSingleton::Instance()->writeLog( emergeLine, EMERGELOG );
				emit signalHistoryChanged();
			}
			else
			if ( emergeLine.contains( "starting rsync" ) ) {
				KurooStatusBar::instance()->setProgressStatus( QString::null, i18n( "Synchronizing Portage..." ) );
				LogSingleton::Instance()->writeLog( i18n( "Synchronizing Portage..." ), EMERGELOG );
			}
			else
			if ( emergeLine.contains( "Sync completed" ) ) {
				syncDone = true;
				KurooStatusBar::instance()->setProgressStatus( QString::null, i18n( "Sync completed." ) );
				LogSingleton::Instance()->writeLog( i18n( "Sync completed." ), EMERGELOG );
			}
			else
			if ( emergeLine.contains( "terminating." ) ) {
				QueueSingleton::Instance()->stopTimer();
				KurooStatusBar::instance()->setProgressStatus( QString::null, i18n( "Done." ) );
				LogSingleton::Instance()->writeLog( i18n( "Done." ), EMERGELOG );
				
				if ( syncDone ) {
					syncDone = false;
					SignalistSingleton::Instance()->syncDone();
				}
				
			}
			else {
				KurooStatusBar::instance()->setProgressStatus( QString::null, emergeLine );
				
				emergeLine.replace( "AUTOCLEAN", i18n( "AUTOCLEAN" ) );
				continue;
				
				emergeLine.replace( "Unmerging", i18n( "Unmerging" ) );
				emergeLine.replace( "Finished. Cleaning up", i18n( "Finished. Cleaning up" ) );
				emergeLine.replace( "exiting successfully", i18n( "exiting successfully" ) );
				emergeLine.replace( "terminating", i18n( "terminating" ) );
				LogSingleton::Instance()->writeLog( emergeLine, EMERGELOG );
			}
		}
	}
	
	// Update history
	if ( !emergeLines.isEmpty() )
		slotScanHistory( emergeLines );
}

/**
 * Update emerge duration statistic.
 */
void History::updateStatistics()
{
	ThreadWeaver::instance()->queueJob( new UpdateStatisticsJob( this ) );
}

/**
 * Register einfo in db for package.
 */
void History::appendEmergeInfo()
{
	QString einfo = EmergeSingleton::Instance()->packageMessage().utf8();
	if ( !einfo.isEmpty() )
		KurooDBSingleton::Instance()->addEmergeInfo( einfo );
}

/**
 * Return all etc-file merge history.
 * @return QStringList
 */
QStringList History::allMergeHistory()
{
	return KurooDBSingleton::Instance()->allMergeHistory();
}

#include "history.moc"
