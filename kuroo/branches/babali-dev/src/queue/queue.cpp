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
 * @short Thread for adding packages to the queue in db.
 */
class AddQueuePackageIdListJob : public ThreadWeaver::DependentJob
{
public:
	AddQueuePackageIdListJob( QObject *dependent, const QStringList& packageIdList ) : DependentJob( dependent, "DBJob" ), 
		m_packageIdList( packageIdList ) {}
	
	virtual bool doJob() {
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->singleQuery(	"CREATE TEMP TABLE queue_temp ( "
		                                    		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		                                    		"idPackage INTEGER, "
		                                    		"idDepend INTEGER, "
		                                    		"use VARCHAR(255), "
		                                    		"size VARCHAR(32), "
		                                    		"version VARCHAR(32) );"
		                                    		, m_db );
		KurooDBSingleton::Instance()->insert( "INSERT INTO queue_temp SELECT * FROM queue;", m_db );
		KurooDBSingleton::Instance()->singleQuery( "BEGIN TRANSACTION;", m_db );
		
		foreach ( m_packageIdList )
			KurooDBSingleton::Instance()->insert( QString( "INSERT INTO queue_temp (idPackage, idDepend) VALUES ('%1', '0');" ).arg(*it), m_db );
		
		KurooDBSingleton::Instance()->singleQuery( "COMMIT TRANSACTION;", m_db );
		
		// Move content from temporary table to installedPackages
		KurooDBSingleton::Instance()->singleQuery( "DELETE FROM queue;", m_db );
		KurooDBSingleton::Instance()->insert( "INSERT INTO queue SELECT * FROM queue_temp;", m_db );
		KurooDBSingleton::Instance()->singleQuery( "DROP TABLE queue_temp;", m_db );
		
		KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
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
	RemoveQueuePackageIdListJob( QObject *dependent, const QStringList& packageIdList ) : DependentJob( dependent, "DBJob" ), 
		m_packageIdList( packageIdList ) {}
	
	virtual bool doJob() {
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		foreach ( m_packageIdList )
			KurooDBSingleton::Instance()->singleQuery( QString( "DELETE FROM queue WHERE ( idPackage = '%1' OR idDepend = '%2' );" )
			                                     		.arg(*it).arg(*it), m_db );
		KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
		return true;
	}
	
	virtual void completeJob() {
		QueueSingleton::Instance()->refresh( false );
	}
	
private:
	const QStringList m_packageIdList;
};

/**
 * @class AddResultsPackageListJob
 * @short Thread for adding packages to results in db. Used by emerge.
 */
class AddResultsPackageListJob : public ThreadWeaver::DependentJob
{
public:
	AddResultsPackageListJob( QObject *dependent, const EmergePackageList &packageList ) : DependentJob( dependent, "DBJob" ), 
		m_packageList( packageList ) {}
	
	virtual bool doJob() {
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		
		// Collect end-user packages
		QMap<QString, int> endUserPackageMap;
		const QStringList endUserPackageList = KurooDBSingleton::Instance()->query( "SELECT idPackage FROM queue WHERE idDepend = '0';", m_db );
		
		foreach ( endUserPackageList )
			endUserPackageMap.insert( *it, 0 );
		
		KurooDBSingleton::Instance()->query("DELETE FROM queue;");
		
		// Iterate the emerge pretend package list
		QString idPackage;
		EmergePackageList::ConstIterator itEnd = m_packageList.end();
		for ( EmergePackageList::ConstIterator it = m_packageList.begin(); it != itEnd; ++it ) {
			
			QString id = KurooDBSingleton::Instance()->singleQuery( 
				" SELECT id FROM package WHERE name = '" + (*it).name + "' AND category = '" + (*it).category + "' LIMIT 1;", m_db );
			
			if ( id.isEmpty() ) {
				kdWarning(0) << QString("Add result package list: Can not find id in database for package %1/%2.")
								.arg( (*it).category ).arg( (*it).name ) << LINE_INFO;
				return false;
			}
			
			// We found a dependency, add it
			if ( !endUserPackageMap.contains( id ) ) {
				KurooDBSingleton::Instance()->insert( QString( 
					"INSERT INTO queue (idPackage, idDepend, use, size, version) VALUES ('%1', '%2', '%3', '%4', '%5');" )
				    .arg( id ).arg( idPackage ).arg( (*it).useFlags ).arg( (*it).size ).arg( (*it).version ), m_db );
			}
			else {
				idPackage = id;
				KurooDBSingleton::Instance()->insert( QString( 
					"INSERT INTO queue (idPackage, idDepend, use, size, version) VALUES ('%1', '0', '%2', '%3', '%4');" )
				    .arg( id ).arg( (*it).useFlags ).arg( (*it).size ).arg( (*it).version ), m_db );
			}
		}
		KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
		return true;
	}
	
	virtual void completeJob() {
		QueueSingleton::Instance()->refresh( true );
	}
	
private:
	const EmergePackageList m_packageList;
	
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Queue
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @class Queue
 * @short Object for packages to be emerged = installation queue.
 */
Queue::Queue( QObject* m_parent )
	: QObject( m_parent ), m_removeInstalled( false ), m_isQueueBusy( false )
{
	// Clock timer for showing progress when emerging
	m_internalTimer = new QTimer( this );
	connect( m_internalTimer, SIGNAL( timeout() ), SLOT( slotOneStep() ) );
	
	// When all packages are emerged...
	connect( EmergeSingleton::Instance(), SIGNAL( signalEmergeComplete() ), this, SLOT( slotClearQueue() ) );
	connect( EmergeSingleton::Instance(), SIGNAL( signalEmergeComplete() ), this, SLOT( slotShutdownComputer() ) );
}

Queue::~Queue()
{}

void Queue::init( QObject *parent )
{
	m_parent = parent;
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
	KurooDBSingleton::Instance()->resetQueue();
	refresh( false );
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Queue cache handling
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Check if package is the queue.
 * @param id
 * @return true/false
 */
bool Queue::isQueued( const QString& id )
{
	return m_queueCache.contains( id );
}

/**
 * Clear Queue.
 * @param id
 */
void Queue::clearCache()
{
	m_queueCache.clear();
}

/**
 * When the package is inserted in the queue register it in the cache too.
 * @param id
 */
void Queue::insertInCache( const QString& id )
{
	if ( id.isEmpty() ) {
		kdWarning(0) << "Package id is empty, skipping!" << LINE_INFO;
		return;
	}
	m_queueCache.insert( id, false );
}

/**
 * When the package is removed from queue remove from cache.
 * @param id
 */
void Queue::deleteFromCache( const QString& id )
{
	if ( id.isEmpty() ) {
		kdWarning(0) << "Package id is empty, skipping!" << LINE_INFO;
		return;
	}
	m_queueCache.remove( id );
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// package progress handling
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Start the package installation timer.
 * @param package
 */
void Queue::emergePackageStart( const QString& package, int order, int total )
{
	QString id = KurooDBSingleton::Instance()->packageId( package );
	if ( isQueued( id ) )
		m_queueCache[ id ] = false;
	
	m_internalTimer->start( 1000 );
	emit signalPackageStart( id );
	m_isQueueBusy = true;
}

/**
 * Set package progress as 100% = complete.
 * @param package
 */
void Queue::emergePackageComplete( const QString& package, int order, int total )
{
	m_internalTimer->stop();
	QString id = KurooDBSingleton::Instance()->packageId( package );
	if ( isQueued( id ) )
		m_queueCache[ id ] = true;
	
	emit signalPackageComplete( id );
}

/**
 * Initialize with package id and after that emit 1sec progress signals.
 * @param id
 */
void Queue::slotOneStep()
{
	emit signalPackageAdvance();
}

bool Queue::isQueueBusy()
{
	return m_isQueueBusy;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Package handling
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Add packages to the results table in the db
 * @param packageList
 */
void Queue::addPackageList( const EmergePackageList &packageList )
{
	if ( !packageList.isEmpty() )
		ThreadWeaver::instance()->queueJob( new AddResultsPackageListJob( this, packageList ) );
}

/**
 * Tells if package is in the queue
 * @param packageList
 * @return packageQueued
 */
bool
Queue::isPackageQueued( const QString& package )
{
	return m_queueCache[package];
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

/**
 * User wants queue to be cleared after emerge is done.
 * @param removeInstalled
 */
void Queue::setRemoveInstalled( bool removeInstalled )
{
	m_removeInstalled = removeInstalled;
}

/**
 * Clear installed packages from queue after emerge is done.
 */
void Queue::slotClearQueue()
{
	// Queue is not busy anymore - off course
	m_isQueueBusy = false;
	
	// Make sure the timer is stopped
	m_internalTimer->stop();
	
	if ( m_removeInstalled ) {
		
		// Collect only 100% complete packages
		QStringList idList;
		for ( QMap<QString, bool>::iterator itMap = m_queueCache.begin(), itMapEnd = m_queueCache.end(); itMap != itMapEnd; ++itMap ) {
			if ( itMap.data() )
				idList += itMap.key();
		}
		if ( !idList.isEmpty() )
			removePackageIdList( idList );
	}
}

#include "queue.moc"

