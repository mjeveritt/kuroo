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

/**
 * Object for the emerge history and statistics.
 * History watches for changes in emerge.log and parses new entries to register emerges and unmerges of packages in database.
 */
History::History( QObject *parent )
	: QObject( parent ), userSync( false )
{
	slotInit();
}

History::~History()
{
	log.close();
	
	delete fileWatcher;
	fileWatcher = 0;
}

void History::init( QObject *myParent )
{
	kdDebug() << "History::init" << endl;
	parent = myParent;
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
	kdDebug() << "History::slotInit" << endl;
	log.setName("/var/log/emerge.log");
	loadTimeStatistics();
	
	connect( SignalistSingleton::Instance(), SIGNAL( signalScanHistoryComplete() ), this, SLOT( slotChanged() ) );
	
	fileWatcher = new KDirWatch(this);
	fileWatcher->addFile("/var/log/emerge.log");
	connect( fileWatcher, SIGNAL( dirty(const QString&) ), this, SLOT( slotParse() ) );
}

/**
 * Check for new entries in emerge.log similar to a "tail".
 * @return false if emerge log shows changes.
 */
bool History::slotRefresh()
{
	QString lastDate = KurooDBSingleton::Instance()->lastHistoryEntry().first();
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

	slotChanged();
	return true;
}

void History::slotChanged()
{
	kdDebug() << "History::slotChanged" << endl;
	emit signalHistoryChanged();
}

/**
 * Load the history map with emerge times statistics from database.
 */
void History::loadTimeStatistics()
{
	m_statisticsMap.clear();
	const QStringList timePackageList = KurooDBSingleton::Instance()->statistic();
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
		return NULL;
}

/**
 * Launch scan to load into db.
 * @param emergeLines
 */
void History::slotScanHistory( const QStringList& lines )
{
	SignalistSingleton::Instance()->scanStarted();
	ThreadWeaver::instance()->queueJob( new ScanHistoryJob(this, lines) );
}

/**
 * Return all emerge/unmerge history.
 * @return QStringList
 */
QStringList History::allHistory()
{
	return KurooDBSingleton::Instance()->history();
}

/**
 * Parse emerge.log when it has been changed, eg after, emerge, unmerge, sync...
 * Allow for translation of the output.
 */
void History::slotParse()
{
	kdDebug() << "History::slotParse" << endl;
	
	static bool syncDone( false );
	QStringList emergeLines;
	
	while ( !stream.atEnd() )
		emergeLines += stream.readLine() + " ";

	// Update history
	if ( !emergeLines.isEmpty() )
		slotScanHistory( emergeLines );
	
	foreach ( emergeLines ) {
		QString line = *it;
		
		if ( !line.isEmpty() ) {
			QRegExp rx( "\\d+:\\s" );
			QString emergeLine = line.section( rx, 1, 1 );
			emergeLine = emergeLine.section( QRegExp( "(!!! )|(>>> )|(=== )|(\\*\\*\\* )|(::: )" ), 1, 1 );
			
			emergeLine.replace( " to ", i18n(" to ") );
			emergeLine.replace( " of ", i18n(" of ") );
			
			// Parse out nice statusbar text
			if ( line.contains( QRegExp("(\\) )(Cleaning)|(Compiling/Merging)|(Post-Build Cleaning)") ) ) {
				QString logLine = "(" + emergeLine.section( "::", 0, 0 ).remove( "(" );
				
				logLine.replace( "Compiling/Merging", i18n( "Compiling/Merging" ) );
				logLine.replace( "Post-Build Cleaning", i18n( "Post-Build Cleaning" ) );
				logLine.replace( "Cleaning", i18n( "Cleaning" ) );
				
				KurooStatusBar::instance()->setProgressStatus( logLine );
				LogSingleton::Instance()->writeLog( logLine, EMERGELOG );
			}
			else
			if ( line.contains( "Started emerge on" ) ) {
				line.replace( "Started emerge on", i18n( "Started emerge on" ) );
				LogSingleton::Instance()->writeLog( line.section( rx, 1, 1 ), EMERGELOG );
			}
			else
			if ( line.contains( ">>> emerge" ) ) {
				rx.setPattern( "\\s\\S+/\\S+\\s" );
				if ( rx.search( emergeLine ) > -1 ) {
					QString package = rx.cap( 0 ).stripWhiteSpace();
					SignalistSingleton::Instance()->emergePackageStart( package );
				}
				else
					kdDebug() << i18n("No package found!") << endl;
			}
			else
			if ( line.contains( "::: completed emerge " ) ) {
				rx.setPattern( "\\s\\S+/\\S+\\s" );
				if ( rx.search( emergeLine ) > -1 ) {
					QString package = rx.cap( 0 ).stripWhiteSpace();
					SignalistSingleton::Instance()->emergePackageStop( package );
					InstalledSingleton::Instance()->addPackage( package );
					QueueSingleton::Instance()->addPackage( package );
					UpdatesSingleton::Instance()->removePackage( package );
				}
				else
					kdDebug() << i18n("No package found!") << endl;
				
				emergeLine.replace( "completed emerge", i18n( "completed emerge" ) );
				LogSingleton::Instance()->writeLog( emergeLine, EMERGELOG );
			}
			else
			if ( emergeLine.contains("unmerge success") ) {
				QString package = emergeLine.section( "unmerge success: ", 1, 1 );
				InstalledSingleton::Instance()->removePackage( package );
				
				emergeLine.replace( "unmerge success", i18n( "unmerge success" ) );
				LogSingleton::Instance()->writeLog( emergeLine, EMERGELOG );
			}
			else
			if ( emergeLine.contains( "starting rsync" ) ) {
				KurooStatusBar::instance()->setProgressStatus( i18n( "Synchronizing Portage..." ) );
				LogSingleton::Instance()->writeLog( i18n( "Synchronizing Portage..." ), EMERGELOG );
			}
			else
			if ( emergeLine.contains( "Sync completed" ) ) {
				syncDone = true;
				KurooStatusBar::instance()->setProgressStatus( i18n( "Sync completed." ) );
				LogSingleton::Instance()->writeLog( i18n( "Sync completed." ), EMERGELOG );
			}
			else
			if ( emergeLine.contains( "terminating." ) ) {
				KurooStatusBar::instance()->setProgressStatus( i18n( "Done." ) );
				LogSingleton::Instance()->writeLog( i18n( "Done." ), EMERGELOG );
				if ( syncDone ) {
					syncDone = false;
					SignalistSingleton::Instance()->syncDone();
				}
			}
			else {
				emergeLine.replace( "AUTOCLEAN", i18n( "AUTOCLEAN" ) );
				emergeLine.replace( "Unmerging", i18n( "Unmerging" ) );
				emergeLine.replace( "Finished. Cleaning up", i18n( "Finished. Cleaning up" ) );
				emergeLine.replace( "exiting successfully", i18n( "exiting successfully" ) );
				emergeLine.replace( "terminating", i18n( "terminating" ) );
				
				KurooStatusBar::instance()->setProgressStatus( emergeLine );
				LogSingleton::Instance()->writeLog( emergeLine, EMERGELOG );
			}
		}
	}
}

/**
 * Register einfo in db for package.
 */
void History::appendEmergeInfo( const QString& einfo )
{
	KurooDBSingleton::Instance()->query( QString("UPDATE history SET einfo = '%1' WHERE id = (SELECT MAX(id) FROM history);").arg( einfo.utf8() ) );
}

/**
 * Return all etc-file merge history.
 * @return QStringList
 */
QStringList History::allMergeHistory()
{
	return KurooDBSingleton::Instance()->mergeHistory();
}

#include "history.moc"
