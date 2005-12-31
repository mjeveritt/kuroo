/***************************************************************************
*   Copyright (C) 2004 by karye                                           *
*   karye@users.sourceforge.net                                           *
* (c) 2004 Mark Kretschmann <markey@web.de>
* (c) 2004 Christian Muehlhaeuser <chris@chris.de>
* (c) 2004 Sami Nieminen <sami.nieminen@iki.fi>
* (c) 2005 Ian Monroe <ian@monroe.nu>
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

#include "portagedb.h"
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
 * All Sqlite related as connections, queries...
 */
KurooDB::KurooDB( QObject *parent )
	: QObject( parent )
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
QString KurooDB::init( QObject *myParent )
{
	parent = myParent;
	
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
	
	QStringList values = dbConn->query(statement);
	
	if ( conn == NULL )
		m_dbConnPool->putDbConnection(dbConn);
	
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
	if (conn != NULL)
		dbConn = conn;
	else
		dbConn = m_dbConnPool->getDbConnection();
	
	int id = dbConn->insert(statement);
	
	if (conn == NULL)
		m_dbConnPool->putDbConnection(dbConn);
	
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
	return (!values1.isEmpty() || !values2.isEmpty());
}

void KurooDB::createTables( DbConnection *conn )
{
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
	      " CREATE INDEX index_name ON package (name)"
	      " ;", conn);
	
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
	      " idPackage INTEGER UNIQUE) "
	      " ;", conn);
	
	query(" CREATE TABLE results ("
	      " id INTEGER PRIMARY KEY AUTOINCREMENT, "
	      " idPackage INTEGER UNIQUE, "
	      " package VARCHAR(64), "
	      " size VARCHAR(32), "
	      " use VARCHAR(255), "
	      " flags VARCHAR(32))"
	      " ;", conn);
	
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
}


//////////////////////////////////////////////////////////////////////////////
// Queries for all content in tables
//////////////////////////////////////////////////////////////////////////////

// QStringList KurooDB::isInstalled( const QString& name, const QString& version )
// {
// 	return query( " SELECT installed "
// 	              " FROM package "
// 	              " WHERE name = '" + name + "'"
// 	              " AND version = '" + version + "'"
// 	              " LIMIT 1;");
// }

QStringList KurooDB::history()
{
	return query( " SELECT timestamp, package, time, einfo "
	              " FROM history "
	              " ORDER BY id ASC;");
}

QStringList KurooDB::mergeHistory()
{
	return query( " SELECT timestamp, source, destination "
	              " FROM mergeHistory "
	              " ORDER BY id ASC;");
}

QStringList KurooDB::statistic()
{
	return query( " SELECT package, time, count FROM statistic "
	              " ORDER BY id ASC;");
}

// QStringList KurooDB::packageKeywords( const QString& idCategory, const QString& name )
// {
// 	return query( " SELECT keywords FROM package "
// 	              " WHERE idCategory = '" + idCategory + "'"
// 	              " AND name = '" + name + "'"
// 	              " ORDER BY id ASC;");
// }

QStringList KurooDB::packageTotal()
{
	return query( " SELECT COUNT(id) FROM package LIMIT 1;" );
}

// QStringList KurooDB::installedTotal()
// {
// 	return query( QString( "SELECT COUNT(id) FROM package WHERE meta != '%1' LIMIT 1;" ).arg( FILTERINSTALLED ) );
// }

QStringList KurooDB::updatesTotal()
{
	return query( "SELECT COUNT(id) FROM updates LIMIT 1;" );
}

QString KurooDB::package( const QString& id )
{
	QString name = (query( QString( "SELECT name FROM package WHERE id = '%1';" ).arg( id ) )).first();
	
	if ( !name.isEmpty() )
		return name;
	else
		kdDebug() << i18n("Can not find package in database for id %1.").arg( id ) << endl;
	
	return NULL;
}

QString KurooDB::category( const QString& id )
{
	QString category = (query( QString( "SELECT name FROM catSubCategory "
	                                    "WHERE id = ( SELECT idCatSubCategory FROM package WHERE id = '%1' );" ).arg( id ) )).first();
	
	if ( !category.isEmpty() )
		return category;
	else
		kdDebug() << i18n("Can not find category in database for id %1.").arg( id ) << endl;
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////
// Queries for portage 
//////////////////////////////////////////////////////////////////////////////

QStringList KurooDB::allCategories()
{
	QStringList resultList( "0" );
	resultList += query( " SELECT name FROM category; " );
	return resultList;
}

QStringList KurooDB::allSubCategories()
{
	return query( " SELECT idCategory, id, name FROM subCategory ORDER BY name; " );
}

QStringList KurooDB::portageCategories( int filter, const QString& text )
{
	QString filterQuery, textQuery;
	
	// Name or description choice from comboBox
	if ( !text.isEmpty() )
		textQuery = " AND (name LIKE '%" + escapeString( text ) + "%' OR description LIKE '%" + escapeString( text ) + "%') ";
	
	switch ( filter ) {
		case FILTER_ALL:
			filterQuery = "";
			if ( !text.isEmpty() )
				textQuery = " WHERE (name LIKE '%" + escapeString( text ) + "%' OR description LIKE '%" + escapeString( text ) + "%')";
			break;
			
		case FILTER_INSTALLED:
			filterQuery = " WHERE package.meta != " + FILTERALL;
			break;
			
		case FILTER_UPDATES:
			filterQuery = " WHERE package.updateVersion != '' ";
	}
	
	return query( " SELECT DISTINCT idCategory FROM package "
	              + filterQuery + textQuery + " ; ");
}

QStringList KurooDB::portageSubCategories( const QString& categoryId, int filter, const QString& text )
{
	QString filterQuery, textQuery;
	QStringList resultList( categoryId );
	
	// Name or description choice from comboBox
	if ( !text.isEmpty() )
		textQuery = " AND (name LIKE '%" + escapeString( text ) + "%' OR description LIKE '%" + escapeString( text ) + "%') ";

	if ( categoryId != "0" ) {

		switch ( filter ) {
			case FILTER_ALL:
				filterQuery = "";
				break;
				
			case FILTER_INSTALLED:
				filterQuery = " AND package.meta != " + FILTERALL;
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

QStringList KurooDB::portagePackagesBySubCategory( const QString& categoryId, const QString& subCategoryId, int filter, const QString& text )
{
	QString filterQuery, textQuery;
	
	// Name or description choice from comboBox
	if ( !text.isEmpty() )
		textQuery = " AND (name LIKE '%" + escapeString( text ) + "%' OR description LIKE '%" + escapeString( text ) + "%') ";
	
	if ( categoryId == "0" ) {
		
		if ( subCategoryId == "0" ) {
			switch ( filter ) {
				case FILTER_ALL:
					filterQuery = "";
					if ( !text.isEmpty() )
						textQuery = " WHERE (name LIKE '%" + escapeString( text ) + "%' OR description LIKE '%" + escapeString( text ) + "%') ";
					break;
					
				case FILTER_INSTALLED:
					filterQuery = " WHERE meta != " + FILTERALL;
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
					filterQuery = "";
					break;
					
				case FILTER_INSTALLED:
					filterQuery = " AND meta != " + FILTERALL;
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
					filterQuery = "";
					break;
					
				case FILTER_INSTALLED:
					filterQuery = " AND meta != " + FILTERALL;
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
					filterQuery = "";
					break;
					
				case FILTER_INSTALLED:
					filterQuery = " AND meta != " + FILTERALL;
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

QStringList KurooDB::portagePackageInfo( const QString& id )
{
	return query(" SELECT description, homepage "
	             " FROM package "
	             " WHERE id = '" + id + "'"
	             " ORDER BY lower(name) LIMIT 1;");
}


//////////////////////////////////////////////////////////////////////////////
// Query for packages
//////////////////////////////////////////////////////////////////////////////

QStringList KurooDB::packageVersions( const QString& id )
{
	return query( " SELECT name, meta, branch "
	              " FROM version "
	              " WHERE idPackage = '" + id + "'"
	              " ORDER BY version.name;");
}

QStringList KurooDB::packageVersionsInfo( const QString& id )
{
	return query( " SELECT name, meta, licenses, useFlags, slot, branch, size "
	              " FROM version "
	              " WHERE idPackage = '" + id + "'"
	              " ORDER BY version.name;");
}


//////////////////////////////////////////////////////////////////////////////
// Query for installation queue
//////////////////////////////////////////////////////////////////////////////

QStringList KurooDB::queuePackages()
{
	return query( QString( " SELECT package.id, category.name, subCategory.name, package.name, "
	                       " package.description, package.meta "
	                       " FROM queue, category, subCategory, package "
	                       " WHERE queue.idPackage = package.id "
	                       " AND category.id = package.idCategory "
	                       " AND subCategory.id = package.idSubCategory "
	                       " ORDER BY queue.id DESC LIMIT %1;" ).arg(ROWLIMIT) );
}


//////////////////////////////////////////////////////////////////////////////
// Query for results
//////////////////////////////////////////////////////////////////////////////

QStringList KurooDB::resultPackages()
{
	return query( QString(" SELECT package.id, results.package, package.description,"
	                      " results.size, results.use, results.flags, package.meta "
	                      " FROM results, catSubCategory, package "
	                      " WHERE results.idPackage = package.id "
	                      " AND catSubCategory.id = package.idCatSubCategory "
	                      " ORDER BY results.id LIMIT %1;").arg(ROWLIMIT) );
}

QString KurooDB::packageIdDB( const QString& category, const QString& name )
{
	QString id = (query( " SELECT package.id FROM catSubCategory, package "
	                    " WHERE catSubCategory.name = '" + category + "'"
	                    " AND package.name = '" + name + "'"
	                    " AND catSubCategory.id = package.idCatSubCategory "
	                     " LIMIT 1;")).first();
	
	if ( !id.isEmpty() )
		return id;
	else
		kdDebug() << i18n("Can not find id in database for package %1/%2.").arg( category ).arg( name ) << endl;
	
	return NULL;
}

QStringList KurooDB::lastHistoryEntry()
{
	return query(" SELECT timestamp "
	             " FROM history "
	             " WHERE id = (SELECT MAX(id) FROM history);");
}

QStringList KurooDB::getLastSync()
{
	return query(" SELECT timestamp "
	             " FROM history "
	             " WHERE id = (SELECT MAX(id) FROM history where package = '');");
}

void KurooDB::addRefreshTime()
{
	QDateTime currentTime( QDateTime::currentDateTime() );
	insert( QString( "INSERT INTO history ( package, timestamp, emerge ) "
	                 " VALUES ('', '%1', 'false');").arg( QString::number( currentTime.toTime_t() ) ) );
}

void KurooDB::addBackup( const QString& source, const QString& destination )
{
	QDateTime currentTime( QDateTime::currentDateTime() );
	insert( QString( "INSERT INTO mergeHistory ( timestamp, source, destination ) "
	                 " VALUES ('%1', '%2', '%3');").arg( QString::number( currentTime.toTime_t() ) ).arg( source ).arg( destination ) );
}

QStringList KurooDB::cache()
{
	return query(" SELECT package, size "
	             " FROM cache "
	             " ;");
}

bool KurooDB::isCacheEmpty()
{
	QStringList values = query("SELECT COUNT(id) FROM cache LIMIT 0, 1;");
	return values.isEmpty() ? true : values.first() == "0";
}

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
		while (true) {
			error = sqlite3_step(stmt);
			
			if ( error == SQLITE_BUSY ) {
				if (busyCnt++ > 99) {
					kdDebug() << "Busy-counter has reached maximum. Aborting this sql statement!\n";
					break;
				}
				::usleep(100000); // Sleep 100 msec
				kdDebug() << "sqlite3_step: BUSY counter: " << busyCnt << endl;
			}
			if ( error == SQLITE_MISUSE )
				kdDebug() << "sqlite3_step: MISUSE" << endl;
			if ( error == SQLITE_DONE || error == SQLITE_ERROR )
				break;
			
            //iterate over columns
			for (int i = 0; i < number; i++) {
				values << QString::fromUtf8((const char*) sqlite3_column_text(stmt, i));
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
		while (true)
		{
			error = sqlite3_step(stmt);
			
			if ( error == SQLITE_BUSY ) {
				if ( busyCnt++ > 99 ) {
					kdDebug() << "Busy-counter has reached maximum. Aborting this sql statement!\n";
					break;
				}
				::usleep(100000); // Sleep 100 msec
				kdDebug() << "sqlite3_step: BUSY counter: " << busyCnt << endl;
			}
			if ( error == SQLITE_MISUSE )
				kdDebug() << "sqlite3_step: MISUSE" << endl;
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
	if(sqlite3_value_type(argv[0])==SQLITE_NULL || sqlite3_value_type(argv[1])==SQLITE_NULL) {
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
	
	while ( (conn = dequeue()) != 0 )
	{
		if (/*m_dbConnType == DbConnection::sqlite && */vacuum)
		{
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
	for ( int i = 0; i < POOL_SIZE - 1; i++ )
	{
		DbConnection *dbConn;
		dbConn = new SqliteConnection(static_cast<SqliteConfig*> (m_dbConfig));
		enqueue(dbConn);
		m_semaphore--;
	}
	kdDebug() << "Available db connections: " << m_semaphore.available() << endl;
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
