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

#ifndef PORTAGEDB_H
#define PORTAGEDB_H

#include <qdir.h>            //stack allocated
#include <qobject.h>         //baseclass
#include <qptrqueue.h>       //baseclass
#include <qsemaphore.h>      //stack allocated
#include <qstringlist.h>     //stack allocated

#include "../sqlite/sqlite3.h"

class Package;
class DbConnection;
class DbConnectionPool;

class DbConfig
{};

class SqliteConfig : public DbConfig
{
public:
	SqliteConfig(const QString& /* dbfile */);
	
	const 				QString dbFile() const { return m_dbfile; }
	
private:
	QString m_dbfile;
};

class DbConnection
{
public:
	enum DbConnectionType { sqlite = 0, mysql = 1 };
	
	DbConnection(DbConfig* /* config */);
	virtual ~DbConnection() = 0;
	
	virtual QStringList query( const QString& /* statement */) = 0;
	virtual int 		insert( const QString& /* statement */) = 0;
	const bool 			isInitialized() const { return m_initialized; }
	virtual bool 		isConnected()const = 0;
	virtual const 		QString lastError() const { return "None"; }
	
protected:
	bool m_initialized;
	DbConfig *m_config;
};

class SqliteConnection : public DbConnection
{
public:
	SqliteConnection(SqliteConfig* /* config */);
	~SqliteConnection();
	
	QStringList 		query( const QString& /* statement */ );
	int 				insert( const QString& /* statement */ );
	bool 				isConnected()const { return true; }
	
private:
	static void 		sqlite_rand( sqlite3_context *context, int /*argc*/, sqlite3_value ** /*argv*/ );
	static void 		sqlite_power( sqlite3_context *context, int argc, sqlite3_value **argv );
	
	sqlite3* m_db;
};

class DbConnectionPool : QPtrQueue<DbConnection>
{
public:
	DbConnectionPool();
	~DbConnectionPool();
	
	const 				DbConnection::DbConnectionType getDbConnectionType() const { return m_dbConnType; }
	const 				DbConfig *getDbConfig() const { return m_dbConfig; }
	void 				createDbConnections();
	
	DbConnection 		*getDbConnection();
	void 				putDbConnection( const DbConnection* /* conn */ );
	
	QString escapeString(QString string) {
		return string.replace('\'', "''");
	}
	
private:
	static const int POOL_SIZE = 10;
	QSemaphore m_semaphore;
	DbConnection::DbConnectionType m_dbConnType;
	DbConfig *m_dbConfig;
};

/**
 * @class KurooDB
 * @short All database related as connections, queries...
 */
class KurooDB : public QObject
{
Q_OBJECT

public:
	KurooDB( QObject *parent = 0 );
	~KurooDB();
	
	/**
	 * Check db integrity and create new db if necessary.
	 */
	QString 		init( QObject *myParent = 0 );
	QString 		escapeString( QString string ) { return m_dbConnPool->escapeString(string); }
	
	/**
	* This method returns a static DbConnection for components that want to use
	* the same connection for the whole time. Should not be used anywhere else
	* but in CollectionReader.
	*
	* @return static DbConnection
	*/
	DbConnection 	*getStaticDbConnection();
	
	/**
	* Returns the DbConnection back to connection pool.
	*
	* @param conn DbConnection to be returned
	*/
	void 			returnStaticDbConnection( DbConnection *conn );
	
	//sql helper methods
	QStringList 	query( const QString& statement, DbConnection *conn = NULL );
	int 			insert( const QString& statement, DbConnection *conn = NULL );
	
	//table management methods
	bool 			isPortageEmpty();
	bool 			isHistoryEmpty();
	bool 			isValid();
	void 			createTables(DbConnection *conn = NULL);
	
	//////////////////////////////////////////////////////////////////////////////
	// Queries for allPackages
	//////////////////////////////////////////////////////////////////////////////
// 	QStringList 	isInstalled( const QString& name, const QString& version );
	QStringList 	packageVersions( const QString& id );
	QStringList 	packageVersionsInfo( const QString& id );
// 	QStringList 	packageKeywords( const QString& idCategory, const QString& name );
	QStringList 	packageTotal();
// 	QStringList 	installedTotal();
	QStringList 	updatesTotal();
	QString		 	package( const QString& id );
	QString		 	category( const QString& id );
	
	
	//////////////////////////////////////////////////////////////////////////////
	// Queries for Portage
	//////////////////////////////////////////////////////////////////////////////
	QStringList		allCategories();
	QStringList		allSubCategories();
	QStringList 	portageCategories( int filter, const QString& text );
	QStringList 	portageSubCategories( const QString& categoryId, int filter, const QString& text );
	QStringList 	portagePackagesBySubCategory( const QString& categoryId, const QString& subCategoryId, int filter, const QString& text );
	QStringList 	portagePackageInfo( const QString& id );
	QString		 	packageIdDB( const QString& category, const QString& name );

	
	//////////////////////////////////////////////////////////////////////////////
	// Queries for Queue 
	//////////////////////////////////////////////////////////////////////////////
	QStringList 	queuePackages();
	
	//////////////////////////////////////////////////////////////////////////////
	// Queries for Results 
	//////////////////////////////////////////////////////////////////////////////
	QStringList 	resultPackages();
	
	//////////////////////////////////////////////////////////////////////////////
	// Queries for History 
	//////////////////////////////////////////////////////////////////////////////
	QStringList 	history();
	QStringList 	mergeHistory();
	QStringList 	lastHistoryEntry();
	QStringList		getLastSync();
	void			addRefreshTime();
	void			addBackup( const QString& source, const QString& destination );
	QStringList 	statistic();
	
	//////////////////////////////////////////////////////////////////////////////
	// Queries for Cache
	//////////////////////////////////////////////////////////////////////////////
	QStringList 	cache();
	bool			isCacheEmpty();
	
	
	
private:
	QObject	*parent;
	
    //bump DATABASE_VERSION whenever changes to the table structure are made. will remove old db file.
	static const int DATABASE_VERSION = 19;
	static const int DATABASE_STATS_VERSION = 3;
	static const int MONITOR_INTERVAL = 60; //sec

	void destroy();

	DbConnectionPool *m_dbConnPool;
	bool m_monitor;
};

#endif /* KUROODB_H */
