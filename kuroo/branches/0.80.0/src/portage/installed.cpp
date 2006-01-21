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
		
// 		QString id = KurooDBSingleton::Instance()->packageId( category, name );
		QString id = KurooDBSingleton::Instance()->query( 
			" SELECT package.id FROM package, catSubCategory WHERE "
			" package.name = '" + name + "' AND catSubCategory.name = '" + category + "' "
			" AND catSubCategory.id = package.idCatSubCategory; ").first();
		
		if ( id.isEmpty() )
			kdDebug() << i18n("AddInstalledPackageJob: Can not find id in database for package %1/%2.").arg( category ).arg( name ) << endl;
		else {
			KurooDBSingleton::Instance()->query( QString( "UPDATE package SET meta = '%1' "
			                                              "WHERE id = '%2';" ).arg( FILTER_INSTALLED_STRING ).arg( id ) );
			KurooDBSingleton::Instance()->query( QString( "UPDATE version SET meta = '%1' "
			                                              "WHERE idPackage = '%2' AND name = '%3';" ).arg( FILTER_INSTALLED_STRING ).arg( id ).arg( version ) );
			return true;
		}
		return false;
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
		QRegExp rxPackage( "(\\S+)/((?:[a-z]|[A-Z]|[0-9]|-|\\+|_)+)(-(?:\\d+\\.)*\\d+[a-z]?)" );
		QString category, name, version;
		
		if ( rxPackage.search( m_package ) > -1 ) {
			category = rxPackage.cap(1);
			name = rxPackage.cap(2);
			version = m_package.section( name + "-", 1, 1 ).remove(' ');
		}
		else
			kdDebug() << i18n("Removing unmerged package: can not match %1.").arg( m_package ) << endl;

// 		QString id = KurooDBSingleton::Instance()->packageId( category, name );
		QString id = KurooDBSingleton::Instance()->query( 
			" SELECT package.id FROM package, catSubCategory WHERE "
			" package.name = '" + name + "' AND catSubCategory.name = '" + category + "' "
			" AND catSubCategory.id = package.idCatSubCategory; ").first();
		
		if ( id.isEmpty() )
			kdDebug() << i18n("RemoveInstalledPackageJob: Can not find id in database for package %1/%2.").arg( category ).arg( name ) << endl;
		else {
			
			// Mark package as uninstalled or remove it if old
			KurooDBSingleton::Instance()->query( QString( "UPDATE package SET meta = '%1' "
			                                              "WHERE meta = '%2' AND id = '%3'").arg( FILTER_ALL_STRING ).arg( FILTER_INSTALLED_STRING ).arg( id ) );
			KurooDBSingleton::Instance()->query( QString( "DELETE FROM package "
			                                              "WHERE meta = '%1' AND id = '%2';" ).arg( FILTER_OLD_STRING ).arg( id ) );
			KurooDBSingleton::Instance()->query( QString( "UPDATE version SET meta = '%1' "
			                                              "WHERE idPackage = '%2' AND name = '%3';" ).arg( FILTER_ALL_STRING ).arg( id ).arg( version ) );
			
			return true;
		}
		
		return false;
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
	emit signalInstalledChanged();
}

/**
 * Clear packages.
 */
void Installed::slotReset()
{
	KurooDBSingleton::Instance()->query( QString( "UPDATE package set installed = '%1';" ).arg( FILTER_ALL_STRING ) );
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
	kdDebug() << "Installed::addPackage package=" << package << endl;
	ThreadWeaver::instance()->queueJob( new AddInstalledPackageJob( this, package ) );
}

/**
 * @fixme: Check for failure.
 * Remove packages from db.
 * @param packageIdList
 */
void Installed::removePackage( const QString& package )
{
	kdDebug() << "Installed::removePackage package=" << package << endl;
	ThreadWeaver::instance()->queueJob( new RemoveInstalledPackageJob( this, package ) );
}

#include "installed.moc"
