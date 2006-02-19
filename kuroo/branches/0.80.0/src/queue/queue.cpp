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
 * @class AddQueuePackageIdListJob
 * @short Thread for adding packages to the queue in db. Used by other views.
 */
class AddQueuePackageIdListJob : public ThreadWeaver::DependentJob
{
public:
	AddQueuePackageIdListJob( QObject *dependent, const QStringList& packageIdList ) : DependentJob( dependent, "DBJob" ), m_packageIdList( packageIdList ) {}
	
	virtual bool doJob() {
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE queue_temp ("
		                                    " id INTEGER PRIMARY KEY AUTOINCREMENT, "
		                                    " idPackage INTEGER, "
		                                    " idDepend INTEGER, "
		                                    " use VARCHAR(255), "
		                                    " size VARCHAR(32), "
		                                    " version VARCHAR(32) "
		                                    " );", m_db);
		KurooDBSingleton::Instance()->insert("INSERT INTO queue_temp SELECT * FROM queue;", m_db);
		KurooDBSingleton::Instance()->query("BEGIN TRANSACTION;", m_db);
		
		foreach ( m_packageIdList )
			KurooDBSingleton::Instance()->insert(QString("INSERT INTO queue_temp (idPackage, idDepend) VALUES ('%1', '0');").arg(*it), m_db);
		
		KurooDBSingleton::Instance()->query("COMMIT TRANSACTION;", m_db);
		
		// Move content from temporary table to installedPackages
		KurooDBSingleton::Instance()->query("DELETE FROM queue;", m_db);
		KurooDBSingleton::Instance()->insert("INSERT INTO queue SELECT * FROM queue_temp;", m_db);
		KurooDBSingleton::Instance()->query("DROP TABLE queue_temp;", m_db);
		
		KurooDBSingleton::Instance()->returnStaticDbConnection(m_db);
		return true;
	}
	
	virtual void completeJob() {
		QueueSingleton::Instance()->refresh( false );
	}
	
private:
	const QStringList m_packageIdList;
};

/**
 * @class RemoveQueuePackageIdListJob
 * @short Thread for removing packages from the queue in db.
 */
class RemoveQueuePackageIdListJob : public ThreadWeaver::DependentJob
{
public:
	RemoveQueuePackageIdListJob( QObject *dependent, const QStringList& packageIdList ) : DependentJob( dependent, "DBJob" ), m_packageIdList( packageIdList ) {}
	
	virtual bool doJob() {
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		foreach ( m_packageIdList )
			KurooDBSingleton::Instance()->query( QString( "DELETE FROM queue "
			                                              "WHERE ( idPackage = '%1' OR idDepend = '%2' );" ).arg(*it).arg(*it), m_db );
		KurooDBSingleton::Instance()->returnStaticDbConnection(m_db);
		return true;
	}
	
	virtual void completeJob() {
		QueueSingleton::Instance()->refresh( false );
	}
	
private:
	const QStringList m_packageIdList;
};


/**
 * @class Queue
 * @short Object for packages to be emerged = installation queue.
 */
Queue::Queue( QObject* m_parent )
	: QObject( m_parent ), m_removeInstalled( false )
{
	// Clock timer for showing progress when emerging
	internalTimer = new QTimer( this );
	connect( internalTimer, SIGNAL( timeout() ), SLOT( slotOneStep() ) );
}

Queue::~Queue()
{
}

void Queue::init( QObject *parent )
{
	m_parent = parent;
}

/**
 * Start the timer.
 * @param package
 */
void Queue::emergePackageStart( const QString& package, int order, int total )
{
	QString id = KurooDBSingleton::Instance()->packageId( package );
	internalTimer->start( 1000 );
	emit signalPackageStart( id );
}

/**
 * Set package progress as 100% = complete.
 * @param package
 */
void Queue::emergePackageComplete( const QString& package, int order, int total )
{
	internalTimer->stop();
	QString id = KurooDBSingleton::Instance()->packageId( package );
	emit signalPackageComplete( id, m_removeInstalled );
}

/**
 * Initialize with package id and after that emit 1sec progress signals.
 * @param id
 */
void Queue::slotOneStep()
{
	emit signalPackageAdvance();
}

void Queue::stopTimer()
{
	internalTimer->stop();
}

/**
 * When the package is inserted in the register it in the cache too.
 * @param id
 */
void Queue::clearCache()
{
	packageCache.clear();
}

/**
 * When the package is inserted in the register it in the cache too.
 * @param id
 */
void Queue::insertInCache( const QString& id )
{
	if ( id.isEmpty() ) {
		kdDebug() << i18n("Package id is empty, skipping!") << endl;
		return;
	}
	
	packageCache[ id ] = true;
}

/**
 * When the package is inserted in the register it in the cache too.
 * @param id
 */
void Queue::deleteFromCache( const QString& id )
{
	if ( id.isEmpty() ) {
		kdDebug() << i18n("Package id is empty, skipping!") << endl;
		return;
	}
	
	packageCache[ id ] = false;
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
void Queue::refresh( bool hasCheckedQueue )
{
	clearCache();
	emit signalQueueChanged( hasCheckedQueue );
}

/**
 * Clear the queue.
 */
void Queue::reset()
{
	KurooDBSingleton::Instance()->query("DELETE FROM queue;");
	refresh( false );
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
void Queue::installQueue( const QStringList& packageList )
{
	EmergeSingleton::Instance()->queue( packageList );
}

void Queue::setRemoveInstalled( bool removeInstalled )
{
	m_removeInstalled = removeInstalled;
}

#include "queue.moc"

