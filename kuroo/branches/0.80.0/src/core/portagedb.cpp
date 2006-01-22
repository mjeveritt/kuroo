/**************************************************************************
*   Copyright (C) 2004 by karye                                           *
*   karye@users.sourceforge.net                                           *
*   From Amarok code.                                                     *
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

#include <math.h>                 //DbConnection::sqlite_power()
#include <time.h>                 //query()
#include <unistd.h>               //usleep()
#include <grp.h>
#include <pwd.h>

#include <qfile.h>
#include <qimage.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kio/job.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kio/netaccess.h>
#include <kcursor.h>

/**
 * @class KurooDB
 * @short Handle database connections and queries.
 */
KurooDB::KurooDB( QObject *m_parent )
	: QObject( m_parent )
{
}

KurooDB::~KurooDB()
{
	destroy();
}

/**
 * Check db integrity and create new db if necessary.
 * Set write permission for regular user.
 * @return database file
 */
QString KurooDB::init( QObject *parent )
{
	m_parent = parent;
	
	m_dbConnPool = new DbConnectionPool();
	DbConnection *dbConn = m_dbConnPool->getDbConnection();
	m_dbConnPool->putDbConnection(dbConn);
	
	if ( !dbConn->isInitialized() || !isValid() )
		createTables();
	
	m_dbConnPool->createDbConnections();
	
	return KUROODIR + KurooConfig::databas();
}

DbConnection *KurooDB::getStaticDbConnection()
{
// 	kdDebug() << "KurooDB::getStaticDbConnection-------------" << endl;
	return m_dbConnPool->getDbConnection();
}

void KurooDB::returnStaticDbConnection(DbConnection *conn)
{
// 	kdDebug() << "--------------KurooDB::returnStaticDbConnection " << endl;
	m_dbConnPool->putDbConnection(conn);
}

void KurooDB::destroy()
{
	delete m_dbConnPool;
}

/**
 * Executes a SQL query on the already opened database
 * @param statement SQL program to execute. Only one SQL statement is allowed.
 * @return          The queried data, or QStringList() on error.
 */
QStringList KurooDB::query( const QString& statement, DbConnection *conn )
{
// 	kdDebug() << "Query-start: " << statement << endl;
// 	clock_t start = clock();
	
	DbConnection *dbConn;
	if ( conn != NULL )
		dbConn = conn;
	else
		dbConn = m_dbConnPool->getDbConnection();
	
	QStringList values = dbConn->query( statement );
	
	if ( conn == NULL )
		m_dbConnPool->putDbConnection( dbConn );
	
// 	clock_t finish = clock();
// 	const double duration = (double) ( finish - start ) / CLOCKS_PER_SEC;
// 	kdDebug() << "SQL-query (" << duration << "s): " << statement << endl;

	return values;
}

/**
 * Executes a SQL insert on the already opened database
 * @param statement SQL statement to execute. Only one SQL statement is allowed.
 * @return          The rowid of the inserted item.
 */
int KurooDB::insert( const QString& statement, DbConnection *conn )
{
// 	kdDebug() << "insert-start: " << statement << endl;
// 	clock_t start = clock();
		
	DbConnection *dbConn;
	if ( conn != NULL )
		dbConn = conn;
	else
		dbConn = m_dbConnPool->getDbConnection();
	
	int id = dbConn->insert( statement );
	
	if ( conn == NULL )
		m_dbConnPool->putDbConnection( dbConn );
	
// 	clock_t finish = clock();
// 	const double duration = (double) (finish - start) / CLOCKS_PER_SEC;
// 	kdDebug() << "SQL-insert (" << duration << "s): " << statement << endl;

	return id;
}

bool KurooDB::isPortageEmpty()
{
	QStringList values = query("SELECT COUNT(id) FROM package LIMIT 0, 1;");
	return values.isEmpty() ? true : values.first() == "0";
}

bool KurooDB::isHistoryEmpty()
{
	QStringList values = query("SELECT COUNT(id) FROM history LIMIT 0, 1;");
	return values.isEmpty() ? true : values.first() == "0";
}

bool KurooDB::isValid()
{
	QStringList values1 = query("SELECT COUNT(id) FROM category LIMIT 0, 1;");
	QStringList values2 = query("SELECT COUNT(id) FROM package LIMIT 0, 1;");
	return ( !values1.isEmpty() || !values2.isEmpty() );
}

/**
 * Return database structure version.
 */
QString KurooDB::kurooDbVersion()
{
	return query( "SELECT version FROM dbInfo ;" ).first();
}

/**
 * Set current db structure version.
 * @param version
 */
void KurooDB::setKurooDbVersion( const QString& version )
{
	bool hasVersion = query("SELECT COUNT(version) FROM dbInfo LIMIT 0, 1;").first() == "0" ;
	
	if ( hasVersion )
		insert( "INSERT INTO dbInfo (version) VALUES ('" + version + "');" );
	else
		query( "UPDATE dbInfo SET version = '" + version + ";" );
}

/**
 * Create all necessary tables.
 */
void KurooDB::createTables( DbConnection *conn )
{
	query(" CREATE TABLE dbInfo ("
	      " version VARCHAR(32) )"
	      " ;", conn);
	
	query(" CREATE TABLE category ("
	      " id INTEGER PRIMARY KEY AUTOINCREMENT,"
	      " name VARCHAR(32)); "
	      " CREATE INDEX index_name ON category (name)"
	      " ;", conn);
	
	query(" CREATE TABLE subCategory ("
	      " id INTEGER PRIMARY KEY AUTOINCREMENT,"
	      " name VARCHAR(32), "
	      " idCategory INTEGER); "
	      " CREATE INDEX index_name ON subCategory (name)"
	      " ;", conn);
	
	query(" CREATE TABLE catSubCategory ("
	      " id INTEGER PRIMARY KEY AUTOINCREMENT,"
	      " name VARCHAR(32), "
	      " idCategory INTEGER, "
	      " idSubCategory INTEGER) "
	      " ;", conn);
	
	query(" CREATE TABLE package ("
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
	      " updateVersion VARCHAR(32)); "
	      " CREATE INDEX index_name ON package (name);"
	      " CREATE INDEX index_description ON package (description);"
	      , conn);
	
	query(" CREATE TABLE version ("
	      " id INTEGER PRIMARY KEY AUTOINCREMENT, "
	      " idPackage INTEGER, "
	      " name VARCHAR(32),"
	      " licenses VARCHAR(32), "
	      " useFlags VARCHAR(32),"
	      " slot VARCHAR(32),"
	      " size VARCHAR(32), "
	      " meta INTEGER, "
	      " branch VARCHAR(32)"
	      " );", conn);
	
	query(" CREATE TABLE updates ("
	      " id INTEGER PRIMARY KEY AUTOINCREMENT, "
	      " idPackage INTEGER UNIQUE, "
	      " installedVersion VARCHAR(32),"
	      " updateFlags VARCHAR(32),"
	      " useFlags VARCHAR(32)"
	      " );", conn);
	
	query(" CREATE TABLE queue ("
	      " id INTEGER PRIMARY KEY AUTOINCREMENT, "
	      " idPackage INTEGER, "
	      " idDepend INTEGER, "
	      " use VARCHAR(255), "
	      " size VARCHAR(32), "
	      " version VARCHAR(32) "
	      " );", conn);
	
	query(" CREATE TABLE history ("
	      " id INTEGER PRIMARY KEY AUTOINCREMENT, "
	      " package VARCHAR(32), "
	      " timestamp VARCHAR(10), "
	      " time INTEGER, "
	      " einfo BLOB, "
	      " emerge BOOL)"
	      " ;", conn);
	
	query(" CREATE TABLE mergeHistory ("
	      " id INTEGER PRIMARY KEY AUTOINCREMENT, "
	      " timestamp VARCHAR(10), "
	      " source VARCHAR(255), "
	      " destination VARCHAR(255))"
	      " ;", conn);
	
	query(" CREATE TABLE statistic ("
	      " id INTEGER UNIQUE, "
	      " package VARCHAR(32), "
	      " time INTEGER, "
	      " count INTEGER)"
	      " ;", conn);
	
	query(" CREATE TABLE cache ("
	      " id INTEGER PRIMARY KEY AUTOINCREMENT, "
	      " package VARCHAR(32), "
	      " size VARCHAR(10))"
	      " ;", conn);
	
	query(" CREATE TABLE packageHardMask ("
	      " id INTEGER PRIMARY KEY AUTOINCREMENT, "
	      " idPackage INTEGER, "
	      " dependAtom VARCHAR(255), "
	      " comment BLOB )"
	      " ;", conn);
	
	query(" CREATE TABLE packageUserMask ("
	      " id INTEGER PRIMARY KEY AUTOINCREMENT, "
	      " idPackage INTEGER UNIQUE, "
	      " dependAtom VARCHAR(255), "
	      " comment BLOB )"
	      " ;", conn);
	
	query(" CREATE TABLE packageUnmask ("
	      " id INTEGER PRIMARY KEY AUTOINCREMENT, "
	      " idPackage INTEGER UNIQUE, "
	      " dependAtom VARCHAR(255), "
	      " comment BLOB )"
	      " ;", conn);
	
	query(" CREATE TABLE packageKeywords ("
	      " id INTEGER PRIMARY KEY AUTOINCREMENT, "
	      " idPackage INTEGER UNIQUE, "
	      " keywords VARCHAR(255) )"
	      " ;", conn);
	
	query(" CREATE TABLE packageUse ("
	      " id INTEGER PRIMARY KEY AUTOINCREMENT, "
	      " idPackage INTEGER UNIQUE, "
	      " use VARCHAR(255) )"
	      " ;", conn);
}


//////////////////////////////////////////////////////////////////////////////
// Database management
//////////////////////////////////////////////////////////////////////////////

/**
 * Backup to file data which can not be recreated, fex history.einfo and mergeHistory.source/destination
 */
void KurooDB::backupDb()
{
	const QStringList historyData = query( "SELECT timestamp, einfo FROM history WHERE einfo NOTNULL; " );
	QFile file( KUROODIR + KurooConfig::fileHistoryBackup() );
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		foreach ( historyData ) {
			QString timestamp = *it++;
			QString einfo = *it;
			stream << timestamp << "\n" << einfo << "\n";
		}
		file.close();
	}
	else
		kdDebug() << i18n("Error writing: %1.").arg( KurooConfig::fileHistoryBackup() ) << endl;
	
	const QStringList mergeData = query( "SELECT timestamp, source, destination FROM mergeHistory; " );
	file.setName( KUROODIR + KurooConfig::fileMergeBackup() );
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		foreach ( mergeData ) {
			QString timestamp = *it++;
			QString source = *it++;
			QString destination = *it;
			stream << timestamp << "\n" << source << "\n" << destination << "\n";
		}
		file.close();
	}
	else
		kdDebug() << i18n("Error writing: %1.").arg( KurooConfig::fileMergeBackup() ) << endl;
}

/**
 * Restore data to tables history and mergeHistory
 */
void KurooDB::restoreBackup()
{
	// Restore einfo into table history
	QFile file( KUROODIR + KurooConfig::fileHistoryBackup() );
	QTextStream stream( &file );
	QStringList lines;
	if ( !file.open( IO_ReadOnly ) )
		kdDebug() << i18n("Error reading: %1.").arg( KurooConfig::fileHistoryBackup() ) << endl;
	else {
		while ( !stream.atEnd() )
			lines += stream.readLine();
		file.close();
	}
	
	for ( QStringList::Iterator it = lines.begin(), end = lines.end(); it != end; ++it ) {
		QString timestamp = *it++;
		QString einfo = *it;
		query( "UPDATE history SET einfo = '" + einfo + "' WHERE timestamp = '" + timestamp + "';" );
	}
	
	// Restore source and destination into table mergeHistory
	file.setName( KUROODIR + KurooConfig::fileMergeBackup() );
	stream.setDevice( &file );
	lines.clear();
	if ( !file.open( IO_ReadOnly ) )
		kdDebug() << i18n("Error reading: %1.").arg( KurooConfig::fileMergeBackup() ) << endl;
	else {
		while ( !stream.atEnd() )
			lines += stream.readLine();
		file.close();
	}
	
	for ( QStringList::Iterator it = lines.begin(), end = lines.end(); it != end; ++it ) {
		QString timestamp = *it++;
		QString source = *it++;
		QString destination = *it;
		query( "INSERT INTO mergeHistory (timestamp, source, destination) "
		       "VALUES ('" + timestamp + "', '" + source + "', '" + destination + "');" );
	}
}

//////////////////////////////////////////////////////////////////////////////
// Queries for portage 
//////////////////////////////////////////////////////////////////////////////

/**
 * Return all categories, eg app in app-portage/kuroo.
 * "0" is appended in front of the list so as not to miss the first category when categories are entered in reverse order in listview.
 */
QStringList KurooDB::allCategories()
{
	QStringList resultList( "0" );
	resultList += query( " SELECT name FROM category; " );
	return resultList;
}

/**
 * Return all subcategories, eg portage in app-portage/kuroo.
 */
QStringList KurooDB::allSubCategories()
{
	return query( " SELECT idCategory, id, name FROM subCategory ORDER BY name; " );
}

/**
 * Return all categories which have packages matching the filter and the text.
 * @param filter	packages status as FILTER_ALL, FILTER_INSTALLED or FILTER_UPDATES
 * @param text		search string
 */
QStringList KurooDB::portageCategories( int filter, const QString& text )
{
	QString filterQuery, textQuery;
	
	// Allow for multiple words match
	QString textString = escapeString( text.simplifyWhiteSpace() );
	textString = textString.replace( ' ', '%' );
	
	// Name or description choice from comboBox
	if ( !text.isEmpty() )
		textQuery = " AND (name LIKE '%" + textString + "%' OR description LIKE '%" + textString + "%') ";
	
	switch ( filter ) {
		case FILTER_ALL:
			filterQuery = QString::null;
			if ( !text.isEmpty() )
				textQuery = " WHERE (name LIKE '%" + textString + "%' OR description LIKE '%" + textString + "%')";
			break;
			
		case FILTER_INSTALLED:
			filterQuery = " WHERE package.meta != " + FILTER_ALL_STRING;
			break;
			
		case FILTER_UPDATES:
			filterQuery = " WHERE package.updateVersion != '' ";
	}
	
	return query( " SELECT DISTINCT idCategory FROM package "
	              + filterQuery + textQuery + " ; ");
}

/**
 * Return all subcategories which have packages matching the filter and the text in this category.
 * @param categoryId 	category id
 * @param filter		packages status as FILTER_ALL, FILTER_INSTALLED or FILTER_UPDATES
 * @param text			search string
 */
QStringList KurooDB::portageSubCategories( const QString& categoryId, int filter, const QString& text )
{
	QString filterQuery, textQuery;
	QStringList resultList( categoryId );
	
	// Allow for multiple words match
	QString textString = escapeString( text.simplifyWhiteSpace() );
	textString = textString.replace( ' ', '%' );
	
	// Name or description choice from comboBox
	if ( !text.isEmpty() )
		textQuery = " AND (name LIKE '%" + textString + "%' OR description LIKE '%" + textString + "%') ";

	if ( categoryId != "0" ) {

		switch ( filter ) {
			case FILTER_ALL:
				filterQuery = QString::null;
				break;
				
			case FILTER_INSTALLED:
				filterQuery = " AND package.meta != " + FILTER_ALL_STRING;
				break;
				
			case FILTER_UPDATES:
				filterQuery = " AND package.updateVersion != '' ";
		}
		
		resultList += query( " SELECT DISTINCT idSubCategory FROM package WHERE idCategory = '"
		               		+ categoryId + "'" + filterQuery + textQuery + " ; " );
	}
	
	// Add meta-subcategory when query is successful
	if ( resultList.size() > 1 )
		resultList += "0";
	
	return resultList;
}

/**
 * Return all packages which are matching the filter and the text in this category-subcategory.
 * @param categoryId 	category id
 * @param subCategoryId subcategory id
 * @param filter		packages status as FILTER_ALL, FILTER_INSTALLED or FILTER_UPDATES
 * @param text			search string
 */
QStringList KurooDB::portagePackagesBySubCategory( const QString& categoryId, const QString& subCategoryId, int filter, const QString& text )
{
	QString filterQuery, textQuery;
	
	// Allow for multiple words match
	QString textString = escapeString( text.simplifyWhiteSpace() );
	textString = textString.replace( ' ', '%' );
	
	// Name or description choice from comboBox
	if ( !text.isEmpty() )
		textQuery = " AND (name LIKE '%" + textString + "%' OR description LIKE '%" + textString + "%') ";
	
	if ( categoryId == "0" ) {
		
		if ( subCategoryId == "0" ) {
			switch ( filter ) {
				case FILTER_ALL:
					filterQuery = QString::null;
					if ( !text.isEmpty() )
						textQuery = " WHERE (name LIKE '%" + textString + "%' OR description LIKE '%" + textString + "%') ";
					break;
					
				case FILTER_INSTALLED:
					filterQuery = " WHERE meta != " + FILTER_ALL_STRING;
					break;
					
				case FILTER_UPDATES:
					filterQuery = " WHERE updateVersion != '' ";
			}
			
			return query( " SELECT id, name, description, meta, updateVersion, homepage "
						  " FROM package "
			              + filterQuery + textQuery + " ORDER BY name DESC;");
		}
		else {
			
			switch ( filter ) {
				case FILTER_ALL:
					filterQuery = QString::null;
					break;
					
				case FILTER_INSTALLED:
					filterQuery = " AND meta != " + FILTER_ALL_STRING;
					break;
					
				case FILTER_UPDATES:
					filterQuery = " AND updateVersion != '' ";
			}
			
			return query( " SELECT id, name, description, meta, updateVersion, homepage "
			              " FROM package "
			              " WHERE idSubCategory = '" + subCategoryId + "'"
			              + filterQuery + textQuery + " ORDER BY name DESC;");
		}
	}
	else {
		if ( subCategoryId == "0" ) {
			
			switch ( filter ) {
				case FILTER_ALL:
					filterQuery = QString::null;
					break;
					
				case FILTER_INSTALLED:
					filterQuery = " AND meta != " + FILTER_ALL_STRING;
					break;
					
				case FILTER_UPDATES:
					filterQuery = " AND updateVersion != '' ";
			}
			
			return query( " SELECT id, name, description, meta, updateVersion, homepage "
						  " FROM package "
						  " WHERE idCategory = '" + categoryId + "'"
			              + filterQuery + textQuery + " ORDER BY name DESC;");
		}
		else {
		
			switch ( filter ) {
				case FILTER_ALL:
					filterQuery = QString::null;
					break;
					
				case FILTER_INSTALLED:
					filterQuery = " AND meta != " + FILTER_ALL_STRING;
					break;
					
				case FILTER_UPDATES:
					filterQuery = " AND updateVersion != '' ";
			}
			
			return query( " SELECT id, name, description, meta, updateVersion, homepage "
					      " FROM package "
						  " WHERE idCategory = '" + categoryId + "'"
						  " AND idSubCategory = '" + subCategoryId + "'"
			              + filterQuery + textQuery + " ORDER BY name DESC;");
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// Query for packages
//////////////////////////////////////////////////////////////////////////////

/**
 * Return package name, eg kuroo in app-portage/kuroo.
 * @param id
 */
QString KurooDB::package( const QString& id )
{
	QString name = (query( "SELECT name FROM package WHERE id = '" + id + "';" ) ).first();
	
	if ( !name.isEmpty() )
		return name;
	else
		kdDebug() << i18n("Can not find package in database for id %1.").arg( id ) << endl;
	
	return QString::null;
}

/**
 * Return category name, eg app-portage in app-portage/kuroo.
 * @param id
 */
QString KurooDB::category( const QString& id )
{
	QString category = query( " SELECT catSubCategory.name FROM package, catSubCategory "
	                          " WHERE package.id = '" + id + "' "
	                          " AND catSubCategory.id = package.idCatSubCategory ;" ).first();
	
	if ( !category.isEmpty() )
		return category;
	else
		kdDebug() << i18n("Can not find category in database for id %1.").arg( id ) << endl;
	
	return QString::null;
}

/**
 * Return package when searching by category-subcategory and name.
 * @param category 		category-subcategory
 * @param name
 */
QString KurooDB::packageId( const QString& package )
{
	QString category = package.section( "/", 0, 0 );
	QString name = package.section( "/", 1, 1 ).section( rxPortageVersion, 0, 0 );
	
	kdDebug() << "KurooDB::packageId package=" << package << ". category=" << category << ". name=" << name << "." << endl;
	
	QString id = query( " SELECT package.id FROM package, catSubCategory WHERE "
	                    " package.name = '" + name + "' AND catSubCategory.name = '" + category + "' "
	                    " AND catSubCategory.id = package.idCatSubCategory; ").first();
	
	if ( !id.isEmpty() )
		return id;
	else
		kdDebug() << i18n("packageId: Can not find id in database for package %1/%2.").arg( category ).arg( name ) << endl;
	
	return QString::null;
}

/**
 * Return all versions and their info for this package.
 * @param id
 */
QStringList KurooDB::packageVersionsInfo( const QString& id )
{
	return query( " SELECT name, meta, licenses, useFlags, slot, branch, size "
	              " FROM version "
	              " WHERE idPackage = '" + id + "'"
	              " ORDER BY version.name;");
}

QString KurooDB::versionSize( const QString& idPackage, const QString& version )
{
	return query( " SELECT size FROM version "
	              " WHERE idPackage = '" + idPackage + "'"
	              " AND name = '" + version + "'"
	              " ;").first();
}

QStringList KurooDB::packageHardMaskInfo( const QString& id )
{
	return query( "SELECT dependAtom, comment FROM packageHardMask WHERE idPackage = '" + id + "' LIMIT 1;" );
}

/**
 * Return package hardmask depend atom.
 * @param id
 */
QString KurooDB::packageHardMaskAtom( const QString& id )
{
	return query( "SELECT dependAtom FROM packageHardMask WHERE idPackage = '" + id + "';" ).first();
}

/**
 * Return package user-mask depend atom.
 * @param id
 */
QString KurooDB::packageUserMaskAtom( const QString& id )
{
	return query( "SELECT dependAtom FROM packageUserMask WHERE idPackage = '" + id + "';" ).first();
}

/**
 * Return package unmask depend atom.
 * @param id
 */
QString KurooDB::packageUnMaskAtom( const QString& id )
{
	return query( "SELECT dependAtom FROM packageUnmask WHERE idPackage = '" + id + "';" ).first();
}

/**
 * Return package keyword atom.
 * @param id
 */
QString KurooDB::packageKeywordsAtom( const QString& id )
{
	return query( "SELECT keywords FROM packageKeywords WHERE idPackage = '" + id + "';" ).first();
}

QString KurooDB::packageUse( const QString& id )
{
	return query( "SELECT use FROM packageUse where idPackage = '" + id + "';" ).first();
}

/**
 * Is the package in package.keywords?
 * @param id
 */
bool KurooDB::isPackageUnTesting( const QString& id )
{
	QString keywords = query( "SELECT keywords FROM packageKeywords where idPackage = '" + id + "';" ).first();
	if ( keywords.contains( QRegExp("(~\\*)|(~" + KurooConfig::arch() + ")") ) )
		return true;
	else
		return false;
}

/**
 * Is the package available in package.keywords?
 * @param id
 */
bool KurooDB::isPackageAvailable( const QString& id )
{
	QString keywords = query( "SELECT keywords FROM packageKeywords where idPackage = '" + id + "';" ).first();
	if ( keywords.contains( QRegExp("(\\-\\*)|(\\-" + KurooConfig::arch() + ")") ) )
		return true;
	else
		return false;
}

/**
 * Is the package in package.unmask?
 * @param id
 */
bool KurooDB::isPackageUnMasked( const QString& id )
{
	return !query( "SELECT id FROM packageUnmask where idPackage = '" + id + "';" ).isEmpty();
}

/**
 * Add use flags for this package.
 * @param id
 */
void KurooDB::setPackageUse( const QString& id, const QString& useFlags )
{
	query( "DELETE FROM packageUse WHERE idPackage = '" + id + "'" );
	insert( "INSERT INTO packageUse (idPackage, use) VALUES ('" + id + "', '" + useFlags + "');" );
}

/**
 * Add package in package.unmask. @fixme: check category and package?
 * @param id
 */
void KurooDB::setPackageUnMasked( const QString& id )
{
	if ( !isPackageUnMasked( id ) )
		insert( "INSERT INTO packageUnmask (idPackage, dependAtom) VALUES ('" + id + "', "
	       		"'" + category( id ) + "/" + package( id ) + "');" );
}

/**
 * Add package in package.mask. @fixme: check category and package?
 * @param id
 */
void KurooDB::setPackageUserMasked( const QString& id, const QString& version )
{
	clearPackageUserMasked( id );
	insert( "INSERT INTO packageUserMask (idPackage, dependAtom) VALUES ('" + id + "', "
	        "'>" + category( id ) + "/" + package( id ) + "-" + version + "');" );
}

/**
 * Set package as testing, eg add keyword ~*.
 * @param id
 */
void KurooDB::setPackageUnTesting( const QString& id )
{
	QString keywords = packageKeywordsAtom( id );
	
	// Aready testing skip!
	if ( keywords.contains( QRegExp("(~\\*)|(~" + KurooConfig::arch() + ")") ) )
		return;
	
	if ( keywords.isEmpty() )
		insert( "INSERT INTO packageKeywords (idPackage, keywords) VALUES ('" + id + "', '~*');" );
	else
		query( " UPDATE packageKeywords SET keywords = '" + keywords + " ~*' WHERE idPackage = '" + id + "';" );
}

/**
 * Set package as available, eg add keywords '-* -arch'
 */
void KurooDB::setPackageAvailable( const QString& id )
{
	QString keywords = packageKeywordsAtom( id );
	
	// Already available skip!
	if ( keywords.contains( QRegExp("(\\-\\*)|(\\-" + KurooConfig::arch() + ")") ) )
		return;
	
	if ( keywords.isEmpty() )
		insert( "INSERT INTO packageKeywords (idPackage, keywords) VALUES ('" + id + "', '-* -" + KurooConfig::arch() + "');" );
	else
		query( " UPDATE packageKeywords SET keywords = '" + keywords + " -* -" + KurooConfig::arch() + "' WHERE idPackage = '" + id + "';" );
}

/**
 * Clear testing keyword from package.
 * @param id
 */
void KurooDB::clearPackageUnTesting( const QString& id )
{
	QString keywords = packageKeywordsAtom( id );
	
	// If only testing keywords - remove it, else set only available keywords
	if ( !keywords.contains( QRegExp("(\\-\\*)|(\\-" + KurooConfig::arch() + ")") ) )
		query( " DELETE FROM packageKeywords WHERE idPackage = '" + id + "';" );
	else
		query( " UPDATE packageKeywords SET keywords = '-* -" + KurooConfig::arch() + "' WHERE idPackage = '" + id + "';" );
}

/**
 * Removing available keywords for package.
 * @param id
 */
void KurooDB::clearPackageAvailable( const QString& id )
{
	QString keywords = packageKeywordsAtom( id );
	
	// If only available keywords - remove it, else set only testing keyword
	if ( !keywords.contains( QRegExp("(~\\*)|(~" + KurooConfig::arch() + ")") ) )
		query( " DELETE FROM packageKeywords WHERE idPackage = '" + id + "';" );
	else
		query( " UPDATE packageKeywords SET keywords = '~*' WHERE idPackage = '" + id + "';" );;
}

/**
 * Clear package from package.unmask.
 * @param id
 */
void KurooDB::clearPackageUnMasked( const QString& id )
{
	query( "DELETE FROM packageUnmask WHERE idPackage = '" + id + "';" );
}

/**
 * Clear package from package.mask.
 * @param id
 */
void KurooDB::clearPackageUserMasked( const QString& id )
{
	query( "DELETE FROM packageUserMask WHERE idPackage = '" + id + "';" );
}

//////////////////////////////////////////////////////////////////////////////
// Query for installation queue
//////////////////////////////////////////////////////////////////////////////

/**
 * Return all packages in the queue.
 */
QStringList KurooDB::allQueuePackages()
{
	return query( " SELECT package.id, catSubCategory.name, package.name, "
	              " package.description, package.meta, queue.idDepend, queue.use, queue.size, queue.version "
	              " FROM queue, catSubCategory, package "
	              " WHERE queue.idPackage = package.id "
	              " AND catSubCategory.id = package.idCatSubCategory "
	              " ORDER BY queue.idDepend;" );
}

/**
 * Return all packages in the queue.
 */
QStringList KurooDB::allQueueId()
{
	return query( " SELECT idPackage FROM queue;" );
}

/**
 * Return all package cache info.
 */
QStringList KurooDB::allCache()
{
	return query(" SELECT package, size "
	             " FROM cache "
	             " ;");
}

/**
 * Return all history.
 */
QStringList KurooDB::allHistory()
{
	return query( " SELECT timestamp, package, time, einfo "
	              " FROM history "
	              " ORDER BY id ASC;");
}

/**
 * Return all etc-update history.
 */
QStringList KurooDB::allMergeHistory()
{
	return query( " SELECT timestamp, source, destination "
	              " FROM mergeHistory "
	              " ORDER BY id ASC;");
}

/**
 * Return all package statistics.
 */
QStringList KurooDB::allStatistic()
{
	return query( " SELECT package, time, count FROM statistic "
	              " ORDER BY id ASC;");
}

/**
 * Return the total number of packages.
 */
QString KurooDB::packageTotal()
{
	return query( " SELECT COUNT(id) FROM package LIMIT 1;" ).first();
}

/**
 * Return the total number of updates.
 */
QString KurooDB::updatesTotal()
{
	return query( "SELECT COUNT(id) FROM updates LIMIT 1;" ).first();
}

/**
 * Return the total number of packages in queue.
 */
QString KurooDB::queueTotal()
{
	return query( "SELECT COUNT(id) FROM queue LIMIT 1;" ).first();
}

/**
 * Clear all updates.
 */
void KurooDB::resetUpdates()
{
	query( "UPDATE package SET updateVersion = '' WHERE updateVersion != '';" );
	query( "DELETE FROM updates;" );
}

void KurooDB::resetInstalled()
{
	query( "UPDATE package set installed = '" + FILTER_ALL_STRING + "';" );
}

/**
 * Return timestamp last entry in history.
 */
QStringList KurooDB::lastHistoryEntry()
{
	return query(" SELECT timestamp "
	             " FROM history "
	             " WHERE id = (SELECT MAX(id) FROM history);");
}

/**
 * Return timestamp for last sync.
 */
QStringList KurooDB::getLastSync()
{
	return query(" SELECT timestamp "
	             " FROM history "
	             " WHERE id = (SELECT MAX(id) FROM history where package = '');");
}

void KurooDB::addEmergeInfo( const QString& einfo )
{
	query( QString("UPDATE history SET einfo = '%1' "
	               "WHERE id = (SELECT MAX(id) FROM history);").arg( einfo ) );
}

/**
 * Insert timestamp for portage view refresh.
 */
void KurooDB::addRefreshTime()
{
	QDateTime currentTime( QDateTime::currentDateTime() );
	insert( QString( "INSERT INTO history ( package, timestamp, emerge ) "
	                 " VALUES ('', '%1', 'false');").arg( QString::number( currentTime.toTime_t() ) ) );
}

/**
 * Insert etc-update file names.
 * @param source
 * @param destination
 */
void KurooDB::addBackup( const QString& source, const QString& destination )
{
	QDateTime currentTime( QDateTime::currentDateTime() );
	insert( QString( "INSERT INTO mergeHistory ( timestamp, source, destination ) "
	                 " VALUES ('%1', '%2', '%3');").arg( QString::number( currentTime.toTime_t() ) ).arg( source ).arg( destination ) );
}

/**
 * Is the cache empty?
 */
bool KurooDB::isCacheEmpty()
{
	QStringList values = query("SELECT COUNT(id) FROM cache LIMIT 0, 1;");
	return values.isEmpty() ? true : values.first() == "0";
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////////////

/**
 */
DbConnection::DbConnection( DbConfig* config )
 	: m_config(config)
{}

DbConnection::~DbConnection()
{}

/** 
 * Sqlite methods
 */
SqliteConnection::SqliteConnection( SqliteConfig* config )
	: DbConnection(config)
{
	const QCString path = QString( KUROODIR + KurooConfig::databas() ).local8Bit();
	
    // Open database file and check for correctness
	m_initialized = false;
	QFile file(path);
	if ( file.open(IO_ReadOnly) ) {
		QString format;
		file.readLine(format, 50);
		
		if ( !format.startsWith("SQLite format 3") ) {
			kdDebug() << "Database versions incompatible. Removing and rebuilding database.\n";
		}
		else
			if ( sqlite3_open(path, &m_db) != SQLITE_OK ) {
				kdDebug() << "Database file corrupt. Removing and rebuilding database.\n";
				sqlite3_close(m_db);
			}
			else
				m_initialized = true;
	}
	
	if ( !m_initialized ) {
		
        // Remove old db file; create new
		QFile::remove(path);
		if ( sqlite3_open(path, &m_db) == SQLITE_OK ) {
			m_initialized = true;
			
		}
	}
	if ( m_initialized ) {
		if ( sqlite3_create_function(m_db, "rand", 0, SQLITE_UTF8, NULL, sqlite_rand, NULL, NULL) != SQLITE_OK )
			m_initialized = false;
		if ( sqlite3_create_function(m_db, "power", 2, SQLITE_UTF8, NULL, sqlite_power, NULL, NULL) != SQLITE_OK )
			m_initialized = false;
	}
	
    //optimization for speeding up SQLite
	query("PRAGMA default_synchronous = OFF;");
}

SqliteConnection::~SqliteConnection()
{
	if (m_db) sqlite3_close(m_db);
}

QStringList SqliteConnection::query( const QString& statement )
{
	QStringList values;
	int error;
	const char* tail;
	sqlite3_stmt* stmt;
	
    //compile SQL program to virtual machine
	error = sqlite3_prepare( m_db, statement.utf8(), statement.length(), &stmt, &tail );
	
	if ( error != SQLITE_OK ) {
		kdDebug() << k_funcinfo << " sqlite3_compile error:" << endl;
		kdDebug() << sqlite3_errmsg(m_db) << endl;
		kdDebug() << "on query: " << statement << endl;
		values = QStringList();
	}
	else {
		int busyCnt = 0;
		int number = sqlite3_column_count(stmt);
		
        //execute virtual machine by iterating over rows
		while ( true ) {
			error = sqlite3_step(stmt);
			
			if ( error == SQLITE_BUSY ) {
				if ( busyCnt++ > 99 ) {
					kdDebug() << "Busy-counter has reached maximum. Aborting this sql statement!\n";
					break;
				}
				::usleep(100000); // Sleep 100 msec
				kdDebug() << "sqlite3_step: BUSY counter: " << busyCnt << " on query: " << statement << endl;
			}
			if ( error == SQLITE_MISUSE )
				kdDebug() << "sqlite3_step: MISUSE on query: " << statement << endl;
			if ( error == SQLITE_DONE || error == SQLITE_ERROR )
				break;
			
            //iterate over columns
			for ( int i = 0; i < number; i++ ) {
				values << QString::fromUtf8( (const char*) sqlite3_column_text(stmt, i) );
			}
		}
        //deallocate vm ressources
		sqlite3_finalize(stmt);
		
		if ( error != SQLITE_DONE ) {
			kdDebug() << k_funcinfo << "sqlite_step error.\n";
			kdDebug() << sqlite3_errmsg(m_db) << endl;
			kdDebug() << "on query: " << statement << endl;
			values = QStringList();
		}
	}
	
	return values;
}

int SqliteConnection::insert( const QString& statement )
{
	int error;
	const char* tail;
	sqlite3_stmt* stmt;
	
    //compile SQL program to virtual machine
	error = sqlite3_prepare( m_db, statement.utf8(), statement.length(), &stmt, &tail );
	
	if ( error != SQLITE_OK ) {
		kdDebug() << k_funcinfo << " sqlite3_compile error:" << endl;
		kdDebug() << sqlite3_errmsg(m_db) << endl;
		kdDebug() << "on insert: " << statement << endl;
	}
	else {
		int busyCnt = 0;
		
        //execute virtual machine by iterating over rows
		while (true) {
			error = sqlite3_step(stmt);
			
			if ( error == SQLITE_BUSY ) {
				if ( busyCnt++ > 99 ) {
					kdDebug() << "Busy-counter has reached maximum. Aborting this sql statement!\n";
					break;
				}
				::usleep(100000); // Sleep 100 msec
				kdDebug() << "sqlite3_step: BUSY counter: " << busyCnt << " on insert: " << statement << endl;
			}
			
			if ( error == SQLITE_MISUSE )
				kdDebug() << "sqlite3_step: MISUSE on insert: " << statement << endl;
			
			if ( error == SQLITE_DONE || error == SQLITE_ERROR )
				break;
		}
        //deallocate vm ressources
		sqlite3_finalize(stmt);
		
		if ( error != SQLITE_DONE ) {
			kdDebug() << k_funcinfo << "sqlite_step error.\n";
			kdDebug() << sqlite3_errmsg(m_db) << endl;
			kdDebug() << "on insert: " << statement << endl;
			return 0;
		}
	}
	return sqlite3_last_insert_rowid(m_db);
}

// this implements a RAND() function compatible with the MySQL RAND() (0-param-form without seed)
void SqliteConnection::sqlite_rand( sqlite3_context *context, int /*argc*/, sqlite3_value ** /*argv*/ )
{
	sqlite3_result_double(context, static_cast<double>(KApplication::random()) / (RAND_MAX+1.0));
}

// this implements a POWER() function compatible with the MySQL POWER()
void SqliteConnection::sqlite_power( sqlite3_context *context, int argc, sqlite3_value **argv )
{
	Q_ASSERT(argc==2);
	if ( sqlite3_value_type(argv[0])==SQLITE_NULL || sqlite3_value_type(argv[1])==SQLITE_NULL ) {
		sqlite3_result_null(context);
		return;
	}
	double a = sqlite3_value_double(argv[0]);
	double b = sqlite3_value_double(argv[1]);
	sqlite3_result_double(context, pow(a,b));
}

SqliteConfig::SqliteConfig( const QString& dbfile )
	: m_dbfile(dbfile)
{
}

/** 
 * Connections pool with thread support
 */
DbConnectionPool::DbConnectionPool() : m_semaphore(POOL_SIZE)
{
// 	m_dbConnType = DbConnection::sqlite;
	
	m_semaphore += POOL_SIZE;
	DbConnection *dbConn;
	m_dbConfig = new SqliteConfig( KurooConfig::databas() );
	dbConn = new SqliteConnection( static_cast<SqliteConfig*> (m_dbConfig) );

	enqueue(dbConn);
	m_semaphore--;
	kdDebug() << "Available db connections: " << m_semaphore.available() << endl;
}

DbConnectionPool::~DbConnectionPool()
{
	m_semaphore += POOL_SIZE;
	DbConnection *conn;
	bool vacuum = true;
	
	while ( (conn = dequeue()) != 0 ) {
		if (/*m_dbConnType == DbConnection::sqlite && */vacuum) {
			vacuum = false;
			kdDebug() << "Running VACUUM" << endl;
			conn->query("VACUUM; ");
		}
		delete conn;
	}
	delete m_dbConfig;
}

void DbConnectionPool::createDbConnections()
{
	for ( int i = 0; i < POOL_SIZE - 1; i++ ) {
		DbConnection *dbConn;
		dbConn = new SqliteConnection(static_cast<SqliteConfig*> (m_dbConfig));
		enqueue(dbConn);
		m_semaphore--;
	}
	kdDebug() << "Create. Available db connections: " << m_semaphore.available() << endl;
}

DbConnection *DbConnectionPool::getDbConnection()
{
	m_semaphore++;
	return dequeue();
}

void DbConnectionPool::putDbConnection( const DbConnection *conn )
{
	enqueue(conn);
	m_semaphore--;
}

#include "portagedb.moc"
