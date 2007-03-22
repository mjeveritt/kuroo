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
#include "scanportagejob.h"
#include "cacheportagejob.h"
#include "scanupdatesjob.h"

#include <kmessagebox.h>

#include <unistd.h>

/**
 * @class CheckUpdatesPackageJob
 * @short Thread for marking packages as updates or downgrades.
 */
class CheckUpdatesPackageJob : public ThreadWeaver::DependentJob
{
public:
	CheckUpdatesPackageJob( QObject *dependent, const QString& id, const QString& updateVersion, int hasUpdate ) : DependentJob( dependent, "DBJob" ), 
		m_id( id ), m_updateVersion( updateVersion ), m_hasUpdate( hasUpdate ) {}
	
	virtual bool doJob() {
		DbConnection* const m_db = KuroolitoDBSingleton::Instance()->getStaticDbConnection();
		
		if ( m_hasUpdate == 0 ) {
			KuroolitoDBSingleton::Instance()->singleQuery( QString("UPDATE package SET updateVersion = '', status = '%1' WHERE id = '%2';")
			                                     		.arg( PACKAGE_INSTALLED_STRING ).arg( m_id ), m_db );
		}
		else {
			QString updateString;
			if ( m_hasUpdate > 0 )
				updateString = m_updateVersion + " (U)";
			else
				updateString = m_updateVersion + " (D)";
			
			KuroolitoDBSingleton::Instance()->singleQuery( QString( "UPDATE package SET updateVersion = '%1', status = '%2' WHERE id = '%3';" )
			                                     		.arg( updateString ).arg( PACKAGE_UPDATES_STRING ).arg( m_id ), m_db );
		}
		
		KuroolitoDBSingleton::Instance()->returnStaticDbConnection( m_db );
		return true;
	}
	
	virtual void completeJob() {
		PortageSingleton::Instance()->slotChanged();
	}
	
private:
	const 	QString m_id, m_updateVersion;
	int 	m_hasUpdate;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @class Portage
 * @short Object handling the Portage tree.
 * 
 * @todo: Register world packages in db.
 */
Portage::Portage( QObject *m_parent )
	: QObject( m_parent )
{
	// When cache scan is done go one scanning portage for all packages
	connect( SignalistSingleton::Instance(), SIGNAL( signalCachePortageComplete() ), this, SLOT( slotScan() ) );
	// Then portage scan is completed
	connect( SignalistSingleton::Instance(), SIGNAL( signalScanPortageComplete() ), this, SLOT( slotScanCompleted() ) );
	
	connect( SignalistSingleton::Instance(), SIGNAL( signalScanUpdatesComplete() ), this, SLOT( slotLoadUpdates() ) );
	connect( SignalistSingleton::Instance(), SIGNAL( signalLoadUpdatesComplete() ), this, SLOT( slotChanged() ) );
}

Portage::~Portage()
{}

void Portage::init( QObject *parent )
{
	m_parent = parent;
	loadWorld();
}

/**
 * Emit signal after changes in Portage.
 */
void Portage::slotChanged()
{
	// Register in db so we can check at next start if user has emerged any packages outside kuroo
	KuroolitoDBSingleton::Instance()->setKuroolitoDbMeta( "scanTimeStamp", QString::number( QDateTime::currentDateTime().toTime_t() ) );
	
	emit signalPortageChanged();
}

/**
 * Reload world when new package is installed/removed in case user not using --oneshot.
 */
void Portage::slotPackageChanged()
{
	loadWorld();
	slotChanged();
}


////////////////////////////////////////////////////////////////////////////////////////
// Portage handling...
////////////////////////////////////////////////////////////////////////////////////////

/**
 * Start scan of portage packages.
 * @return bool
 */
bool Portage::slotRefresh()
{
	// Update cache if empty
	if ( KuroolitoDBSingleton::Instance()->isCacheEmpty() ) {
		SignalistSingleton::Instance()->scanStarted();
		ThreadWeaver::instance()->queueJob( new CachePortageJob( this ) );
	}
	else
		slotScan();
	
	return true;
}

/**
 * Continue with scan of portage packages.
 * @return bool
 */
bool Portage::slotScan()
{
	DEBUG_LINE_INFO;
	
	// Wait for cache job to finish before launching the scan.
	int maxLoops( 99 );
	while ( true ) {
		if ( KuroolitoDBSingleton::Instance()->isCacheEmpty() )
			::usleep( 100000 ); // Sleep 100 msec
		else
			break;
		
		if ( maxLoops-- == 0 ) {
			kdWarning(0) << "Scanning Portage. Wait-counter has reached maximum. Attempting to scan Portage." << LINE_INFO;
			break;
		}
	}
	
	SignalistSingleton::Instance()->scanStarted();
	ThreadWeaver::instance()->queueJob( new ScanPortageJob( this ) );
	
	return true;
}

/**
 * After portage has completed scanning for all packages, check for updates.
 */
void Portage::slotScanCompleted()
{
	// Now all Portage files
	PortageFilesSingleton::Instance()->loadPackageFiles();
	
	// Ready to roll!
	SignalistSingleton::Instance()->setKuroolitoReady( true );
	slotChanged();
	
	// Go on with checking for updates
	if ( KuroolitoDBSingleton::Instance()->isUpdatesEmpty() )
		slotRefreshUpdates();
}


////////////////////////////////////////////////////////////////////////////////////////
// World handling...
////////////////////////////////////////////////////////////////////////////////////////

/**
 * Load packages in world file. @fixme: optimize this...
 */
void Portage::loadWorld()
{
	m_mapWorld.clear();
	
	QFile file( KuroolitoConfig::fileWorld() );
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		while ( !stream.atEnd() ) {
			QString package = stream.readLine();
			m_mapWorld[ package.stripWhiteSpace() ] = QString::null;
		}
		emit signalWorldChanged();
	}
	else
		kdError(0) << "Loading packages in world. Reading: " << KuroolitoConfig::fileWorld() << LINE_INFO;
}

/**
 * Check if this package in is world file.
 * @param package
 * @return true/false
 */
bool Portage::isInWorld( const QString& package )
{
	return m_mapWorld.contains( package );
}


////////////////////////////////////////////////////////////////////////////////////////
// Package handlling...
////////////////////////////////////////////////////////////////////////////////////////


/**
 * Start scan of update packages.
 * @return bool
 */
bool Portage::slotRefreshUpdates()
{
	EmergeSingleton::Instance()->checkUpdates();
	return true;
}

/**
 * Start scan of update packages.
 * @return bool
 */
bool Portage::slotLoadUpdates()
{
	SignalistSingleton::Instance()->scanStarted();
	ThreadWeaver::instance()->queueJob( new ScanUpdatesJob( this, EmergeSingleton::Instance()->packageList() ) );
	return true;
}

/**
 * Update packages when user changes package stability.
 */
void Portage::checkUpdates( const QString& id, const QString& emergeVersion, int hasUpdate )
{
	ThreadWeaver::instance()->queueJob( new CheckUpdatesPackageJob( this, id, emergeVersion, hasUpdate ) );
}

#include "portage.moc"
