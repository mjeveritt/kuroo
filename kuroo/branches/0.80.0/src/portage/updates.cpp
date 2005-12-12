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
#include "scanupdatesjob.h"

#include <kuser.h>

/**
 * Thread for removing single package from updates.
 */
class RemoveUpdatesPackageJob : public ThreadWeaver::DependentJob
{
public:
	RemoveUpdatesPackageJob( QObject *dependent, const QString& package ) : DependentJob( dependent, "DBJob" ), m_package( package ) {}
	
	virtual bool doJob() {
		QString category = m_package.section( "/", 0, 0 );
		QString name = ( m_package.section( "/", 1, 1 ) ).section( pv, 0, 0 );
		QString version = m_package.section( name + "-", 1, 1 );
		
		QString idCategory = KurooDBSingleton::Instance()->query( QString( "SELECT id FROM category WHERE name = '%1';" ).arg( category ) ).first();
		QString idPackage = KurooDBSingleton::Instance()->query( QString( "SELECT id FROM package WHERE name = '%1' AND idCategory = '%2' AND version = '%3';" ).arg( name ).arg( idCategory ).arg( version ) ).first();
		
		KurooDBSingleton::Instance()->query( QString( "UPDATE package SET updateVersion = '' WHERE name = '%1' AND updateVersion = '%2';" ).arg( name ).arg( version ) );
		KurooDBSingleton::Instance()->query( QString( "DELETE FROM updates WHERE idPackage = '%1';" ).arg( idPackage ) );
		return true;
	}
	
	virtual void completeJob() {
		UpdatesSingleton::Instance()->slotChanged();
	}
	
private:
	const QString m_package;
};

/**
 * Object for update packages.
 */
Updates::Updates( QObject *parent )
	: QObject( parent )
{
	connect( SignalistSingleton::Instance(), SIGNAL( signalScanUpdatesComplete() ), this, SLOT( slotLoadUpdates() ) );
	connect( SignalistSingleton::Instance(), SIGNAL( signalLoadUpdatesComplete() ), this, SLOT( slotChanged() ) );
}

Updates::~Updates()
{
}

void Updates::init( QObject *myParent )
{
	parent = myParent;
}

/**
 * Forward signal after a new scan, and updates are loaded into db.
 */
void Updates::slotChanged()
{
	emit signalUpdatesChanged();
}

/**
 * Reset list.
 */
void Updates::slotReset()
{
	KurooDBSingleton::Instance()->query( "UPDATE package SET updateVersion = '' WHERE updateVersion != '';" );
	KurooDBSingleton::Instance()->query( "DELETE FROM updates;" );
	slotChanged();
}

/**
 * Start scan of update packages.
 * @return bool
 */
bool Updates::slotRefresh()
{
	EmergeSingleton::Instance()->checkUpdates();
	return true;
}

/**
 * Start scan of update packages.
 * @return bool
 */
bool Updates::slotLoadUpdates()
{
	SignalistSingleton::Instance()->scanStarted();
	ThreadWeaver::instance()->queueJob( new ScanUpdatesJob( this, EmergeSingleton::Instance()->packageList() ) );
	return true;
}

/**
 * @fixme: Check for failure.
 * Remove packages from db.
 * @param packageIdList
 */
void Updates::removePackage( const QString& package )
{
	ThreadWeaver::instance()->queueJob( new RemoveUpdatesPackageJob( this, package ) );
}

/**
 * Count packages.
 * @return total
 */
QString Updates::count()
{
	return KurooDBSingleton::Instance()->updatesTotal().first();
}

#include "updates.moc"
