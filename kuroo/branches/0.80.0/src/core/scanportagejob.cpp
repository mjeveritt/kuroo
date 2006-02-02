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
#include <qevent.h>
#include <qtextstream.h>

#include <kglobal.h>

// capture positions inside the regexp. (like m_rxAtom.cap(POS_CALLSIGN))
#define POS_PACKAGE     1
#define POS_VERSION     2

/**
 * @class ScanPortageJob
 * @short Thread for scanning local portage tree for available packages.
 * The packages are counted first, this to get a correct refresh progress in the gui.
 * Next portage cache in KurooConfig::dirEdbDep() is scanned for packages,
 * first the portage overlay cache the official portage cache.
 * All packages are stored in table "package" in the database.
 */
ScanPortageJob::ScanPortageJob( QObject* parent )
	: ThreadWeaver::DependentJob( parent, "DBJob" ),
	m_db( KurooDBSingleton::Instance()->getStaticDbConnection() ), aborted( true ),
	rxAtom( "((?:[a-z]|[A-Z]|[0-9]|-|\\+|_)+)" // package name
			"-("            // start of the version part
			"(?:\\d+(?:\\.\\d+)*[a-z]?)" // base version number, including wildcard version matching (*)
			"(?:_(?:alpha|beta|pre|rc|p)\\d*)?" // version suffix
			"(?:-r\\d*)?"  // revision
			")?$"          // end of the (optional) version part and the atom string
		)
{
}

ScanPortageJob::~ScanPortageJob()
{
	KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
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
 * Scan Portage cache for packages in portage tree.
 * Inserting found packages in db.
 * @return success
 */
bool ScanPortageJob::doJob()
{
	int count;
	QDir dCategory, dPackage;
	dCategory.setFilter(QDir::Dirs | QDir::NoSymLinks);
	dCategory.setSorting(QDir::Name);
	
	if ( !m_db->isConnected() ) {
		kdDebug() << i18n("Can not connect to database") << endl;
		return false;
	}
	
	if ( !KurooConfig::portageCount().isEmpty() )
		setProgressTotalSteps( KurooConfig::portageCount().toInt() );
	else
		setProgressTotalSteps( 10000 );
	
	setStatus( "ScanPortage", i18n("Refreshing Portage view...") );
	
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
	                                    " name VARCHAR(32), "
	                                    " latest VARCHAR(32), "
	                                    " description VARCHAR(255), "
	                                    " homepage VARCHAR(32), "
	                                    " date VARCHAR(32), "
	                                    " meta INTEGER, "
	                                    " updateVersion VARCHAR(32) "
	                                    " );", m_db);
	
	// Temporary table for all versions
	KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE version_temp ("
	                                    " id INTEGER PRIMARY KEY AUTOINCREMENT, "
	                                    " idPackage INTEGER, "
	                                    " name VARCHAR(32),"
	                                    " licenses VARCHAR(32), "
	                                    " useFlags VARCHAR(32),"
	                                    " slot VARCHAR(32),"
	                                    " size VARCHAR(32), "
	                                    " meta INTEGER, "
	                                    " path VARCHAR(64), "
	                                    " branch VARCHAR(32)"
	                                    " );", m_db);
	
	
	// Gather all path = portage and overlays
	QStringList pathList = KurooConfig::dirPortage();
	const QStringList pathOverlays = QStringList::split( " ", KurooConfig::dirPortageOverlayAll() );
	foreach ( pathOverlays )
		pathList += *it;
	
	// Scan Portage cache
	for ( QStringList::Iterator itPath = pathList.begin(), itPathEnd = pathList.end(); itPath != itPathEnd; ++itPath ) {
	
		if ( !dCategory.cd( KurooConfig::dirEdbDep() + *itPath ) ) {
			kdDebug() << i18n("Can not access ") << KurooConfig::dirEdbDep() + *itPath  << endl;
			continue;
		}
		
		// Get list of categories in Portage
		int idCategory;
		QString lastCategory;
		QStringList categoryList = dCategory.entryList();
		for ( QStringList::Iterator itCategory = categoryList.begin(), itCategoryEnd = categoryList.end(); itCategory != itCategoryEnd; ++itCategory ) {
			
			if ( *itCategory == "." || *itCategory == ".." )
				continue;
			
			// Abort the scan
			if ( isAborted() ) {
				kdDebug() << i18n("Portage scan aborted") << endl;
				KurooDBSingleton::Instance()->query( "ROLLBACK TRANSACTION;", m_db );
				return false;
			}
			
			QString category = ( *itCategory ) .section( "-", 0, 0 );
			QString subCategory = ( *itCategory ).section( "-", 1, 1 );
			
			if ( lastCategory != category )
				idCategory = KurooDBSingleton::Instance()->insert( QString( 
					"INSERT INTO category_temp (name) VALUES ('%1');" ).arg( category ), m_db );
			
			int idSubCategory = KurooDBSingleton::Instance()->insert(QString( 
				"INSERT INTO subCategory_temp (name, idCategory) "
				"VALUES ('%1', '%2');").arg(subCategory).arg(QString::number(idCategory)), m_db);
			
			int idCatSubCategory = KurooDBSingleton::Instance()->insert( QString( 
				"INSERT INTO catSubCategory_temp (name, idCategory, idSubCategory) "
				"VALUES ('%1', '%2', '%3');").arg(*itCategory).arg(QString::number(idCategory)).arg(QString::number(idSubCategory)), m_db);
			
			// Get list of packages in this category
			dPackage.setFilter( QDir::Files | QDir::NoSymLinks );
			dPackage.setSorting( QDir::Name );
			
			if ( dPackage.cd( KurooConfig::dirEdbDep() + *itPath + "/" + *itCategory) ) {
				
				QStringList packageList = dPackage.entryList();
				QString meta, lastPackage;
				for ( QStringList::Iterator itPackage = packageList.begin(), itPackageEnd = packageList.end(); itPackage != itPackageEnd; ++itPackage ) {
					
					if ( *itPackage == "." || *itPackage == ".." || (*itPackage).contains("MERGING") )
						continue;
					
					// Abort the scan
					if ( isAborted() ) {
						kdDebug() << i18n( "Portage scan aborted" ) << endl;
						KurooDBSingleton::Instance()->query( "ROLLBACK TRANSACTION;", m_db );
						return false;
					}
					
					if ( rxAtom.exactMatch( *itPackage ) ) {
						
						// Get the captured strings
						QString name = rxAtom.cap( POS_PACKAGE );
						QString version = rxAtom.cap( POS_VERSION );
						
						Info info( scanInfo( KurooConfig::dirEdbDep() + *itPath, *itCategory, name, version ) );
						
						// Insert category and db id's in portage
						if ( !categories.contains( *itCategory ) ) {
							categories[ *itCategory ].idCategory = QString::number( idCategory );
							categories[ *itCategory ].idSubCategory = QString::number( idSubCategory );
							categories[ *itCategory ].idCatSubCategory = QString::number( idCatSubCategory );
						}
						
						// Insert package in portage
						if ( !categories[ *itCategory ].packages.contains( name ) ) {
							categories[ *itCategory ].packages[ name ];
							categories[ *itCategory ].packages[ name ].meta = FILTER_ALL_STRING;
							categories[ *itCategory ].packages[ name ].description = info.description;
							categories[ *itCategory ].packages[ name ].homepage = info.homepage;
						}
						
						// Insert version in portage
						if ( !categories[ *itCategory ].packages[ name ].versions.contains( version ) ) {
							categories[ *itCategory ].packages[ name ].versions[ version ].meta = FILTER_ALL_STRING;
							categories[ *itCategory ].packages[ name ].versions[ version ].licenses = info.licenses;
							categories[ *itCategory ].packages[ name ].versions[ version ].useFlags = info.useFlags;
							categories[ *itCategory ].packages[ name ].versions[ version ].slot = info.slot;
							categories[ *itCategory ].packages[ name ].versions[ version ].size = info.size;
							categories[ *itCategory ].packages[ name ].versions[ version ].keywords = info.keywords;
							categories[ *itCategory ].packages[ name ].versions[ version ].path = *itPath;
						}
					
					}
					else
						kdDebug() << i18n("Scanning Portage cache: can not match package %1.").arg( *itPackage ) << endl;
					
					// Post scan count progress
					if ( ( ++count % 100 ) == 0 )
						setProgress( count );
				}
			}
			else
				kdDebug() << i18n( "Can not access " ) << KurooConfig::dirEdbDep() << *itPath << *itCategory << endl;
			
			lastCategory = category;
		}
	}
	KurooConfig::setPortageCount( QString::number(count) );
	KurooConfig::writeConfig();
	
	scanInstalledPackages();
	
	// Iterate through portage map and insert everything in db
	PortageCategories::iterator itCategoryEnd = categories.end();
	for ( PortageCategories::iterator itCategory = categories.begin(); itCategory != itCategoryEnd; ++itCategory ) {
	
		PortagePackages::iterator itPackageEnd = itCategory.data().packages.end();
		for ( PortagePackages::iterator itPackage = itCategory.data().packages.begin(); itPackage != itPackageEnd; ++itPackage ) {
			
			QString idPackage;
			QString idCategory = itCategory.data().idCategory;
			QString idSubCategory = itCategory.data().idSubCategory;
			QString idCatSubCategory = itCategory.data().idCatSubCategory;
				
			QString category = itCategory.key();
			QString package = itPackage.key();
			QString meta = itPackage.data().meta;
			QString description = itPackage.data().description;
			QString homepage = itPackage.data().homepage;
			
			QString sql = QString( "INSERT INTO package_temp (idCategory, idSubCategory, idCatSubCategory, name, description, homepage, meta) "
			                       "VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7' " 
			                     )
				.arg( idCategory ).arg( idSubCategory ).arg( idCatSubCategory ).arg( package ).arg( description ).arg( homepage ).arg( meta );
			
			sql += QString( ");" );
			idPackage = QString::number( KurooDBSingleton::Instance()->insert( sql, m_db ) );
			
			PortageVersions::iterator itVersionEnd = itPackage.data().versions.end();
			for ( PortageVersions::iterator itVersion = itPackage.data().versions.begin(); itVersion != itVersionEnd; ++itVersion ) {
				
				QString version = itVersion.key();
				QString meta = itVersion.data().meta;
				QString licenses = itVersion.data().licenses;
				QString useFlags = itVersion.data().useFlags;
				QString slot = itVersion.data().slot;
				QString size = itVersion.data().size;
				QString keywords = itVersion.data().keywords;
				QString path = itVersion.data().path;
				
				KurooDBSingleton::Instance()->insert( QString(
					"INSERT INTO version_temp (idPackage, name, size, branch, meta, licenses, useFlags, slot, path) "
					"VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9');" 
					).arg( idPackage ).arg( version ).arg( size ).arg( keywords ).arg( meta ).arg( licenses ).arg( useFlags ).arg( slot ).arg( path ), m_db );
				
			}
		}
	}
	categories.clear();
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
	
	setStatus( "ScanPortage", i18n("Done.") );
	setProgress( 0 );
	return true;
}

/**
 * Collect list of installed packages.
 */
void ScanPortageJob::scanInstalledPackages()
{
	installedMap.clear();
	QDir dCategory, dPackage;
	dCategory.setFilter( QDir::Dirs | QDir::NoSymLinks );
	dCategory.setSorting( QDir::Name );
	
	if ( !dCategory.cd( KurooConfig::dirDbPkg() ) )
		kdDebug() << i18n( "Can not access " ) << KurooConfig::dirDbPkg() << endl;

	setStatus( "ScanInstalled", i18n("Collecting installed packages...") );
	
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
				
				if ( *itPackage == "." || *itPackage == ".." || ( *itPackage ).contains("MERGING") )
					continue;
				
				if ( rxAtom.exactMatch( *itPackage ) ) {
					
					// Get the captured strings
					QString name = rxAtom.cap( POS_PACKAGE );
					QString version = rxAtom.cap( POS_VERSION );
				
					// Insert category if not found in portage
					if ( !categories.contains( *itCategory ) )
						categories[ *itCategory ];
					
					// Insert and/or mark package as installed (old is package not in portage anymore)
					if ( !categories[ *itCategory ].packages.contains( name ) ) {
						categories[ *itCategory ].packages[ name ];
						categories[ *itCategory ].packages[ name ].meta = FILTER_OLD_STRING;
					}
					else
						categories[ *itCategory ].packages[ name ].meta = FILTER_INSTALLED_STRING;
					
					// Insert old version in portage
					if ( !categories[ *itCategory ].packages[ name ].versions.contains( version ) )
						categories[ *itCategory ].packages[ name ].versions[ version ];
					
					// Mark version as installed
					categories[ *itCategory ].packages[ name ].versions[ version ].meta = FILTER_INSTALLED_STRING;
					
				}
				else
					kdDebug() << i18n("Scanning installed packages: can not match %1.").arg( *itPackage ) << endl;
			}
		}
		else
			kdDebug() << i18n( "Can not access " ) << KurooConfig::dirDbPkg() << "/" << *itCategory << endl;
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
	
	if( !file.open( IO_ReadOnly ) ) {
		kdDebug() << i18n("Error reading: ") << path << "/" << category << "/" << name << "-" << version << endl;
		
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
	int lineNumber = 0;
	
	// Read out the package info strings
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
	file.close();
	
	// Get package size. Try in cache first.
	QString size = PortageSingleton::Instance()->cacheFind( category + "/" + name + "-" + version ) ;
	if ( !size.isEmpty() )
		info.size = formatSize( size );
	else {
		QString path = KurooConfig::dirPortage() + "/" + category + "/" + name + "/files/digest-" + name + "-" + version;
		file.setName( path );
		if ( file.open( IO_ReadOnly ) ) {
			std::ifstream in( path );
			std::string word;
			while ( in >> word );
			file.close();
			info.size = formatSize( word );
			
			// Add new value into cache.
			KurooDBSingleton::Instance()->insert( QString("INSERT INTO cache (package, size) "
			                                              "VALUES ('%1', '%2');").arg( name + "-" + version ).arg( word ), m_db);
		}
		else
			kdDebug() << i18n("Error reading: ") << path << endl;
	}
	
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

#include "scanportagejob.moc"

