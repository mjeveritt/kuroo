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
#include "pythonizer.h"

#include "loadportagejob.h"

/**
 * @class LoadPortageJob
 * @short Collect portage packages from portage api.
 */
LoadPortageJob::LoadPortageJob( QObject *parent )
	: ThreadWeaver::DependentJob( parent, "DBJob" ),
	m_db( KurooDBSingleton::Instance()->getStaticDbConnection() ), m_aborted( true )
{
}

LoadPortageJob::~LoadPortageJob()
{
	KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
}

/**
 * Inform gui thread that scan is completed.
 */
void LoadPortageJob::completeJob()
{
	DEBUG_LINE_INFO;
	
	SignalistSingleton::Instance()->scanPortageComplete();
	m_aborted = false;
}

/**
 * Inserting found packages in db.
 * @return success
 */
bool LoadPortageJob::doJob()
{
	if ( !m_db->isConnected() ) {
		kdError(0) << i18n("Scanning Portage. Can not connect to database") << LINE_INFO;
		return false;
	}
	
	setStatus( "LoadPortage", i18n("Refreshing Portage packages view...") );
	
	// Temporary table for all categories
	KurooDBSingleton::Instance()->query("BEGIN TRANSACTION;", m_db);
	KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE category_temp ("
	                                    " id INTEGER PRIMARY KEY AUTOINCREMENT,"
	                                    " name VARCHAR(32) UNIQUE "
	                                    " );", m_db);
	
	KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE subCategory_temp ("
	                                    " id INTEGER PRIMARY KEY AUTOINCREMENT,"
	                                    " name VARCHAR(32), "
	                                    " idCategory INTEGER "
	                                    " );", m_db);
	
	KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE catSubCategory_temp ("
	                                    " id INTEGER PRIMARY KEY AUTOINCREMENT,"
	                                    " name VARCHAR(32) UNIQUE, "
	                                    " idCategory INTEGER, "
	                                    " idSubCategory INTEGER "
	                                    " );", m_db);
	
	// Temporary table for all packages
	KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE package_temp ("
	                                    " id INTEGER PRIMARY KEY AUTOINCREMENT,"
	                                    " idCategory INTEGER, "
	                                    " idSubCategory INTEGER, "
	                                    " idCatSubCategory INTEGER, "
	                                    " category VARCHAR(32), "
	                                    " name VARCHAR(32), "
	                                    " description VARCHAR(255), "
	                                    " latest VARCHAR(32), "
	                                    " date VARCHAR(32), "
	                                    " status INTEGER, "
	                                    " meta VARCHAR(255), "
	                                    " updateVersion VARCHAR(32) "
	                                    " );", m_db);
	
	// Temporary table for all versions
	KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE version_temp ("
	                                    " id INTEGER PRIMARY KEY AUTOINCREMENT, "
	                                    " idPackage INTEGER, "
	                                    " name VARCHAR(32),"
	                                    " description VARCHAR(255), "
	                                    " homepage VARCHAR(128), "
	                                    " licenses VARCHAR(64), "
	                                    " useFlags VARCHAR(255),"
	                                    " slot VARCHAR(32),"
	                                    " size VARCHAR(32), "
	                                    " status INTEGER, "
	                                    " path VARCHAR(64), "
	                                    " branch VARCHAR(32)"
	                                    " );", m_db);
	
	// Connect to portage api and request portage packages.
	int packageCount = loadPackages();
	if ( packageCount == 0 )
		return false;
	setProgressTotalSteps( packageCount );
	
	// Iterate the portage map and insert categories in db.
	insertCategories();
	
	// Iterate the portage map and insert packages and versions in db
	int count( 0 );
	PortageCategories::iterator itCategoryEnd = m_categories.end();
	for ( PortageCategories::iterator itCategory = m_categories.begin(); itCategory != itCategoryEnd; ++itCategory ) {
		
		PortagePackages::iterator itPackageEnd = itCategory.data().packages.end();
		for ( PortagePackages::iterator itPackage = itCategory.data().packages.begin(); itPackage != itPackageEnd; ++itPackage ) {
			
			QString idCategory = itCategory.data().idCategory;
			QString idSubCategory = itCategory.data().idSubCategory;
			QString idCatSubCategory = itCategory.data().idCatSubCategory;
			
			QString category = itCategory.key();
			QString package = itPackage.key();
			QString status = itPackage.data().status;
			QString description = itPackage.data().description;
			
			// Create meta tag containing all text of interest for searching
			QString meta = category + " " + package + " " + description;
			
			QString sql = QString( "INSERT INTO package_temp (idCategory, idSubCategory, idCatSubCategory, category, "
			                       "name, description, status, meta) "
			                       "VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8');")
				.arg( idCategory ).arg( idSubCategory ).arg( idCatSubCategory ).arg( category ).arg( package )
				.arg( description ).arg( status ).arg( meta );
			
			QString idPackage = QString::number( KurooDBSingleton::Instance()->insert( sql, m_db ) );
			
			PortageVersions::iterator itVersionEnd = itPackage.data().versions.end();
			for ( PortageVersions::iterator itVersion = itPackage.data().versions.begin(); itVersion != itVersionEnd; ++itVersion ) {
				
				QString version = itVersion.key();
				description = itVersion.data().description;
				QString homepage = itVersion.data().homepage;
				QString status = itVersion.data().status;
				QString licenses = itVersion.data().licenses;
				QString iuse = itVersion.data().iuse;
				QString slot = itVersion.data().slot;
				QString size = itVersion.data().size;
				QString keywords = itVersion.data().keywords;
				QString path = itVersion.data().path;
				
				QString sqlVersion = QString( "INSERT INTO version_temp "
				                              "(idPackage, name, description, homepage, size, branch, status, "
				                              "licenses, useFlags, slot, path) "
				                              "VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9'," )
					.arg( idPackage ).arg( version ).arg( description ).arg( homepage ).arg( size )
					.arg( keywords ).arg( status ).arg( licenses ).arg( iuse );
				
				sqlVersion += QString( "'%1', '%2');" ).arg( slot ).arg( path );
				KurooDBSingleton::Instance()->insert( sqlVersion, m_db );
				
				// Post scan count progress
				if ( ( ++count % 100 ) == 0 )
					setProgress( count );
			}
		}
	}
	m_categories.clear();
	KurooDBSingleton::Instance()->query("COMMIT TRANSACTION;", m_db);
	KurooDBSingleton::Instance()->query( QString("UPDATE dbInfo SET data = '%1' WHERE meta = 'packageCount';")
	                                     .arg( count ), m_db );
	
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
	
	setStatus( "LoadPortage", i18n("Done.") );
	setProgressTotalSteps( 0 );
	return true;
}

/**
 * Iterate the portage map and insert categories in db
 */
bool LoadPortageJob::insertCategories()
{
	QString lastCategory;
	int idCategory;
	PortageCategories::iterator itCategoryEnd = m_categories.end();
	for ( PortageCategories::iterator itCategory = m_categories.begin(); itCategory != itCategoryEnd; ++itCategory ) {
		
		QString category = itCategory.data().category;
		QString subCategory = itCategory.data().subCategory;
		
		if ( lastCategory != category )
			idCategory = KurooDBSingleton::Instance()->insert( 
				QString( "INSERT INTO category_temp (name) VALUES ('%1');" ).arg( category ), m_db );
		
		lastCategory = category;
		
		int idSubCategory = KurooDBSingleton::Instance()->insert(
			QString( "INSERT INTO subCategory_temp (name, idCategory) VALUES ('%1', '%2');")
			.arg( subCategory ).arg( QString::number( idCategory) ), m_db );
		
		int idCatSubCategory = KurooDBSingleton::Instance()->insert( 
			QString( "INSERT INTO catSubCategory_temp (name, idCategory, idSubCategory) VALUES ('%1', '%2', '%3');")
			.arg( itCategory.key() ).arg( QString::number( idCategory ) ).arg( QString::number( idSubCategory ) ), m_db );
		
		m_categories[ itCategory.key() ].idCategory = QString::number( idCategory );
		m_categories[ itCategory.key() ].idSubCategory = QString::number( idSubCategory );
		m_categories[ itCategory.key() ].idCatSubCategory = QString::number( idCatSubCategory );
	}
	
	return true;
}

/**
 *  Connect to portage api, request portage packages and load in m_categories map.
 */
int LoadPortageJob::loadPackages()
{
	// Initialize the python module
	Pythonizer* pythonizer = new Pythonizer( "kuroo_portage_api", "PortageInterface" );
	
	// Collect all portage packages list
	const QStringList allPackagesList = pythonizer->getPackages( "allPackagesData" );
	kdDebug() << "allPackagesData.size()=" << allPackagesList.size() << LINE_INFO;
	
	if ( allPackagesList.isEmpty() ) {
		delete pythonizer;
		return 0;
	}
	
	// Parse the list of packages which has the format:
	// ...(category-subCategory/name, description, homepage, licenses, keywords, iuse, slot)...
	foreach ( allPackagesList ) {
		
		// Parse out package parts
		QString categorySubCategory = (*it).section( "/", 0, 0 );
		QString category = categorySubCategory.section( "-", 0, 0 );
		QString subCategory = categorySubCategory.section( "-", 1, 1 );
		QString name = (*it).section( "/", 1, 1 );
		QString version = (*it).section( "/", 2, 2 );
		it++;
		
		// Ebuild info
		QString description = escapeString( *it++ );
		QString homepage = escapeString( *it++ );
		QString licenses = escapeString( *it++ );
		QString keywords = *it++;
		QString iuse = *it++;
		QString slot = *it;
		QString size;
		QString path;
		
		// Insert category and db id's in portage
		if ( !m_categories.contains( categorySubCategory ) ) {
			m_categories[ categorySubCategory ].category = category;
			m_categories[ categorySubCategory ].subCategory = subCategory;
		}
		
		// Insert package in portage
		if ( !m_categories[ categorySubCategory ].packages.contains( name ) ) {
			m_categories[ categorySubCategory ].packages[ name ];
			m_categories[ categorySubCategory ].packages[ name ].status = PACKAGE_AVAILABLE_STRING;
			m_categories[ categorySubCategory ].packages[ name ].description = description;
		}
		
		// Insert version in portage
		if ( !m_categories[ categorySubCategory ].packages[ name ].versions.contains( version ) ) {
			m_categories[ categorySubCategory ].packages[ name ].versions[ version ].description = description;
			m_categories[ categorySubCategory ].packages[ name ].versions[ version ].homepage = homepage;
			m_categories[ categorySubCategory ].packages[ name ].versions[ version ].licenses = licenses;
			m_categories[ categorySubCategory ].packages[ name ].versions[ version ].keywords = keywords;
			m_categories[ categorySubCategory ].packages[ name ].versions[ version ].iuse = iuse;
			m_categories[ categorySubCategory ].packages[ name ].versions[ version ].slot = slot;
			m_categories[ categorySubCategory ].packages[ name ].versions[ version ].size = size;
			m_categories[ categorySubCategory ].packages[ name ].versions[ version ].path = path;
			m_categories[ categorySubCategory ].packages[ name ].versions[ version ].status = PACKAGE_AVAILABLE_STRING;
		}
	}
	
	// Collect installed packages list
	const QStringList installedPackagesList = pythonizer->getPackages( "installedPackagesVersion" );
	kdDebug() << "installedPackagesList.size()=" << installedPackagesList.size() << LINE_INFO;
	
	// Parse the list of packages which has the format:
	// ...(category-subCategory/name/version)...
	foreach ( installedPackagesList ) {
		
		// Parse out package parts
		QString categorySubCategory = (*it).section( "/", 0, 0 );
		QString category = categorySubCategory.section( "-", 0, 0 );
		QString subCategory = categorySubCategory.section( "-", 1, 1 );
		QString name = (*it).section( "/", 1, 1 );
		QString version = (*it).section( "/", 2, 2 );
		
		// Insert category if not found in portage
		if ( !m_categories.contains( categorySubCategory ) ) {
			m_categories[ categorySubCategory ].category = category;
			m_categories[ categorySubCategory ].subCategory = subCategory;
		}
		
		// Insert and/or mark package as installed (old is package not in portage anymore)
		if ( !m_categories[ categorySubCategory ].packages.contains( name ) ) {
			m_categories[ categorySubCategory ].packages[ name ];
			m_categories[ categorySubCategory ].packages[ name ].status = PACKAGE_OLD_STRING;
		}
		else
			m_categories[ categorySubCategory ].packages[ name ].status = PACKAGE_INSTALLED_STRING;
		
		// Insert old version in portage
		if ( !m_categories[ categorySubCategory ].packages[ name ].versions.contains( version ) )
			m_categories[ categorySubCategory ].packages[ name ].versions[ version ];
		
		// Mark version as installed
		m_categories[ categorySubCategory ].packages[ name ].versions[ version ].status = PACKAGE_INSTALLED_STRING;
	}
	
	delete pythonizer;
	return allPackagesList.size() / 7;
}

#include "loadportagejob.moc"
