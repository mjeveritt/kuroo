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

#include <kmessagebox.h>

#include <unistd.h>

/**
 * @class Portage
 * @short Object handling the Portage tree.
 */
Portage::Portage( QObject *m_parent )
	: QObject( m_parent )
{
	connect( SignalistSingleton::Instance(), SIGNAL( signalScanPortageComplete() ), this, SLOT( slotChanged() ) );
	connect( SignalistSingleton::Instance(), SIGNAL( signalCachePortageComplete() ), this, SLOT( slotScan() ) );
	
	// Start refresh directly after emerge sync
	connect( SignalistSingleton::Instance(), SIGNAL( signalSyncDone() ), this, SLOT( slotRefresh() ) );
}

Portage::~Portage()
{
}

void Portage::init( QObject *parent )
{
	m_parent = parent;
	loadCache();
	loadWorld();
}

/**
 * Forward signal after a new portage scan.
 */
void Portage::slotChanged()
{
	clearCache();
	emit signalPortageChanged();
	KurooDBSingleton::Instance()->addRefreshTime();
	PortageFilesSingleton::Instance()->loadPackageMask();
}

/**
 * Start scan of portage packages.
 * @return bool
 */
bool Portage::slotRefresh()
{
	// Update cache if empty
	if ( KurooDBSingleton::Instance()->isCacheEmpty() ) {
		SignalistSingleton::Instance()->scanStarted();
		ThreadWeaver::instance()->queueJob( new CachePortageJob( this ) );
	}
	else
		slotScan();
	
	return true;
}

/**
 * Continue scan of portage packages.
 * @return bool
 */
bool Portage::slotScan()
{
	int maxLoops(99);
	
	// Wait for cache job to finish before launching the scan.
	while ( true ) {
		if ( KurooDBSingleton::Instance()->isCacheEmpty() )
			::usleep(100000); // Sleep 100 msec
		else
			break;
		
		if ( maxLoops-- == 0 ) {
			kdDebug() << i18n("Wait-counter has reached maximum. Attempting to scan Portage.") << endl;
			break;
		}
	}
	SignalistSingleton::Instance()->scanStarted();
	ThreadWeaver::instance()->queueJob( new ScanPortageJob( this ) );
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
 * Free cache memory.
 */
void Portage::clearCache()
{
	mapCache.clear();
}

/**
 * Load packages in world file. @fixme: optimize this...
 */
void Portage::loadWorld()
{
	mapWorld.clear();
	
	QFile file( KurooConfig::dirWorldFile() );
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		while ( !stream.atEnd() ) {
			QString package = stream.readLine();
			mapWorld[ package.stripWhiteSpace() ] = QString::null;
		}
	}
	else
		kdDebug() << i18n("Loading packages in world. Error reading: ") << KurooConfig::dirWorldFile() << endl;
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
	else
		kdDebug() << i18n("Adding to world. Error writing: ") << KurooConfig::dirWorldFile() << endl;
	
	return false;
}

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

#include "portage.moc"
