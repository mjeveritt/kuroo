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
 * @class: LoadPackageKeywordsJob
 * @short: Thread for loading packages keyword-unmasked by user.
 */
class LoadPackageKeywordsJob : public ThreadWeaver::DependentJob
{
public:
	LoadPackageKeywordsJob( QObject *dependent ) : DependentJob( dependent, "DBJob" ) {}
	
	virtual bool doJob() {
		DEBUG_LINE_INFO;
		
		// Collect all mask dependatoms
		QFileInfo fileInfo( KurooConfig::filePackageKeywords() );
		if( fileInfo.isDir() ) {
			kDebug(0) << KurooConfig::filePackageKeywords() << " is a dir" << LINE_INFO;
			if( !mergeDirIntoFile( KurooConfig::filePackageKeywords() ) ) {
				return false;
			}
		}
		QFile file( KurooConfig::filePackageKeywords() );
		QTextStream stream( &file );
		QStringList linesPackage;
		if ( !file.open( QIODevice::ReadOnly ) )
			kWarning(0) << "Parsing package.keywords. Reading: " << KurooConfig::filePackageKeywords() << LINE_INFO;
		else {
			while ( !stream.atEnd() )
				linesPackage += stream.readLine();
			file.close();
		}
		
		// Something is wrong, no files found, get outta here
		if ( linesPackage.isEmpty() )
			return false;
		
		setStatus( "PackageKeywords", i18n("Collecting user package keywords...") );
		
		DbConnection* const m_db = KurooDBSingleton::Instance()->getStaticDbConnection();
		KurooDBSingleton::Instance()->singleQuery(	"CREATE TEMP TABLE packageKeywords_temp ( "
		                                    		"idPackage INTEGER UNIQUE, "
		                                    		"keywords VARCHAR(255) );"
		                                    		, m_db);
		
		KurooDBSingleton::Instance()->singleQuery( "BEGIN TRANSACTION;", m_db );
		
		for ( QStringList::Iterator it = linesPackage.begin(), end = linesPackage.end(); it != end; ++it ) {
			
			// set the atom string
            QStringList tokens = (*it).split(' ');
			QString package = tokens[0];
			
			if( !(*it).trimmed().startsWith( "#" ) && !(*it).trimmed().isEmpty() ) {
				if ( rxAtom.exactMatch( package ) ) {

					// Get the captured strings
					QString category = rxAtom.cap( ThreadWeaver::POS_CATEGORY ) + "-" + rxAtom.cap( ThreadWeaver::POS_SUBCATEGORY );
					QString name = rxAtom.cap( ThreadWeaver::POS_PACKAGE );
					QString keywords;
					
					// extract this line's keywords
					QStringList::iterator tokenIterator = tokens.begin();
					tokenIterator++;
					while ( tokenIterator != tokens.end() ) {
						keywords += *tokenIterator + " ";
						tokenIterator++;
					}
					if ( keywords.isEmpty() )
					keywords = "~" + KurooConfig::arch();
	
					QString id = KurooDBSingleton::Instance()->singleQuery( 
						"SELECT id FROM package WHERE name = '" + name + "' AND category = '" + category + "' LIMIT 1;", m_db );
					
					if ( id.isEmpty() )
						kWarning(0) << QString("Load package keywords: Can not find id in database for package %1/%2.")
						.arg( category ).arg( name ) << LINE_INFO;
					else
						KurooDBSingleton::Instance()->insert( QString( 
							"INSERT INTO packageKeywords_temp (idPackage, keywords) VALUES ('%1', '%2');" )
										.arg( id ).arg( keywords ), m_db );
				}
				else
					kWarning(0) << QString("Parsing package.keywords. Can not match package %1 in %2.").arg( *it )
						.arg( KurooConfig::filePackageKeywords() ) << LINE_INFO;
			}
			
		}
		file.close();
		KurooDBSingleton::Instance()->singleQuery( "COMMIT TRANSACTION;", m_db );
		
		// Move content from temporary table to installedPackages
		KurooDBSingleton::Instance()->singleQuery( "DELETE FROM packageKeywords;", m_db );
		KurooDBSingleton::Instance()->insert( "INSERT INTO packageKeywords SELECT * FROM packageKeywords_temp;", m_db );
		KurooDBSingleton::Instance()->singleQuery( "DROP TABLE packageKeywords_temp;", m_db );
		
		KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
		setStatus( "PackageKeywords", i18n("Done.") );
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( ThreadWeaver::PACKAGE_KEYWORDS_SCANNED );
	}
};
