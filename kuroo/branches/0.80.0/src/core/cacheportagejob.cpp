/***************************************************************************
 *   Copyright (C) 2005 by Karye   *
 *   karye@users.sourceforge.net   *
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
#include "cacheportagejob.h"

#include <fstream>
#include <string>

#include <qdir.h>
#include <qfileinfo.h>

/**
 * @class CachePortageJob
 * @short Thread to cache package information from the Portage directory to speed up portage refreshing.
 * 
 * Portage cache is scanned for package sizes, and stored in portage cache map and in the database.
 */
CachePortageJob::CachePortageJob( QObject* parent )
	: ThreadWeaver::DependentJob( parent, "CachePortageJob" ),
	m_db( KurooDBSingleton::Instance()->getStaticDbConnection() )
{
}

CachePortageJob::~CachePortageJob()
{
	KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
}

void CachePortageJob::completeJob()
{
	DEBUG_LINE_INFO;
	
	SignalistSingleton::Instance()->cachePortageComplete();
}

/**
 * Scan for package size found in digest files.
 * @return success
 */
bool CachePortageJob::doJob()
{
	DEBUG_LINE_INFO;
	
	if ( !m_db->isConnected() ) {
		kdError(0) << i18n("Creating cache. Can not connect to database") << LINE_INFO;
		return false;
	}
	
	int count( 0 );
	QMap <QString, QString> mapCache;
	QDir dCategory, dPackage;
	dCategory.setFilter( QDir::Dirs | QDir::NoSymLinks );
	dCategory.setSorting( QDir::Name );
	
	// Get a count of total packages for proper progress
	QString packageCount = KurooDBSingleton::Instance()->singleQuery( "SELECT data FROM dbInfo WHERE meta = 'packageCount';", m_db );
	if ( packageCount == "0" )
		setProgressTotalSteps( 35000 );
	else
		setProgressTotalSteps( packageCount.toInt() );
	setStatus( "CachePortage", i18n("Collecting package information...") );
	
	// Get list of categories in Portage and Overlays
	QStringList pathList = KurooConfig::dirPortage();
	const QStringList pathOverlays = QStringList::split( " ", KurooConfig::dirPortageOverlay() );
	foreach ( pathOverlays )
		pathList += *it;
	
	// Scan Portage cache
	for ( QStringList::Iterator itPath = pathList.begin(), itPathEnd = pathList.end(); itPath != itPathEnd; ++itPath ) {
		if ( !dCategory.cd( *itPath ) ) {
			kdWarning(0) << i18n("Creating cache. Can not access ") << *itPath << LINE_INFO;
			continue;
		}
		
		QStringList categoryList = dCategory.entryList();
		QStringList::Iterator itCategoryEnd = categoryList.end();
		for ( QStringList::Iterator itCategory = categoryList.begin(); itCategory != itCategoryEnd; ++itCategory ) {
			
			if ( *itCategory == "." || *itCategory == ".." )
				continue;
			
			// Abort the scan
			if ( isAborted() ) {
				kdWarning(0) << i18n("Creating cache. Caching aborted.") << LINE_INFO;
				setStatus( "CachePortage", i18n("Caching aborted.") );
				return false;
			}
			
			// Get list of packages in this category
			dPackage.setFilter( QDir::Files | QDir::NoSymLinks );
			dPackage.setSorting( QDir::Name );
			if ( dPackage.cd( KurooConfig::dirEdbDep() + *itPath + "/" + *itCategory ) ) {
				QStringList packageList = dPackage.entryList();
				QStringList::Iterator itPackageEnd = packageList.end();
				for ( QStringList::Iterator itPackage = packageList.begin(); itPackage != itPackageEnd; ++itPackage ) {
					
					if ( *itPackage == "." || *itPackage == ".." )
						continue;
					
					// Abort the scan
					if ( isAborted() ) {
						kdWarning(0) << i18n("Creating cache. Caching aborted.") << LINE_INFO;
						setStatus( "CachePortage", i18n("Caching aborted.") );
						return false;
					}
					QString package = *itCategory + "/" + *itPackage;
					
					QStringList parts = GlobalSingleton::Instance()->parsePackage( *itPackage );
					if ( !parts.isEmpty() ) {
						QString packageName = parts[1];
						
						// Get package size
						QString path = *itPath + "/" + *itCategory + "/" + packageName + "/files/digest-" + *itPackage;
						QFile file( path );
						if ( file.open( IO_ReadOnly ) ) {
							std::ifstream in( path );
							std::string word;
							while ( in >> word );
							mapCache.insert( package, word );
							file.close();
						}
						else
							kdWarning(0) << i18n("Creating cache. Reading: ") << path << LINE_INFO;
					}
					else
						kdWarning(0) << i18n("Creating cache. Can not parse: ") << *itPackage << LINE_INFO;
					
					// Post scan count progress
					if ( (++count % 100) == 0 )
						setProgress( count );
				}
			}
			else {
				kdWarning(0) << i18n("Creating cache. Can not access ") << *itPath << "/" << *itCategory << LINE_INFO;
				kdWarning(0) << "Creating cache. Can not access " << *itPath << "/" << *itCategory << LINE_INFO;
			}
		}
	}
	KurooDBSingleton::Instance()->query( QString("UPDATE dbInfo SET data = '%1' WHERE meta = 'packageCount';")
	                                     .arg( count ), m_db );
	
	// Store cache in DB
	KurooDBSingleton::Instance()->query( "DELETE FROM cache;", m_db );
	KurooDBSingleton::Instance()->query( "BEGIN TRANSACTION;", m_db );
	QMap< QString, QString >::iterator itMapEnd = mapCache.end();
	for ( QMap< QString, QString >::iterator itMap = mapCache.begin(); itMap != itMapEnd; ++itMap )
		KurooDBSingleton::Instance()->insert( QString("INSERT INTO cache (package, size) VALUES ('%1', '%2');").
		                                      arg( itMap.key() ).arg( itMap.data() ), m_db );

	KurooDBSingleton::Instance()->query("COMMIT TRANSACTION;", m_db );
	
	setStatus( "CachePortage", i18n("Done.") );
	setProgress( 0 );
	return true;
}

#include "cacheportagejob.moc"
