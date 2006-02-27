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

/**
 * @class AddInstalledPackageJob
 * @short Thread for adding packages into installed in db.
 */
class AddInstalledPackageJob : public ThreadWeaver::DependentJob
{
public:
	AddInstalledPackageJob( QObject *dependent, const QString& package ) : DependentJob( dependent, "DBJob" ), m_package( package ) {}
	
	virtual bool doJob() {
		QRegExp rxPackage( "(\\S+)/((?:[a-z]|[A-Z]|[0-9]|-|\\+|_)+)(-(?:\\d+\\.)*\\d+[a-z]?)" );
		QString category, name, version;
		
		if ( rxPackage.search( m_package ) > -1 ) {
			category = rxPackage.cap(1);
			name = rxPackage.cap(2);
			version = m_package.section( name + "-", 1, 1 ).remove(' ');
		}
		else
			kdDebug() << i18n("Inserting emerged package: can not match %1.").arg( m_package ) << endl;
		
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		QString id = KurooDBSingleton::Instance()->singleQuery( 
			" SELECT id FROM package WHERE name = '" + name + "' AND idCatSubCategory = "
			" ( SELECT id from catSubCategory WHERE name = '" + category + "' ); ", m_db);
		
		if ( id.isEmpty() ) {
			kdDebug() << i18n("Inserting emerged package: Can not find id in database for package %1/%2.").arg( category ).arg( name ) << endl;
			KurooDBSingleton::Instance()->returnStaticDbConnection(m_db);
			return false;
		}
		else {
			KurooDBSingleton::Instance()->query( QString( "UPDATE package SET status = '%1' WHERE id = '%2'"
			                                              ";" ).arg( FILTER_INSTALLED_STRING ).arg( id ), m_db );
			KurooDBSingleton::Instance()->query( QString( " UPDATE version SET status = '%1' WHERE idPackage = '%2' AND name = '%3'"
			                                              ";" ).arg( FILTER_INSTALLED_STRING ).arg( id ).arg( version ), m_db );
			KurooDBSingleton::Instance()->returnStaticDbConnection(m_db);
			return true;
		}
	}
	
	virtual void completeJob() {
		InstalledSingleton::Instance()->slotChanged();
	}
	
private:
	const QString m_package;
};

/**
 * @class RemoveInstalledPackageJob
 * @short Thread for removing packages from installed in db.
 */
class RemoveInstalledPackageJob : public ThreadWeaver::DependentJob
{
public:
	RemoveInstalledPackageJob( QObject *dependent, const QString& package ) : DependentJob( dependent, "DBJob" ), m_package( package ) {}
	
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
			kdDebug() << i18n("Removing unmerged package: can not match %1.").arg( m_package ) << endl;

		QString id = KurooDBSingleton::Instance()->singleQuery( 
			" SELECT id FROM package WHERE name = '" + name + "' AND idCatSubCategory = "
			" ( SELECT id from catSubCategory WHERE name = '" + category + "' ); ", m_db);
		
		if ( id.isEmpty() ) {
			kdDebug() << i18n("Remove unmerged package: Can not find id in database for package %1/%2.").arg( category ).arg( name ) << endl;
			KurooDBSingleton::Instance()->returnStaticDbConnection(m_db);
			return false;
		}
		else {
			
			// Mark package as uninstalled or remove it if old
			KurooDBSingleton::Instance()->query( QString(
			                        "UPDATE package SET status = '%1' "
			                        "WHERE status = '%2' AND id = '%3'").arg( FILTER_ALL_STRING ).arg( FILTER_INSTALLED_STRING ).arg( id ), m_db );
			KurooDBSingleton::Instance()->query( QString(
									"DELETE FROM package "
			                        "WHERE status = '%1' AND id = '%2';" ).arg( FILTER_OLD_STRING ).arg( id ), m_db );
			KurooDBSingleton::Instance()->query( QString(
									"UPDATE version SET status = '%1' "
									"WHERE idPackage = '%2' AND name = '%3';" ).arg( FILTER_ALL_STRING ).arg( id ).arg( version ), m_db );
			KurooDBSingleton::Instance()->returnStaticDbConnection(m_db);
			return true;
		}
	}
	
	virtual void completeJob() {
		InstalledSingleton::Instance()->slotChanged();
	}
	
private:
	const QString m_package;
};

/**
 * @class Installed
 * @short Object for installed packages.
 */
Installed::Installed( QObject *m_parent )
	: QObject( m_parent )
{
}

Installed::~Installed()
{
}

void Installed::init( QObject *parent )
{
	m_parent = parent;
}

/**
 * Forward signal after a new scan.
 */
void Installed::slotChanged()
{
	PortageSingleton::Instance()->loadWorld();
	emit signalInstalledChanged();
}

/**
 * Clear packages.
 */
void Installed::slotReset()
{
	KurooDBSingleton::Instance()->resetInstalled();
	emit signalInstalledReset();
}

/**
 * Launch unmerge of packages
 * @param category
 * @param packageList
 */
void Installed::uninstallPackageList( const QStringList& packageIdList )
{
	QStringList packageList;
	foreach ( packageIdList )
		packageList += KurooDBSingleton::Instance()->category( *it ) + "/" + KurooDBSingleton::Instance()->package( *it );
	
	EmergeSingleton::Instance()->unmerge( packageList );
}

/**
 * @fixme: Check for failure.
 * Add package as installed in db.
 * @param package
 */
void Installed::addPackage( const QString& package )
{
	ThreadWeaver::instance()->queueJob( new AddInstalledPackageJob( this, package ) );
}

/**
 * @fixme: Check for failure.
 * Remove packages from db.
 * @param packageIdList
 */
void Installed::removePackage( const QString& package )
{
	ThreadWeaver::instance()->queueJob( new RemoveInstalledPackageJob( this, package ) );
}

#include "installed.moc"
