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
 * @class RemoveUpdatesPackageJob
 * @short Thread for removing single package from updates.
 */
class RemoveUpdatesPackageJob : public ThreadWeaver::DependentJob
{
public:
	RemoveUpdatesPackageJob( QObject *dependent, const QString& package ) : DependentJob( dependent, "DBJob" ), m_package( package ) {}
	
	virtual bool doJob() {
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		QRegExp rxPackage( "(\\S+)/((?:[a-z]|[A-Z]|[0-9]|-|\\+|_)+)(-(?:\\d+\\.)*\\d+[a-z]?)" );
		QString category, name, version;
		
		if ( rxPackage.search( m_package ) > -1 ) {
			category = rxPackage.cap(1);
			name = rxPackage.cap(2);
			version = m_package.section( name + "-", 1, 1 ).remove(' ');
		}
		else
			kdDebug() << i18n("Removing update package: can not match package %1.").arg( m_package ) << endl;
		
		QString id = KurooDBSingleton::Instance()->singleQuery(
			" SELECT id FROM package WHERE name = '" + name + "' AND idCatSubCategory = "
			" ( SELECT id from catSubCategory WHERE name = '" + category + "' ); ", m_db );
		
		if ( id.isEmpty() ) {
			kdDebug() << i18n("Removing update package: Can not find id in database for package %1/%2.").arg( category ).arg( name ) << endl;
			KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
			return false;
		}
		else {
			KurooDBSingleton::Instance()->query( QString( "UPDATE package SET updateVersion = '' "
			                                              "WHERE name = '%1' AND ( updateVersion = '%2' OR updateVersion = '%3' );"
			                                            ).arg( name ).arg( version + " (D)" ).arg( version + " (U)" ), m_db );
			KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
			return true;
		}
	}
	
	virtual void completeJob() {
		UpdatesSingleton::Instance()->slotChanged();
	}
	
private:
	const QString m_package;
};

/**
 * @class CheckUpdatesPackageJob
 * @short Thread for marking packages as updates or donwgrades.
 */
class CheckUpdatesPackageJob : public ThreadWeaver::DependentJob
{
public:
	CheckUpdatesPackageJob( QObject *dependent, const QString& id, const QString& updateVersion, int hasUpdate ) : DependentJob( dependent, "DBJob" ), 
		m_id( id ), m_updateVersion( updateVersion ), m_hasUpdate( hasUpdate ) {}
	
	virtual bool doJob() {
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		
		QString updateString;
		if ( m_hasUpdate > 0 )
			updateString = m_updateVersion + " (U)";
		else
			if ( m_hasUpdate < 0 )
				updateString = m_updateVersion + " (D)";
				
		KurooDBSingleton::Instance()->query( QString( "UPDATE package SET updateVersion = '%1' WHERE id = '%2';"
				                                            ).arg( updateString ).arg( m_id ), m_db );
		
		KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
		return true;
	}
	
	virtual void completeJob() {
		UpdatesSingleton::Instance()->slotChanged();
	}
	
private:
	const 	QString m_id, m_updateVersion;
	int 	m_hasUpdate;
};

/**
 * @class Updates
 * @short Object for update packages.
 */
Updates::Updates( QObject *m_parent )
	: QObject( m_parent )
{
	connect( SignalistSingleton::Instance(), SIGNAL( signalScanUpdatesComplete() ), this, SLOT( slotLoadUpdates() ) );
	connect( SignalistSingleton::Instance(), SIGNAL( signalLoadUpdatesComplete() ), this, SLOT( slotChanged() ) );
}

Updates::~Updates()
{
}

void Updates::init( QObject *parent )
{
	m_parent = parent;
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
	KurooDBSingleton::Instance()->resetUpdates();
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
 * Update packages when user changes package stability.
 */
void Updates::checkUpdates( const QString& id, const QString& installedVersion, const QString& emergeVersion, int hasUpdate )
{
	ThreadWeaver::instance()->queueJob( new CheckUpdatesPackageJob( this, id, emergeVersion, hasUpdate ) );
}

#include "updates.moc"
