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

// From amaroK

#include "watcher_portagedb.h"
#include "watchersettings.h"

#include <qfile.h>
#include <qimage.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kio/job.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kio/netaccess.h>
#include <kdebug.h>

#include <math.h>                 //DbConnection::sqlite_power()
#include <time.h>                 //query()
#include <unistd.h>               //usleep()

KurooWatcherDB* KurooWatcherDB::instance()
{
	static KurooWatcherDB db;
	return &db;
}

KurooWatcherDB::KurooWatcherDB()
{
	initialize();
}

KurooWatcherDB::~KurooWatcherDB()
{
	destroy();
}

DbConnection *KurooWatcherDB::getStaticDbConnection()
{
	return m_dbConnPool->getDbConnection();
}

void KurooWatcherDB::returnStaticDbConnection( DbConnection *conn )
{
	m_dbConnPool->putDbConnection( conn );
}

/**
 * Executes a SQL query on the already opened database
 * @param statement SQL program to execute. Only one SQL statement is allowed.
 * @return          The queried data, or QStringList() on error.
 */
QStringList KurooWatcherDB::query( const QString& statement, DbConnection *conn )
{
// 	kdDebug() << "Query-start: " << statement << endl;
// 	clock_t start = clock();
	
	DbConnection *dbConn;
	if ( conn != NULL )
	{
		dbConn = conn;
	}
	else
	{
		dbConn = m_dbConnPool->getDbConnection();
	}
	
	QStringList values = dbConn->query( statement );
	
	if ( conn == NULL )
	{
		m_dbConnPool->putDbConnection( dbConn );
	}
	
// 	clock_t finish = clock();
// 	const double duration = (double) (finish - start) / CLOCKS_PER_SEC;
// 	kdDebug() << "SQL-query (" << duration << "s): " << statement << endl;

	return values;
}

/**
 * Executes a SQL insert on the already opened database
 * @param statement SQL statement to execute. Only one SQL statement is allowed.
 * @return          The rowid of the inserted item.
 */
int KurooWatcherDB::insert( const QString& statement, DbConnection *conn )
{
/*	kdDebug() << "insert-start: " << statement << endl;*/
/*	clock_t start = clock();*/
	
	DbConnection *dbConn;
	if ( conn != NULL )
	{
		dbConn = conn;
	}
	else
	{
		dbConn = m_dbConnPool->getDbConnection();
	}
	
	int id = dbConn->insert( statement );
	
	if ( conn == NULL )
	{
		m_dbConnPool->putDbConnection( dbConn );
	}
	
// 	clock_t finish = clock();
// 	const double duration = (double) (finish - start) / CLOCKS_PER_SEC;
// 	kdDebug() << "SQL-insert (" << duration << "s): " << statement << endl;

	return id;
}

bool KurooWatcherDB::isInstalledEmpty()
{
	QStringList values = query( "SELECT COUNT( id ) FROM package LIMIT 0, 1;" );
	return values.isEmpty() ? true : values.first() == "0";
}

/////////////////////////////////////////////////////////////////


bool KurooWatcherDB::isInstalled( const QString& package )
{
	QString category = package.section("/", 0, 0);
	QString name = package.section("/", 1, 1);
	
	kdDebug() << "category=" << category << endl;
	kdDebug() << "name=" << name << endl;
	
	QStringList list = query( "SELECT name FROM package "
	                          " WHERE installed != '0' "
	                          " AND name == '" + name + "'"
	                          " AND idCategory == ( SELECT id from category WHERE name == '" + category + "')"
	                          " ;" );
	
	kdDebug() << "list=" << list << endl;
	
	if ( list.isEmpty() )
		return false;
	else
		return true;
}

QStringList KurooWatcherDB::installedPackages()
{
	return query( "SELECT name FROM package "
	              " WHERE installed != 0 "
	              " ORDER BY lower( name );" );
}

QStringList KurooWatcherDB::updatePackages()
{
	return query( "SELECT name FROM package "
	              " WHERE updateVersion != '' "
	              " ORDER BY lower( name );" );
}

///////////////////////////////////////////////////////////////////

void
KurooWatcherDB::initialize()
{
	m_dbConnPool = new DbConnectionPool();
	DbConnection *dbConn = m_dbConnPool->getDbConnection();
	m_dbConnPool->putDbConnection( dbConn );
	
	if ( !dbConn->isInitialized() || !isInstalledEmpty() )
	{
		// Warn for empty installed packages
	}
	
	m_dbConnPool->createDbConnections();
}

void
KurooWatcherDB::destroy()
{
	delete m_dbConnPool;
}

/**
 */
DbConnection::DbConnection( DbConfig* config )
: m_config( config )
{}

DbConnection::~DbConnection()
{}

/** Sqlite methods
 */
SqliteConnection::SqliteConnection( SqliteConfig* config )
: DbConnection( config )
{
	const QCString path = QString( watcherSettings::dirHome() + "/" + watcherSettings::databas() ).local8Bit();

    // Open database file and check for correctness
	m_initialized = false;
	QFile file( path );
	if ( file.open( IO_ReadOnly ) )
	{
		QString format;
		file.readLine( format, 50 );
		if ( !format.startsWith( "SQLite format 3" ) )
		{
			kdDebug() << "Database versions incompatible. Removing and rebuilding database.\n";
		}
		else if ( sqlite3_open( path, &m_db ) != SQLITE_OK )
		{
			kdDebug() << "Database file corrupt. Removing and rebuilding database.\n";
			sqlite3_close( m_db );
		}
		else
			m_initialized = true;
	}
	
	if ( !m_initialized )
	{
        // Remove old db file; create new
		QFile::remove( path );
		if ( sqlite3_open( path, &m_db ) == SQLITE_OK )
		{
			m_initialized = true;
		}
	}
	if ( m_initialized )
	{
		if( sqlite3_create_function(m_db, "rand", 0, SQLITE_UTF8, NULL, sqlite_rand, NULL, NULL) != SQLITE_OK )
			m_initialized = false;
		if( sqlite3_create_function(m_db, "power", 2, SQLITE_UTF8, NULL, sqlite_power, NULL, NULL) != SQLITE_OK )
			m_initialized = false;
	}
	
    //optimization for speeding up SQLite
	query( "PRAGMA default_synchronous = OFF;" );
}

SqliteConnection::~SqliteConnection()
{
	if ( m_db ) sqlite3_close( m_db );
}

QStringList SqliteConnection::query( const QString& statement )
{
	QStringList values;
	int error;
	const char* tail;
	sqlite3_stmt* stmt;
	
    //compile SQL program to virtual machine
	error = sqlite3_prepare( m_db, statement.utf8(), statement.length(), &stmt, &tail );
	
	if ( error != SQLITE_OK )
	{
		kdDebug() << k_funcinfo << " sqlite3_compile error:" << endl;
		kdDebug() << sqlite3_errmsg( m_db ) << endl;
		kdDebug() << "on query: " << statement << endl;
		values = QStringList();
	}
	else
	{
		int busyCnt = 0;
		int number = sqlite3_column_count( stmt );
        //execute virtual machine by iterating over rows
		while ( true )
		{
			error = sqlite3_step( stmt );
			
			if ( error == SQLITE_BUSY )
			{
				if ( busyCnt++ > 20 ) {
					kdDebug() << "Busy-counter has reached maximum. Aborting this sql statement!\n";
					break;
				}
				::usleep( 100000 ); // Sleep 100 msec
				kdDebug() << "sqlite3_step: BUSY counter: " << busyCnt << endl;
			}
			if ( error == SQLITE_MISUSE )
				kdDebug() << "sqlite3_step: MISUSE" << endl;
			if ( error == SQLITE_DONE || error == SQLITE_ERROR )
				break;
			
            //iterate over columns
			for ( int i = 0; i < number; i++ )
			{
				values << QString::fromUtf8( (const char*) sqlite3_column_text( stmt, i ) );
			}
		}
        //deallocate vm ressources
		sqlite3_finalize( stmt );
		
		if ( error != SQLITE_DONE )
		{
			kdDebug() << k_funcinfo << "sqlite_step error.\n";
			kdDebug() << sqlite3_errmsg( m_db ) << endl;
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
	
	if ( error != SQLITE_OK )
	{
	kdDebug() << k_funcinfo << " sqlite3_compile error:" << endl;
		kdDebug() << sqlite3_errmsg( m_db ) << endl;
	kdDebug() << "on insert: " << statement << endl;
	}
	else
	{
		int busyCnt = 0;
        //execute virtual machine by iterating over rows
		while ( true )
		{
			error = sqlite3_step( stmt );
			
			if ( error == SQLITE_BUSY )
			{
				if ( busyCnt++ > 20 ) {
					kdDebug() << "Busy-counter has reached maximum. Aborting this sql statement!\n";
					break;
				}
				::usleep( 100000 ); // Sleep 100 msec
				kdDebug() << "sqlite3_step: BUSY counter: " << busyCnt << endl;
			}
			if ( error == SQLITE_MISUSE )
				kdDebug() << "sqlite3_step: MISUSE" << endl;
			if ( error == SQLITE_DONE || error == SQLITE_ERROR )
				break;
		}
        //deallocate vm ressources
		sqlite3_finalize( stmt );
		
		if ( error != SQLITE_DONE )
		{
			kdDebug() << k_funcinfo << "sqlite_step error.\n";
			kdDebug() << sqlite3_errmsg( m_db ) << endl;
			kdDebug() << "on insert: " << statement << endl;
		}
	}
	return sqlite3_last_insert_rowid( m_db );
}

// this implements a RAND() function compatible with the MySQL RAND() (0-param-form without seed)
void SqliteConnection::sqlite_rand(sqlite3_context *context, int /*argc*/, sqlite3_value ** /*argv*/)
{
	sqlite3_result_double( context, static_cast<double>(KApplication::random()) / (RAND_MAX+1.0) );
}

// this implements a POWER() function compatible with the MySQL POWER()
void SqliteConnection::sqlite_power(sqlite3_context *context, int argc, sqlite3_value **argv)
{
	Q_ASSERT( argc==2 );
	if( sqlite3_value_type(argv[0])==SQLITE_NULL || sqlite3_value_type(argv[1])==SQLITE_NULL ) {
		sqlite3_result_null(context);
		return;
	}
	double a = sqlite3_value_double(argv[0]);
	double b = sqlite3_value_double(argv[1]);
	sqlite3_result_double( context, pow(a,b) );
}

SqliteConfig::SqliteConfig( const QString& dbfile )
: m_dbfile( dbfile )
{
}

/** Connections pool with thread support
 */
DbConnectionPool::DbConnectionPool() : m_semaphore( POOL_SIZE )
{
// 	m_dbConnType = DbConnection::sqlite;
	
	m_semaphore += POOL_SIZE;
	DbConnection *dbConn;
	m_dbConfig = new SqliteConfig( watcherSettings::databas() );
	dbConn = new SqliteConnection( static_cast<SqliteConfig*> ( m_dbConfig ) );

	enqueue( dbConn );
	m_semaphore--;
// 	kdDebug() << "Available db connections: " << m_semaphore.available() << endl;
}

DbConnectionPool::~DbConnectionPool()
{
	m_semaphore += POOL_SIZE;
	DbConnection *conn;
// 	bool vacuum = true;
	
	while ( ( conn = dequeue() ) != 0 )
	{
// 		if ( /*m_dbConnType == DbConnection::sqlite && */vacuum )
// 		{
// 			vacuum = false;
// 			kdDebug() << "Running VACUUM" << endl;
// 			conn->query( "VACUUM; ");
// 		}
		
		delete conn;
	}
	
	delete m_dbConfig;
}

void DbConnectionPool::createDbConnections()
{
	for ( int i = 0; i < POOL_SIZE - 1; i++ )
	{
		DbConnection *dbConn;
		dbConn = new SqliteConnection( static_cast<SqliteConfig*> ( m_dbConfig ) );
		enqueue( dbConn );
		m_semaphore--;
	}
// 	kdDebug() << "Available db connections: " << m_semaphore.available() << endl;
}

DbConnection *DbConnectionPool::getDbConnection()
{
	m_semaphore++;
	return dequeue();
}

void DbConnectionPool::putDbConnection( const DbConnection *conn )
{
	enqueue( conn );
	m_semaphore--;
}

#include "watcher_portagedb.moc"
