/***************************************************************************
 *   Copyright (C) 2007 by Karye                                           *
 *   info@kuroo.org                                                        *
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

	// Fetch all portage packages by attaching external sqlite database
	QStringList cachePackages;
	const QStringList& sqliteFiles = GlobalSingleton::Instance()->sqliteFileList();
	
	foreach ( sqliteFiles ) {
		KuroolitoDBSingleton::Instance()->singleQuery( QString( "ATTACH DATABASE '%1' AS portage;" ).arg( *it ), m_db );
		QString path = ( *it ).section( KuroolitoConfig::dirEdbDep(), 1, 1 );
		cachePackages += KuroolitoDBSingleton::Instance()->query( QString( "SELECT '%1' AS path, portage_package_key, _mtime_, homepage, license, description, keywords, iuse FROM portage.portage_packages;" ).arg( path ), m_db );
		KuroolitoDBSingleton::Instance()->singleQuery( "DETACH DATABASE portage;", m_db );
	}
	
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
	const QStringList& cachePackagesList = cachePackages;
	foreach ( cachePackagesList ) {
		QString path = *it++;
		QString package = *it++;
		QString mtime = *it++;
		QString homepage = *it++;
		homepage = homepage.replace('\'', "''").replace('%', "&#37;");
		QString license = *it++;
		license = license.replace('\'', "''").replace('%', "&#37;");
		QString description = *it++;
		description = description.replace('\'', "''").replace('%', "&#37;");
		QString keywords = *it++;
		QString iuse = *it;

		QString categorySubcategory = package.section( "/", 0, 0 );
		QString category = ( categorySubcategory ) .section( "-", 0, 0 );
		QString subCategory = ( categorySubcategory ).section( "-", 1, 1 );
		
		QString nameVersion = package.section("/", 1, 1);
		QStringList parts = GlobalSingleton::Instance()->parsePackage( nameVersion );
		
		if ( !parts.isEmpty() ) {
			QString name = parts[1];
			QString version = parts[2];

			// Insert unique category-subcategory and db id's in portage
			if ( !m_categories.contains( categorySubcategory ) ) {
				
				// Only insert category once
				if ( lastCategory != category )
					idCategory = KuroolitoDBSingleton::Instance()->insert( QString( "INSERT INTO category_temp (name) VALUES ('%1');" ).arg( category ), m_db );
				
				// .. and now insert next found subcategory
				int idSubCategory = KuroolitoDBSingleton::Instance()->insert( QString( "INSERT INTO subCategory_temp (name, idCategory) VALUES ('%1', '%2');")
				.arg( subCategory ).arg( QString::number( idCategory ) ), m_db);
				
				m_categories[ categorySubcategory ].idCategory = QString::number( idCategory );
				m_categories[ categorySubcategory ].idSubCategory = QString::number( idSubCategory );
			}
			
			// Insert package in portage
			if ( !m_categories[ categorySubcategory ].packages.contains( name ) ) {
				m_categories[ categorySubcategory ].packages[ name ];
				m_categories[ categorySubcategory ].packages[ name ].status = PACKAGE_AVAILABLE_STRING;
				m_categories[ categorySubcategory ].packages[ name ].description = description;
				m_categories[ categorySubcategory ].packages[ name ].path = path;
			}
			
			// Insert version in portage
			if ( !m_categories[ categorySubcategory ].packages[ name ].versions.contains( version ) ) {
				m_categories[ categorySubcategory ].packages[ name ].versions[ version ].description = description;
				m_categories[ categorySubcategory ].packages[ name ].versions[ version ].homepage = homepage;
				m_categories[ categorySubcategory ].packages[ name ].versions[ version ].status = PACKAGE_AVAILABLE_STRING;
				m_categories[ categorySubcategory ].packages[ name ].versions[ version ].licenses = license;
				m_categories[ categorySubcategory ].packages[ name ].versions[ version ].useFlags = iuse;
				m_categories[ categorySubcategory ].packages[ name ].versions[ version ].slot = QString::null;
				m_categories[ categorySubcategory ].packages[ name ].versions[ version ].size = QString::null;
				m_categories[ categorySubcategory ].packages[ name ].versions[ version ].keywords = keywords;
			}
		}
		else
			kdWarning(0) << "Scanning Portage. Scanning Portage cache: can not match package " << package << LINE_INFO;
		
		lastCategory = category;
	}
	
	// Now scan installed packages, eg mark packages as installed and add "old" packages (not in Portage anymore)
	scanInstalledPackages();
	
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

/**
 * Collect list of installed packages.
 */
void ScanPortageJob::scanInstalledPackages()
{
	QDir dCategory, dPackage;
	dCategory.setFilter( QDir::Dirs | QDir::NoSymLinks );
	dCategory.setSorting( QDir::Name );
	
	if ( !dCategory.cd( KuroolitoConfig::dirDbPkg() ) )
		kdWarning(0) << "Scanning installed packages. Can not access " << KuroolitoConfig::dirDbPkg() << LINE_INFO;
	
	// Get list of categories for installed packages
	QStringList categoryList = dCategory.entryList();
	QStringList::Iterator itCategoryEnd = categoryList.end();
	for ( QStringList::Iterator itCategory = categoryList.begin(); itCategory != itCategoryEnd; ++itCategory ) {
		
		if ( *itCategory == "." || *itCategory == ".." )
			continue;
		
		// Get list of packages in this category
		dPackage.setFilter( QDir::Dirs | QDir::NoSymLinks );
		dPackage.setSorting( QDir::Name );
		
		if ( dPackage.cd( KuroolitoConfig::dirDbPkg() + "/" + *itCategory ) ) {
			QStringList packageList = dPackage.entryList();
			QStringList::Iterator itPackageEnd = packageList.end();
			for ( QStringList::Iterator itPackage = packageList.begin(); itPackage != itPackageEnd; ++itPackage ) {
				
				if ( *itPackage == "." || *itPackage == ".." || ( *itPackage ).contains("MERGING") )
					continue;
				
				QStringList parts = GlobalSingleton::Instance()->parsePackage( *itPackage );
				if ( !parts.isEmpty() ) {
					QString name = parts[1];
					QString version = parts[2];

					// Insert category if not found in portage
					if ( !m_categories.contains( *itCategory ) )
						m_categories[ *itCategory ];
					
					// Insert and/or mark package as installed (old is package not in portage anymore)
					if ( !m_categories[ *itCategory ].packages.contains( name ) ) {
						m_categories[ *itCategory ].packages[ name ];
						m_categories[ *itCategory ].packages[ name ].status = PACKAGE_OLD_STRING;
					}
					else
						m_categories[ *itCategory ].packages[ name ].status = PACKAGE_INSTALLED_STRING;
					
					// Insert old version in portage
					if ( !m_categories[ *itCategory ].packages[ name ].versions.contains( version ) )
						m_categories[ *itCategory ].packages[ name ].versions[ version ];
					
					// Mark version as installed
					m_categories[ *itCategory ].packages[ name ].versions[ version ].status = PACKAGE_INSTALLED_STRING;
					
				}
				else
					kdWarning(0) << "Scanning installed packages. Can not match " << *itPackage << LINE_INFO;
			}
		}
		else
			kdWarning(0) << "Scanning installed packages. Can not access " << KuroolitoConfig::dirDbPkg() << "/" << *itCategory << LINE_INFO;
	}
}

#include "scanportagejob.moc"
