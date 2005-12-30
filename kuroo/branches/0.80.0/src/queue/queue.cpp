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

#include "common.h"
#include "threadweaver.h"

#include <qtimer.h>

/**
 * Thread for adding a single package to the queue in db. Used by emerge.
 */
class AddQueuePackageJob : public ThreadWeaver::DependentJob
{
public:
	AddQueuePackageJob( QObject *dependent, const QString& package ) : DependentJob( dependent, "DBJob" ), m_package( package ) {}
	
	virtual bool doJob() {
		QString category = m_package.section( "/", 0, 0 );
		QString name = ( m_package.section( "/", 1, 1 ) ).section( rxPortageVersion, 0, 0 );
		
		QString idCategory = KurooDBSingleton::Instance()->query( QString( "SELECT id FROM catSubCategory WHERE name = '%1';" ).arg( category ) ).first();
		packageId = KurooDBSingleton::Instance()->query( QString( "SELECT id FROM package WHERE idCatSubCategory = '%1' AND name  = '%2';" ).arg( idCategory ).arg( name ) ).first();
		int rowId = KurooDBSingleton::Instance()->insert( QString( "INSERT INTO queue (idPackage) VALUES ('%1');" ).arg( packageId ) );
		
		// Add this package to the world file if not dependency.
		if ( rowId == 0 ) {
			QFile file( KurooConfig::dirWorldFile() );
			QStringList lines;
			if ( file.open( IO_ReadOnly ) ) {
				QTextStream stream( &file );
				while ( !stream.atEnd() )
					lines += stream.readLine();
				file.close();
				
				if ( file.open( IO_WriteOnly ) ) {
					bool found;
					QTextStream stream( &file );
					foreach ( lines ) {
						stream << *it << endl;
						if ( *it == ( category + "/" + name ) )
							found = true;
					}
					if ( !found )
						stream << category + "/" + name << endl;
					file.close();
				}
				else
					kdDebug() << i18n("Error writing: ") << KurooConfig::dirWorldFile() << endl;
			}
			else
				kdDebug() << i18n("Error reading: ") << KurooConfig::dirWorldFile() << endl;
		}
		
		return true;
	}
	
	virtual void completeJob() {
		QueueSingleton::Instance()->refresh();
	}
	
private:
	const QString m_package;
	QString packageId;
};

/**
 * Thread for adding packages to the queue in db. Used by other views.
 */
class AddQueuePackageIdListJob : public ThreadWeaver::DependentJob
{
public:
	AddQueuePackageIdListJob( QObject *dependent, const QStringList& packageIdList ) : DependentJob( dependent, "DBJob" ), m_packageIdList( packageIdList ) {}
	
	virtual bool doJob() {
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE queue_temp ("
		                                    " id INTEGER PRIMARY KEY AUTOINCREMENT, "
		                                    " idPackage INTEGER UNIQUE) "
		                                    " ;", m_db);
		KurooDBSingleton::Instance()->insert("INSERT INTO queue_temp SELECT * FROM queue;", m_db);
		KurooDBSingleton::Instance()->query("BEGIN TRANSACTION;", m_db);
		
		foreach ( m_packageIdList ) {
			KurooDBSingleton::Instance()->insert(QString("INSERT INTO queue_temp (idPackage) VALUES ('%1');").arg(*it), m_db);
		}
		
		KurooDBSingleton::Instance()->query("COMMIT TRANSACTION;", m_db);
		
		// Move content from temporary table to installedPackages
		KurooDBSingleton::Instance()->query("DELETE FROM queue;", m_db);
		KurooDBSingleton::Instance()->insert("INSERT INTO queue SELECT * FROM queue_temp;", m_db);
		KurooDBSingleton::Instance()->query("DROP TABLE queue_temp;", m_db);
		
		KurooDBSingleton::Instance()->returnStaticDbConnection(m_db);
		return true;
	}
	
	virtual void completeJob() {
		QueueSingleton::Instance()->refresh();
	}
	
private:
	const QStringList m_packageIdList;
};

/**
 * Thread for removing packages from the queue in db.
 */
class RemoveQueuePackageIdListJob : public ThreadWeaver::DependentJob
{
public:
	RemoveQueuePackageIdListJob( QObject *dependent, const QStringList& packageIdList ) : DependentJob( dependent, "DBJobGui" ), m_packageIdList( packageIdList ) {}
	
	virtual bool doJob() {
		foreach ( m_packageIdList ) {
			KurooDBSingleton::Instance()->query( QString( "DELETE FROM queue WHERE idPackage = '%1';" ).arg(*it) );
		}
		return true;
	}
	
	virtual void completeJob() {
		QueueSingleton::Instance()->refresh();
	}
	
private:
	const QStringList m_packageIdList;
};

/**
 * Thread for setting queue in db to packages list.
 */
class InstallQueueJob : public ThreadWeaver::DependentJob
{
public:
	InstallQueueJob( QObject *dependent, const QStringList& packageIdList ) : DependentJob( dependent, "DBJob" ), m_packageIdList( packageIdList ) {}
	
	virtual bool doJob() {
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->query("DELETE FROM queue;", m_db);
		KurooDBSingleton::Instance()->query("CREATE TEMP TABLE queue_temp (	"
		                                    " id INTEGER PRIMARY KEY AUTOINCREMENT, "
		                                    " idPackage INTEGER UNIQUE) "
		                                    " ;", m_db);
		KurooDBSingleton::Instance()->insert("INSERT INTO queue_temp SELECT * FROM queue;", m_db);
		KurooDBSingleton::Instance()->query("BEGIN TRANSACTION;", m_db);
		
		foreach ( m_packageIdList ) {
			KurooDBSingleton::Instance()->insert(QString("INSERT INTO queue_temp (idPackage) VALUES ('%1');").arg(*it), m_db);
		}
		
		KurooDBSingleton::Instance()->query("COMMIT TRANSACTION;", m_db);
		
		// Move content from temporary table to installedPackages
		KurooDBSingleton::Instance()->insert("INSERT INTO queue SELECT * FROM queue_temp;", m_db);
		KurooDBSingleton::Instance()->query("DROP TABLE queue_temp;", m_db);
		
		KurooDBSingleton::Instance()->returnStaticDbConnection(m_db);
		return true;
	}
	
	virtual void completeJob() {
		QueueSingleton::Instance()->refresh();
		SignalistSingleton::Instance()->startInstallQueue();
	}
	
private:
	const QStringList m_packageIdList;
};

/**
 * Object for packages to be emerged = installation queue.
 * @fixme: manage the cache.
 */
Queue::Queue( QObject* parent )
	: QObject( parent )
{
	// Clock timer for showing progress when emerging
	internalTimer = new QTimer( this );
	connect( internalTimer, SIGNAL( timeout() ), SLOT( slotOneStep() ) );
	
	connect( SignalistSingleton::Instance(), SIGNAL( signalEmergePackageStart( const QString& ) ), this, SLOT( slotEmergePackageStart( const QString& ) ) );
	connect( SignalistSingleton::Instance(), SIGNAL( signalEmergePackageStop( const QString& ) ), this, SLOT( slotEmergePackageStop( const QString& ) ) );
}

Queue::~Queue()
{
}

void Queue::slotEmergePackageStart( const QString& package )
{
	internalTimer->start( 1000 );
	m_id = PortageSingleton::Instance()->idDb( package );
	
	kdDebug() << "Queue::slotEmergePackageStart m_id=" << m_id << endl;
}

void Queue::slotEmergePackageStop( const QString& package )
{
	internalTimer->stop();
	
	kdDebug() << "Queue::slotEmergePackageStop package=" << package << endl;
}

void Queue::slotOneStep()
{
	emit signalPackageAdvance( m_id );
}

void Queue::init( QObject *myParent )
{
	parent = myParent;
}

/**
 * When the package is inserted in the register it in the cache too.
 * @param id
 */
void Queue::clearCache()
{
	packageCache.clear();
	SignalistSingleton::Instance()->clearQueued();
}

/**
 * When the package is inserted in the register it in the cache too.
 * @param id
 */
void Queue::insertInCache( const QString& id )
{
	packageCache[ id ] = true;
	SignalistSingleton::Instance()->setQueued( id, true );
}

/**
 * When the package is inserted in the register it in the cache too.
 * @param id
 */
void Queue::deleteFromCache( const QString& id )
{
	packageCache[ id ] = false;
	SignalistSingleton::Instance()->setQueued( id, false );
}

/**
 * Check if package is the queue.
 * @param id
 * @return true/false
 */
bool Queue::isQueued( const QString& id )
{
	QMap<QString, bool>::iterator itMap = packageCache.find( id );
	if ( itMap == packageCache.end() )
		return false;
	else
		return true;
}

/**
 * Forward signal to refresh queue.
 */
void Queue::refresh()
{
	clearCache();
	emit signalQueueChanged();
}

/**
 * Clear the queue.
 */
void Queue::reset()
{
	KurooDBSingleton::Instance()->query("DELETE FROM queue;");
	clearCache();
	emit signalQueueChanged();
}

/**
 * Launch emerge pretend of packages.
 * @param packageList
 */
void Queue::pretendPackageList( const QStringList& packageList )
{
	EmergeSingleton::Instance()->pretend( packageList );
}

/**
 * Launch emerge package list
 * @param packageList
 */
void Queue::installPackageList( const QStringList& packageList )
{
	EmergeSingleton::Instance()->queue( packageList );
}

/**
 * Remove packages from queue.
 * @param packageIdList
 */
void Queue::removePackageIdList( const QStringList& packageIdList )
{
	ThreadWeaver::instance()->queueJob( new RemoveQueuePackageIdListJob( this, packageIdList ) );
}

/**
 * Add packages to the installation queue table in the db.
 * @param packageIdList
 */
void Queue::addPackageIdList( const QStringList& packageIdList )
{
	ThreadWeaver::instance()->queueJob( new AddQueuePackageIdListJob( this, packageIdList ) );
}

/**
 * Launch emerge of all packages in the queue.
 * @param packageIdList
 */
void Queue::installQueue( const QStringList& packageIdList )
{
	ThreadWeaver::instance()->queueJob( new InstallQueueJob( this, packageIdList ) );
}

/**
 * Add package as installed in db.
 * @param package
 */
void Queue::addPackage( const QString& package )
{
	ThreadWeaver::instance()->queueJob( new AddQueuePackageJob( this, package ) );
}

/**
 * Get list of all Queue packages.
 * @return QStringList
 */
QStringList Queue::allPackages()
{
	return KurooDBSingleton::Instance()->queuePackages();
}

/**
 * Count packages in queue.
 * @return count
 */
QString Queue::count()
{
	QStringList total = KurooDBSingleton::Instance()->query("SELECT COUNT(id) FROM queue LIMIT 1;");
	return total.first();
}

#include "queue.moc"

