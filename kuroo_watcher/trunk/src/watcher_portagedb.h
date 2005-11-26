// (c) 2004 Mark Kretschmann <markey@web.de>
// (c) 2004 Christian Muehlhaeuser <chris@chris.de>
// (c) 2004 Sami Nieminen <sami.nieminen@iki.fi>
// See COPYING file for licensing information.

#ifndef COLLECTIONDB_H
#define COLLECTIONDB_H

#include <qdir.h>            //stack allocated
#include <qobject.h>         //baseclass
#include <qptrqueue.h>       //baseclass
#include <qsemaphore.h>      //stack allocated
#include <qstringlist.h>     //stack allocated

#include "../sqlite/sqlite3.h"

class DbConnection;
class DbConnectionPool;
class CoverFetcher;
class MetaBundle;
class Scrobbler;

class DbConfig
{};

class SqliteConfig : public DbConfig
{
public:
	SqliteConfig( const QString& /* dbfile */ );
	
	const QString dbFile() const { return m_dbfile; }
	
private:
	QString m_dbfile;
};

class DbConnection
{
public:
	enum DbConnectionType { sqlite = 0, mysql = 1 };
	
	DbConnection( DbConfig* /* config */ );
	virtual ~DbConnection() = 0;
	
	virtual QStringList query( const QString& /* statement */ ) = 0;
	virtual int insert( const QString& /* statement */ ) = 0;
	const bool isInitialized() const { return m_initialized; }
	virtual bool isConnected()const = 0;
	virtual const QString lastError() const { return "None"; }
	
protected:
	bool m_initialized;
	DbConfig *m_config;
};

class SqliteConnection : public DbConnection
{
public:
	SqliteConnection( SqliteConfig* /* config */ );
	~SqliteConnection();
	
	QStringList query( const QString& /* statement */ );
	int insert( const QString& /* statement */ );
	bool isConnected()const { return true; }
	
private:
	static void sqlite_rand(sqlite3_context *context, int /*argc*/, sqlite3_value ** /*argv*/);
	static void sqlite_power(sqlite3_context *context, int argc, sqlite3_value **argv);
	
	sqlite3* m_db;
};

class DbConnectionPool : QPtrQueue<DbConnection>
{
public:
	DbConnectionPool();
	~DbConnectionPool();
	
	const DbConnection::DbConnectionType getDbConnectionType() const { return m_dbConnType; }
	const DbConfig *getDbConfig() const { return m_dbConfig; }
	void createDbConnections();
	
	DbConnection *getDbConnection();
	void putDbConnection( const DbConnection* /* conn */ );
	
	QString escapeString( QString string )
	{
		return
#ifdef USE_MYSQL
            // We have to escape "\" for mysql, but can't do so for sqlite
			(m_dbConnType == DbConnection::mysql)
			? string.replace("\\", "\\\\").replace( '\'', "''" )
			:
#endif
			string.replace( '\'', "''" );
	}
	
private:
	static const int POOL_SIZE = 5;
	QSemaphore m_semaphore;
	DbConnection::DbConnectionType m_dbConnType;
	DbConfig *m_dbConfig;
};


class KurooWatcherDB : public QObject
{
Q_OBJECT
		
signals:
	void scanStarted();
	void scanDone( bool changed );
	void databaseEngineChanged();

public:
	static KurooWatcherDB *instance();
	
	QString escapeString( QString string ) { return m_dbConnPool->escapeString(string); }
	
	/**
		* This method returns a static DbConnection for components that want to use
		* the same connection for the whole time. Should not be used anywhere else
		* but in CollectionReader.
		*
		* @return static DbConnection
		*/
	DbConnection *getStaticDbConnection();
	
	/**
		* Returns the DbConnection back to connection pool.
		*
		* @param conn DbConnection to be returned
		*/
	void returnStaticDbConnection( DbConnection *conn );
	
	//sql helper methods
	QStringList query( const QString& statement, DbConnection *conn = NULL );
	int insert( const QString& statement, DbConnection *conn = NULL );
	
	//table management methods
	bool isInstalledEmpty();
	
	QStringList installedPackages();
	QStringList updatePackages();
	
protected:
	KurooWatcherDB();
	~KurooWatcherDB();

public slots:
	
private slots:
	
private:
    //bump DATABASE_VERSION whenever changes to the table structure are made. will remove old db file.
	static const int DATABASE_VERSION = 18;
	static const int DATABASE_STATS_VERSION = 3;
	static const int MONITOR_INTERVAL = 60; //sec

	void initialize();
	void destroy();

	DbConnectionPool *m_dbConnPool;
	bool m_monitor;
};

#endif /* KUROODB_H */
