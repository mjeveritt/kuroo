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
#include "results.h"
#include "threadweaver.h"

#include <qvaluestack.h>

/**
 * Thread for adding packages to results in db. Used by emerge.
 */
class AddResultsPackageListJob : public ThreadWeaver::DependentJob
{
public:
	AddResultsPackageListJob( QObject *dependent, const EmergePackageList &packageList ) : DependentJob( dependent, "DBJob" ), m_packageList( packageList ) {}
	
	virtual bool doJob() {
		EmergePackageList::ConstIterator itEnd = ( m_packageList ).end();
		for ( EmergePackageList::ConstIterator it = m_packageList.begin(); it != itEnd; ++it ) {
			
			QString id = KurooDBSingleton::Instance()->packageId( (*it).category, (*it).name );
			if ( !id.isEmpty() ) {
				int idPackage = KurooDBSingleton::Instance()->insert( QString( "INSERT INTO queue (idPackage, idDepend) VALUES ('%1', '0');" ).arg( id ) );
				if ( idPackage == 0 ) {

					// Insert dependency packages if any
					while ( !dependencyPackages.isEmpty() ) {
						KurooDBSingleton::Instance()->insert( QString( 
							"INSERT INTO queue (idPackage, idDepend) "
							"VALUES ('%1', '%2');" ).arg( dependencyPackages.pop() ).arg( QString::number(idPackage) ) );
					}
				}
				else { // We have a dependency
					dependencyPackages.push( id );
				}
			}
		}
		return true;
	}
	
	virtual void completeJob() {
		QueueSingleton::Instance()->refresh();
	}
	
private:
	const EmergePackageList m_packageList;
	QValueStack<QString> dependencyPackages;
};

/**
 * Object for resulting list of packages from emerge actions.
 */
Results::Results( QObject *parent )
	: QObject( parent )
{
}

Results::~Results()
{
}

void Results::init( QObject *myParent )
{
	parent = myParent;
}

/**
 * Forward signal to refresh results.
 */
void Results::refresh()
{
	emit signalResultsChanged();
}

/**
 * Add packages to the results table in the db
 * @param packageList
 */
void Results::addPackageList( const EmergePackageList &packageList )
{
	if ( !packageList.isEmpty() )
		ThreadWeaver::instance()->queueJob( new AddResultsPackageListJob( this, packageList ) );
}

/**
 * Get list of all packages.
 * @return package list
 */
QStringList Results::allPackages()
{
	return KurooDBSingleton::Instance()->allResultPackages();
}

#include "results.moc"
