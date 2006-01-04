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
 * Object handling the Portage tree.
 */
Portage::Portage( QObject *parent )
	: QObject( parent )
{
	connect( SignalistSingleton::Instance(), SIGNAL( signalScanPortageComplete() ), this, SLOT( slotChanged() ) );
	connect( SignalistSingleton::Instance(), SIGNAL( signalCachePortageComplete() ), this, SLOT( slotScan() ) );
	
	// Start refresh directly after emerge sync
	connect( SignalistSingleton::Instance(), SIGNAL( signalSyncDone() ), this, SLOT( slotRefresh() ) );
}

Portage::~Portage()
{
}

void Portage::init( QObject *myParent )
{
	parent = myParent;
	loadPackageKeywords();
	loadCache();
}

/**
 * Forward signal after a new portage scan.
 */
void Portage::slotChanged()
{
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
	while ( true )
	{
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
	foreach ( packageIdList ) {
		packageList += Portage::category( *it ) + "/" + Portage::package( *it );
	}
	EmergeSingleton::Instance()->pretend( packageList );
}

/**
 * Get list of all categories for portage packages.
 * @return QStringList
 */
QStringList Portage::categories( int filter, const QString& text )
{
	return KurooDBSingleton::Instance()->portageCategories( filter, text );
}

/**
 * Get list of all subcategories for portage packages.
 * @return QStringList
 */
QStringList Portage::subCategories( const QString& categoryId, int filter, const QString& text )
{
	return KurooDBSingleton::Instance()->portageSubCategories( categoryId, filter, text );
}

/**
 * Get list of packages in this subcategory from database.
 * @param category
 * @return QStringList
 */

QStringList Portage::packagesInSubCategory( const QString& categoryId, const QString& subCategoryId, int filter, const QString& text )
{
	return KurooDBSingleton::Instance()->portagePackagesBySubCategory( categoryId, subCategoryId, filter, text );
}

/**
 * Count packages.
 * @return total
 */
QString Portage::count()
{
	return KurooDBSingleton::Instance()->packageTotal().first();
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
		return NULL;
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
 * Load untesting packages list = packages in package.keyword.
 */
void Portage::loadPackageKeywords()
{
	mapPackageKeywords.clear();
	
	// Load package.keyword
	QFile file( KurooConfig::filePackageKeywords() );
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		while ( !stream.atEnd() ) {
			QString line( stream.readLine() );
			if ( !line.isEmpty() && !line.startsWith( "#" ) )
				mapPackageKeywords.insert( line.section( " ", 0, 0 ), line.section( " ", 1, 1 ) );
		}
		file.close();
	}
	else
		kdDebug() << i18n("Error reading: package.keyword.") << endl;
}

/**
 * Check if package is unmasked. @fixme not checking if just testing or hardmasked.
 * @param package
 * @return success
 */
bool Portage::isUntesting( const QString& package )
{
	QMap<QString, QString>::iterator itMap = mapPackageKeywords.find( package ) ;
	if ( itMap != mapPackageKeywords.end() )
		return true;
	else
		return false;
}

/**
 * Unmask list of packages by adding them to package.keyword.
 * @param category
 * @param packageList
 */
void Portage::untestingPackageList( const QStringList& packageIdList )
{
	foreach ( packageIdList ) {
		QString package = Portage::category( *it ) + "/" + Portage::package( *it );
	
		if ( !unmaskPackage( package, KurooConfig::filePackageKeywords() ) )
			break;
		else
			mapPackageKeywords.insert( package, "~" + KurooConfig::arch() );
	}
}

/**
 * Unmask package by adding to "maskFile".
 * @param package
 * @param maskFile
 * @return success
 */
bool Portage::unmaskPackage( const QString& package, const QString& maskFile )
{
	QStringList packageList;
	bool found = false;
	QFile file( maskFile );
	
	if( !file.open( IO_ReadOnly ) ) {
		file.close();
		kdDebug() << i18n("Error reading: ") << maskFile << endl;
		return false;
	}
	else {
		QTextStream stream( &file );
		while ( !stream.atEnd() ) {
			QString line = stream.readLine();
			if ( line.contains( package ) )
				found = true;
			packageList += line;
		}
		file.close();
		
		if ( !found )
			packageList += package;
		
		if ( file.open( IO_WriteOnly ) ) {
			QTextStream stream( &file );
			foreach ( packageList ) {
				stream << *it + "\n";
			}
		}
		else {
			kdDebug() << i18n("Error writing: ") << maskFile << endl;
			KMessageBox::error( 0, i18n("Failed to save. Please run as root."), i18n("Saving"));
			return false;
		}
		
		file.close();
	}
	
	// Signal to gui to mark package as unmasked
	QString temp( package.section( "/", 1, 1 ).section( " ", 0, 0 ) );
	QString name( temp.section ( rxPortageVersion, 0, 0 ) );
	SignalistSingleton::Instance()->setUnmasked( name, true );
	
	return true;
}

/**
 * Clear the unmasking of packages by removing from package.keyword.
 * @param category
 * @param packageList
 */
void Portage::clearUntestingPackageList( const QStringList& packageIdList )
{
	QFile file( KurooConfig::filePackageKeywords() );
	
	// Store back list of unmasked packages
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		
		foreach ( packageIdList ) {
			QString package = Portage::category( *it ) + "/" + Portage::package( *it ).section( rxPortageVersion, 0, 0 );
			mapPackageKeywords.remove( package );
			
			// Signal to gui to mark package as not unmasked anymore
			QString temp( package.section( "/", 1, 1 ).section( " ", 0, 0 ) );
			QString name( temp.section( rxPortageVersion, 0, 0 ) );
			SignalistSingleton::Instance()->setUnmasked( name, false );
		}
		
		QMap< QString, QString >::iterator itMapEnd = mapPackageKeywords.end();
		for ( QMap< QString, QString >::iterator itMap = mapPackageKeywords.begin(); itMap != itMapEnd; ++itMap ) {
			stream << itMap.key() + " " + itMap.data() + "\n";
		}
		file.close();
	}
	else {
		kdDebug() << i18n("Error writing: ") << KurooConfig::filePackageKeywords() << endl;
		KMessageBox::error( 0, i18n("Failed to save. Please run as root."), i18n("Saving"));
	}
}

/**
 * Add package to world file.
 * @param package
 */
void Portage::appendWorld( const QString& package )
{
	QString category = package.section( "/", 0, 0 );
	QString name = ( package.section( "/", 1, 1 ) ).section( rxPortageVersion, 0, 0 );
	
	QFile file( KurooConfig::dirWorldFile() );
	QStringList lines;
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		while ( !stream.atEnd() )
			lines += stream.readLine();
		file.close();
		
		if ( file.open( IO_WriteOnly ) ) {
			bool found;
			QTextStream stream( &file );
			foreach ( lines ) {
				stream << *it << endl;
				if ( *it == ( category + "/" + name ) )
					found = true;
			}
			if ( !found )
				stream << category + "/" + name << endl;
			file.close();
		}
		else
			kdDebug() << i18n("Error writing: ") << KurooConfig::dirWorldFile() << endl;
	}
	else
		kdDebug() << i18n("Error reading: ") << KurooConfig::dirWorldFile() << endl;
}

/**
 * Get this packages database id.
 * @param package
 * @return idDB
 */
QString Portage::id( const QString& package )
{
	QString category = package.section( "/", 0, 0 );
	QString temp( package.section( "/", 1, 1 ).section( " ", 0, 0 ) );
	QString name( temp.section( rxPortageVersion, 0, 0 ) );
	
	return KurooDBSingleton::Instance()->packageId( category, name );
}

/**
 * Get this packages category.
 * @param id
 * @return category
 */
QString Portage::category( const QString& id )
{
	return KurooDBSingleton::Instance()->category( id );
}

/**
 * Get this packages category.
 * @param id
 * @return package
 */
QString Portage::package( const QString& id )
{
	return KurooDBSingleton::Instance()->package( id );
}

QStringList Portage::packageVersionsInfo( const QString& id )
{
	return KurooDBSingleton::Instance()->packageVersionsInfo( id );
}

#include "portage.moc"
