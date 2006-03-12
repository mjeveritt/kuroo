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
		QRegExp rxPackage( "(\\S+)/((?:[a-z]|[A-Z]|[0-9]|-|\\+|_)+)(-(?:\\d+\\.)*\\d+[a-z]?)" );
		QString category, name, version;
		
		if ( rxPackage.search( m_package ) > -1 ) {
			category = rxPackage.cap(1);
			name = rxPackage.cap(2);
			version = m_package.section( name + "-", 1, 1 ).remove(' ');
		}
		else {
			kdDebug() << i18n("Inserting emerged package: can not match %1.").arg( m_package ) << endl;
			kdDebug() << QString("Inserting emerged package: can not match %1.").arg( m_package ) << endl;
		}
		
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		QString id = KurooDBSingleton::Instance()->singleQuery( 
			" SELECT id FROM package WHERE name = '" + name + "' AND idCatSubCategory = "
			" ( SELECT id from catSubCategory WHERE name = '" + category + "' ); ", m_db);
		
		if ( id.isEmpty() ) {
			kdDebug() << i18n("Inserting emerged package: Can not find id in database for package %1/%2.").arg( category ).arg( name ) << endl;
			kdDebug() << QString("Inserting emerged package: Can not find id in database for package %1/%2.").arg( category ).arg( name ) << endl;
			KurooDBSingleton::Instance()->returnStaticDbConnection(m_db);
			return false;
		}
		else {
			KurooDBSingleton::Instance()->query( QString( "UPDATE package SET status = '%1' WHERE id = '%2'"
			                                              ";" ).arg( FILTER_INSTALLED_STRING ).arg( id ), m_db );
			KurooDBSingleton::Instance()->query( QString( " UPDATE version SET status = '%1' WHERE idPackage = '%2' AND name = '%3'"
			                                              ";" ).arg( FILTER_INSTALLED_STRING ).arg( id ).arg( version ), m_db );
			KurooDBSingleton::Instance()->returnStaticDbConnection(m_db);
			return true;
		}
	}
	
	virtual void completeJob() {
		PortageSingleton::Instance()->slotChanged();
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
		QRegExp rxPackage( "(\\S+)/((?:[a-z]|[A-Z]|[0-9]|-|\\+|_)+)(-(?:\\d+\\.)*\\d+[a-z]?)" );
		QString category, name, version;
		
		if ( rxPackage.search( m_package ) > -1 ) {
			category = rxPackage.cap(1);
			name = rxPackage.cap(2);
			version = m_package.section( name + "-", 1, 1 ).remove(' ');
		}
		else {
			kdDebug() << i18n("Removing unmerged package: can not match %1.").arg( m_package ) << endl;
			kdDebug() << QString("Removing unmerged package: can not match %1.").arg( m_package ) << endl;
		}
		
		QString id = KurooDBSingleton::Instance()->singleQuery( 
			" SELECT id FROM package WHERE name = '" + name + "' AND idCatSubCategory = "
			" ( SELECT id from catSubCategory WHERE name = '" + category + "' ); ", m_db);
		
		if ( id.isEmpty() ) {
			kdDebug() << i18n("Removing unmerged package: Can not find id in database for package %1/%2.").arg( category ).arg( name ) << endl;
			kdDebug() << QString("Removing unmerged package: Can not find id in database for package %1/%2.").arg( category ).arg( name ) << endl;
			KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
			return false;
		}
		else {
			
			QString installedVersionCount = KurooDBSingleton::Instance()->singleQuery( QString( 
					"SELECT COUNT(id) FROM version WHERE idPackage = '%1' LIMIT 1;").arg( id ), m_db );
			
			// Mark package as uninstalled only when one version is found
			if ( installedVersionCount == "1" ) {
			
				// Mark package as uninstalled
				KurooDBSingleton::Instance()->query( QString( "UPDATE package SET status = '%1' "
															  "WHERE status = '%2' AND id = '%3'").arg( FILTER_ALL_STRING ).arg( FILTER_INSTALLED_STRING ).arg( id ), m_db );
			
				// Remove package completely if "old" = not in official Portage anymore
				KurooDBSingleton::Instance()->query( QString( "DELETE FROM package "
															  "WHERE status = '%1' AND id = '%2';" ).arg( FILTER_OLD_STRING ).arg( id ), m_db );
			}
			
			KurooDBSingleton::Instance()->query( QString( "UPDATE version SET status = '%1' "
			                                              "WHERE idPackage = '%2' AND name = '%3';" ).arg( FILTER_ALL_STRING ).arg( id ).arg( version ), m_db );
			
			KurooDBSingleton::Instance()->returnStaticDbConnection(m_db);
			return true;
		}
	}
	
	virtual void completeJob() {
		PortageSingleton::Instance()->slotChanged();
	}
	
private:
	const QString m_package;
};

/**
 * @class RemoveUpdatesPackageJob
 * @short Thread for removing single package from updates.
 */
class RemoveUpdatesPackageJob : public ThreadWeaver::DependentJob
{
public:
	RemoveUpdatesPackageJob( QObject *dependent, const QString& package ) : DependentJob( dependent, "DBJob" ), m_package( package ) {}
	
	virtual bool doJob() {
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		QRegExp rxPackage( "(\\S+)/((?:[a-z]|[A-Z]|[0-9]|-|\\+|_)+)(-(?:\\d+\\.)*\\d+[a-z]?)" );
		QString category, name, version;
		
		if ( rxPackage.search( m_package ) > -1 ) {
			category = rxPackage.cap(1);
			name = rxPackage.cap(2);
			version = m_package.section( name + "-", 1, 1 ).remove(' ');
		}
		else {
			kdDebug() << i18n("Removing update package: can not match package %1.").arg( m_package ) << endl;
			kdDebug() << QString("Removing update package: can not match package %1.").arg( m_package ) << endl;
		}
		
		QString id = KurooDBSingleton::Instance()->singleQuery(
			" SELECT id FROM package WHERE name = '" + name + "' AND idCatSubCategory = "
			" ( SELECT id from catSubCategory WHERE name = '" + category + "' ); ", m_db );
		
		if ( id.isEmpty() ) {
			kdDebug() << i18n("Removing update package: Can not find id in database for package %1/%2.").arg( category ).arg( name ) << endl;
			kdDebug() << QString("Removing update package: Can not find id in database for package %1/%2.").arg( category ).arg( name ) << endl;
			KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
			return false;
		}
		else {
			KurooDBSingleton::Instance()->query( QString( "UPDATE package SET updateVersion = '' "
			                                              "WHERE name = '%1' AND ( updateVersion = '%2' OR updateVersion = '%3' );"
			                                            ).arg( name ).arg( version + " (D)" ).arg( version + " (U)" ), m_db );
			KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
			return true;
		}
	}
	
	virtual void completeJob() {
		PortageSingleton::Instance()->slotChanged();
	}
	
private:
	const QString m_package;
};

/**
 * @class CheckUpdatesPackageJob
 * @short Thread for marking packages as updates or donwgrades.
 */
class CheckUpdatesPackageJob : public ThreadWeaver::DependentJob
{
public:
	CheckUpdatesPackageJob( QObject *dependent, const QString& id, const QString& updateVersion, int hasUpdate ) : DependentJob( dependent, "DBJob" ), 
		m_id( id ), m_updateVersion( updateVersion ), m_hasUpdate( hasUpdate ) {}
	
	virtual bool doJob() {
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		
		QString updateString;
		if ( m_hasUpdate > 0 )
			updateString = m_updateVersion + " (U)";
		else
			if ( m_hasUpdate < 0 )
				updateString = m_updateVersion + " (D)";
		
		KurooDBSingleton::Instance()->query( QString( "UPDATE package SET updateVersion = '%1' WHERE id = '%2';"
		                                            ).arg( updateString ).arg( m_id ), m_db );
		
		KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
		return true;
	}
	
	virtual void completeJob() {
		PortageSingleton::Instance()->slotChanged();
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
	connect( SignalistSingleton::Instance(), SIGNAL( signalCachePortageComplete() ), this, SLOT( slotScan() ) );
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
	kdDebug() << "Portage::slotChanged" << endl;
	emit signalPortageChanged();
}


////////////////////////////////////////////////////////////////////////////////////////
// Parse Portage cache files to speed up portage scan
////////////////////////////////////////////////////////////////////////////////////////

/**
 * Load mapCache with items from DB.
 */
void Portage::loadCache()
{
	mapCache.clear();
	
	const QStringList cacheList = KurooDBSingleton::Instance()->allCache();
	foreach ( cacheList ) {
		QString package = *it++;
		QString size = *it;
		mapCache.insert( package, size );
	}
}

/**
 * Find cached size for package.
 * @param packages
 * @return size or NULL if na
 */
QString Portage::cacheFind( const QString& package )
{
	QMap<QString, QString>::iterator it = mapCache.find( package ) ;
	if ( it != mapCache.end() )
		return it.data();
	else
		return QString::null;
}

/**
 * Get cache from threaded scan.
 * @param map of cached packages
 */
void Portage::setCache( const QMap<QString, QString> &mapCacheIn )
{
	mapCache = mapCacheIn;
}

/**
 * Free cache memory.
 */
void Portage::clearCache()
{
	mapCache.clear();
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
	kdDebug() << "Portage::slotRefresh" << endl;
	
	// Update cache if empty
	if ( KurooDBSingleton::Instance()->isCacheEmpty() ) {
		SignalistSingleton::Instance()->scanStarted();
		ThreadWeaver::instance()->queueJob( new CachePortageJob( this ) );
	}
	else {
		loadCache();
		slotScan();
	}
	
	return true;
}

/**
 * Start emerge sync.
 * @return bool
 */
bool Portage::slotSync()
{
	EmergeSingleton::Instance()->sync();
	return true;
}

/**
 * Continue with scan of portage packages.
 * @return bool
 */
bool Portage::slotScan()
{
	kdDebug() << "Portage::slotScan" << endl;
	
	int maxLoops(99);
	
	// Wait for cache job to finish before launching the scan.
	while ( true ) {
		if ( KurooDBSingleton::Instance()->isCacheEmpty() )
			::usleep(100000); // Sleep 100 msec
		else
			break;
		
		if ( maxLoops-- == 0 ) {
			kdDebug() << i18n("Scanning Portage. Wait-counter has reached maximum. Attempting to scan Portage.") << endl;
			kdDebug() << QString("Scanning Portage. Wait-counter has reached maximum. Attempting to scan Portage.") << endl;
			break;
		}
	}
	
	SignalistSingleton::Instance()->scanStarted();
	ThreadWeaver::instance()->queueJob( new ScanPortageJob( this ) );
	return true;
}

/**
 * Forward signal after a new portage scan.
 */
void Portage::slotScanCompleted()
{
	kdDebug() << "Portage::slotScanCompleted" << endl;
	
	// Packages are scanned, we don't the cache anymore
	clearCache();
	
	// Reset Queue with it's own cache
	QueueSingleton::Instance()->reset();
	
	// Register this scan in history so we can check at next start if user has emerged any packages outside kuroo
	KurooDBSingleton::Instance()->addRefreshTime();
	
	// Now all Portage files
	PortageFilesSingleton::Instance()->loadPackageFiles();
	
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
	kdDebug() << "Portage::loadWorld" << endl;
	
	mapWorld.clear();
	
	QFile file( KurooConfig::dirWorldFile() );
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		while ( !stream.atEnd() ) {
			QString package = stream.readLine();
			mapWorld[ package.stripWhiteSpace() ] = QString::null;
		}
	}
	else {
		kdDebug() << i18n("Loading packages in world. Error reading: ") << KurooConfig::dirWorldFile() << endl;
		kdDebug() << QString("Loading packages in world. Error reading: ") << KurooConfig::dirWorldFile() << endl;
	}
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
			stream << it.key() << endl;
		file.close();
		
		return true;
	}
	else {
		kdDebug() << i18n("Adding to world. Error writing: ") << KurooConfig::dirWorldFile() << endl;
		kdDebug() << QString("Adding to world. Error writing: ") << KurooConfig::dirWorldFile() << endl;
	}
	
	return false;
}

/**
 * Check if this package in is world file.
 * @param package
 * @return true/false
 */
bool Portage::isInWorld( const QString& package )
{
	if ( mapWorld.contains( package ) )
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
	QMap<QString, QString> map = mapWorld;
	map[ package ] = QString::null;
	
	// Try saving changes first
	if ( saveWorld( map ) ) {
		mapWorld = map;
		emit signalWorldChanged();
	}
}

/**
 * Remove package from world file.
 * @param package
 */
void Portage::removeFromWorld( const QString& package )
{
	QMap<QString, QString> map = mapWorld;
	map.remove( package );
	
	// Try saving changes first
	if ( saveWorld( map ) ) {
		mapWorld = map;
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
	ThreadWeaver::instance()->queueJob( new AddInstalledPackageJob( this, package ) );
}

/**
 * @fixme: Check for failure.
 * Remove packages from db.
 * @param packageIdList
 */
void Portage::removeInstalledPackage( const QString& package )
{
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
 * @fixme: Check for failure.
 * Remove packages from db.
 * @param packageIdList
 */
void Portage::removeUpdatePackage( const QString& package )
{
	ThreadWeaver::instance()->queueJob( new RemoveUpdatesPackageJob( this, package ) );
}

/**
 * Update packages when user changes package stability.
 */
void Portage::checkUpdates( const QString& id, const QString& emergeVersion, int hasUpdate )
{
	ThreadWeaver::instance()->queueJob( new CheckUpdatesPackageJob( this, id, emergeVersion, hasUpdate ) );
}

#include "portage.moc"
