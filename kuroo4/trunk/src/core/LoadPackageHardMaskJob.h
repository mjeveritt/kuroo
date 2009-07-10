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

#include <qfileinfo.h>

/**
 * @class: LoadPackageMaskJob
 * @short: Thread for loading masked packages into db.
 */
class LoadPackageHardMaskJob : public ThreadWeaver::DependentJob
{
public:
	LoadPackageHardMaskJob( QObject *dependent ) : DependentJob( dependent, "DBJob" ) {}
	
	virtual bool doJob() {
		
		// Collect all mask dependatoms
		QFileInfo fileInfo( KurooConfig::filePackageHardMask() );
		if( fileInfo.isDir() ) {
			kDebug(0) << KurooConfig::filePackageHardMask() << " is a dir" << LINE_INFO;
			if( !mergeDirIntoFile( KurooConfig::filePackageHardMask() ) ) {
				return false;
			}
		}
		QFile file( KurooConfig::filePackageHardMask() );
		QTextStream stream( &file );
		QStringList linesDependAtom;
		if ( !file.open( QIODevice::ReadOnly ) )
			kError(0) << "Parsing package.mask. Reading: " << KurooConfig::filePackageHardMask() << LINE_INFO;
		else {
			while ( !stream.atEnd() )
				linesDependAtom += stream.readLine();
			file.close();
		}
		
		// Something is wrong, no files found, get outta here
		if ( linesDependAtom.isEmpty() )
			return false;
		
		setStatus( "PackageHardMask", i18n("Collecting hardmasked packages...") );
		
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->singleQuery(	"CREATE TEMP TABLE packageHardMask_temp ( "
		                                    		"idPackage INTEGER, "
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
                    commentLines += (*it).section( "#", 1, 1 ).replace('\'', "''").replace('%', "&#37;").toUtf8();
				}
				else {
					if ( rxAtom.exactMatch( *it ) ) {
						
						// Get the captured strings
						QString category = rxAtom.cap( ThreadWeaver::POS_CATEGORY ) + "-" + rxAtom.cap( ThreadWeaver::POS_SUBCATEGORY );
						QString name = rxAtom.cap( ThreadWeaver::POS_PACKAGE );
						
						QString id = KurooDBSingleton::Instance()->singleQuery( 
							"SELECT id FROM package WHERE name = '" + name + "' AND category = '" + category + "' LIMIT 1;", m_db );
						
						if ( id.isEmpty() )
							kWarning(0) << QString("Parsing package.mask. Can not find id in database for package %1/%2.")
							.arg( category ).arg( name ) << LINE_INFO;
						else
							KurooDBSingleton::Instance()->insert( QString( 
								"INSERT INTO packageHardMask_temp (idPackage, dependAtom, comment) "
								"VALUES ('%1', '%2', '%3');" ).arg( id ).arg( *it ).arg( commentLines.join( "<br>" ) ), m_db );

					}
					else
						kWarning(0) << QString("Parsing package.mask. Can not match package %1 in %2.").arg( *it )
							.arg( KurooConfig::filePackageHardMask() ) << LINE_INFO;
				}
			}
		}
		file.close();
		KurooDBSingleton::Instance()->singleQuery( "COMMIT TRANSACTION;", m_db );
		
		// Move content from temporary table to installedPackages
		KurooDBSingleton::Instance()->singleQuery( "DELETE FROM packageHardMask;", m_db );
		KurooDBSingleton::Instance()->insert( "INSERT INTO packageHardMask SELECT * FROM packageHardMask_temp;", m_db );
		KurooDBSingleton::Instance()->singleQuery( "DROP TABLE packageHardMask_temp;", m_db );
		
		KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
		setStatus( "PackageHardMask", i18n("Done.") );
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( ThreadWeaver::PACKAGE_HARDMASK_SCANNED );
	}
};
