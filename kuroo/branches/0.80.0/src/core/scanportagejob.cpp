/***************************************************************************
 *   Copyright (C) 2005 by Karye   *
 *   karye@users.sourceforge.net   *
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

#include <fstream>
#include <string>
#include <vector>

#include <qdir.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qevent.h>
#include <qtextstream.h>

#include <kglobal.h>

/**
 * Thread for scanning local portage tree for available packages.
 * The packages are counted first, this to get a correct refresh progress in the gui.
 * Next portage cache in KurooConfig::dirEdbDep() is scanned for packages,
 * first the portage overlay cache the official portage cache.
 * All packages are stored in table "package" in the database.
 */
ScanPortageJob::ScanPortageJob( QObject* parent )
	: ThreadWeaver::DependentJob( parent, "DBJob" ),
	m_db( KurooDBSingleton::Instance()->getStaticDbConnection() ), aborted(true)
{
}

ScanPortageJob::~ScanPortageJob()
{
	KurooDBSingleton::Instance()->returnStaticDbConnection(m_db);
	if ( aborted )
		SignalistSingleton::Instance()->scanAborted();
}

/**
 * Inform gui thread that scan is completed.
 */
void ScanPortageJob::completeJob()
{
	SignalistSingleton::Instance()->scanPortageComplete();
	aborted = false;
}


/**
 * Scan KurooConfig::dirEdbDep() + "/usr/portage" for packages in portage tree.
 * Inserting found packages in db.
 * @return success
 */
bool ScanPortageJob::doJob()
{
	int count(0);
	QDir dCategory, dPackage;
	dCategory.setFilter(QDir::Dirs | QDir::NoSymLinks);
	dCategory.setSorting(QDir::Name);
	
	if ( !m_db->isConnected() ) {
		kdDebug() << i18n("Can not connect to database") << endl;
		return false;
	}
	
	QString path = KurooConfig::dirEdbDep() + "/usr/portage/";
	if ( !dCategory.cd( path ) ) {
		kdDebug() << i18n("Can not access ") << path << endl;
		return false;
	}
	
	setProgressTotalSteps( KurooConfig::portageCount().toInt() );
	setStatus( i18n("Refreshing Portage view...") );
	
	getInstalled();
	
	// Temporary table for all categories
	KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE category_temp ("
	                                    " id INTEGER PRIMARY KEY AUTOINCREMENT,"
	                                    " name VARCHAR(32) "
	                                    " );", m_db);
	
	KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE subCategory_temp ("
	                                    " id INTEGER PRIMARY KEY AUTOINCREMENT,"
	                                    " name VARCHAR(32), "
	                                    " idCategory INTEGER "
	                                    " );", m_db);
	
	KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE catSubCategory_temp ("
	                                    " id INTEGER PRIMARY KEY AUTOINCREMENT,"
	                                    " name VARCHAR(32), "
	                                    " idCategory INTEGER, "
	                                    " idSubCategory INTEGER "
	                                    " );", m_db);
	
	// Temporary table for all packages
	KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE package_temp ("
	                                    " id INTEGER PRIMARY KEY AUTOINCREMENT,"
	                                    " idCategory INTEGER, "
	                                    " idSubCategory INTEGER, "
	                                    " idCatSubCategory INTEGER, "
	                                    " name VARCHAR(32), "
	                                    " latest VARCHAR(32), "
	                                    " description VARCHAR(255), "
	                                    " homepage VARCHAR(32), "
	                                    " licenses VARCHAR(32), "
	                                    " useFlags VARCHAR(32),"
	                                    " packageSlots VARCHAR(32),"
	                                    " date VARCHAR(32), "
	                                    " installed INTEGER, "
	                                    " updateVersion VARCHAR(32) "
	                                    " );", m_db);
	
	// Temporary table for all versions
	KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE version_temp ("
	                                    " id INTEGER PRIMARY KEY AUTOINCREMENT, "
	                                    " idPackage INTEGER, "
	                                    " name VARCHAR(32),"
	                                    " size VARCHAR(32), "
	                                    " branch VARCHAR(32)"
	                                    " );", m_db);
	
	KurooDBSingleton::Instance()->query("BEGIN TRANSACTION;", m_db);
	
	// Get list of categories in Portage
	int idCategory;
	QString installed, lastCategory;
	QStringList categoryList = dCategory.entryList();
	for ( QStringList::Iterator itCategory = categoryList.begin(), itCategoryEnd = categoryList.end(); itCategory != itCategoryEnd; ++itCategory ) {
		
		if ( *itCategory == "." || *itCategory == ".." )
			continue;
		
		// Abort the scan
		if ( isAborted() ) {
			kdDebug() << i18n("Portage scan aborted") << endl;
			KurooDBSingleton::Instance()->query("ROLLBACK TRANSACTION;", m_db);
			return false;
		}
		
		QString category = (*itCategory).section("-", 0, 0);
		QString subCategory = (*itCategory).section("-", 1, 1);
		
		if ( lastCategory != category )
			idCategory = KurooDBSingleton::Instance()->insert(QString("INSERT INTO category_temp (name) VALUES ('%1');").arg(category), m_db);
		
		int idSubCategory = KurooDBSingleton::Instance()->insert(QString("INSERT INTO subCategory_temp (name, idCategory) VALUES ('%1', '%2');").arg(subCategory).arg(QString::number(idCategory)), m_db);
		
		int idCatSubCategory = KurooDBSingleton::Instance()->insert(QString("INSERT INTO catSubCategory_temp (name, idCategory, idSubCategory) VALUES ('%1', '%2', '%3');").arg(*itCategory).arg(QString::number(idCategory)).arg(QString::number(idSubCategory)), m_db);
		
		// Get list of packages in this category
		dPackage.setFilter( QDir::Files | QDir::NoSymLinks );
		dPackage.setSorting( QDir::Name );
		if ( dPackage.cd( path + *itCategory) ) {
			
			QStringList packageList = dPackage.entryList();
			QString lastPackage;
			int idPackage( 0 );
			for ( QStringList::Iterator itPackage = packageList.begin(), itPackageEnd = packageList.end(); itPackage != itPackageEnd; ++itPackage ) {
				
				if ( *itPackage == "." || *itPackage == ".." || (*itPackage).contains("MERGING") )
					continue;
				
				// Abort the scan
				if ( isAborted() ) {
					kdDebug() << i18n( "Portage scan aborted" ) << endl;
					KurooDBSingleton::Instance()->query( "ROLLBACK TRANSACTION;", m_db );
					return false;
				}
				
				QString package = (*itPackage).section( pv, 0, 0 );
				QString version = (*itPackage).section( package + "-", 1, 1 );

				// Check if package is installed
				InstalledMap::iterator itMap = installedMap.find( package );
				if ( itMap != installedMap.end() ) {
					installed = QString::number( FILTER_INSTALLED );
					installedMap.erase( itMap );
				}
				else
					installed = QString::number( FILTER_ALL );
				
				if ( package != lastPackage ) {
				
					if ( scanInfo( path, *itCategory, *itPackage ) ) {
						QString sql = QString("INSERT INTO package_temp (idCategory, idSubCategory, idCatSubCategory, name, description, installed, homepage, licenses, useFlags, packageSlots) VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9', ").arg(QString::number(idCategory)).arg(QString::number(idSubCategory)).arg(QString::number(idCatSubCategory)).arg(package).arg(info.description).arg(installed).arg(info.homepage).arg(info.licenses).arg(info.useFlags);
						sql += QString("'%1');").arg(info.packageSlots);
						idPackage = KurooDBSingleton::Instance()->insert( sql, m_db );
					}
					else
						idPackage = KurooDBSingleton::Instance()->insert( QString("INSERT INTO package_temp (idCategory, idSubCategory, idCatSubCategory, name, installed) VALUES ('%1', '%2', '%3', '%4', '%5');").arg(QString::number(idCategory)).arg(QString::number(idSubCategory)).arg(QString::number(idCatSubCategory)).arg(package).arg(installed), m_db);
					
					KurooDBSingleton::Instance()->insert( QString("INSERT INTO version_temp (idPackage, name, size, branch) VALUES ('%1', '%2', '%3', '%4');").arg(QString::number(idPackage)).arg(version).arg(info.size).arg(info.keywords), m_db);
				}
				else {
					KurooDBSingleton::Instance()->insert( QString("INSERT INTO version_temp (idPackage, name, size, branch) VALUES ('%1', '%2', '%3', '%4');").arg(QString::number(idPackage)).arg(version).arg(info.size).arg(info.keywords), m_db);
				}
				lastPackage = package;
				
				// Post scan count progress
				if ( (++count % 100) == 0 )
					setProgress( count );
			}
		}
		else
			kdDebug() << i18n("Can not access ") << KurooConfig::dirEdbDep() << "/usr/portage/" << *itCategory << endl;
		
		lastCategory = category;
	}
	KurooDBSingleton::Instance()->query("COMMIT TRANSACTION;", m_db);
		
	// Move content from temporary table 
	KurooDBSingleton::Instance()->query("DELETE FROM category;", m_db);
	KurooDBSingleton::Instance()->query("DELETE FROM subCategory;", m_db);
	KurooDBSingleton::Instance()->query("DELETE FROM catSubCategory;", m_db);
	KurooDBSingleton::Instance()->query("DELETE FROM package;", m_db);
	KurooDBSingleton::Instance()->query("DELETE FROM version;", m_db);
	
	KurooDBSingleton::Instance()->insert("INSERT INTO category SELECT * FROM category_temp;", m_db);
	KurooDBSingleton::Instance()->insert("INSERT INTO subCategory SELECT * FROM subCategory_temp;", m_db);
	KurooDBSingleton::Instance()->insert("INSERT INTO catSubCategory SELECT * FROM catSubCategory_temp;", m_db);
	KurooDBSingleton::Instance()->insert("INSERT INTO package SELECT * FROM package_temp;", m_db);
	KurooDBSingleton::Instance()->insert("INSERT INTO version SELECT * FROM version_temp;", m_db);
	
	KurooDBSingleton::Instance()->query("DROP TABLE category_temp;", m_db);
	KurooDBSingleton::Instance()->query("DROP TABLE subCategory_temp;", m_db);
	KurooDBSingleton::Instance()->query("DROP TABLE catSubCategory_temp;", m_db);
	KurooDBSingleton::Instance()->query("DROP TABLE package_temp;", m_db);
	KurooDBSingleton::Instance()->query("DROP TABLE version_temp;", m_db);
	
	setStatus( i18n("Done.") );
	setProgress(0);
	return true;
}

/**
 * Collect list of installed packages.
 */
void ScanPortageJob::getInstalled()
{
	installedMap.clear();
	int count( 0 );
	QDir dCategory, dPackage;
	dCategory.setFilter( QDir::Dirs | QDir::NoSymLinks );
	dCategory.setSorting( QDir::Name );
	
	if ( !m_db->isConnected() ) {
		kdDebug() << i18n( "Can not connect to database" ) << endl;
	}
	
	if ( !dCategory.cd( KurooConfig::dirDbPkg() ) ) {
		kdDebug() << i18n( "Can not access " ) << KurooConfig::dirDbPkg() << endl;
	}
	
	// Get list of categories for installed packages
	QStringList categoryList = dCategory.entryList();
	QStringList::Iterator itCategoryEnd = categoryList.end();
	for ( QStringList::Iterator itCategory = categoryList.begin(); itCategory != itCategoryEnd; ++itCategory ) {
		
		if ( *itCategory == "." || *itCategory == ".." )
			continue;
		
		// Get list of packages in this category
		dPackage.setFilter( QDir::Dirs | QDir::NoSymLinks );
		dPackage.setSorting( QDir::Name );
		
		if ( dPackage.cd( KurooConfig::dirDbPkg() + "/" + *itCategory ) ) {
			QStringList packageList = dPackage.entryList();
			QStringList::Iterator itPackageEnd = packageList.end();
			for ( QStringList::Iterator itPackage = packageList.begin(); itPackage != itPackageEnd; ++itPackage ) {
				
				if ( *itPackage == "." || *itPackage == ".." || (*itPackage).contains("MERGING") )
					continue;
				
				installedMap.insert( (*itPackage).section(pv, 0, 0), 0 );
			}
		}
		else
			kdDebug() << i18n( "Can not access " ) << KurooConfig::dirDbPkg() << "/" << *itCategory << endl;
	}
}

/**
 * Collect info about this ebuild. Based on Jakob Petsovits code.
 * @param category   	
 * @param package  	
 * @return  false if the file can't be opened, true otherwise.
 */
bool ScanPortageJob::scanInfo( const QString& path, const QString& category, const QString& package )
{
	QFile file( path + category + "/" + package);
	
	if( !file.open(IO_ReadOnly) ) {
		kdDebug() << i18n("Error reading: ") << path << category << "/" << package << endl;
		return false;
	}
	
	QString line;
	QTextStream stream( &file );
	int lineNumber = 0;
	
	// Read out the package info strings
	while ( !stream.atEnd() )
	{
		line = stream.readLine();
		lineNumber++;
		
		// each line has a fixed meaning, as it seems.
		// so iterate through the lines.
		switch( lineNumber )
		{
		case 1: // some dependency stuff
			break;
		case 2: // some other dependency stuff
			break;
		case 3: // the package slot
			info.packageSlots = line;
			break;
		case 4: // file location, starting with mirror://
			break;
		case 5: // empty?
			break;
		case 6: // DirHome page
			info.homepage = line.replace('\'', "''");
			break;
		case 7: // licenses
			info.licenses = line.replace('\'', "''");
			break;
		case 8: // description
			info.description = line.replace('\'', "''");
			break;
		case 9: // keywords
			info.keywords = line;
			break;
		case 10: // inherited eclasses?
			break;
		case 11: // useFlags
			info.useFlags = line;
			break;
		default:
			break;
		}
	}
	file.close();
	
	// Get package size. Try in cache first.
	QString packageNoVersion = category + "/" + package.section(pv, 0, 0);
	QString size = PortageSingleton::Instance()->cacheFind( category + "/" + package ) ;
	if ( size != NULL )
		info.size = kBSize(size);
	else {
		QString path = KurooConfig::dirPortage() + "/" + packageNoVersion + "/files/digest-" + package;
		file.setName( path );
		if ( file.open( IO_ReadOnly ) ) {
			std::ifstream in( path );
			std::string word;
			while ( in >> word );
			file.close();
			info.size = kBSize(word);
			
			// Add new value into cache.
			KurooDBSingleton::Instance()->insert( QString("INSERT INTO cache (package, size) VALUES ('%1', '%2');").arg(package).arg(word), m_db);
		}
		else
			kdDebug() << i18n("Error reading: ") << path << endl;
	}
	
	return true;
}

/**
 * Format package size nicely 
 * @fixme: Check out KIO_EXPORT QString KIO::convertSize
 * @param size 
 * @return total		as "xxx kB"
 */
QString ScanPortageJob::kBSize( const QString& size )
{
	QString total = "";
	
	int num = ("0" + size).toInt();
	if (num < 1024) {
		total = "1 kB ";
	}
	else {
		QString eString = QString::number(num / 1024);
		
		while ( !eString.isEmpty() ) {
			QString part = eString.right(3);
			eString = eString.left(eString.length() - part.length());
			
			if ( !total.isEmpty() )
				total = part + "," + total;
			else
				total = part;
		}
		total += " kB ";
	}
	
	return total;
}

#include "scanportagejob.moc"

