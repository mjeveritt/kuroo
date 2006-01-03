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

/**
 * Thread for adding packages to results in db. Used by emerge.
 */
class AddResultsPackageListJob : public ThreadWeaver::DependentJob
{
public:
	AddResultsPackageListJob( QObject *dependent, const EmergePackageList &packageList ) : DependentJob( dependent, "DBJob" ), m_packageList( packageList ) {}
	
	virtual bool doJob() {
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE results_temp ("
		                                    " id INTEGER PRIMARY KEY AUTOINCREMENT, "
		                                    " idPackage INTEGER UNIQUE, "
		                                    " package VARCHAR(64), "
		                                    " size VARCHAR(32), "
		                                    " use VARCHAR(255), "
		                                    " flags VARCHAR(32))"
		                                    " ;", m_db);
		
		KurooDBSingleton::Instance()->query( "BEGIN TRANSACTION;", m_db );
		EmergePackageList::ConstIterator itEnd = ( m_packageList ).end();
		for ( EmergePackageList::ConstIterator it = m_packageList.begin(); it != itEnd; ++it ) {
			QString id = KurooDBSingleton::Instance()->packageId( (*it).category, (*it).name );
			if ( !id.isEmpty() )
				KurooDBSingleton::Instance()->insert( QString( "INSERT INTO results_temp (idPackage, package, size, use, flags) VALUES ('%1', '%2', '%3', '%4', '%5');" ).arg( id ).arg( (*it).package ).arg( (*it).size ).arg( (*it).useFlags ).arg( (*it).updateFlags ), m_db );
		}
		KurooDBSingleton::Instance()->query( "COMMIT TRANSACTION;", m_db );
		
		// Move content from temporary table to installedPackages
		KurooDBSingleton::Instance()->query( "DELETE FROM results;", m_db );
		KurooDBSingleton::Instance()->insert( "INSERT INTO results SELECT * FROM results_temp;", m_db );
		KurooDBSingleton::Instance()->query( "DROP TABLE results_temp;", m_db );
		
		KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
		return true;
	}
	
	virtual void completeJob() {
		ResultsSingleton::Instance()->refresh();
	}
	
private:
	const EmergePackageList m_packageList;
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
