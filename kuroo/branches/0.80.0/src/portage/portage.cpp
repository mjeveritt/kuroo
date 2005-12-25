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

#include <qregexp.h>

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
	loadUnmaskedList();
	loadCache();
}

/**
 * Forward signal after a new scan.
 */
void Portage::slotChanged()
{
	emit signalPortageChanged();
	KurooDBSingleton::Instance()->addRefreshTime();
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
 * Get list of versions available.
 * @param package name
 * @return list of versions
 */
QStringList Portage::packageVersions( const QString& id )
{
	return KurooDBSingleton::Instance()->packageVersions( id );
}

QStringList Portage::packageVersionsInfo( const QString& id )
{
	return KurooDBSingleton::Instance()->packageVersionsInfo( id );
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
 * Return info for package as description, homepage ...
 * @param package id
 * @return info
 */
Info Portage::packageInfo( const QString& packageId )
{
	Info info;
	
	QStringList packageList = KurooDBSingleton::Instance()->portagePackageInfo( packageId );
	QStringList::Iterator it = packageList.begin();
	info.description = *it++;
	info.homepage = *it;
	
	return info;
}

/**
 * Find cached size for package.
 * @param packages
 * @return size or NULL if na
 */
QString Portage::cacheFind( const QString& package )
{
	QMap<QString, QString>::iterator it = cacheMap.find( package ) ;
	if ( it != cacheMap.end() )
		return it.data();
	else
		return NULL;
}

/**
 * Get cache from threaded scan.
 * @param map of cached packages
 */
void Portage::setCache( const QMap<QString, QString> &cacheMapIn )
{
	cacheMap = cacheMapIn;
}

/**
 * Load cacheMap with items from DB.
 */
void Portage::loadCache()
{
	cacheMap.clear();
	
	const QStringList cacheList = KurooDBSingleton::Instance()->cache();
	foreach ( cacheList ) {
		QString package = *it++;
		QString size = *it;
		cacheMap.insert( package, size );
	}
}

/**
 * Free cache memory.
 */
void Portage::clearCache()
{
	cacheMap.clear();
}

/**
 * Load unmasked packages list = packages in package.keyword.
 */
void Portage::loadUnmaskedList()
{
	unmaskedMap.clear();
	
	// Load package.keyword
	QFile file( KurooConfig::dirPackageKeywords() );
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		while ( !stream.atEnd() ) {
			QString line( stream.readLine() );
			if ( !line.startsWith( "#" ) )
				unmaskedMap.insert( line.section( " ", 0, 0 ), line.section( " ", 1, 1 ) );
		}
	}
	else
		kdDebug() << i18n("Error reading: package.keyword.") << endl;
	
	file.close();
}

/**
 * Check if package is unmasked. @fixme not checking if just testing or hardmasked.
 * @param package
 * @return success
 */
bool Portage::isUnmasked( const QString& package )
{
	QMap<QString, QString>::iterator itMap = unmaskedMap.find( package ) ;
	if ( itMap != unmaskedMap.end() )
		return true;
	else
		return false;
}

/**
 * Unmask list of packages by adding them to package.keyword.
 * @param category
 * @param packageList
 */
void Portage::unmaskPackageList( const QStringList& packageIdList )
{
	foreach ( packageIdList ) {
		QString package = Portage::category( *it ) + "/" + Portage::package( *it );
	
		if ( !unmaskPackage( package /*+ " ~" + KurooConfig::arch()*/, KurooConfig::dirPackageKeywords() ) )
			break;
		else
			unmaskedMap.insert( package, "~" + KurooConfig::arch() );
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
void Portage::clearUnmaskPackageList( const QStringList& packageIdList )
{
	QFile file( KurooConfig::dirPackageKeywords() );
	
	// Store back list of unmasked packages
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		
		foreach ( packageIdList ) {
			QString package = Portage::category( *it ) + "/" + Portage::package( *it ).section( rxPortageVersion, 0, 0 );
			unmaskedMap.remove( package );
			
			// Signal to gui to mark package as not unmasked anymore
			QString temp( package.section( "/", 1, 1 ).section( " ", 0, 0 ) );
			QString name( temp.section( rxPortageVersion, 0, 0 ) );
			SignalistSingleton::Instance()->setUnmasked( name, false );
		}
		
		QMap< QString, QString >::iterator itMapEnd = unmaskedMap.end();
		for ( QMap< QString, QString >::iterator itMap = unmaskedMap.begin(); itMap != itMapEnd; ++itMap ) {
			stream << itMap.key() + " " + itMap.data() + "\n";
		}
		file.close();
	}
	else {
		kdDebug() << i18n("Error writing: ") << KurooConfig::dirPackageKeywords() << endl;
		KMessageBox::error( 0, i18n("Failed to save. Please run as root."), i18n("Saving"));
	}
}

/**
 * Get this packages database id.
 * @param package
 * @return idDB
 */
QString Portage::idDb( const QString& package )
{
	QString category = package.section( "/", 0, 0 );
	QString temp( package.section( "/", 1, 1 ).section( " ", 0, 0 ) );
	QString name( temp.section( rxPortageVersion, 0, 0 ) );
	QString version( temp.section( name + "-", 1, 1 ) );
	
	return KurooDBSingleton::Instance()->portageIdByCategoryNameVersion( category, name, version ).first();
}

/**
 * Get this packages category.
 * @param id
 * @return category
 */
QString Portage::category( const QString& id )
{
	return KurooDBSingleton::Instance()->category( id ).first();
}

/**
 * Get this packages category.
 * @param id
 * @return package
 */
QString Portage::package( const QString& id )
{
	return KurooDBSingleton::Instance()->package( id ).first();
}

#include "portage.moc"
