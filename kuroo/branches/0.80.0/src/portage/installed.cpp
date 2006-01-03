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
 * Thread for adding packages into installed in db.
 */
class AddInstalledPackageJob : public ThreadWeaver::DependentJob
{
public:
	AddInstalledPackageJob( QObject *dependent, const QString& package ) : DependentJob( dependent, "DBJob" ), m_package( package ) {}
	
	virtual bool doJob() {
		QString category = m_package.section( "/", 0, 0);
		QString name = ( m_package.section( "/", 1, 1) ).section( rxPortageVersion, 0, 0 );
		QString version = m_package.section( name + "-", 1, 1 );
		
		QString id = KurooDBSingleton::Instance()->packageId( category, name );
		
		if ( !id.isEmpty() ) {
			KurooDBSingleton::Instance()->query( QString( "UPDATE package SET meta = '%1' WHERE id = '%2';" ).arg( FILTERINSTALLED_STRING ).arg( id ) );
			KurooDBSingleton::Instance()->query( QString( "UPDATE version SET meta = '%1' WHERE idPackage = '%2' AND name = '%3';" ).arg( FILTERINSTALLED_STRING ).arg( id ).arg( version ) );
			return true;
		}
		else
			kdDebug() << i18n("Adding installed: Package not found!") << endl;
		
		return false;
	}
	
	virtual void completeJob() {
		InstalledSingleton::Instance()->slotChanged();
	}
	
private:
	const QString m_package;
};

/**
 * Thread for removing packages from installed in db.
 */
class RemoveInstalledPackageJob : public ThreadWeaver::DependentJob
{
public:
	RemoveInstalledPackageJob( QObject *dependent, const QString& package ) : DependentJob( dependent, "DBJob" ), m_package( package ) {}
	
	virtual bool doJob() {
		QString category = m_package.section( "/", 0, 0 );
		QString name = ( m_package.section( "/", 1, 1 ) ).section( rxPortageVersion, 0, 0 );
		QString version = m_package.section( name + "-", 1, 1 );
			
		QString id = KurooDBSingleton::Instance()->packageId( category, name );
		if ( !id.isEmpty() ) {
			
			// Mark package as uninstalled or remove it if old
			KurooDBSingleton::Instance()->query( QString( "UPDATE package SET meta = '%1' WHERE meta = '%2' AND id = '%3'").arg( FILTERALL_STRING ).arg( FILTERINSTALLED_STRING ).arg( id ) );
	
			KurooDBSingleton::Instance()->query( QString( "DELETE FROM package WHERE meta = '%1' AND id = '%2';" ).arg( FILTEROLD_STRING ).arg( id ) );
	
			KurooDBSingleton::Instance()->query( QString( "UPDATE version SET meta = '%1' WHERE idPackage = '%2' AND name = '%3';" ).arg( FILTERALL_STRING ).arg( id ).arg( version ) );
			
			// Remove package from world file
			QFile file( KurooConfig::dirWorldFile() );
			QStringList lines;
			if ( file.open( IO_ReadOnly ) ) {
				QTextStream stream( &file );
				while ( !stream.atEnd() )
					lines += stream.readLine();
				file.close();
				
				if ( file.open( IO_WriteOnly ) ) {
					QTextStream stream( &file );
					foreach ( lines ) {
						if ( *it != ( category + "/" + name ) )
							stream << *it << endl;
					}
					file.close();
				}
				else
					kdDebug() << i18n("Error writing: ") << KurooConfig::dirWorldFile() << endl;
			}
			else
				kdDebug() << i18n("Error reading: ") << KurooConfig::dirWorldFile() << endl;
			
			return true;
		}
		else
			kdDebug() << i18n("Removing installed: No package found!") << endl;

		
		return false;
	}
	
	virtual void completeJob() {
		InstalledSingleton::Instance()->slotChanged();
	}
	
private:
	const QString m_package;
};

/**
 * Object for installed packages.
 */
Installed::Installed( QObject *parent )
	: QObject( parent )
{
}

Installed::~Installed()
{
}

void Installed::init( QObject *myParent )
{
	parent = myParent;
}

/**
 * Forward signal after a new scan.
 */
void Installed::slotChanged()
{
	kdDebug() << "Installed::slotChanged" << endl;
	emit signalInstalledChanged();
}

/**
 * Clear packages.
 */
void Installed::slotReset()
{
	KurooDBSingleton::Instance()->query( QString( "UPDATE package set installed = '%1';" ).arg( FILTERALL_STRING ) );
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
	foreach ( packageIdList ) {
		packageList += PortageSingleton::Instance()->category( *it ) + "/" + PortageSingleton::Instance()->package( *it );
	}
	
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
