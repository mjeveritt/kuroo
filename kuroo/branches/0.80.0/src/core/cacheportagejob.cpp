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
	kdDebug() << k_funcinfo << endl;
	
	SignalistSingleton::Instance()->cachePortageComplete();
}

/**
 * Scan for package size found in digest files.
 * @return success
 */
bool CachePortageJob::doJob()
{
	kdDebug() << k_funcinfo << endl;
	
	if ( !m_db->isConnected() ) {
		kdDebug() << i18n("Creating cache. Can not connect to database") << endl;
		kdDebug() << "Creating cache. Can not connect to database" << endl;
		return false;
	}
	
	int count( 0 );
	QMap <QString, QString> mapCache;
	QDir dCategory, dPackage;
	dCategory.setFilter( QDir::Dirs | QDir::NoSymLinks );
	dCategory.setSorting( QDir::Name );
	
	// Get a count of total packages for proper progress
	QString packageCount = KurooDBSingleton::Instance()->singleQuery( "SELECT data FROM dbInfo WHERE meta = 'packageCount';", m_db );
	if ( packageCount.isEmpty() )
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
			kdDebug() << i18n("Creating cache. Can not access ") << *itPath << endl;
			kdDebug() << "Creating cache. Can not access " << *itPath << endl;
			continue;
		}
		
		QStringList categoryList = dCategory.entryList();
		QStringList::Iterator itCategoryEnd = categoryList.end();
		for ( QStringList::Iterator itCategory = categoryList.begin(); itCategory != itCategoryEnd; ++itCategory ) {
			
			if ( *itCategory == "." || *itCategory == ".." )
				continue;
			
			// Abort the scan
			if ( isAborted() ) {
				kdDebug() << i18n("Creating cache. Caching aborted.") << endl;
				kdDebug() << "Creating cache. Caching aborted." << endl;
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
						kdDebug() << i18n("Creating cache. Caching aborted.") << endl;
						kdDebug() << "Creating cache. Caching aborted." << endl;
						setStatus( "CachePortage", i18n("Caching aborted.") );
						return false;
					}
					QString package = *itCategory + "/" + *itPackage;
					
					QString versionString = GlobalSingleton::Instance()->getPackageVersion( *itPackage );
					if ( !versionString.isEmpty() ) {
						QString packageName = (*itPackage).section( versionString, 0, 0 );
						
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
						else {
							kdDebug() << i18n("Creating cache. Error reading: ") << path << endl;
							kdDebug() << "Creating cache. Error reading: " << path << endl;
						}
					}
					else {
						kdDebug() << i18n("Creating cache. Can not parse: ") << *itPackage << endl;
						kdDebug() << "Creating cache. Can not parse: " << *itPackage << endl;
					}
					
					// Post scan count progress
					if ( (++count % 100) == 0 )
						setProgress( count );
				}
			}
			else {
				kdDebug() << i18n("Creating cache. Can not access ") << *itPath << "/" << *itCategory << endl;
				kdDebug() << "Creating cache. Can not access " << *itPath << "/" << *itCategory << endl;
			}
		}
	}
	setKurooDbMeta( "packageCount", QString::number( count ) );
	
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

void CachePortageJob::setKurooDbMeta( const QString& meta, const QString& data )
{
	if ( KurooDBSingleton::Instance()->singleQuery( QString("SELECT COUNT(meta) FROM dbInfo WHERE meta = '%1' LIMIT 1;").
	                                                arg( meta ), m_db ) == "0" )
		KurooDBSingleton::Instance()->query( QString("INSERT INTO dbInfo (meta, data) VALUES ('%1', '%2') ;").
		                                     arg( meta ).arg( data ), m_db );
	else
		KurooDBSingleton::Instance()->query( QString("UPDATE dbInfo SET data = '%2' WHERE meta = '%1';").
		                                     arg( meta ).arg( data ), m_db );
}

#include "cacheportagejob.moc"
