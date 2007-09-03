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

#include <sqlite3.h>
#include <fstream>
#include <string>
#include <vector>

#include <qdir.h>
#include <qfileinfo.h>
#include <qevent.h>
#include <qtextstream.h>

#include <kglobal.h>

/**
 * @class ScanPortageJob
 * @short Thread for scanning local portage tree for available packages.
 * 
 * The packages are counted first, this to get a correct refresh progress in the gui.
 * Next portage cache in KuroolitoConfig::dirEdbDep() is scanned for packages.
 * All packages are stored in table "package" in the database.
 */
ScanPortageJob::ScanPortageJob( QObject* parent )
	: ThreadWeaver::DependentJob( parent, "DBJob" ),
	m_db( KuroolitoDBSingleton::Instance()->getStaticDbConnection() )
{}

ScanPortageJob::~ScanPortageJob()
{
	KuroolitoDBSingleton::Instance()->returnStaticDbConnection( m_db );
	
	if ( isAborted() )
		SignalistSingleton::Instance()->scanAborted();
}

/**
 * Inform gui thread that scan is completed.
 */
void ScanPortageJob::completeJob()
{
	SignalistSingleton::Instance()->scanPortageComplete();
}

/**
 * Scan Portage cache for packages in portage tree. Inserting found packages in db.
 * @return success
 */
bool ScanPortageJob::doJob()
{
	int count( 0 );
	QDir dCategory, dPackage;
	dCategory.setFilter( QDir::Dirs | QDir::NoSymLinks );
	dCategory.setSorting( QDir::Name );
	
	if ( !m_db->isConnected() ) {
		kdError(0) << "Scanning Portage. Can not connect to database" << LINE_INFO;
		return false;
	}
	
	// Get a count of total packages for proper progress
	QString packageCount = KuroolitoDBSingleton::Instance()->singleQuery( "SELECT data FROM dbInfo WHERE meta = 'packageCount' LIMIT 1;", m_db );
	if ( packageCount == "0" )
		setProgressTotalSteps( 25000 );
	else
		setProgressTotalSteps( packageCount.toInt() );
	
// 	setStatus( "ScanPortage", i18n("Refreshing Portage packages view...") );
	
	// Load Portage cache files to speed up portage scan
// 	loadCache();
	
	// Fetch all portage packages by attaching external sqlite database
	KuroolitoDBSingleton::Instance()->singleQuery( "ATTACH DATABASE '/var/cache/edb/dep/usr/portage.sqlite' AS portage;", m_db );
	const QStringList& cachePackages = KuroolitoDBSingleton::Instance()->query( "SELECT portage_package_key FROM portage.portage_packages;", m_db );
	kdWarning(0) << "cachePackages.size()=" << cachePackages.size() << LINE_INFO;
	KuroolitoDBSingleton::Instance()->singleQuery( "DETACH DATABASE portage;", m_db );
	
	// Temporary table for all categories
	KuroolitoDBSingleton::Instance()->singleQuery("BEGIN TRANSACTION;", m_db );
	KuroolitoDBSingleton::Instance()->singleQuery("CREATE TEMP TABLE category_temp ( "
	                                    		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
	                                    		"name VARCHAR(32) UNIQUE );"
	                                    		, m_db );
	
	KuroolitoDBSingleton::Instance()->singleQuery("CREATE TEMP TABLE subCategory_temp ( "
	                                    		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
	                                    		"name VARCHAR(32), "
	                                    		"idCategory INTEGER );"
	                                    		, m_db );
	
	// Temporary table for all packages
	KuroolitoDBSingleton::Instance()->singleQuery("CREATE TEMP TABLE package_temp ( "
	                                    		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
	                                    		"idCategory INTEGER, "
	                                          	"idSubCategory INTEGER, "
	                                          	"category VARCHAR(32), "
	                                          	"name VARCHAR(32), "
	                                          	"description VARCHAR(255), "
	                                          	"path VARCHAR(64), "
	                                          	"status INTEGER, "
	                                          	"meta VARCHAR(255), "
	                                          	"updateVersion VARCHAR(32) );"
	                                          	, m_db );
	
	// Temporary table for all versions
	KuroolitoDBSingleton::Instance()->singleQuery("CREATE TEMP TABLE version_temp ( "
	                                          	"id INTEGER PRIMARY KEY AUTOINCREMENT, "
	                                          	"idPackage INTEGER, "
	                                          	"name VARCHAR(32),"
	                                          	"description VARCHAR(255), "
	                                          	"homepage VARCHAR(128), "
	                                          	"licenses VARCHAR(64), "
	                                          	"useFlags VARCHAR(255),"
	                                          	"slot VARCHAR(32),"
	                                          	"size VARCHAR(32), "
	                                          	"status INTEGER, "
	                                          	"keywords VARCHAR(32) );"
	                                          	, m_db );
	
	int idCategory;
	QString lastCategory;
	foreach ( cachePackages ) {
		QString package = *it;
// 		kdWarning(0) << "package=" << package << LINE_INFO;
		
		QString categorySubcategory = package.section( "/", 0, 0 );
		QString category = ( categorySubcategory ) .section( "-", 0, 0 );
		QString subCategory = ( categorySubcategory ).section( "-", 1, 1 );
		
		QString nameVersion = package.section("/", 1, 1);
		QStringList parts = GlobalSingleton::Instance()->parsePackage( nameVersion );
		
		if ( !parts.isEmpty() ) {
			QString name = parts[1];
			QString version = parts[2];
		
/*			if ( lastCategory != category )
				idCategory = KuroolitoDBSingleton::Instance()->insert( QString( "INSERT INTO category_temp (name) VALUES ('%1');" ).arg( category ), m_db );*/
			
// 			int idSubCategory = KuroolitoDBSingleton::Instance()->insert( QString( "INSERT INTO subCategory_temp (name, idCategory) VALUES ('%1', '%2');")
// 				.arg( subCategory ).arg( QString::number( idCategory ) ), m_db);
			
			// Insert unique category-subcategory and db id's in portage
			if ( !m_categories.contains( categorySubcategory ) ) {
				
				// Only insert category once
				if ( lastCategory != category )
					idCategory = KuroolitoDBSingleton::Instance()->insert( QString( "INSERT INTO category_temp (name) VALUES ('%1');" ).arg( category ), m_db );
				// .. and now insert next found subcategory
				int idSubCategory = KuroolitoDBSingleton::Instance()->insert( QString( "INSERT INTO subCategory_temp (name, idCategory) VALUES ('%1', '%2');")
				.arg( subCategory ).arg( QString::number( idCategory ) ), m_db);
				
				kdWarning(0) << "_package=" << package << LINE_INFO;
				kdWarning(0) << "categorySubcategory=" << categorySubcategory << LINE_INFO;
				kdWarning(0) << "category=" << category << LINE_INFO;
				kdWarning(0) << "subCategory=" << subCategory << LINE_INFO;
				kdWarning(0) << "name=" << name << LINE_INFO;
				kdWarning(0) << "version=" << version << LINE_INFO;
				kdWarning(0) << "idCategory=" << idCategory << LINE_INFO;
				kdWarning(0) << "idSubCategory=" << idSubCategory << LINE_INFO;
				
				m_categories[ categorySubcategory ].idCategory = QString::number( idCategory );
				m_categories[ categorySubcategory ].idSubCategory = QString::number( idSubCategory );
			}
			
			// Insert package in portage
			if ( !m_categories[ categorySubcategory ].packages.contains( name ) ) {
				m_categories[ categorySubcategory ].packages[ name ];
				m_categories[ categorySubcategory ].packages[ name ].status = PACKAGE_AVAILABLE_STRING;
				m_categories[ categorySubcategory ].packages[ name ].description = QString::null;
				m_categories[ categorySubcategory ].packages[ name ].path = QString::null;
			}
			
			// Insert version in portage
			if ( !m_categories[ categorySubcategory ].packages[ name ].versions.contains( version ) ) {
				m_categories[ categorySubcategory ].packages[ name ].versions[ version ].description = QString::null;
				m_categories[ categorySubcategory ].packages[ name ].versions[ version ].homepage = QString::null;
				m_categories[ categorySubcategory ].packages[ name ].versions[ version ].status = PACKAGE_AVAILABLE_STRING;
				m_categories[ categorySubcategory ].packages[ name ].versions[ version ].licenses = QString::null;
				m_categories[ categorySubcategory ].packages[ name ].versions[ version ].useFlags = QString::null;
				m_categories[ categorySubcategory ].packages[ name ].versions[ version ].slot = QString::null;
				m_categories[ categorySubcategory ].packages[ name ].versions[ version ].size = QString::null;
				m_categories[ categorySubcategory ].packages[ name ].versions[ version ].keywords = QString::null;
			}
		}
		else
			kdWarning(0) << "Scanning Portage. Scanning Portage cache: can not match package " << package << LINE_INFO;
		
		lastCategory = category;
	}
	
	// Now scan installed packages, eg mark packages as installed and add "old" packages (not in Portage anymore)
// 	scanInstalledPackages();
	
	// Iterate through portage map and insert everything in db
	PortageCategories::iterator itCategoryEnd = m_categories.end();
	for ( PortageCategories::iterator itCategory = m_categories.begin(); itCategory != itCategoryEnd; ++itCategory ) {
	
		PortagePackages::iterator itPackageEnd = itCategory.data().packages.end();
		for ( PortagePackages::iterator itPackage = itCategory.data().packages.begin(); itPackage != itPackageEnd; ++itPackage ) {
			
			QString idPackage;
			QString idCategory = itCategory.data().idCategory;
			QString idSubCategory = itCategory.data().idSubCategory;
			
			QString category = itCategory.key();
			QString package = itPackage.key();
			QString status = itPackage.data().status;
			QString description = itPackage.data().description;
			QString path = itPackage.data().path;
			
			// Create meta tag containing all text of interest for searching
			QString meta = category + " " + package + " " + description;
			
			QString sql = QString( "INSERT INTO package_temp (idCategory, idSubCategory, category, name, description, status, path, meta) "
								"VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8');")
								.arg( idCategory ).arg( idSubCategory ).arg( category ).arg( package )
								.arg( description ).arg( status ).arg( path ).arg( meta );
			
			idPackage = QString::number( KuroolitoDBSingleton::Instance()->insert( sql, m_db ) );
			
			PortageVersions::iterator itVersionEnd = itPackage.data().versions.end();
			for ( PortageVersions::iterator itVersion = itPackage.data().versions.begin(); itVersion != itVersionEnd; ++itVersion ) {
				
				QString version = itVersion.key();
				description = itVersion.data().description;
				QString homepage = itVersion.data().homepage;
				QString status = itVersion.data().status;
				QString licenses = itVersion.data().licenses;
				QString useFlags = itVersion.data().useFlags;
				QString slot = itVersion.data().slot;
				QString size = itVersion.data().size;
				QString keywords = itVersion.data().keywords;
				
				QString sqlVersion = QString( "INSERT INTO version_temp "
											"(idPackage, name, description, homepage, size, keywords, status, licenses, useFlags, slot) "
											"VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9', " )
					.arg( idPackage ).arg( version ).arg( description ).arg( homepage ).arg( size )
					.arg( keywords ).arg( status ).arg( licenses ).arg( useFlags );
				
				sqlVersion += QString( "'%1');" ).arg( slot );
				KuroolitoDBSingleton::Instance()->insert( sqlVersion, m_db );
			}
		}
	}
	m_categories.clear();
	
	KuroolitoDBSingleton::Instance()->singleQuery( "COMMIT TRANSACTION;", m_db );
	KuroolitoDBSingleton::Instance()->singleQuery( QString("UPDATE dbInfo SET data = '%1' WHERE meta = 'packageCount';").arg( count ), m_db );
	
	// Move content from temporary table 
	KuroolitoDBSingleton::Instance()->singleQuery( "DELETE FROM category;", m_db );
	KuroolitoDBSingleton::Instance()->singleQuery( "DELETE FROM subCategory;", m_db );
	KuroolitoDBSingleton::Instance()->singleQuery( "DELETE FROM package;", m_db );
	KuroolitoDBSingleton::Instance()->singleQuery( "DELETE FROM version;", m_db );

	KuroolitoDBSingleton::Instance()->singleQuery( "BEGIN TRANSACTION;", m_db );
	KuroolitoDBSingleton::Instance()->insert( "INSERT INTO category SELECT * FROM category_temp;", m_db );
	KuroolitoDBSingleton::Instance()->insert( "INSERT INTO subCategory SELECT * FROM subCategory_temp;", m_db );
	KuroolitoDBSingleton::Instance()->insert( "INSERT INTO package SELECT * FROM package_temp;", m_db );
	KuroolitoDBSingleton::Instance()->insert( "INSERT INTO version SELECT * FROM version_temp;", m_db );
	KuroolitoDBSingleton::Instance()->singleQuery( "COMMIT TRANSACTION;", m_db );
	
	KuroolitoDBSingleton::Instance()->singleQuery( "DROP TABLE category_temp;", m_db );
	KuroolitoDBSingleton::Instance()->singleQuery( "DROP TABLE subCategory_temp;", m_db );
	KuroolitoDBSingleton::Instance()->singleQuery( "DROP TABLE package_temp;", m_db );
	KuroolitoDBSingleton::Instance()->singleQuery( "DROP TABLE version_temp;", m_db );
	
	return true;
}
	
	
	
	
	
	
// 	// Gather all path = portage and overlays
// 	QStringList pathList = KuroolitoConfig::dirPortage();
// 	const QStringList pathOverlays = QStringList::split( " ", KuroolitoConfig::dirPortageOverlay() );
// 	foreach ( pathOverlays )
// 		pathList += *it;
// 	
// 	// Scan Portage cache
// 	for ( QStringList::Iterator itPath = pathList.begin(), itPathEnd = pathList.end(); itPath != itPathEnd; ++itPath ) {
// 	
// 		if ( !dCategory.cd( KuroolitoConfig::dirEdbDep() + *itPath ) ) {
// 			kdWarning(0) << "Scanning Portage. Can not access " << KuroolitoConfig::dirEdbDep() + *itPath  << LINE_INFO;
// 			continue;
// 		}
// 		
// 		// Get list of categories in Portage
// 		int idCategory;
// 		QString lastCategory;
// 		QStringList categoryList = dCategory.entryList();
// 		for ( QStringList::Iterator itCategory = categoryList.begin(), itCategoryEnd = categoryList.end(); itCategory != itCategoryEnd; ++itCategory ) {
// 			
// 			if ( *itCategory == "." || *itCategory == ".." )
// 				continue;
// 			
// 			// Abort the scan
// 			if ( isAborted() ) {
// 				kdWarning(0) << "Scanning Portage. Portage scan aborted" << LINE_INFO;
// 				KuroolitoDBSingleton::Instance()->singleQuery( "ROLLBACK TRANSACTION;", m_db );
// 				return false;
// 			}
// 			
// 			QString category = ( *itCategory ) .section( "-", 0, 0 );
// 			QString subCategory = ( *itCategory ).section( "-", 1, 1 );
// 			
// 			if ( lastCategory != category )
// 				idCategory = KuroolitoDBSingleton::Instance()->insert( QString( "INSERT INTO category_temp (name) VALUES ('%1');" ).arg( category ), m_db );
// 			
// 			int idSubCategory = KuroolitoDBSingleton::Instance()->insert(QString( "INSERT INTO subCategory_temp (name, idCategory) VALUES ('%1', '%2');")
//     			.arg( subCategory ).arg( QString::number( idCategory ) ), m_db);
// 			
// 			// Get list of packages in this category
// 			dPackage.setFilter( QDir::Files | QDir::NoSymLinks );
// 			dPackage.setSorting( QDir::Name );
// 			
// 			if ( dPackage.cd( KuroolitoConfig::dirEdbDep() + *itPath + "/" + *itCategory) ) {
// 				
// 				QStringList packageList = dPackage.entryList();
// 				QString status, lastPackage;
// 				for ( QStringList::Iterator itPackage = packageList.begin(), itPackageEnd = packageList.end(); itPackage != itPackageEnd; ++itPackage ) {
// 					
// 					if ( *itPackage == "." || *itPackage == ".." || (*itPackage).contains("MERGING") )
// 						continue;
// 					
// 					// Abort the scan
// 					if ( isAborted() ) {
// 						kdWarning(0) << "Scanning Portage. Portage scan aborted" << LINE_INFO;
// 						KuroolitoDBSingleton::Instance()->singleQuery( "ROLLBACK TRANSACTION;", m_db );
// 						return false;
// 					}
// 					
// 					QStringList parts = GlobalSingleton::Instance()->parsePackage( *itPackage );
// 					if ( !parts.isEmpty() ) {
// 						QString name = parts[1];
// 						QString version = parts[2];
// 						
// 						Info info( scanInfo( KuroolitoConfig::dirEdbDep() + *itPath, *itCategory, name, version ) );
// 						
// 						// Insert category and db id's in portage
// 						if ( !m_categories.contains( *itCategory ) ) {
// 							m_categories[ *itCategory ].idCategory = QString::number( idCategory );
// 							m_categories[ *itCategory ].idSubCategory = QString::number( idSubCategory );
// 						}
// 						
// 						// Insert package in portage
// 						if ( !m_categories[ *itCategory ].packages.contains( name ) ) {
// 							m_categories[ *itCategory ].packages[ name ];
// 							m_categories[ *itCategory ].packages[ name ].status = PACKAGE_AVAILABLE_STRING;
// 							m_categories[ *itCategory ].packages[ name ].description = info.description;
// 							m_categories[ *itCategory ].packages[ name ].path = *itPath;
// 						}
// 						
// 						// Insert version in portage
// 						if ( !m_categories[ *itCategory ].packages[ name ].versions.contains( version ) ) {
// 							m_categories[ *itCategory ].packages[ name ].versions[ version ].description = info.description;
// 							m_categories[ *itCategory ].packages[ name ].versions[ version ].homepage = info.homepage;
// 							m_categories[ *itCategory ].packages[ name ].versions[ version ].status = PACKAGE_AVAILABLE_STRING;
// 							m_categories[ *itCategory ].packages[ name ].versions[ version ].licenses = info.licenses;
// 							m_categories[ *itCategory ].packages[ name ].versions[ version ].useFlags = info.useFlags;
// 							m_categories[ *itCategory ].packages[ name ].versions[ version ].slot = info.slot;
// 							m_categories[ *itCategory ].packages[ name ].versions[ version ].size = info.size;
// 							m_categories[ *itCategory ].packages[ name ].versions[ version ].keywords = info.keywords;
// 						}
// 					
// 					}
// 					else
// 						kdWarning(0) << "Scanning Portage. Scanning Portage cache: can not match package " << *itPackage << LINE_INFO;
// 					
// 					// Post scan count progress
// 					if ( ( ++count % 100 ) == 0 )
// 						setProgress( count );
// 				}
// 			}
// 			else
// 				kdWarning(0) << "Scanning Portage. Can not access " << KuroolitoConfig::dirEdbDep() << *itPath << *itCategory << LINE_INFO;
// 			
// 			lastCategory = category;
// 		}
// 	}
// 	
// 	DEBUG_LINE_INFO;
// 	// Now scan installed packages, eg mark packages as installed and add "old" packages (not in Portage anymore)
// 	scanInstalledPackages();
// 	
// 	// Iterate through portage map and insert everything in db
// 	PortageCategories::iterator itCategoryEnd = m_categories.end();
// 	for ( PortageCategories::iterator itCategory = m_categories.begin(); itCategory != itCategoryEnd; ++itCategory ) {
// 	
// 		PortagePackages::iterator itPackageEnd = itCategory.data().packages.end();
// 		for ( PortagePackages::iterator itPackage = itCategory.data().packages.begin(); itPackage != itPackageEnd; ++itPackage ) {
// 			
// 			QString idPackage;
// 			QString idCategory = itCategory.data().idCategory;
// 			QString idSubCategory = itCategory.data().idSubCategory;
// 			
// 			QString category = itCategory.key();
// 			QString package = itPackage.key();
// 			QString status = itPackage.data().status;
// 			QString description = itPackage.data().description;
// 			QString path = itPackage.data().path;
// 			
// 			// Create meta tag containing all text of interest for searching
// 			QString meta = category + " " + package + " " + description;
// 			
// 			QString sql = QString( "INSERT INTO package_temp (idCategory, idSubCategory, category, "
// 			                       "name, description, status, path, meta) "
// 			                       "VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8');")
// 				.arg( idCategory ).arg( idSubCategory ).arg( category ).arg( package )
// 				.arg( description ).arg( status ).arg( path ).arg( meta );
// 			
// 			idPackage = QString::number( KuroolitoDBSingleton::Instance()->insert( sql, m_db ) );
// 			
// 			PortageVersions::iterator itVersionEnd = itPackage.data().versions.end();
// 			for ( PortageVersions::iterator itVersion = itPackage.data().versions.begin(); itVersion != itVersionEnd; ++itVersion ) {
// 				
// 				QString version = itVersion.key();
// 				description = itVersion.data().description;
// 				QString homepage = itVersion.data().homepage;
// 				QString status = itVersion.data().status;
// 				QString licenses = itVersion.data().licenses;
// 				QString useFlags = itVersion.data().useFlags;
// 				QString slot = itVersion.data().slot;
// 				QString size = itVersion.data().size;
// 				QString keywords = itVersion.data().keywords;
// 				
// 				QString sqlVersion = QString( "INSERT INTO version_temp "
// 				                              "(idPackage, name, description, homepage, size, keywords, status, licenses, useFlags, slot) "
// 				                              "VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9', " )
// 					.arg( idPackage ).arg( version ).arg( description ).arg( homepage ).arg( size )
// 					.arg( keywords ).arg( status ).arg( licenses ).arg( useFlags );
// 				
// 				sqlVersion += QString( "'%1');" ).arg( slot );
// 				KuroolitoDBSingleton::Instance()->insert( sqlVersion, m_db );
// 			}
// 		}
// 	}
// 	m_categories.clear();
// 	
// 	KuroolitoDBSingleton::Instance()->singleQuery( "COMMIT TRANSACTION;", m_db );
// 	KuroolitoDBSingleton::Instance()->singleQuery( QString("UPDATE dbInfo SET data = '%1' WHERE meta = 'packageCount';").arg( count ), m_db );
// 	
// 	// Move content from temporary table 
// 	KuroolitoDBSingleton::Instance()->singleQuery( "DELETE FROM category;", m_db );
// 	KuroolitoDBSingleton::Instance()->singleQuery( "DELETE FROM subCategory;", m_db );
// 	KuroolitoDBSingleton::Instance()->singleQuery( "DELETE FROM package;", m_db );
// 	KuroolitoDBSingleton::Instance()->singleQuery( "DELETE FROM version;", m_db );
// 
// 	KuroolitoDBSingleton::Instance()->singleQuery( "BEGIN TRANSACTION;", m_db );
// 	KuroolitoDBSingleton::Instance()->insert( "INSERT INTO category SELECT * FROM category_temp;", m_db );
// 	KuroolitoDBSingleton::Instance()->insert( "INSERT INTO subCategory SELECT * FROM subCategory_temp;", m_db );
// 	KuroolitoDBSingleton::Instance()->insert( "INSERT INTO package SELECT * FROM package_temp;", m_db );
// 	KuroolitoDBSingleton::Instance()->insert( "INSERT INTO version SELECT * FROM version_temp;", m_db );
// 	KuroolitoDBSingleton::Instance()->singleQuery( "COMMIT TRANSACTION;", m_db );
// 	
// 	KuroolitoDBSingleton::Instance()->singleQuery( "DROP TABLE category_temp;", m_db );
// 	KuroolitoDBSingleton::Instance()->singleQuery( "DROP TABLE subCategory_temp;", m_db );
// 	KuroolitoDBSingleton::Instance()->singleQuery( "DROP TABLE package_temp;", m_db );
// 	KuroolitoDBSingleton::Instance()->singleQuery( "DROP TABLE version_temp;", m_db );
	
// 	setStatus( "ScanPortage", i18n("Done.") );
// 	setProgressTotalSteps( 0 );
// 	return true;
// }

/**
 * Collect list of installed packages.
 */
void ScanPortageJob::scanInstalledPackages()
{
	DEBUG_LINE_INFO;
	
	setStatus( "ScanInstalled", i18n("Collecting installed packages...") );
	
	QFile file( "/var/cache/kuroo/installedPackages.lst" );
	if ( !file.open( IO_ReadOnly ) )
		kdError(0) << "Cannot read /var/cache/kuroo/installedPackages.lst." << LINE_INFO;
	else {
		QTextStream stream( &file );
		while ( !stream.atEnd() ) {
			QString package = stream.readLine();
			
			QString category = package.section("/", 0, 0);
			QString name = package.section("/", 1, 1);
			QString version = package.section("/", 2, 2);

			// Insert category if not found in portage
			if ( !m_categories.contains( category ) )
				m_categories[ category ];
			
			// Insert and/or mark package as installed (old is package not in portage anymore)
			if ( !m_categories[ category ].packages.contains( name ) ) {
				m_categories[ category ].packages[ name ];
				m_categories[ category ].packages[ name ].status = PACKAGE_OLD_STRING;
			}
			else
				m_categories[ category ].packages[ name ].status = PACKAGE_INSTALLED_STRING;
			
			// Insert old version in portage
			if ( !m_categories[ category ].packages[ name ].versions.contains( version ) )
				m_categories[ category ].packages[ name ].versions[ version ];
			
			// Mark version as installed
			m_categories[ category ].packages[ name ].versions[ version ].status = PACKAGE_INSTALLED_STRING;
		}
		file.close();
	}
	setStatus( "ScanInstalled", i18n("Done.") );
}

/**
 * Collect info about this ebuild. Based on Jakob Petsovits code.
 * @param category   	
 * @param package  	
 * @return  false if the file can't be opened, true otherwise.
 */
Info ScanPortageJob::scanInfo( const QString& path, const QString& category, const QString& name, const QString& version )
{
	Info info;
	QFile file( path + "/" + category + "/" + name + "-" + version );
	
	if ( !file.open( IO_ReadOnly ) ) {
		kdWarning(0) << "Scanning Portage cache. Reading: " << path << "/" << category << "/" << name << "-" << version << LINE_INFO;
		
		info.slot = "0";
		info.homepage = "0";
		info.licenses = "0";
		info.description = "0";
		info.keywords = "0";
		info.useFlags = "0";
		info.size = "0";
		return info;
	}
	
	QString line;
	QTextStream stream( &file );
	int lineNumber(0);
	
	// Check portage version and read out the package info strings
	if ( KuroolitoConfig::portageVersion21() ) {
		
		// We are on portage version post 2.1
		while ( !stream.atEnd() ) {
			line = stream.readLine();
			
			if ( line.startsWith( "LICENSE=" ) )
				info.licenses = line.section("LICENSE=", 1, 1).replace('\'', "''").replace('%', "&#37;");
			else
				if ( line.startsWith( "KEYWORDS=" ) )
					info.keywords = line.section("KEYWORDS=", 1, 1);
				else
					if ( line.startsWith( "SLOT=" ) )
						info.slot = line.section("SLOT=", 1, 1);
					else
						if ( line.startsWith( "DESCRIPTION=" ) )
							info.description = line.section("DESCRIPTION=", 1, 1).replace('\'', "''").replace('%', "&#37;");
						else
							if ( line.startsWith( "IUSE=" ) )
								info.useFlags = line.section("IUSE=", 1, 1);
							else
								if ( line.startsWith( "HOMEPAGE=" ) )
									info.homepage = line.section("HOMEPAGE=", 1, 1).replace('\'', "''").replace('%', "&#37;");
		}
	}
	else {
	
		// We are on portage version pre 2.1
		while ( !stream.atEnd() ) {
			line = stream.readLine();
			lineNumber++;
			
			// each line has a fixed meaning, as it seems.
			// so iterate through the lines.
			switch( lineNumber ) {
				case 1: // some dependency stuff
					break;
				case 2: // some other dependency stuff
					break;
				case 3: // the package slot
					info.slot = line;
					break;
				case 4: // file location, starting with mirror://
					break;
				case 5: // empty?
					break;
				case 6: // DirHome page
					info.homepage = line.replace('\'', "''").replace('%', "&#37;");
					break;
				case 7: // licenses
					info.licenses = line.replace('\'', "''").replace('%', "&#37;");
					break;
				case 8: // description
					info.description = line.replace('\'', "''").replace('%', "&#37;");
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
	}
	file.close();
	return info;
}

/**
 * Format package size nicely 
 * @fixme: Check out KIO_EXPORT QString KIO::convertSize
 * @param size 
 * @return total		as "xxx kB"
 */
QString ScanPortageJob::formatSize( const QString& size )
{
	KLocale *loc = KGlobal::locale();
	QString total;
	
	uint num = ("0" + size).toInt();
	if ( num < 1024 )
		total = "1 kB ";
	else
		total = loc->formatNumber((double)(num / 1024), 0) + " kB ";
	
	return total;
}

/**
 * Load m_mapCache with items from DB.
 */
void ScanPortageJob::loadCache()
{
	
	KuroolitoDBSingleton::Instance()->singleQuery( "ATTACH /var/cache/edb/dep/usr/portage.sqlite AS CACHE;", m_db );
	
// 	m_mapCache.clear();
// 	const QStringList cacheList = KuroolitoDBSingleton::Instance()->query( "SELECT package, size FROM cache ;", m_db );
// 	foreach ( cacheList ) {
// 		QString package = *it++;
// 		QString size = *it;
// 		m_mapCache.insert( package, size );
// 	}
}

/**
 * Find cached size for package.
 * @param packages
 * @return size or NULL if na
 */
QString ScanPortageJob::cacheFind( const QString& package )
{
	QMap<QString, QString>::iterator it = m_mapCache.find( package ) ;
	if ( it != m_mapCache.end() )
		return it.data();
	else
		return QString::null;
}

#include "scanportagejob.moc"
