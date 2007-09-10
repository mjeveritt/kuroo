/**************************************************************************
*   Copyright (C) 2007 by Karye                                           *
*   info@kuroo.org                                                        *
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
enum Positions {
		POS_CALLSIGN = 1,
		POS_PREFIX,
		POS_CATEGORY,
		POS_SUBCATEGORY,
		POS_PACKAGE,
		POS_VERSION
};

enum PortageFilesAction {
		PACKAGE_KEYWORDS_SCANNED,
		PACKAGE_HARDMASK_SCANNED,
		PACKAGE_USER_MASK_SCANNED,
		PACKAGE_USER_UNMASK_SCANNED,
		PACKAGE_USER_USE_SCANNED,
		PACKAGE_KEYWORDS_SAVED,
		PACKAGE_USER_MASK_SAVED,
		PACKAGE_USER_UNMASK_SAVED,
		PACKAGE_USER_USE_SAVED
};

QRegExp
rxAtom(	
       	"^"    															// Start of the string
       	"(!)?" 															// "Block these packages" flag, only occurring in ebuilds
       	"(~|(?:<|>|=|<=|>=))?" 											// greater-than/less-than/equal, or "all revisions" prefix
       	"((?:[a-z]|[0-9])+)-((?:[a-z]|[0-9])+)/"   						// category and subcategory
       	"((?:[a-z]|[A-Z]|[0-9]|-(?=\\d+dpi)|-(?!\\d)|\\+|_)+)" 			// package name
       	"("           													// start of the version part
       	"(?:-\\d*(?:\\.\\d+)*[a-z]?)" 									// base version number, including wildcard version matching (*)
       	"(?:_(?:alpha|beta|pre|rc|p)\\d*)?" 							// version suffix
       	"(?:-r\\d*)?"  													// revision
       	"\\*?)?$"          												// end of the (optional) version part and the atom string
      );

/**
 * @class: LoadPackageKeywordsJob
 * @short: Thread for loading packages keyword-unmasked by user.
 */
class LoadPackageKeywordsJob : public ThreadWeaver::DependentJob
{
public:
	LoadPackageKeywordsJob( QObject *dependent ) : DependentJob( dependent, "DBJob" ) {}
	
	virtual bool doJob() {
		
		// Collect all mask dependatoms
		QFile file( KuroolitoConfig::filePackageKeywords() );
		QTextStream stream( &file );
		QStringList linesPackage;
		if ( !file.open( IO_ReadOnly ) )
			kdWarning(0) << "Parsing package.keywords. Reading: " << KuroolitoConfig::filePackageKeywords() << LINE_INFO;
		else {
			while ( !stream.atEnd() )
				linesPackage += stream.readLine();
			file.close();
		}
		
		// Something is wrong, no files found, get outta here
		if ( linesPackage.isEmpty() )
			return false;
		
		setStatus( "PackageKeywords", i18n("Collecting user package keywords...") );
		
		DbConnection* const m_db = KuroolitoDBSingleton::Instance()->getStaticDbConnection();
		KuroolitoDBSingleton::Instance()->singleQuery(	"CREATE TEMP TABLE packageKeywords_temp ( "
		                                    		"idPackage INTEGER UNIQUE, keywords VARCHAR(255) );", m_db);
		
		KuroolitoDBSingleton::Instance()->singleQuery( "BEGIN TRANSACTION;", m_db );
		
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
				if ( keywords.isEmpty() )
    				keywords = "~" + KuroolitoConfig::arch();

				QString id = KuroolitoDBSingleton::Instance()->singleQuery( 
					"SELECT id FROM package WHERE name = '" + name + "' AND category = '" + category + "' LIMIT 1;", m_db );
				
				if ( id.isEmpty() )
					kdWarning(0) << QString("Load package keywords: Can not find id in database for package %1/%2.")
					.arg( category ).arg( name ) << LINE_INFO;
				else
					KuroolitoDBSingleton::Instance()->insert( QString( 
						"INSERT INTO packageKeywords_temp (idPackage, keywords) VALUES ('%1', '%2');" )
					                                      .arg( id ).arg( keywords ), m_db );
			}
			else
				kdWarning(0) << QString("Parsing package.keywords. Can not match package %1 in %2.").arg( *it )
					.arg( KuroolitoConfig::filePackageKeywords() ) << LINE_INFO;
			
		}
		file.close();
		KuroolitoDBSingleton::Instance()->singleQuery( "COMMIT TRANSACTION;", m_db );
		
		// Move content from temporary table to installedPackages
		KuroolitoDBSingleton::Instance()->singleQuery( "DELETE FROM packageKeywords;", m_db );
		KuroolitoDBSingleton::Instance()->insert( "INSERT INTO packageKeywords SELECT * FROM packageKeywords_temp;", m_db );
		KuroolitoDBSingleton::Instance()->singleQuery( "DROP TABLE packageKeywords_temp;", m_db );
		
		KuroolitoDBSingleton::Instance()->returnStaticDbConnection( m_db );
		setStatus( "PackageKeywords", i18n("Done.") );
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( PACKAGE_KEYWORDS_SCANNED );
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
		QFile file( KuroolitoConfig::filePackageUserUnMask() );
		QTextStream stream( &file );
		QStringList linesDependAtom;
		if ( !file.open( IO_ReadOnly ) )
			kdError(0) << "Parsing package.unmask. Reading: " << KuroolitoConfig::filePackageUserUnMask() << LINE_INFO;
		else {
			while ( !stream.atEnd() )
				linesDependAtom += stream.readLine();
			file.close();
		}
		
		// Something is wrong, no files found, get outta here
		if ( linesDependAtom.isEmpty() )
			return false;
		
		setStatus( "PackageUserUnMask", i18n("Collecting user unmasked packages...") );
		
		DbConnection* const m_db = KuroolitoDBSingleton::Instance()->getStaticDbConnection();
		KuroolitoDBSingleton::Instance()->singleQuery(	"CREATE TEMP TABLE packageUnmask_temp ( "
													"idPackage INTEGER UNIQUE, dependAtom VARCHAR(255), comment BLOB );"
													, m_db);
		
		KuroolitoDBSingleton::Instance()->singleQuery( "BEGIN TRANSACTION;", m_db );
		
		QStringList commentLines;
		for ( QStringList::Iterator it = linesDependAtom.begin(), end = linesDependAtom.end(); it != end; ++it ) {
			
			// Collect comment lines above the dependatom
			if ( (*it).isEmpty() )
				commentLines.clear();
			else {
				if ( (*it).startsWith( "#" ) ) {
					continue;
				}
				else {
					if ( rxAtom.exactMatch( *it ) ) {

						// Get the captured strings
						QString category = rxAtom.cap( POS_CATEGORY ) + "-" + rxAtom.cap( POS_SUBCATEGORY );
						QString name = rxAtom.cap( POS_PACKAGE );
						
						QString id = KuroolitoDBSingleton::Instance()->singleQuery( 
							"SELECT id FROM package WHERE name = '" + name + "' AND category = '" + category + "' LIMIT 1;", m_db );
						
						if ( id.isEmpty() )
							kdWarning(0) << QString("Load user package unmask: Can not find id in database for package %1/%2.")
							.arg( category ).arg( name ) << LINE_INFO;
						else
							KuroolitoDBSingleton::Instance()->insert( QString( 
								"INSERT INTO packageUnmask_temp (idPackage, dependAtom) "
								"VALUES ('%1', '%2');" ).arg( id ).arg( *it ), m_db );
						
					}
					else
						kdWarning(0) << QString("Parsing package.unmask. Can not match package %1 in %2.").arg( *it )
							.arg( KuroolitoConfig::filePackageUserUnMask() ) << LINE_INFO;
				}
			}
		}
		file.close();
		KuroolitoDBSingleton::Instance()->singleQuery( "COMMIT TRANSACTION;", m_db );
		
		// Move content from temporary table to installedPackages
		KuroolitoDBSingleton::Instance()->singleQuery( "DELETE FROM packageUnmask;", m_db );
		KuroolitoDBSingleton::Instance()->insert( "INSERT INTO packageUnmask SELECT * FROM packageUnmask_temp;", m_db );
		KuroolitoDBSingleton::Instance()->singleQuery( "DROP TABLE packageUnmask_temp;", m_db );
		
		KuroolitoDBSingleton::Instance()->returnStaticDbConnection( m_db );
		setStatus( "PackageUserUnMask", i18n("Done.") );
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( PACKAGE_USER_UNMASK_SCANNED );
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
		QFile file( KuroolitoConfig::filePackageHardMask() );
		QTextStream stream( &file );
		QStringList linesDependAtom;
		if ( !file.open( IO_ReadOnly ) )
			kdError(0) << "Parsing package.mask. Reading: " << KuroolitoConfig::filePackageHardMask() << LINE_INFO;
		else {
			while ( !stream.atEnd() )
				linesDependAtom += stream.readLine();
			file.close();
		}
		
		// Something is wrong, no files found, get outta here
		if ( linesDependAtom.isEmpty() )
			return false;
		
		setStatus( "PackageHardMask", i18n("Collecting hardmasked packages...") );
		
		DbConnection* const m_db = KuroolitoDBSingleton::Instance()->getStaticDbConnection();
		KuroolitoDBSingleton::Instance()->singleQuery(	"CREATE TEMP TABLE packageHardMask_temp ( "
		                                    		"idPackage INTEGER, dependAtom VARCHAR(255), comment BLOB );"
		                                    		, m_db);
		
		KuroolitoDBSingleton::Instance()->singleQuery( "BEGIN TRANSACTION;", m_db );
		
		QStringList commentLines;
		for ( QStringList::Iterator it = linesDependAtom.begin(), end = linesDependAtom.end(); it != end; ++it ) {
			
			// Collect comment lines above the dependatom
			if ( (*it).isEmpty() )
				commentLines.clear();
			else {
				if ( (*it).startsWith( "#" ) ) {
					continue;
				}
				else {
					if ( rxAtom.exactMatch( *it ) ) {
						
						// Get the captured strings
						QString category = rxAtom.cap( POS_CATEGORY ) + "-" + rxAtom.cap( POS_SUBCATEGORY );
						QString name = rxAtom.cap( POS_PACKAGE );
						
						QString id = KuroolitoDBSingleton::Instance()->singleQuery( 
							"SELECT id FROM package WHERE name = '" + name + "' AND category = '" + category + "' LIMIT 1;", m_db );
						
						if ( id.isEmpty() )
							kdWarning(0) << QString("Parsing package.mask. Can not find id in database for package %1/%2.")
							.arg( category ).arg( name ) << LINE_INFO;
						else
							KuroolitoDBSingleton::Instance()->insert( QString( 
								"INSERT INTO packageHardMask_temp (idPackage, dependAtom) "
								"VALUES ('%1', '%2');" ).arg( id ).arg( *it ), m_db );

					}
					else
						kdWarning(0) << QString("Parsing package.mask. Can not match package %1 in %2.").arg( *it )
							.arg( KuroolitoConfig::filePackageHardMask() ) << LINE_INFO;
				}
			}
		}
		file.close();
		KuroolitoDBSingleton::Instance()->singleQuery( "COMMIT TRANSACTION;", m_db );
		
		// Move content from temporary table to installedPackages
		KuroolitoDBSingleton::Instance()->singleQuery( "DELETE FROM packageHardMask;", m_db );
		KuroolitoDBSingleton::Instance()->insert( "INSERT INTO packageHardMask SELECT * FROM packageHardMask_temp;", m_db );
		KuroolitoDBSingleton::Instance()->singleQuery( "DROP TABLE packageHardMask_temp;", m_db );
		
		KuroolitoDBSingleton::Instance()->returnStaticDbConnection( m_db );
		setStatus( "PackageHardMask", i18n("Done.") );
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( PACKAGE_HARDMASK_SCANNED );
	}
};

/**
 * @class: LoadPackageMaskJob
 * @short: Thread for loading user masked packages into db.
 */
class LoadPackageUserMaskJob : public ThreadWeaver::DependentJob
{
public:
	LoadPackageUserMaskJob( QObject *dependent ) : DependentJob( dependent, "DBJob" ) {}
	
	virtual bool doJob() {
		
		// Collect all mask dependatoms from /etc/portage/package.mask
		QFile file( KuroolitoConfig::filePackageUserMask() );
		QTextStream stream( &file );
		QStringList linesDependAtom;
		if ( !file.open( IO_ReadOnly ) )
			kdError(0) << "Parsing user package.mask. Reading: " << KuroolitoConfig::filePackageUserMask() << LINE_INFO;
		else {
			while ( !stream.atEnd() )
				linesDependAtom += stream.readLine();
			file.close();
		}
		
		// Something is wrong, no files found, get outta here
		if ( linesDependAtom.isEmpty() )
			return false;
		
		setStatus( "PackageUserMask", i18n("Collecting user masked packages...") );
		
		DbConnection* const m_db = KuroolitoDBSingleton::Instance()->getStaticDbConnection();
		KuroolitoDBSingleton::Instance()->singleQuery(	"CREATE TEMP TABLE packageUserMask_temp ( "
		                                    		"idPackage INTEGER UNIQUE, dependAtom VARCHAR(255), comment BLOB );"
		                                    		, m_db);
		
		KuroolitoDBSingleton::Instance()->singleQuery( "BEGIN TRANSACTION;", m_db );
		
		QStringList commentLines;
		for ( QStringList::Iterator it = linesDependAtom.begin(), end = linesDependAtom.end(); it != end; ++it ) {
			
			// Collect comment lines above the dependatom
			if ( (*it).isEmpty() )
				commentLines.clear();
			else {
				if ( (*it).startsWith( "#" ) ) {
					continue;
				}
				else {
					if ( rxAtom.exactMatch( *it ) ) {
						
						// Get the captured strings
						QString category = rxAtom.cap( POS_CATEGORY ) + "-" + rxAtom.cap( POS_SUBCATEGORY );
						QString name = rxAtom.cap( POS_PACKAGE );
						
						QString id = KuroolitoDBSingleton::Instance()->singleQuery( 
							"SELECT id FROM package WHERE name = '" + name + "' AND category = '" + category + "' LIMIT 1;", m_db );
						
						if ( id.isEmpty() )
							kdWarning(0) << QString("Parsing user package.mask. Can not find id in database for package %1/%2.")
								.arg( category ).arg( name ) << LINE_INFO;
						else
							KuroolitoDBSingleton::Instance()->insert( QString( 
								"INSERT INTO packageUserMask_temp (idPackage, dependAtom) "
								"VALUES ('%1', '%2');" ).arg( id ).arg( *it ), m_db );
						
					}
					else
						kdWarning(0) << QString("Parsing user package.mask. Can not match package %1 in %2.").arg( *it )
							.arg( KuroolitoConfig::filePackageUserMask() ) << LINE_INFO;
				}
			}
		}
		file.close();
		KuroolitoDBSingleton::Instance()->singleQuery( "COMMIT TRANSACTION;", m_db );
		
		// Move content from temporary table to installedPackages
		KuroolitoDBSingleton::Instance()->singleQuery( "DELETE FROM packageUserMask;", m_db );
		KuroolitoDBSingleton::Instance()->insert( "INSERT INTO packageUserMask SELECT * FROM packageUserMask_temp;", m_db );
		KuroolitoDBSingleton::Instance()->singleQuery( "DROP TABLE packageUserMask_temp;", m_db );
		
		KuroolitoDBSingleton::Instance()->returnStaticDbConnection( m_db );
		setStatus( "PackageUserMask", i18n("Done.") );
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( PACKAGE_USER_MASK_SCANNED );
	}
};

/**
 * @class: LoadPackageUseJob
 * @short: Thread for loading packages use into db.
 */
class LoadPackageUseJob : public ThreadWeaver::DependentJob
{
public:
	LoadPackageUseJob( QObject *dependent ) : DependentJob( dependent, "DBJob" ) {}
	
	virtual bool doJob() {
		
		QFile file( KuroolitoConfig::filePackageUserUse() );
		QTextStream stream( &file );
		QStringList linesUse;
		if ( !file.open( IO_ReadOnly ) )
			kdError(0) << "Parsing user package.use. Reading: %1." << KuroolitoConfig::filePackageUserUse() << LINE_INFO;
		else {
			while ( !stream.atEnd() )
				linesUse += stream.readLine();
			file.close();
		}
		
		// Something is wrong, no files found, get outta here
		if ( linesUse.isEmpty() )
			return false;
		
		DbConnection* const m_db = KuroolitoDBSingleton::Instance()->getStaticDbConnection();
		KuroolitoDBSingleton::Instance()->singleQuery(	"CREATE TEMP TABLE packageUse_temp ( "
		                                    		"idPackage INTEGER UNIQUE, use VARCHAR(255) );", m_db);
		
		KuroolitoDBSingleton::Instance()->singleQuery( "BEGIN TRANSACTION;", m_db );
		
		for ( QStringList::Iterator it = linesUse.begin(), end = linesUse.end(); it != end; ++it ) {
			QString category = (*it).section( '/', 0, 0 );
			QString name = ( (*it).section( '/', 1 ) ).section( ' ', 0, 0 );
			QString use = (*it).section( ' ', 1 );
			use.simplifyWhiteSpace();
			
			QString id = KuroolitoDBSingleton::Instance()->singleQuery( 
				"SELECT id FROM package WHERE name = '" + name + "' AND category = '" + category + "' LIMIT 1;", m_db );
			
			if ( id.isEmpty() )
				kdWarning(0) << QString("Parsing user package.use. Can not find id in database for package %1/%2.")
					.arg( category ).arg( name ) << LINE_INFO;
			else
				KuroolitoDBSingleton::Instance()->insert( QString( "INSERT INTO packageUse_temp (idPackage, use) VALUES ('%1', '%2');" )
				                                      .arg( id ).arg( use ), m_db );
			
		}
		file.close();
		KuroolitoDBSingleton::Instance()->singleQuery( "COMMIT TRANSACTION;", m_db );
		
		// Move content from temporary table to installedPackages
		KuroolitoDBSingleton::Instance()->singleQuery( "DELETE FROM packageUse;", m_db );
		KuroolitoDBSingleton::Instance()->insert( "INSERT INTO packageUse SELECT * FROM packageUse_temp;", m_db );
		KuroolitoDBSingleton::Instance()->singleQuery( "DROP TABLE packageUse_temp;", m_db );
		
		KuroolitoDBSingleton::Instance()->returnStaticDbConnection( m_db );
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( PACKAGE_USER_USE_SCANNED );
	}
};

/**
 * Object for resulting list of packages from emerge actions.
 */
PortageFiles::PortageFiles( QObject *m_parent )
	: QObject( m_parent )
{}

PortageFiles::~PortageFiles()
{}

void PortageFiles::init( QObject *parent )
{
	m_parent = parent;
}

/**
 * Forward signal to refresh results.
 */
void PortageFiles::refresh( int mask )
{
	switch ( mask ) {
		case PACKAGE_KEYWORDS_SCANNED:
			kdDebug() << i18n( "Completed scanning for package keywords in %1." ).arg( KuroolitoConfig::filePackageKeywords() ) << endl;
			break;
			
		case PACKAGE_USER_UNMASK_SCANNED:
			 kdDebug() << i18n("Completed scanning for unmasked packages in %1.").arg( KuroolitoConfig::filePackageUserUnMask() ) << endl;
			break;
			
		case PACKAGE_HARDMASK_SCANNED:
			kdDebug() << i18n("Completed scanning for hardmasked packages in %1.").arg( KuroolitoConfig::filePackageHardMask() ) << endl;
			break;
			
		case PACKAGE_USER_MASK_SCANNED:
			kdDebug() << i18n("Completed scanning for user masked packages in %1.").arg( KuroolitoConfig::filePackageUserMask() ) << endl;
			break;

		case PACKAGE_USER_MASK_SAVED:
			kdDebug() << i18n("Completed saving user masked packages in %1.").arg( KuroolitoConfig::filePackageUserMask() ) << endl;
			break;

		case PACKAGE_USER_USE_SCANNED:
			kdDebug() << i18n("Completed scanning user package use flags in %1.").arg( KuroolitoConfig::filePackageUserUse() ) << endl;
			break;

	}
}


/**
 * Load all!
 */
void PortageFiles::loadPackageFiles()
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

#include "portagefiles.moc"
