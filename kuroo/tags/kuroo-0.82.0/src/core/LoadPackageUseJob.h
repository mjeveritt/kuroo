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
#include "portagefiles.h"
#include "threadweaver.h"

/**
 * @class: LoadPackageUseJob
 * @short: Thread for loading packages use into db.
 */
class LoadPackageUseJob : public ThreadWeaver::DependentJob
{
public:
	LoadPackageUseJob( QObject *dependent ) : DependentJob( dependent, "DBJob" ) {}
	
	virtual bool doJob() {
		
		QFileInfo fileInfo( KurooConfig::filePackageUserUse() );
		if( fileInfo.isDir() ) {
			kdDebug(0) << KurooConfig::filePackageUserUse() << " is a dir" << LINE_INFO;
			if( !mergeDirIntoFile( KurooConfig::filePackageUserUse() ) ) {
				return false;
			}
		}

		QFile file( KurooConfig::filePackageUserUse() );
		QTextStream stream( &file );
		QStringList linesUse;
		if ( !file.open( IO_ReadOnly ) )
			kdError(0) << "Parsing user package.use. Reading: %1." << KurooConfig::filePackageUserUse() << LINE_INFO;
		else {
			while ( !stream.atEnd() )
				linesUse += stream.readLine();
			file.close();
		}
		
		// Something is wrong, no files found, get outta here
		if ( linesUse.isEmpty() )
			return false;
		
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->singleQuery(	"CREATE TEMP TABLE packageUse_temp ( "
		                                    		"idPackage INTEGER UNIQUE, "
		                                    		"use VARCHAR(255) );"
		                                    		, m_db);
		
		KurooDBSingleton::Instance()->singleQuery( "BEGIN TRANSACTION;", m_db );
		
		for ( QStringList::Iterator it = linesUse.begin(), end = linesUse.end(); it != end; ++it ) {
			if( !(*it).stripWhiteSpace().startsWith( "#" ) && !(*it).stripWhiteSpace().isEmpty() ) {
				QString category = (*it).section( '/', 0, 0 );
				QString name = ( (*it).section( '/', 1 ) ).section( ' ', 0, 0 );
				QString use = (*it).section( ' ', 1 );
				use.simplifyWhiteSpace();
				
				QString id = KurooDBSingleton::Instance()->singleQuery( 
					"SELECT id FROM package WHERE name = '" + name + "' AND category = '" + category + "' LIMIT 1;", m_db );
				
				if ( id.isEmpty() )
					kdWarning(0) << QString("Parsing user package.use. Can not find id in database for package %1/%2.")
						.arg( category ).arg( name ) << LINE_INFO;
				else
					KurooDBSingleton::Instance()->insert( QString( "INSERT INTO packageUse_temp (idPackage, use) VALUES ('%1', '%2');" )
									.arg( id ).arg( use ), m_db );
			}
		}
		file.close();
		KurooDBSingleton::Instance()->singleQuery( "COMMIT TRANSACTION;", m_db );
		
		// Move content from temporary table to installedPackages
		KurooDBSingleton::Instance()->singleQuery( "DELETE FROM packageUse;", m_db );
		KurooDBSingleton::Instance()->insert( "INSERT INTO packageUse SELECT * FROM packageUse_temp;", m_db );
		KurooDBSingleton::Instance()->singleQuery( "DROP TABLE packageUse_temp;", m_db );
		
		KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( ThreadWeaver::PACKAGE_USER_USE_SCANNED );
	}
};
