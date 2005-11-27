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
	setRefreshTime();
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
		ThreadWeaver::instance()->queueJob( new CachePortageJob(this) );
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
	while (true)
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
	ThreadWeaver::instance()->queueJob( new ScanPortageJob(this) );
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
 * Add timestamp in history for when kuroo database is refreshed.
 */
void Portage::setRefreshTime()
{
	QDateTime currentTime( QDateTime::currentDateTime() );
	KurooDBSingleton::Instance()->insert( QString("INSERT INTO history (package, timestamp, emerge) VALUES ('', '%1', 'false');").arg( QString::number( currentTime.toTime_t() ) ) );
}

/**
 * Launch emerge pretend of packages.
 * @param category
 * @param packageList
 */
void Portage::pretendPackage( const QString& category, const QStringList& packageList )
{
	EmergeSingleton::Instance()->pretend( category, packageList );
}

/**
 * Check if package is installed.
 * @param package
 * @return success
 */
bool Portage::isInstalled( const QString& package )
{
	QString tmp = package.section("/", 1, 1);
	QString name = tmp.section(pv, 0, 0);
	QString version = tmp.section(name + "-", 1, 1);
	
	QString installedFlag = KurooDBSingleton::Instance()->isInstalled( name, version ).first();
	
	if ( installedFlag == "1" )
		return true;
	else
		return false;
}

/**
 * Get list of all categories for portage packages.
 * @return QStringList
 */
QStringList Portage::categories()
{
	return KurooDBSingleton::Instance()->portageCategories();
}

/**
 * Get list of packages in this category from database.
 * @param category
 * @return QStringList
 */
QStringList Portage::packagesInCategory( const QString& category )
{
	QString idCategory = KurooDBSingleton::Instance()->portageCategoryId(category).first();	
	return KurooDBSingleton::Instance()->portagePackagesByCategory(idCategory);
}

/**
 * Get list of versions available.
 * @param package name
 * @return list of versions
 */
QStringList Portage::packageVersions( const QString& name )
{
	return KurooDBSingleton::Instance()->packageVersions(name);
}

/**
 * Find packages by name or description.
 * @param text		string
 * @param isName	find in name or description
 */
void Portage::findPackage( const QString& text, const bool& isName )
{
	QStringList packageIdList;
	
	if ( isName )
		packageIdList = KurooDBSingleton::Instance()->findPortagePackagesDescription(text);
	else
		packageIdList = KurooDBSingleton::Instance()->findPortagePackagesName(text);
	
	if ( !packageIdList.isEmpty() )
		ResultsSingleton::Instance()->addPackageIdList( packageIdList );
	else
		LogSingleton::Instance()->writeLog( i18n("<br>No packages found matching: %1").arg(text), KUROO );
}

/**
 * Count packages.
 * @return total
 */
QString Portage::count()
{
	QStringList total = KurooDBSingleton::Instance()->query("SELECT COUNT(id) FROM package WHERE installed != '2' LIMIT 1;");
	return total.first();
}

/**
 * Return info for package as description, homepage ...
 * @param package id
 * @return info
 */
Info Portage::packageInfo( const QString& packageId )
{
	Info info;
	
	QStringList packageList = KurooDBSingleton::Instance()->portagePackageInfo(packageId);
	QStringList::Iterator it = packageList.begin();
	info.description = *it++;
	info.size = *it++;
	info.keywords = *it++;
	info.homepage = *it++;
	info.licenses = *it++;
	info.useFlags = *it++;
	info.packageSlots = *it;
	
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
	QFile file( KurooConfig::filePackageKeywords() );
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		while ( !stream.atEnd() ) {
			QString line(stream.readLine());
			if ( !line.startsWith("#") )
				unmaskedMap.insert( line.section(" ", 0, 0), line.section(" ", 1, 1) );
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
void Portage::unmaskPackageList( const QString& category, const QStringList& packageList )
{
	foreach ( packageList ) {
		QString name = (*it).section(pv, 0, 0);
		QString package = category + "/" + name;
	
		if ( !unmaskPackage( package + " ~" + KurooConfig::arch(), KurooConfig::filePackageKeywords() ) )
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
			for ( QStringList::Iterator it0 = packageList.begin(); it0 != packageList.end(); ++it0 ) {
				stream << *it0 + "\n";
			}
		}
		else {
			kdDebug() << i18n("Error writing: ") << maskFile << endl;
			KMessageBox::error( 0, i18n("Failed to save to %1. Please run as root.").arg(maskFile), i18n("Saving"));
			return false;
		}
		
		file.close();
	}
	
	// Signal to gui to mark package as unmasked
	QString temp( package.section("/", 1, 1).section(" ", 0, 0) );
	QString name( temp.section(pv, 0, 0) );
	SignalistSingleton::Instance()->setUnmasked( name, true );
	
	return true;
}

/**
 * Clear the unmasking of packages by removing from package.keyword.
 * @param category
 * @param packageList
 */
void Portage::clearUnmaskPackageList( const QString& category, const QStringList& packageList )
{
	QFile file( KurooConfig::filePackageKeywords() );
	
	// Store back list of unmasked packages
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		
		foreach ( packageList ) {
			QString package = category + "/" + (*it).section(pv, 0, 0);
			unmaskedMap.remove( package );
			
			// Signal to gui to mark package as not unmasked anymore
			QString temp( package.section("/", 1, 1).section(" ", 0, 0) );
			QString name( temp.section(pv, 0, 0) );
			SignalistSingleton::Instance()->setUnmasked( name, false );
		}
		
		QMap< QString, QString >::iterator itMapEnd = unmaskedMap.end();
		for ( QMap< QString, QString >::iterator itMap = unmaskedMap.begin(); itMap != itMapEnd; ++itMap ) {
			stream << itMap.key() + " " + itMap.data() + "\n";
		}
		file.close();
	}
	else {
		kdDebug() << i18n("Error writing: ") << KurooConfig::filePackageKeywords() << endl;
		KMessageBox::error( 0, i18n("Failed to saveto %1. Please run as root.").arg(KurooConfig::filePackageKeywords()), i18n("Saving"));
	}
}

/**
 * Get this packages database id.
 * @param package
 * @return idDB
 */
QString Portage::idDb( const QString& package )
{
	QString category = package.section("/", 0, 0);
	QString temp( package.section("/", 1, 1).section(" ", 0, 0) );
	QString name( temp.section(pv, 0, 0) );
	QString version( temp.section(name + "-", 1, 1) );
	
	return KurooDBSingleton::Instance()->portageIdByCategoryNameVersion( category, name, version ).first();
}

/**
 * Get this packages category.
 * @param id
 * @return category
 */
QString Portage::category( const QString& id )
{
	return KurooDBSingleton::Instance()->query(QString("SELECT name FROM category WHERE id = ( SELECT idCategory FROM package WHERE id = '%1' );").arg(id)).first();
}

/**
 * Get this packages category.
 * @param id
 * @return package
 */
QString Portage::package( const QString& id )
{
	QStringList packageList = KurooDBSingleton::Instance()->query(QString("SELECT name, version FROM package WHERE id = '%1';").arg(id));
	QStringList::Iterator it = packageList.begin();
	QString name = *it++;
	QString version = *it;
	return name + "-" + version;
}

/**
 * Get summary for selected package.
 * @param id
 * @return summary
 */
QString Portage::packageSummary( const QString& packageId )
{
	QString package( Portage::package( packageId ) );
	QString category( Portage::category( packageId ) );
	Info info( packageInfo( packageId ) );

	QString textLines = "<font size=\"+2\">" + category + "/" + package.section(pv, 0, 0) + "</font><br>";
			textLines += info.description + "<br>";
			textLines += "<a href=\"" + info.homepage + "\">" + info.homepage + "</a><br>";
			textLines += i18n("<b>Licenses:</b> ") + info.licenses + "<br>";
			textLines += i18n("<b>Available versions:</b> ");
	
	const QStringList versionList = packageVersions( package.section(pv, 0, 0) );
	foreach ( versionList ) {
		QString version = *it++;
		QString installed = *it;
		
		if ( installed == "1" )
			textLines += "<font color=darkGreen><b>" + version + "</b></font>, ";
		else
			textLines += version + ", ";
	}
	textLines.truncate( textLines.length() - 2 );
	return textLines;
}

/**
 * Get summary for selected version.
 * @param id
 * @return summary
 */
QString Portage::versionSummary( const QString& packageId )
{
	QString package(Portage::package( packageId ));
	QString category(Portage::category( packageId ));
	Info info( packageInfo( packageId ) );
	
	QString textLines = "<font size=\"+2\">" + category + "/" + package + "</font><br>";
			textLines += info.description + "<br>";
			textLines += "<a href=\"" + info.homepage + "\">" + info.homepage + "</a><br>";
			textLines += i18n("<b>Licenses:</b> ") + info.licenses + "<br>";
			textLines += i18n("<b>Slot:</b> ") + info.packageSlots + "<br>";
			textLines += i18n("<b>Branches:</b> ") + info.keywords + "<br>";
			textLines += i18n("<b>Use flags:</b> ") + info.useFlags + "<br>";
			textLines += i18n("<b>Size:</b> ") + info.size;

	return textLines;
}

/**
 * Get this version ebuild.
 * @param id
 * @return ebuild text
 */
QString Portage::ebuild( const QString& packageId )
{
	QString package(Portage::package( packageId ));
	QString category(Portage::category( packageId ));
	
	QString fileName = KurooConfig::dirPortage() + "/" + category + "/" + package.section(pv, 0, 0) + "/" + package + ".ebuild";
	QFile file( fileName );
	
	if ( !file.exists() ) {
		fileName = KurooConfig::dirPortageOverlay() + "/" + category + "/" + package.section(pv, 0, 0) + "/" + package + ".ebuild";
		file.setName( fileName );
	}
	
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		QString textLines;
		while ( !stream.atEnd() )
			textLines += stream.readLine() + "<br>";
		file.close();
		return textLines;
	}
	else {
		kdDebug() << i18n("Error reading: ") << fileName << endl;
		return i18n("na");
	}
}

/**
 * Get this package changelog.
 * @param id
 * @return changelog text
 */
QString Portage::changelog( const QString& packageId )
{
	QString package(Portage::package( packageId ));
	QString category(Portage::category( packageId ));
	
	QString fileName = KurooConfig::dirPortage() + "/" + category + "/" + package.section(pv, 0, 0) + "/ChangeLog";
	QFile file( fileName );
	
	if ( !file.exists() ) {
		fileName = KurooConfig::dirPortageOverlay() + "/" + category + "/" + package.section(pv, 0, 0) + "/ChangeLog";
		file.setName( fileName );
	}
	
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		QString textLines;
		while ( !stream.atEnd() )
			textLines += stream.readLine() + "<br>";
		file.close();
		return textLines;
	}
	else {
		kdDebug() << i18n("Error reading: ") << fileName << endl;
		return i18n("na");
	}
}

/**
 * Get this package dependencies.
 * @param id
 * @return summary
 */
QString Portage::dependencies( const QString& packageId )
{
	QString package( Portage::package( packageId ) );
	QString category( Portage::category( packageId ) );
	
	QString fileName = KurooConfig::dirEdbDep() + "/usr/portage/" + category + "/" + package;
	QFile file( fileName );
	
	if ( !file.exists() ) {
		fileName = KurooConfig::dirEdbDep() + "/usr/local/portage/" + category + "/" + package;
		file.setName( fileName );
	}
	
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		QString textLines;
		int lineCount(0);
		while ( !stream.atEnd() ) {
			QString line = stream.readLine();
			if ( line.isEmpty() )
				continue;
			
			if ( lineCount++ > 1 || line == "0" )
				break;
			else
				textLines += line + "<br>";
		}
		file.close();
		return textLines;
	}
	else {
		kdDebug() << i18n("Error reading: ") << fileName << endl;
		return i18n("na");
	}
}

#include "portage.moc"
