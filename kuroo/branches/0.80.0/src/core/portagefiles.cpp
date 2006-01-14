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

// For more info on DEPEND atoms, see the DEPEND Atoms section of man 5 ebuild

// The complete atom regexp in non-escaped form (for testing, or similar):
// ^(!)?(~|(?:<|>|=|<=|>=))?((?:[a-z]|[0-9])+)-((?:[a-z]|[0-9])+)/((?:[a-z]|[A-Z]|[0-9]|-|\+|_)+)((?:\*$|-\d+(?:\.\d+)*[a-z]?(?:\*$)?)(?:_(?:alpha|beta|pre|rc|p)\d+)?(?:-r\d+)?)?$

QRegExp
rxAtom(	"^"    // Start of the string
       	"(!)?" // "Block these packages" flag, only occurring in ebuilds
       	"(~|(?:<|>|=|<=|>=))?" // greater-than/less-than/equal, or "all revisions" prefix
       	"((?:[a-z]|[0-9])+)-((?:[a-z]|[0-9])+)/"   // category and subcategory
       	"((?:[a-z]|[A-Z]|[0-9]|-|\\+|_)+)" // package name
       	"("            // start of the version part
       	"(?:\\*$|-\\d+(?:\\.\\d+)*[a-z]?(?:\\*$)?)" // base version number, including wildcard version matching (*)
       	"(?:_(?:alpha|beta|pre|rc|p)(\\d+|\\*$))?" // version suffix
       	"(?:-r(\\d+|\\*$))?"  // revision
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
		
		setStatus( "PackageKeywords", i18n("Collecting user package keywords...") );
		
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE packageKeywords_temp ("
		                                    " id INTEGER PRIMARY KEY AUTOINCREMENT, "
		                                    " idPackage INTEGER UNIQUE, "
		                                    " keywords VARCHAR(255) )"
		                                    " ;", m_db);
		
		KurooDBSingleton::Instance()->query( "BEGIN TRANSACTION;", m_db );
		
		for ( QStringList::Iterator it = linesPackage.begin(), end = linesPackage.end(); it != end; ++it ) {
			
			// set the atom string
			QStringList tokens = QStringList::split( ' ', *it );
			QString package = tokens[0];
			
			if ( !(*it).isEmpty() && rxAtom.exactMatch( package ) ) {
					
				// Get the captured strings
				QString category = rxAtom.cap( POS_CATEGORY ) + "-" + rxAtom.cap( POS_SUBCATEGORY );
				QString name = rxAtom.cap( POS_PACKAGE );
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

				QString id = KurooDBSingleton::Instance()->packageId( category, name );
				if ( !id.isEmpty() )
					KurooDBSingleton::Instance()->insert( QString( 
						"INSERT INTO packageKeywords_temp (idPackage, keywords) "
						"VALUES ('%1', '%2');" ).arg( id ).arg( keywords ), m_db );
			}
			else
				kdDebug() << i18n("Can not match package %1 in %2.").arg( *it ).arg( KurooConfig::filePackageKeywords() ) << endl;
			
		}
		file.close();
		KurooDBSingleton::Instance()->query( "COMMIT TRANSACTION;", m_db );
		
		// Move content from temporary table to installedPackages
		KurooDBSingleton::Instance()->query( "DELETE FROM packageKeywords;", m_db );
		KurooDBSingleton::Instance()->insert( "INSERT INTO packageKeywords SELECT * FROM packageKeywords_temp;", m_db );
		KurooDBSingleton::Instance()->query( "DROP TABLE packageKeywords_temp;", m_db );
		
		KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
		setStatus( "PackageKeywords", i18n("Done.") );
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( 0 );
	}
};

/**
 * @class: LoadPackageUserUnMaskJob
 * @short: Thread for loading packages unmasked by user.
 */
class LoadPackageUserUnMaskJob : public ThreadWeaver::DependentJob
{
public:
	LoadPackageUserUnMaskJob( QObject *dependent ) : DependentJob( dependent, "DBJob" ) {}
	
	virtual bool doJob() {
		
		// Collect all unmask dependatoms
		QFile file( KurooConfig::filePackageUserUnMask() );
		QTextStream stream( &file );
		QStringList linesDependAtom;
		if ( !file.open( IO_ReadOnly ) ) {
			kdDebug() << i18n("Error reading: %1.").arg( KurooConfig::filePackageUserUnMask() ) << endl;
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
		
		setStatus( "PackageUserUnMask", i18n("Collecting user unmasked packages...") );
		
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE packageUnmask_temp ("
											" id INTEGER PRIMARY KEY AUTOINCREMENT, "
											" idPackage INTEGER UNIQUE, "
											" dependAtom VARCHAR(255), "
											" comment BLOB )"
											" ;", m_db);
		
		KurooDBSingleton::Instance()->query( "BEGIN TRANSACTION;", m_db );
		
		QStringList commentLines;
		for ( QStringList::Iterator it = linesDependAtom.begin(), end = linesDependAtom.end(); it != end; ++it ) {
			
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
						QString category = rxAtom.cap( POS_CATEGORY ) + "-" + rxAtom.cap( POS_SUBCATEGORY );
						QString name = rxAtom.cap( POS_PACKAGE );
						
						QString id = KurooDBSingleton::Instance()->packageId( category, name );
						if ( !id.isEmpty() )
							KurooDBSingleton::Instance()->insert( QString( "INSERT INTO packageUnmask_temp (idPackage, dependAtom, comment) VALUES ('%1', '%2', '%3');" ).arg( id ).arg( *it ).arg( commentLines.join( "\n" ) ), m_db );
						
					}
					else
						kdDebug() << i18n("Can not match package %1 in %2.").arg( *it ).arg( KurooConfig::filePackageUserUnMask() ) << endl;
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
		setStatus( "PackageUserUnMask", i18n("Done.") );
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
class LoadPackageHardMaskJob : public ThreadWeaver::DependentJob
{
public:
	LoadPackageHardMaskJob( QObject *dependent ) : DependentJob( dependent, "DBJob" ) {}
	
	virtual bool doJob() {
		
		// Collect all mask dependatoms
		QFile file( KurooConfig::filePackageHardMask() );
		QTextStream stream( &file );
		QStringList linesDependAtom;
		if ( !file.open( IO_ReadOnly ) ) {
			kdDebug() << i18n("Error reading: %1.").arg( KurooConfig::filePackageHardMask() ) << endl;
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
		
		setStatus( "PackageHardMask", i18n("Collecting hardmasked packages...") );
		
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE packageHardMask_temp ("
		                                    " id INTEGER PRIMARY KEY AUTOINCREMENT, "
		                                    " idPackage INTEGER, "
		                                    " dependAtom VARCHAR(255), "
		                                    " comment BLOB )"
		                                    " ;", m_db);
		
		KurooDBSingleton::Instance()->query( "BEGIN TRANSACTION;", m_db );
		
		QStringList commentLines;
		for ( QStringList::Iterator it = linesDependAtom.begin(), end = linesDependAtom.end(); it != end; ++it ) {
			
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
						QString category = rxAtom.cap( POS_CATEGORY ) + "-" + rxAtom.cap( POS_SUBCATEGORY );
						QString name = rxAtom.cap( POS_PACKAGE );
						
						QString id = KurooDBSingleton::Instance()->packageId( category, name );
						if ( !id.isEmpty() )
							KurooDBSingleton::Instance()->insert( QString( "INSERT INTO packageHardMask_temp (idPackage, dependAtom, comment) VALUES ('%1', '%2', '%3');" ).arg( id ).arg( *it ).arg( commentLines.join( "\n" ) ), m_db );

					}
					else
						kdDebug() << i18n("Can not match package %1 in %2.").arg( *it ).arg( KurooConfig::filePackageHardMask() ) << endl;
				}
			}
		}
		file.close();
		KurooDBSingleton::Instance()->query( "COMMIT TRANSACTION;", m_db );
		
		// Move content from temporary table to installedPackages
		KurooDBSingleton::Instance()->query( "DELETE FROM packageHardMask;", m_db );
		KurooDBSingleton::Instance()->insert( "INSERT INTO packageHardMask SELECT * FROM packageHardMask_temp;", m_db );
		KurooDBSingleton::Instance()->query( "DROP TABLE packageHardMask_temp;", m_db );
		
		KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
		setStatus( "PackageHardMask", i18n("Done.") );
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( 2 );
	}
};

/**
 * @class: LoadPackageMaskJob
 * @short: Thread for loading masked packages into db.
 */
class LoadPackageUserMaskJob : public ThreadWeaver::DependentJob
{
public:
LoadPackageUserMaskJob( QObject *dependent ) : DependentJob( dependent, "DBJob" ) {}
	
	virtual bool doJob() {
		
		// Collect all mask dependatoms from /etc/portage/package.mask
		QFile file( KurooConfig::filePackageUserMask() );
		QTextStream stream( &file );
		QStringList linesDependAtom;
		if ( !file.open( IO_ReadOnly ) ) {
			kdDebug() << i18n("Error reading: %1.").arg( KurooConfig::filePackageUserMask() ) << endl;
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
		
		setStatus( "PackageUserMask", i18n("Collecting user masked packages...") );
		
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE packageUserMask_temp ("
		                                    " id INTEGER PRIMARY KEY AUTOINCREMENT, "
		                                    " idPackage INTEGER UNIQUE, "
		                                    " dependAtom VARCHAR(255), "
		                                    " comment BLOB )"
		                                    " ;", m_db);
		
		KurooDBSingleton::Instance()->query( "BEGIN TRANSACTION;", m_db );
		
		QStringList commentLines;
		for ( QStringList::Iterator it = linesDependAtom.begin(), end = linesDependAtom.end(); it != end; ++it ) {
			
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
						QString category = rxAtom.cap( POS_CATEGORY ) + "-" + rxAtom.cap( POS_SUBCATEGORY );
						QString name = rxAtom.cap( POS_PACKAGE );
						
						QString id = KurooDBSingleton::Instance()->packageId( category, name );
						if ( !id.isEmpty() )
							KurooDBSingleton::Instance()->insert( QString( "INSERT INTO packageUserMask_temp (idPackage, dependAtom, comment) VALUES ('%1', '%2', '%3');" ).arg( id ).arg( *it ).arg( commentLines.join( "\n" ) ), m_db );
						
					}
					else
						kdDebug() << i18n("Can not match package %1 in %2.").arg( *it ).arg( KurooConfig::filePackageUserMask() ) << endl;
				}
			}
		}
		file.close();
		KurooDBSingleton::Instance()->query( "COMMIT TRANSACTION;", m_db );
		
		// Move content from temporary table to installedPackages
		KurooDBSingleton::Instance()->query( "DELETE FROM packageUserMask;", m_db );
		KurooDBSingleton::Instance()->insert( "INSERT INTO packageUserMask SELECT * FROM packageUserMask_temp;", m_db );
		KurooDBSingleton::Instance()->query( "DROP TABLE packageUserMask_temp;", m_db );
		
		KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
		setStatus( "PackageUserMask", i18n("Done.") );
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( 3 );
	}
};

/**
 * @class: SavePackageKeywordsJob
 * @short: Thread for loading packages unmasked by user.
 */
class SavePackageKeywordsJob : public ThreadWeaver::DependentJob
{
public:
	SavePackageKeywordsJob( QObject *dependent ) : DependentJob( dependent, "DBJob1" ) {}
	
	virtual bool doJob() {
		
		const QStringList lines = KurooDBSingleton::Instance()->query( 
			"SELECT catSubCategory.name, package.name, packageKeywords.keywords FROM package, catSubCategory, packageKeywords "
			" WHERE catSubCategory.id = package.idCatSubCategory"
			" AND package.id = packageKeywords.idPackage "
			";" );
		if ( lines.isEmpty() ) {
			kdDebug() << i18n("No package keywords found. Saving to %1 aborted!").arg( KurooConfig::filePackageKeywords() ) << endl;
			return false;
		}
		
		QFile file( KurooConfig::filePackageKeywords() );
		QTextStream stream( &file );
		if ( !file.open( IO_WriteOnly ) ) {
			kdDebug() << i18n("Error writing: %1.").arg( KurooConfig::filePackageKeywords() ) << endl;
			return false;
		}
		
		foreach ( lines ) {
			QString category = *it++;
			QString package = *it++;
			QString keywords = *it;
			if ( !package.isEmpty() )
				stream << category << "/" << package << " " << keywords << "\n";
		}
		
		file.close();
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( 4 );
	}
};

/**
 * @class: SavePackageUserMaskJob
 * @short: Thread for loading packages unmasked by user.
 */
class SavePackageUserMaskJob : public ThreadWeaver::DependentJob
{
public:
	SavePackageUserMaskJob( QObject *dependent ) : DependentJob( dependent, "DBJob2" ) {}
	
	virtual bool doJob() {
		
		const QStringList lines = KurooDBSingleton::Instance()->query( "SELECT dependAtom FROM packageUserMask ;" );
		if ( lines.isEmpty() ) {
			kdDebug() << i18n("No user mask depend atom found. Saving to %1 aborted!").arg( KurooConfig::filePackageUserMask() ) << endl;
			return false;
		}
		
		QFile file( KurooConfig::filePackageUserMask() );
		QTextStream stream( &file );
		if ( !file.open( IO_WriteOnly ) ) {
			kdDebug() << i18n("Error writing: %1.").arg( KurooConfig::filePackageUserMask() ) << endl;
			return false;
		}
		
		foreach ( lines )
				stream << *it << "\n";
		
		file.close();
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( 5 );
	}
};

/**
 * @class: SavePackageUserMaskJob
 * @short: Thread for loading packages unmasked by user.
 */
class SavePackageUserUnMaskJob : public ThreadWeaver::DependentJob
{
public:
	SavePackageUserUnMaskJob( QObject *dependent ) : DependentJob( dependent, "DBJob3" ) {}
	
	virtual bool doJob() {
		
		const QStringList lines = KurooDBSingleton::Instance()->query( "SELECT dependAtom FROM packageUnMask ;" );
		if ( lines.isEmpty() ) {
			kdDebug() << i18n("No user unmask depend atom found. Saving to %1 aborted!").arg( KurooConfig::filePackageUserUnMask() ) << endl;
			return false;
		}
		
		QFile file( KurooConfig::filePackageUserUnMask() );
		QTextStream stream( &file );
		if ( !file.open( IO_WriteOnly ) ) {
		kdDebug() << i18n("Error writing: %1.").arg( KurooConfig::filePackageUserUnMask() ) << endl;
			return false;
		}
		
		foreach ( lines )
			stream << *it << "\n";
		
		file.close();
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( 6 );
	}
};

/**
 * @class: LoadPackageMaskJob
 * @short: Thread for loading masked packages into db.
 */
class LoadPackageUseJob : public ThreadWeaver::DependentJob
{
public:
	LoadPackageUseJob( QObject *dependent ) : DependentJob( dependent, "DBJob" ) {}
	
	virtual bool doJob() {
		
		QFile file( KurooConfig::filePackageUse() );
		QTextStream stream( &file );
		QStringList linesUse;
		if ( !file.open( IO_ReadOnly ) ) {
		kdDebug() << i18n("Error reading: %1.").arg( KurooConfig::filePackageUse() ) << endl;
		}
		else {
			while ( !stream.atEnd() ) {
				linesUse += stream.readLine();
			}
			file.close();
		}
		
		// Something is wrong, no files found, get outta here
		if ( linesUse.isEmpty() )
			return false;
		
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE packageUse_temp ("
		                                    " id INTEGER PRIMARY KEY AUTOINCREMENT, "
		                                    " idPackage INTEGER UNIQUE, "
		                                    " use VARCHAR(255) "
		                                    " );", m_db);
		
		KurooDBSingleton::Instance()->query( "BEGIN TRANSACTION;", m_db );
		
		for ( QStringList::Iterator it = linesUse.begin(), end = linesUse.end(); it != end; ++it ) {
			QString category = (*it).section( '/', 0, 0 );
			QString name = ( (*it).section( '/', 1 ) ).section( ' ', 0, 0 );
			QString use = (*it).section( ' ', 1 );
			use.simplifyWhiteSpace();
			
			QString id = KurooDBSingleton::Instance()->packageId( category, name );
			if ( !id.isEmpty() )
				KurooDBSingleton::Instance()->insert( QString( "INSERT INTO packageUse_temp (idPackage, use) VALUES ('%1', '%2');" ).arg( id ).arg( use ), m_db );
			
		}
		file.close();
		KurooDBSingleton::Instance()->query( "COMMIT TRANSACTION;", m_db );
		
		// Move content from temporary table to installedPackages
		KurooDBSingleton::Instance()->query( "DELETE FROM packageUse;", m_db );
		KurooDBSingleton::Instance()->insert( "INSERT INTO packageUse SELECT * FROM packageUse_temp;", m_db );
		KurooDBSingleton::Instance()->query( "DROP TABLE packageUse_temp;", m_db );
		
		KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( 7 );
	}
};

/**
 * @class: SavePackageUserMaskJob
 * @short: Thread for loading packages unmasked by user.
 */
class SavePackageUseJob : public ThreadWeaver::DependentJob
{
public:
	SavePackageUseJob( QObject *dependent ) : DependentJob( dependent, "DBJob3" ) {}
	
	virtual bool doJob() {
		
		const QStringList lines = KurooDBSingleton::Instance()->query( 
			"SELECT catSubCategory.name, package.name, packageUse.use FROM package, catSubCategory, packageUse "
			" WHERE catSubCategory.id = package.idCatSubCategory"
			" AND package.id = packageUse.idPackage "
			";" );
		if ( lines.isEmpty() ) {
			kdDebug() << i18n("No package use found. Saving to %1 aborted!").arg( KurooConfig::filePackageUse() ) << endl;
			return false;
		}
		
		QFile file( KurooConfig::filePackageUse() );
		QTextStream stream( &file );
		if ( !file.open( IO_WriteOnly ) ) {
			kdDebug() << i18n("Error writing: %1.").arg( KurooConfig::filePackageUse() ) << endl;
			return false;
		}
		
		foreach ( lines ) {
			QString category = *it++;
			QString package = *it++;
			QString use = *it;
			stream << category << "/" << package << " " << use << "\n";
		}
		
		file.close();
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( 8 );
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
			break;
		case 1:
			kdDebug() << i18n("Completed scanning for unmasked packages in %1.").arg( KurooConfig::filePackageUserUnMask() ) << endl;
			break;
		case 2:
			kdDebug() << i18n("Completed scanning for hardmasked packages in %1.").arg( KurooConfig::filePackageHardMask() ) << endl;
			break;
		case 3:
			kdDebug() << i18n("Completed scanning for user masked packages in %1.").arg( KurooConfig::filePackageUserMask() ) << endl;
			break;
		case 4:
			kdDebug() << i18n("Completed saving package keywords in %1.").arg( KurooConfig::filePackageKeywords() ) << endl;
			break;
		case 5:
			kdDebug() << i18n("Completed saving unmasked packages in %1.").arg( KurooConfig::filePackageUserUnMask() ) << endl;
			break;
		case 6:
			kdDebug() << i18n("Completed saving unmasked packages in %1.").arg( KurooConfig::filePackageUserUnMask() ) << endl;
			break;
		case 7:
			kdDebug() << i18n("Completed scanning user package use flags in %1.").arg( KurooConfig::filePackageUse() ) << endl;
			break;
		case 8:
			kdDebug() << i18n("Completed saving user package use in %1.").arg( KurooConfig::filePackageUse() ) << endl;
	}
}


/**
 * 
 */
void PortageFiles::loadPackageMask()
{
	ThreadWeaver::instance()->queueJob( new LoadPackageHardMaskJob( this ) );
	ThreadWeaver::instance()->queueJob( new LoadPackageUserMaskJob( this ) );
	ThreadWeaver::instance()->queueJob( new LoadPackageUserUnMaskJob( this ) );
	ThreadWeaver::instance()->queueJob( new LoadPackageKeywordsJob( this ) );
	ThreadWeaver::instance()->queueJob( new LoadPackageUseJob( this ) );
}

void PortageFiles::loadPackageKeywords()
{
	ThreadWeaver::instance()->queueJob( new LoadPackageKeywordsJob( this ) );
}

void PortageFiles::loadPackageUnmask()
{
	ThreadWeaver::instance()->queueJob( new LoadPackageUserUnMaskJob( this ) );
}

void PortageFiles::loadPackageUserMask()
{
	ThreadWeaver::instance()->queueJob( new LoadPackageUserMaskJob( this ) );
}

void PortageFiles::loadPackageUse()
{
	ThreadWeaver::instance()->queueJob( new LoadPackageUseJob( this ) );
}

void PortageFiles::savePackageKeywords()
{
	ThreadWeaver::instance()->queueJob( new SavePackageKeywordsJob( this ) );
}

void PortageFiles::savePackageUserUnMask()
{
	ThreadWeaver::instance()->queueJob( new SavePackageUserUnMaskJob( this ) );
}

void PortageFiles::savePackageUserMask()
{
	ThreadWeaver::instance()->queueJob( new SavePackageUserMaskJob( this ) );
}

void PortageFiles::savePackageUse()
{
	ThreadWeaver::instance()->queueJob( new SavePackageUseJob( this ) );
}


#include "portagefiles.moc"
