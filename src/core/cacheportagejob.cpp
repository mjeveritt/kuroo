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
 * Thread to cache package information from the Portage directory so as to speed up portage refreshing.
 */
CachePortageJob::CachePortageJob( QObject* parent )
	: ThreadWeaver::DependentJob( parent, "CachePortageJob" ),
	m_db( KurooDBSingleton::Instance()->getStaticDbConnection() )
{
	KurooConfig::setPortageCount( QString::number(countPackages()) );
	KurooConfig::writeConfig();
}

CachePortageJob::~CachePortageJob()
{
	KurooDBSingleton::Instance()->returnStaticDbConnection(m_db);
}

void CachePortageJob::completeJob()
{
	SignalistSingleton::Instance()->cachePortageComplete();
}

/**
 * Count packages in Portage.
 * @return count
 */
int CachePortageJob::countPackages()
{
	QDir dCategory;
	dCategory.setFilter(QDir::Dirs | QDir::NoSymLinks);
	dCategory.setSorting(QDir::Name);
	
	QDir dPackage;
	dPackage.setFilter(QDir::Files | QDir::NoSymLinks);
	dPackage.setSorting(QDir::Name);
	
	if ( !dCategory.cd(KurooConfig::dirEdbDep() + "/usr/portage") ) {
		kdDebug() << i18n("Can not access ") << KurooConfig::dirEdbDep() << "/usr/portage" << endl;
		return 0;
	}
	
	setStatus( i18n("Counting packages...") );
	
	int count(0);
	const QStringList categoryList = dCategory.entryList();
	foreach ( categoryList ) {
		if ( *it == "." || *it == ".." )
			continue;
		if ( dPackage.cd( KurooConfig::dirEdbDep() + "/usr/portage/" + *it ) )
			count += dPackage.entryList().count() - 2;
	}
	return count;
}

/**
 * Scan for package size found in digest files.
 * @return success
 */
bool CachePortageJob::doJob()
{
	if ( !m_db->isConnected() ) {
		kdDebug() << i18n("Can not connect to database") << endl;
		return false;
	}
	
	int count(0);
	QMap <QString, QString> cacheMap;
	QDir dCategory, dPackage;
	dCategory.setFilter(QDir::Dirs | QDir::NoSymLinks);
	dCategory.setSorting(QDir::Name);
	
	setProgressTotalSteps( KurooConfig::portageCount().toInt() );
	setStatus( i18n("Collecting package information...") );
	
	// Get list of categories in Portage Overlay
	if ( !dCategory.cd(KurooConfig::dirEdbDep() + "/usr/local/portage") )
		kdDebug() << i18n("Can not access ") << KurooConfig::dirEdbDep() << "/usr/local/portage" << endl;
	
	QStringList categoryList = dCategory.entryList();
	QStringList::Iterator itCategoryEnd = categoryList.end();
	for ( QStringList::Iterator itCategory = categoryList.begin(); itCategory != itCategoryEnd; ++itCategory ) {
		
		if ( *itCategory == "." || *itCategory == ".." )
			continue;
		
		// Abort the scan
		if ( isAborted() ) {
			kdDebug() << i18n("Caching aborted") << endl;
			return false;
		}
		
		// Get list of packages in this category
		dPackage.setFilter(QDir::Files | QDir::NoSymLinks);
		dPackage.setSorting(QDir::Name);
		if ( dPackage.cd(KurooConfig::dirEdbDep() + "/usr/local/portage/" + *itCategory) ) {
			QStringList packageList = dPackage.entryList();
			QStringList::Iterator itPackageEnd = packageList.end();
			for ( QStringList::Iterator itPackage = packageList.begin(); itPackage != itPackageEnd; ++itPackage ) {
				
				if ( *itPackage == "." || *itPackage == ".." )
					continue;
				
				// Abort the scan
				if ( isAborted() ) {
					kdDebug() << i18n("Caching aborted") << endl;
					return false;
				}
				
				QString package = *itCategory + "/" + *itPackage;
				
				// Get package size
				QString path = KurooConfig::dirPortageOverlay() + "/" + *itCategory + "/" + (*itPackage).section(pv, 0, 0) + "/files/digest-" + *itPackage;
				QFile file( path );
				if ( file.open( IO_ReadOnly ) ) {
					std::ifstream in( path );
					std::string word;
					while ( in >> word );
					cacheMap.insert( package, word );
					file.close();
				}
				else
					kdDebug() << i18n("Error reading: ") << path << endl;
				
				// Post scan count progress
				if ( (++count % 100) == 0 ) {
					setProgress( count );
				}
			}
		}
		else
			kdDebug() << i18n("Can not access ") << KurooConfig::dirEdbDep() << "/usr/local/portage/" << *itCategory << endl;
	}
	
	// Get list of categories in Portage
	if ( !dCategory.cd(KurooConfig::dirEdbDep() + "/usr/portage") ) {
		kdDebug() << i18n("Can not access ") << KurooConfig::dirEdbDep() << "/usr/portage" << endl;
		return false;
	}
	categoryList = dCategory.entryList();
	itCategoryEnd = categoryList.end();
	for ( QStringList::Iterator itCategory = categoryList.begin(); itCategory != itCategoryEnd; ++itCategory ) {
		
		if ( *itCategory == "." || *itCategory == ".." )
			continue;
		
		// Abort the scan
		if ( isAborted() ) {
			kdDebug() << i18n("Caching aborted") << endl;
			return false;
		}

		// Get list of packages in this category
		dPackage.setFilter(QDir::Files | QDir::NoSymLinks);
		dPackage.setSorting(QDir::Name);
		if ( dPackage.cd(KurooConfig::dirEdbDep() + "/usr/portage/" + *itCategory) ) {
			QStringList packageList = dPackage.entryList();
			QStringList::Iterator itPackageEnd = packageList.end();
			for ( QStringList::Iterator itPackage = packageList.begin(); itPackage != itPackageEnd; ++itPackage ) {
				
				if ( *itPackage == "." || *itPackage == ".." )
					continue;
				
				// Abort the scan
				if ( isAborted() ) {
					kdDebug() << i18n("Caching aborted") << endl;
					return false;
				}
				
				QString package = *itCategory + "/" + *itPackage;
				
				// Get package size
				QString path = KurooConfig::dirPortage() + "/" + *itCategory + "/" + (*itPackage).section(pv, 0, 0) + "/files/digest-" + *itPackage;
				QFile file( path );
				if ( file.open( IO_ReadOnly ) ) {
					std::ifstream in( path );
					std::string word;
					while ( in >> word );
					cacheMap.insert( package, word );
					file.close();
				}
				else
					kdDebug() << i18n("Error reading: ") << path << endl;
				
				// Post scan count progress
				if ( (++count % 100) == 0 ) {
					setProgress( count );
				}
			}
		}
		else
			kdDebug() << i18n("Can not access ") << KurooConfig::dirEdbDep() << "/usr/portage/" << *itCategory << endl;
	}
	PortageSingleton::Instance()->setCache( cacheMap );
	
	// Store cache in DB
	KurooDBSingleton::Instance()->query("DELETE FROM cache;", m_db);
	KurooDBSingleton::Instance()->query("BEGIN TRANSACTION;", m_db);
	QMap< QString, QString >::iterator itMapEnd = cacheMap.end();
	for ( QMap< QString, QString >::iterator itMap = cacheMap.begin(); itMap != itMapEnd; ++itMap ) {
		KurooDBSingleton::Instance()->insert( QString("INSERT INTO cache (package, size) VALUES ('%1', '%2');").arg(itMap.key()).arg(itMap.data()), m_db);
	}
	KurooDBSingleton::Instance()->query("COMMIT TRANSACTION;", m_db);
	
	return true;
}

#include "cacheportagejob.moc"
