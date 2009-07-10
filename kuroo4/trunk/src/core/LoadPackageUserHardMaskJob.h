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
 * @class: LoadPackageUserMaskJob
 * @short: Thread for loading user masked packages into db.
 */
class LoadPackageUserMaskJob : public ThreadWeaver::DependentJob
{
public:
	LoadPackageUserMaskJob( QObject *dependent ) : DependentJob( dependent, "DBJob" ) {}
	
	virtual bool doJob() {
		
		// Collect all mask dependatoms from /etc/portage/package.mask
		QFileInfo fileInfo( KurooConfig::filePackageUserMask() );
		if( fileInfo.isDir() ) {
			kDebug(0) << KurooConfig::filePackageUserMask() << " is a dir" << LINE_INFO;
			if( !mergeDirIntoFile( KurooConfig::filePackageUserMask() ) ) {
				return false;
			}
		}

		QFile file( KurooConfig::filePackageUserMask() );
		QTextStream stream( &file );
		QStringList linesDependAtom;
		if ( !file.open( QIODevice::ReadOnly ) )
			kError(0) << "Parsing user package.mask. Reading: " << KurooConfig::filePackageUserMask() << LINE_INFO;
		else {
			while ( !stream.atEnd() )
				linesDependAtom += stream.readLine();
			file.close();
		}
		
		// Something is wrong, no files found, get outta here
		if ( linesDependAtom.isEmpty() )
			return false;
		
		setStatus( "PackageUserMask", i18n("Collecting user masked packages...") );
		
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->singleQuery(	"CREATE TEMP TABLE packageUserMask_temp ( "
		                                    		"idPackage INTEGER UNIQUE, "
		                                    		"dependAtom VARCHAR(255), "
		                                    		"comment BLOB );"
		                                    		, m_db);
		
		KurooDBSingleton::Instance()->singleQuery( "BEGIN TRANSACTION;", m_db );
		
		QStringList commentLines;
		for ( QStringList::Iterator it = linesDependAtom.begin(), end = linesDependAtom.end(); it != end; ++it ) {
			
			// Collect comment lines above the dependatom
			if ( (*it).isEmpty() )
				commentLines.clear();
			else {
				if ( (*it).startsWith( "#" ) ) {
                    commentLines += (*it).replace('\'', "''").replace('%', "&#37;").toUtf8();
				}
				else {
					if ( rxAtom.exactMatch( *it ) ) {
						
						// Get the captured strings
						QString category = rxAtom.cap( ThreadWeaver::POS_CATEGORY ) + "-" + rxAtom.cap( ThreadWeaver::POS_SUBCATEGORY );
						QString name = rxAtom.cap( ThreadWeaver::POS_PACKAGE );
						
						QString id = KurooDBSingleton::Instance()->singleQuery( 
							"SELECT id FROM package WHERE name = '" + name + "' AND category = '" + category + "' LIMIT 1;", m_db );
						
						if ( id.isEmpty() )
							kWarning(0) << QString("Parsing user package.mask. Can not find id in database for package %1/%2.")
								.arg( category ).arg( name ) << LINE_INFO;
						else
							KurooDBSingleton::Instance()->insert( QString( 
								"INSERT INTO packageUserMask_temp (idPackage, dependAtom, comment) "
								"VALUES ('%1', '%2', '%3');" ).arg( id ).arg( *it ).arg( commentLines.join( "\n" ) ), m_db );
						
					}
					else
						kWarning(0) << QString("Parsing user package.mask. Can not match package %1 in %2.").arg( *it )
							.arg( KurooConfig::filePackageUserMask() ) << LINE_INFO;
				}
			}
		}
		file.close();
		KurooDBSingleton::Instance()->singleQuery( "COMMIT TRANSACTION;", m_db );
		
		// Move content from temporary table to installedPackages
		KurooDBSingleton::Instance()->singleQuery( "DELETE FROM packageUserMask;", m_db );
		KurooDBSingleton::Instance()->insert( "INSERT INTO packageUserMask SELECT * FROM packageUserMask_temp;", m_db );
		KurooDBSingleton::Instance()->singleQuery( "DROP TABLE packageUserMask_temp;", m_db );
		
		KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
		setStatus( "PackageUserMask", i18n("Done.") );
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( ThreadWeaver::PACKAGE_USER_MASK_SCANNED );
	}
};
