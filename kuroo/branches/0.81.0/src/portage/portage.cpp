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
#include "scanportagejob.h"
#include "cacheportagejob.h"
#include "scanupdatesjob.h"
#include "loadportagejob.h"

#include <kmessagebox.h>

#include <unistd.h>

/**
 * @class AddInstalledPackageJob
 * @short Thread for registrating packages as installed in db.
 */
class AddInstalledPackageJob : public ThreadWeaver::DependentJob
{
public:
	AddInstalledPackageJob( QObject *dependent, const QString& package ) : DependentJob( dependent, "DBJob" ), m_package( package ) {}
	
	virtual bool doJob() {
		
		QStringList parts = GlobalSingleton::Instance()->parsePackage( m_package );
		if ( parts.isEmpty() ) {
			kdWarning(0) << QString("Inserting emerged package: can not match %1.").arg( m_package ) << LINE_INFO;
			return false;
		}
		QString category = parts[0];
		QString name = parts[1];
		QString version = parts[2];
		
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		QString id = KurooDBSingleton::Instance()->singleQuery( QString( "SELECT id FROM package "
			"WHERE name = '%1' AND idCatSubCategory = ( SELECT id from catSubCategory WHERE name = '%2' ); ")
			.arg( name ).arg( category ), m_db );
		
		if ( id.isEmpty() ) {
			
			kdWarning(0) << QString("Inserting emerged package: Can not find id in database for package %1/%2.")
				.arg( category ).arg( name ) << LINE_INFO;
			
			KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
			return false;
		}
		else {
			KurooDBSingleton::Instance()->query( QString( "UPDATE package SET status = '%1' WHERE id = '%2';" )
			                                     .arg( PACKAGE_INSTALLED_STRING ).arg( id ), m_db );
			
			KurooDBSingleton::Instance()->query( QString( "UPDATE version SET status = '%1' WHERE idPackage = '%2' AND name = '%3';" )
			                                     .arg( PACKAGE_INSTALLED_STRING ).arg( id ).arg( version ), m_db );
			
			// Clean out the update string if this version was installed
			KurooDBSingleton::Instance()->query( QString( "UPDATE package SET updateVersion = '' "
			                                              "WHERE id = '%1' AND ( updateVersion = '%2' OR updateVersion = '%3' );" )
			                                     .arg( id ).arg( version + " (D)" ).arg( version + " (U)" ), m_db );
			
			KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
			return true;
		}
	}
	
	virtual void completeJob() {
		PortageSingleton::Instance()->slotPackageChanged();
	}
	
private:
	const QString m_package;
};

/**
 * @class RemoveInstalledPackageJob
 * @short Thread for removing packages as installed in db.
 */
class RemoveInstalledPackageJob : public ThreadWeaver::DependentJob
{
public:
	RemoveInstalledPackageJob( QObject *dependent, const QString& package ) : DependentJob( dependent, "DBJob" ), m_package( package ) {}
	
	virtual bool doJob() {
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		
		QStringList parts = GlobalSingleton::Instance()->parsePackage( m_package );
		if ( parts.isEmpty() ) {
			kdWarning(0) << QString("Removing unmerged package: can not match %1.").arg( m_package ) << LINE_INFO;
			return false;
		}
		QString category = parts[0];
		QString name = parts[1];
		QString version = parts[2];
		
		QString id = KurooDBSingleton::Instance()->singleQuery( QString( "SELECT id FROM package WHERE "
			"name = '%1' AND idCatSubCategory = ( SELECT id from catSubCategory WHERE name = '%2' ); ")
			.arg( name ).arg( category ), m_db );
		
		if ( id.isEmpty() ) {
			kdWarning(0) << QString("Removing unmerged package: Can not find id in database for package %1/%2.")
				.arg( category ).arg( name ) << LINE_INFO;
			
			KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
			return false;
		}
		else {
			
			// Check how many version are installed
			QString installedVersionCount = KurooDBSingleton::Instance()->singleQuery( 
				QString( "SELECT COUNT(id) FROM version WHERE idPackage = '%1' AND status = '%2' LIMIT 1;")
				.arg( id ).arg( PACKAGE_INSTALLED_STRING ), m_db );
			

			// Mark package as uninstalled only when one version is found
			if ( installedVersionCount == "1" ) {
			
				// Mark package as uninstalled
				KurooDBSingleton::Instance()->query( QString( "UPDATE package SET status = '%1' WHERE id = '%3'")
				                                     .arg( PACKAGE_AVAILABLE_STRING ).arg( id ), m_db );
			
				// Delete package if "old" = not in official Portage anymore
				KurooDBSingleton::Instance()->query( QString( "UPDATE package SET status = '%1' WHERE status = '%1' AND id = '%2';" )
				                                     .arg( PACKAGE_DELETED_STRING ).arg( PACKAGE_OLD_STRING ).arg( id ), m_db );
			}
			
			// And now mark this specific version as not installed
			KurooDBSingleton::Instance()->query( QString( "UPDATE version SET status = '%1' WHERE idPackage = '%2' AND name = '%3';" )
			                                     .arg( PACKAGE_AVAILABLE_STRING ).arg( id ).arg( version ), m_db );
			
			KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
			return true;
		}
	}
	
	virtual void completeJob() {
		PortageSingleton::Instance()->slotPackageChanged();
	}
	
private:
	const QString m_package;

};

/**
 * @class CheckUpdatesPackageJob
 * @short Thread for marking packages as updates or downgrades.
 */
class CheckUpdatesPackageJob : public ThreadWeaver::DependentJob
{
public:
	CheckUpdatesPackageJob( QObject *dependent, const QString& id, const QString& updateVersion, int hasUpdate ) : DependentJob( dependent, "DBJob" ), 
		m_id( id ), m_updateVersion( updateVersion ), m_hasUpdate( hasUpdate ) {}
	
	virtual bool doJob() {
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		
		if ( m_hasUpdate == 0 ) {
			KurooDBSingleton::Instance()->query( QString("UPDATE package SET updateVersion = '', status = '%1' WHERE id = '%2';")
			                                     .arg( PACKAGE_INSTALLED_STRING ).arg( m_id ), m_db );
		}
		else {
			QString updateString;
			if ( m_hasUpdate > 0 )
				updateString = m_updateVersion + " (U)";
			else
				if ( m_hasUpdate < 0 )
					updateString = m_updateVersion + " (D)";
			
			KurooDBSingleton::Instance()->query( QString( "UPDATE package SET updateVersion = '%1', status = '%2' WHERE id = '%3';" )
			                                     .arg( updateString ).arg( PACKAGE_UPDATES_STRING ).arg( m_id ), m_db );
		}
		
		KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
		return true;
	}
	
	virtual void completeJob() {
		PortageSingleton::Instance()->slotChanged(); // @fixme: Use signal instead?
	}
	
private:
	const 	QString m_id, m_updateVersion;
	int 	m_hasUpdate;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @class Portage
 * @short Object handling the Portage tree.
 */
Portage::Portage( QObject *m_parent )
	: QObject( m_parent )
{
	// When cache scan is done go one scanning portage for all packages
	connect( SignalistSingleton::Instance(), SIGNAL( signalCachePortageComplete() ), this, SLOT( slotScan() ) );
	// Then portage scan is completed
	connect( SignalistSingleton::Instance(), SIGNAL( signalScanPortageComplete() ), this, SLOT( slotScanCompleted() ) );
	
	connect( SignalistSingleton::Instance(), SIGNAL( signalScanUpdatesComplete() ), this, SLOT( slotLoadUpdates() ) );
	connect( SignalistSingleton::Instance(), SIGNAL( signalLoadUpdatesComplete() ), this, SLOT( slotChanged() ) );

	// Start refresh directly after emerge sync
	connect( SignalistSingleton::Instance(), SIGNAL( signalSyncDone() ), this, SLOT( slotRefresh() ) );
}

Portage::~Portage()
{
}

void Portage::init( QObject *parent )
{
	m_parent = parent;
	loadWorld();
}

/**
 * Emit signal after changes in Portage.
 */
void Portage::slotChanged()
{
	DEBUG_LINE_INFO;
	
	// Register in db so we can check at next start if user has emerged any packages outside kuroo
	KurooDBSingleton::Instance()->setKurooDbMeta( "scanTimeStamp", QString::number( QDateTime::currentDateTime().toTime_t() ) );
	
	emit signalPortageChanged();
}

/**
 * Reload world when new package is installed/removed in case user not using --oneshot.
 */
void Portage::slotPackageChanged()
{
	DEBUG_LINE_INFO;
	
	loadWorld();
	slotChanged();
}

////////////////////////////////////////////////////////////////////////////////////////
// Portage handling...
////////////////////////////////////////////////////////////////////////////////////////

/**
 * Start scan of portage packages.
 * @return bool
 */
bool Portage::slotRefresh()
{
	DEBUG_LINE_INFO;
	
	// Update cache if empty
// 	if ( KurooDBSingleton::Instance()->isCacheEmpty() ) {
// 		SignalistSingleton::Instance()->scanStarted();
// 		ThreadWeaver::instance()->queueJob( new CachePortageJob( this ) );
// 	}
// 	else
	
	slotScan();
	
	return true;
}

/**
 * Start emerge sync.
 * @return bool
 */
bool Portage::slotSync()
{
	DEBUG_LINE_INFO;
	
	EmergeSingleton::Instance()->sync();
	return true;
}

/**
 * Continue with scan of portage packages.
 * @return bool
 */
bool Portage::slotScan()
{
	DEBUG_LINE_INFO;
	
// 	int maxLoops( 99 );
// 	
// 	// Wait for cache job to finish before launching the scan.
// 	while ( true ) {
// 		if ( KurooDBSingleton::Instance()->isCacheEmpty() )
// 			::usleep( 100000 ); // Sleep 100 msec
// 		else
// 			break;
// 		
// 		if ( maxLoops-- == 0 ) {
// 			kdWarning(0) << i18n("Scanning Portage. Wait-counter has reached maximum. Attempting to scan Portage.") << LINE_INFO;
// 			break;
// 		}
// 	}
// 	
	
	SignalistSingleton::Instance()->scanStarted();
	ThreadWeaver::instance()->queueJob( new LoadPortageJob( this ) );
	return true;
}

/**
 * After portage has completed scanning for all packages, check for updates.
 */
void Portage::slotScanCompleted()
{
	DEBUG_LINE_INFO;
	
	// Reset Queue with it's own cache
	QueueSingleton::Instance()->reset();
	
	// Now all Portage files
// 	PortageFilesSingleton::Instance()->loadPackageFiles();
	
	// Ready to roll!
	SignalistSingleton::Instance()->setKurooReady( true );
	slotChanged();
	
	// Go on with checking for updates
	if ( KurooDBSingleton::Instance()->isUpdatesEmpty() )
		slotRefreshUpdates();
}


////////////////////////////////////////////////////////////////////////////////////////
// World handling...
////////////////////////////////////////////////////////////////////////////////////////

/**
 * Load packages in world file. @fixme: optimize this...
 */
void Portage::loadWorld()
{
	m_mapWorld.clear();
	
	QFile file( KurooConfig::dirWorldFile() );
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		while ( !stream.atEnd() ) {
			QString package = stream.readLine();
			m_mapWorld[ package.stripWhiteSpace() ] = QString::null;
		}
		emit signalWorldChanged();
	}
	else
		kdError(0) << "Loading packages in world. Reading: " << KurooConfig::dirWorldFile() << LINE_INFO;
}

/**
 * Save back content of map to world file.
 */
bool Portage::saveWorld( const QMap<QString, QString>& map )
{
	QFile file( KurooConfig::dirWorldFile() );
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		for ( QMap<QString, QString>::ConstIterator it = map.begin(); it != map.end(); ++it )
    		stream << it.key() << "\n";
		file.close();
		
		return true;
	}
	else
		kdError(0) << "Adding to world. Writing: " << KurooConfig::dirWorldFile() << LINE_INFO;
	
	return false;
}

/**
 * Check if this package in is world file.
 * @param package
 * @return true/false
 */
bool Portage::isInWorld( const QString& package )
{
	if ( m_mapWorld.contains( package ) )
		return true;
	else
		return false;
}

/**
 * Add package to world file.
 * @param package
 */
void Portage::appendWorld( const QString& package )
{
	QMap<QString, QString> map = m_mapWorld;
	map[ package ] = QString::null;
	
	// Try saving changes first
	if ( saveWorld( map ) ) {
		m_mapWorld = map;
		emit signalWorldChanged();
	}
}

/**
 * Remove package from world file.
 * @param package
 */
void Portage::removeFromWorld( const QString& package )
{
	QMap<QString, QString> map = m_mapWorld;
	map.remove( package );
	
	// Try saving changes first
	if ( saveWorld( map ) ) {
		m_mapWorld = map;
		emit signalWorldChanged();
	}
}


////////////////////////////////////////////////////////////////////////////////////////
// Package handlling...
////////////////////////////////////////////////////////////////////////////////////////

/**
 * Launch emerge pretend of packages.
 * @param category
 * @param packageList
 */
void Portage::pretendPackageList( const QStringList& packageIdList )
{	
	QStringList packageList;
	foreach ( packageIdList )
		packageList += KurooDBSingleton::Instance()->category( *it ) + "/" + KurooDBSingleton::Instance()->package( *it );
	
	EmergeSingleton::Instance()->pretend( packageList );
}

/**
 * Launch unmerge of packages
 * @param category
 * @param packageList
 */
void Portage::uninstallInstalledPackageList( const QStringList& packageIdList )
{
	QStringList packageList;
	foreach ( packageIdList )
		packageList += KurooDBSingleton::Instance()->category( *it ) + "/" + KurooDBSingleton::Instance()->package( *it );
	
	EmergeSingleton::Instance()->unmerge( packageList );
}

/**
 * @fixme: Check for failure.
 * Add package as installed in db.
 * @param package
 */
void Portage::addInstalledPackage( const QString& package )
{
	DEBUG_LINE_INFO;
	
	ThreadWeaver::instance()->queueJob( new AddInstalledPackageJob( this, package ) );
}

/**
 * @fixme: Check for failure.
 * Remove packages from db.
 * @param packageIdList
 */
void Portage::removeInstalledPackage( const QString& package )
{
	DEBUG_LINE_INFO;
	
	ThreadWeaver::instance()->queueJob( new RemoveInstalledPackageJob( this, package ) );
}

/**
 * Start scan of update packages.
 * @return bool
 */
bool Portage::slotRefreshUpdates()
{
	EmergeSingleton::Instance()->checkUpdates();
	return true;
}

/**
 * Start scan of update packages.
 * @return bool
 */
bool Portage::slotLoadUpdates()
{
	SignalistSingleton::Instance()->scanStarted();
	ThreadWeaver::instance()->queueJob( new ScanUpdatesJob( this, EmergeSingleton::Instance()->packageList() ) );
	return true;
}

/**
 * Update packages when user changes package stability.
 */
void Portage::checkUpdates( const QString& id, const QString& emergeVersion, int hasUpdate )
{
	ThreadWeaver::instance()->queueJob( new CheckUpdatesPackageJob( this, id, emergeVersion, hasUpdate ) );
}

#include "portage.moc"
