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
#include "packagemask.h"
#include "threadweaver.h"

#include <kdirwatch.h>

// capture positions inside the regexp. (like m_rxAtom.cap(POS_CALLSIGN))
#define POS_CALLSIGN    1
#define POS_PREFIX      2
#define POS_CATEGORY    3
#define POS_SUBCATEGORY 4
#define POS_PACKAGE     5
#define POS_VERSION     6

/**
 * 
 */
class LoadPackageMaskJob : public ThreadWeaver::DependentJob
{
public:
	LoadPackageMaskJob( QObject *dependent ) : DependentJob( dependent, "DBJob" ) {}
	
	virtual bool doJob() {
		QRegExp rxAtom(	"^"    // Start of the string
		               	"(!)?" // "Block these packages" flag, only occurring in ebuilds
		               	"(~|(?:<|>|=|<=|>=))?" // greater-than/less-than/equal, or "all revisions" prefix
		               	"((?:[a-z]|[0-9])+)-((?:[a-z]|[0-9])+)/"   // category and subcategory
		               	"((?:[a-z]|[A-Z]|[0-9]|-|\\+|_)+)" // package name
		               	"("            // start of the version part
		               	"(?:\\*$|-\\d+(?:\\.\\d+)*[a-z]?(?:\\*$)?)" // base version number, including wildcard version matching (*)
		               	"(?:_(?:alpha|beta|pre|rc|p)\\d+)?" // version suffix
		               	"(?:-r\\d+)?"  // revision
		               	")?$"          // end of the (optional) version part and the atom string
		              );
		
		QFile file( KurooConfig::filePackageMask() );
		if ( !file.open( IO_ReadOnly ) ) {
			kdDebug() << i18n("Error reading: %1.").arg( KurooConfig::filePackageMask() ) << endl;
			return false;
		}
		
		QTextStream stream( &file );
		QString name, category;
		QStringList commentLines;
		
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE packageMask_temp ("
											" id INTEGER PRIMARY KEY AUTOINCREMENT, "
											" idPackage INTEGER, "
											" dependAtom VARCHAR(255), "
											" meta VARCHAR(1), "
											" comment BLOB )"
											" ;", m_db);
		
		KurooDBSingleton::Instance()->query( "BEGIN TRANSACTION;", m_db );
		while ( !stream.atEnd() ) {
			QString line( stream.readLine() );
			
			// Collect comment lines above the dependatom
			if ( line.isEmpty() )
				commentLines.clear();
			else {
				if ( line.startsWith( "#" ) ) {
					commentLines += line.replace('\'', "''").replace('%', "&#37;").utf8();
				}
				else {
					if ( rxAtom.exactMatch( line ) ) {

						// Get the captured strings
						name = rxAtom.cap( POS_PACKAGE );
						category = rxAtom.cap( POS_CATEGORY ) + "-" + rxAtom.cap( POS_SUBCATEGORY );
						
						QString idCategory = KurooDBSingleton::Instance()->query( QString( "SELECT id FROM catSubCategory WHERE name = '%1';" ).arg( category ), m_db ).first();
						if ( !idCategory.isEmpty() ) {
							
							// Find id for this package in db
							QString id = KurooDBSingleton::Instance()->query( QString( " SELECT id FROM package WHERE idCatSubCategory = '%1' AND name = '%2' LIMIT 1;" ).arg( idCategory ).arg( name ), m_db ).first();
						
							if ( !id.isEmpty() )
								KurooDBSingleton::Instance()->insert( QString( "INSERT INTO packageMask_temp (idPackage, dependAtom, meta, comment) VALUES ('%1', '%2', '%3', '%4');" ).arg( id ).arg( line ).arg( HARDMASKED_STRING ).arg( commentLines.join( "\n" ) ), m_db );
							else
								kdDebug() << i18n( "Parsing hardmasked packages: Can not find %1/%2 in database." ).arg( category ).arg( name ) << endl;
							
						}
					}
				}
			}
		}
		file.close();
		KurooDBSingleton::Instance()->query( "COMMIT TRANSACTION;", m_db );
		
		// Move content from temporary table to installedPackages
		KurooDBSingleton::Instance()->query( "DELETE FROM packageMask;", m_db );
		KurooDBSingleton::Instance()->insert( "INSERT INTO packageMask SELECT * FROM packageMask_temp;", m_db );
		KurooDBSingleton::Instance()->query( "DROP TABLE packageMask_temp;", m_db );
		
		KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
		return true;
	}
	
	virtual void completeJob() {
		PackageMaskSingleton::Instance()->refresh();
	}
};

/**
 * Object for resulting list of packages from emerge actions.
 */
PackageMask::PackageMask( QObject *parent )
	: QObject( parent )
{
}

PackageMask::~PackageMask()
{
}

void PackageMask::init( QObject *myParent )
{
	parent = myParent;
}

/**
 * Forward signal to refresh results.
 */
void PackageMask::refresh()
{
	isDirtyPackageMask = false;
	LogSingleton::Instance()->writeLog( i18n("\nCompleted scanning for hardmasked packages in %1.").arg( KurooConfig::filePackageMask() ), KUROO );
}


/**
 * 
 */
void PackageMask::loadPackageMask()
{
	kdDebug() << "PackageMask::loadPackageMask" << endl;
	ThreadWeaver::instance()->queueJob( new LoadPackageMaskJob( this ) );
}

void PackageMask::loadPackageUnmask()
{
	kdDebug() << "Not working..." << endl;
// 	ThreadWeaver::instance()->queueJob( new LoadPackageMaskJob( this ) );
}

QStringList PackageMask::getHardMaskedAtom( const QString& id )
{
	return KurooDBSingleton::Instance()->packageHardMaskAtom( id );
}

#include "packagemask.moc"
