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

// capture positions inside the regexp. (like m_rxAtom.cap(POS_CALLSIGN))
#define POS_CALLSIGN    1
#define POS_PREFIX      2
#define POS_CATEGORY    3
#define POS_SUBCATEGORY 4
#define POS_PACKAGE     5
#define POS_VERSION     6

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

/**
 * @class: LoadPackageKeywordsJob
 * @short: Thread for loading packages unmasked by user.
 */
class LoadPackageKeywordsJob : public ThreadWeaver::DependentJob
{
public:
	LoadPackageKeywordsJob( QObject *dependent ) : DependentJob( dependent, "DBJob" ) {}
	
	virtual bool doJob() {
		
		// Collect all mask dependatoms
		QFile file( KurooConfig::filePackageKeywords() );
		QTextStream stream( &file );
		QStringList linesPackage;
		if ( !file.open( IO_ReadOnly ) ) {
			kdDebug() << i18n("Error reading: %1.").arg( KurooConfig::filePackageKeywords() ) << endl;
		}
		else {
			while ( !stream.atEnd() ) {
				linesPackage += stream.readLine();
			}
			file.close();
		}
		
		// Something is wrong, no files found, get outta here
		if ( linesPackage.isEmpty() )
			return false;
		
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE packageKeywords_temp ("
		                                    " id INTEGER PRIMARY KEY AUTOINCREMENT, "
		                                    " idPackage INTEGER UNIQUE, "
		                                    " keywords VARCHAR(255) )"
		                                    " ;", m_db);
		
		KurooDBSingleton::Instance()->query( "BEGIN TRANSACTION;", m_db );
		
		for ( QStringList::Iterator it = linesPackage.begin(); it != linesPackage.end(); ++it ) {
			
			// set the atom string
			QStringList tokens = QStringList::split( ' ', *it );
			QString package = tokens[0];
			
			// Collect comment lines above the dependatom
			if ( !(*it).isEmpty() && rxAtom.exactMatch( package ) ) {
					
				// Get the captured strings
				QString name = rxAtom.cap( POS_PACKAGE );
				QString category = rxAtom.cap( POS_CATEGORY ) + "-" + rxAtom.cap( POS_SUBCATEGORY );
				QString keywords;
				
				// extract this line's keywords
				QStringList::iterator tokenIterator = tokens.begin();
				tokenIterator++;
				while ( tokenIterator != tokens.end() ) {
					keywords += *tokenIterator + " ";
					tokenIterator++;
				}
				if ( keywords.isEmpty() ) {
					keywords = "~*"; // in fact, it would be: m_keywords.prepend("~" + arch), but anyways
				}
				
				QString idCategory = KurooDBSingleton::Instance()->query( QString( "SELECT id FROM catSubCategory WHERE name = '%1';" ).arg( category ), m_db ).first();
				if ( !idCategory.isEmpty() ) {
					
					// Find id for this package in db
					QString id = KurooDBSingleton::Instance()->query( QString( " SELECT id FROM package WHERE idCatSubCategory = '%1' AND name = '%2' LIMIT 1;" ).arg( idCategory ).arg( name ), m_db ).first();
					
					if ( !id.isEmpty() )
						KurooDBSingleton::Instance()->insert( QString( "INSERT INTO packageKeywords_temp (idPackage, keywords) VALUES ('%1', '%2');" ).arg( id ).arg( keywords ), m_db );
					else
						kdDebug() << i18n( "Parsing package keywords: Can not find %1/%2 in database." ).arg( category ).arg( name ) << endl;
					
				}
			}
		}
		file.close();
		KurooDBSingleton::Instance()->query( "COMMIT TRANSACTION;", m_db );
		
		// Move content from temporary table to installedPackages
		KurooDBSingleton::Instance()->query( "DELETE FROM packageKeywords;", m_db );
		KurooDBSingleton::Instance()->insert( "INSERT INTO packageKeywords SELECT * FROM packageKeywords_temp;", m_db );
		KurooDBSingleton::Instance()->query( "DROP TABLE packageKeywords_temp;", m_db );
		
		KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( 0 );
	}
};

/**
 * @class: LoadPackageUnmaskJob
 * @short: Thread for loading packages unmasked by user.
 */
class LoadPackageUnmaskJob : public ThreadWeaver::DependentJob
{
public:
	LoadPackageUnmaskJob( QObject *dependent ) : DependentJob( dependent, "DBJob" ) {}
	
	virtual bool doJob() {
		
		// Collect all unmask dependatoms
		QFile file( KurooConfig::filePackageUnmask() );
		QTextStream stream( &file );
		QStringList linesDependAtom;
		if ( !file.open( IO_ReadOnly ) ) {
			kdDebug() << i18n("Error reading: %1.").arg( KurooConfig::filePackageUnmask() ) << endl;
		}
		else {
			while ( !stream.atEnd() ) {
				linesDependAtom += stream.readLine();
			}
			file.close();
		}
		
		// Something is wrong, no files found, get outta here
		if ( linesDependAtom.isEmpty() )
			return false;
		
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE packageUnmask_temp ("
											" id INTEGER PRIMARY KEY AUTOINCREMENT, "
											" idPackage INTEGER, "
											" dependAtom VARCHAR(255), "
											" comment BLOB )"
											" ;", m_db);
		
		KurooDBSingleton::Instance()->query( "BEGIN TRANSACTION;", m_db );
		
		QStringList commentLines;
		for ( QStringList::Iterator it = linesDependAtom.begin(); it != linesDependAtom.end(); ++it ) {
			
			// Collect comment lines above the dependatom
			if ( (*it).isEmpty() )
				commentLines.clear();
			else {
				if ( (*it).startsWith( "#" ) ) {
					commentLines += (*it).replace('\'', "''").replace('%', "&#37;").utf8();
				}
				else {
					if ( rxAtom.exactMatch( *it ) ) {

						// Get the captured strings
						QString name = rxAtom.cap( POS_PACKAGE );
						QString category = rxAtom.cap( POS_CATEGORY ) + "-" + rxAtom.cap( POS_SUBCATEGORY );
						
						QString idCategory = KurooDBSingleton::Instance()->query( QString( "SELECT id FROM catSubCategory WHERE name = '%1';" ).arg( category ), m_db ).first();
						if ( !idCategory.isEmpty() ) {
							
							// Find id for this package in db
							QString id = KurooDBSingleton::Instance()->query( QString( " SELECT id FROM package WHERE idCatSubCategory = '%1' AND name = '%2' LIMIT 1;" ).arg( idCategory ).arg( name ), m_db ).first();
						
							if ( !id.isEmpty() )
								KurooDBSingleton::Instance()->insert( QString( "INSERT INTO packageUnmask_temp (idPackage, dependAtom, comment) VALUES ('%1', '%2', '%3');" ).arg( id ).arg( *it ).arg( commentLines.join( "\n" ) ), m_db );
							else
								kdDebug() << i18n( "Parsing unmasked packages: Can not find %1/%2 in database." ).arg( category ).arg( name ) << endl;
							
						}
					}
				}
			}
		}
		file.close();
		KurooDBSingleton::Instance()->query( "COMMIT TRANSACTION;", m_db );
		
		// Move content from temporary table to installedPackages
		KurooDBSingleton::Instance()->query( "DELETE FROM packageUnmask;", m_db );
		KurooDBSingleton::Instance()->insert( "INSERT INTO packageUnmask SELECT * FROM packageUnmask_temp;", m_db );
		KurooDBSingleton::Instance()->query( "DROP TABLE packageUnmask_temp;", m_db );
		
		KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( 1 );
	}
};

/**
 * @class: LoadPackageMaskJob
 * @short: Thread for loading masked packages into db.
 */
class LoadPackageMaskJob : public ThreadWeaver::DependentJob
{
public:
	LoadPackageMaskJob( QObject *dependent ) : DependentJob( dependent, "DBJob" ) {}
	
	virtual bool doJob() {
		
		// Collect all mask dependatoms
		// First in Gentoo package.mask
		QFile file( KurooConfig::filePackageMask() );
		QTextStream stream( &file );
		QStringList linesDependAtom;
		if ( !file.open( IO_ReadOnly ) ) {
			kdDebug() << i18n("Error reading: %1.").arg( KurooConfig::filePackageMask() ) << endl;
		}
		else {
			while ( !stream.atEnd() ) {
				linesDependAtom += stream.readLine();
			}
			file.close();
		}
		
		// Append users own from /etc/portage/package.mask
		file.setName( KurooConfig::filePackageMaskUser() );
		if ( !file.open( IO_ReadOnly ) ) {
			kdDebug() << i18n("Error reading: %1.").arg( KurooConfig::filePackageMaskUser() ) << endl;
		}
		else {
			while ( !stream.atEnd() ) {
				QString line = stream.readLine();
				if ( !line.isEmpty() && !line.startsWith( "#" ) ) {
					linesDependAtom += i18n("\n#Masked by user.");
					linesDependAtom += line;
				}
			}
			file.close();
		}
		
		// Something is wrong, no files found, get outta here
		if ( linesDependAtom.isEmpty() )
			return false;
		
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE packageMask_temp ("
		                                    " id INTEGER PRIMARY KEY AUTOINCREMENT, "
		                                    " idPackage INTEGER, "
		                                    " dependAtom VARCHAR(255), "
		                                    " comment BLOB )"
		                                    " ;", m_db);
		
		KurooDBSingleton::Instance()->query( "BEGIN TRANSACTION;", m_db );
		
		QStringList commentLines;
		for ( QStringList::Iterator it = linesDependAtom.begin(); it != linesDependAtom.end(); ++it ) {
			
			// Collect comment lines above the dependatom
			if ( (*it).isEmpty() )
				commentLines.clear();
			else {
				if ( (*it).startsWith( "#" ) ) {
					commentLines += (*it).replace('\'', "''").replace('%', "&#37;").utf8();
				}
				else {
					if ( rxAtom.exactMatch( *it ) ) {
						
						// Get the captured strings
						QString name = rxAtom.cap( POS_PACKAGE );
						QString category = rxAtom.cap( POS_CATEGORY ) + "-" + rxAtom.cap( POS_SUBCATEGORY );
						
						QString idCategory = KurooDBSingleton::Instance()->query( QString( "SELECT id FROM catSubCategory WHERE name = '%1';" ).arg( category ), m_db ).first();
						if ( !idCategory.isEmpty() ) {
							
							// Find id for this package in db
							QString id = KurooDBSingleton::Instance()->query( QString( " SELECT id FROM package WHERE idCatSubCategory = '%1' AND name = '%2' LIMIT 1;" ).arg( idCategory ).arg( name ), m_db ).first();
							
							if ( !id.isEmpty() )
								KurooDBSingleton::Instance()->insert( QString( "INSERT INTO packageMask_temp (idPackage, dependAtom, comment) VALUES ('%1', '%2', '%3');" ).arg( id ).arg( *it ).arg( commentLines.join( "\n" ) ), m_db );
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
		PortageFilesSingleton::Instance()->refresh( 2 );
	}
};

/**
 * Object for resulting list of packages from emerge actions.
 */
PortageFiles::PortageFiles( QObject *parent )
	: QObject( parent )
{
}

PortageFiles::~PortageFiles()
{
}

void PortageFiles::init( QObject *myParent )
{
	parent = myParent;
}

/**
 * Forward signal to refresh results.
 */
void PortageFiles::refresh( int mask )
{
	switch ( mask ) {
		case 0:
			kdDebug() << i18n("Completed scanning for package keywords in %1.").arg( KurooConfig::filePackageKeywords() ) << endl;
		case 1:
			kdDebug() << i18n("Completed scanning for unmasked packages in %1.").arg( KurooConfig::filePackageUnmask() ) << endl;
		case 2:
			kdDebug() << i18n("Completed scanning for hardmasked packages in %1.").arg( KurooConfig::filePackageMask() ) << endl;
	}
}


/**
 * 
 */
void PortageFiles::loadPackageMask()
{
	kdDebug() << "PortageFiles::loadPackageMask" << endl;
	ThreadWeaver::instance()->queueJob( new LoadPackageMaskJob( this ) );
	ThreadWeaver::instance()->queueJob( new LoadPackageUnmaskJob( this ) );
	ThreadWeaver::instance()->queueJob( new LoadPackageKeywordsJob( this ) );
}

void PortageFiles::loadPackageUnmask()
{
	kdDebug() << "PortageFiles::loadPackageUnmask" << endl;
	ThreadWeaver::instance()->queueJob( new LoadPackageUnmaskJob( this ) );
}

void PortageFiles::loadPackageKeywords()
{
	kdDebug() << "PortageFiles::loadPackageKeywords" << endl;
	ThreadWeaver::instance()->queueJob( new LoadPackageKeywordsJob( this ) );
}

QStringList PortageFiles::getHardMaskedAtom( const QString& id )
{
	return KurooDBSingleton::Instance()->packageHardMaskAtom( id );
}

QStringList PortageFiles::getUnmaskedAtom( const QString& id )
{
	return KurooDBSingleton::Instance()->packageUnmaskAtom( id );
}

QStringList PortageFiles::getKeywordsAtom( const QString& id )
{
	return KurooDBSingleton::Instance()->packageKeywordsAtom( id );
}

#include "portagefiles.moc"
