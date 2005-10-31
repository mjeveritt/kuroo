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
#include "scaninstalledjob.h"

#include <qdir.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qtextstream.h>

#include <kglobal.h>

/**
 * @class ScanInstalledJob
 * @short Thread for scanning installed packages.
 */
ScanInstalledJob::ScanInstalledJob( QObject* parent )
	: ThreadWeaver::DependentJob( parent, "DBJob" ),
	m_db( KurooDBSingleton::Instance()->getStaticDbConnection() ), aborted(true)
{
}

ScanInstalledJob::~ScanInstalledJob()
{
	KurooDBSingleton::Instance()->returnStaticDbConnection(m_db);
	if ( aborted )
		SignalistSingleton::Instance()->scanAborted();
}

/**
 * Inform gui thread that scanning is completed.
 */
void ScanInstalledJob::completeJob()
{
	SignalistSingleton::Instance()->scanInstalledComplete();
	aborted = false;
}

/**
 * Count installed packages.
 * @return count
 */
int ScanInstalledJob::countPackages()
{
	QDir dCategory;
	dCategory.setFilter(QDir::Dirs | QDir::NoSymLinks);
	dCategory.setSorting(QDir::Name);
	
	if ( !dCategory.cd(KurooConfig::dirDbPkg()) ) {
		kdDebug() << i18n("Can not access ") << KurooConfig::dirDbPkg() << endl;
		return 0;
	}
	
	setStatus( i18n("Counting packages...") );
	
	int count(0);
	const QStringList categoryList = dCategory.entryList();
	foreach ( categoryList ) {
		if ( *it == "." || *it == ".." )
			continue;
		if ( dCategory.cd( KurooConfig::dirDbPkg() + "/" + *it ) )
			count += dCategory.entryList().count() - 2;
	}
	return count;
}

/**
 * Scan KurooConfig::dirDbPkg() for packages.
 * Update packages with an installed flag.
 * @return success
 */
bool ScanInstalledJob::doJob()
{
	int count(0);
	QDir dCategory, dPackage;
	dCategory.setFilter(QDir::Dirs | QDir::NoSymLinks);
	dCategory.setSorting(QDir::Name);
	
	if ( !m_db->isConnected() ) {
		kdDebug() << i18n("Can not connect to database") << endl;
		return false;
	}
	
	if ( !dCategory.cd(KurooConfig::dirDbPkg()) ) {
		kdDebug() << i18n("Can not access ") << KurooConfig::dirDbPkg() << endl;
		return false;
	}

	// Assume total count of package
	setProgressTotalSteps( countPackages() );
	
	setStatus( i18n("Refreshing Installed view...") );
		
	// Temporary table for all packages to avoid locking main table
	KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE package_temp ("
	                                    " id INTEGER PRIMARY KEY AUTOINCREMENT,"
	                                    " idCategory INTEGER, "
	                                    " name VARCHAR(32), "
	                                    " latest VARCHAR(32), "
	                                    " description VARCHAR(255), "
	                                    " homepage VARCHAR(32), "
	                                    " licenses VARCHAR(32), "
	                                    " size VARCHAR(32), "
	                                    " keywords VARCHAR(32),"
	                                    " useFlags VARCHAR(32),"
	                                    " packageSlots VARCHAR(32),"
	                                    " version VARCHAR(32), "
	                                    " date VARCHAR(32), "
	                                    " installed INTEGER, "
	                                    " updateVersion VARCHAR(32)); "
	                                   	");", m_db);
	KurooDBSingleton::Instance()->insert("INSERT INTO package_temp SELECT * FROM package WHERE installed != 2;", m_db);
	KurooDBSingleton::Instance()->query("UPDATE package_temp SET installed = 0;", m_db);
	
	KurooDBSingleton::Instance()->query("BEGIN TRANSACTION", m_db);
	
	// Get list of categories for installed packages
	QStringList categoryList = dCategory.entryList();
	QStringList::Iterator itCategoryEnd = categoryList.end();
	for ( QStringList::Iterator itCategory = categoryList.begin(); itCategory != itCategoryEnd; ++itCategory ) {
		
		if ( *itCategory == "." || *itCategory == ".." )
			continue;
		
		// Abort the scan
		if ( isAborted() ) {
			kdDebug() << i18n("Installed scan aborted") << endl;
			KurooDBSingleton::Instance()->query("ROLLBACK TRANSACTION;", m_db);
			return false;
		}
		
		// Find id for this category in db
		QString idCategory = KurooDBSingleton::Instance()->query(QString("SELECT id FROM category WHERE name = '%1';").arg(*itCategory), m_db).first();
		
		// Get list of packages in this category
		dPackage.setFilter(QDir::Dirs | QDir::NoSymLinks);
		dPackage.setSorting(QDir::Name);
		
		if ( dPackage.cd( KurooConfig::dirDbPkg() + "/" + *itCategory ) ) {
			QStringList packageList = dPackage.entryList();
			QStringList::Iterator itPackageEnd = packageList.end();
			for ( QStringList::Iterator itPackage = packageList.begin(); itPackage != itPackageEnd; ++itPackage ) {
				
				if ( *itPackage == "." || *itPackage == ".." || (*itPackage).contains("MERGING") )
					continue;
				
				// Abort the scan
				if ( isAborted() ) {
					kdDebug() << i18n("Installed scan aborted") << endl;
					KurooDBSingleton::Instance()->query("ROLLBACK TRANSACTION;", m_db);
					return false;
				}
				
				QString package = (*itPackage).section(pv, 0, 0);
				QString version = (*itPackage).section(package + "-", 1, 1);
				
				QFileInfo fi1(*itPackage);
				QDateTime syncDate = fi1.created();
				QString date = syncDate.toString("yyyy MM dd");
				
				QString idPackage = KurooDBSingleton::Instance()->query(QString("SELECT id FROM package_temp WHERE name = '%1' AND idCategory = '%2' AND version = '%3';").arg(package).arg(idCategory).arg(version), m_db).first();
				
				// Mark package as installed in table package or add old package (=2) @todo
				// Installed packages present in Portage are flaged 1
				// Installed packages not present in Portage anymore are flaged 2
				if ( idPackage.isEmpty() ) {
					KurooDBSingleton::Instance()->insert(QString("INSERT INTO package_temp (idCategory, name, version, installed) VALUES ('%1', '%2', '%3', 2);").arg(idCategory).arg(package).arg(version), m_db);
				}
				else {
					KurooDBSingleton::Instance()->query(QString("UPDATE package_temp SET installed = '1' WHERE id = '%1';").arg(idPackage), m_db).first();
				}
				
				// Post scan count progress
				if ( (++count % 10) == 0 ) {
					setProgress( count );
				}
			}
		}
		else
			kdDebug() << i18n("Can not access ") << KurooConfig::dirDbPkg() << "/" << *itCategory << endl;
	}
	KurooDBSingleton::Instance()->query("COMMIT TRANSACTION;", m_db);
	
	// Move content from temporary table to installedPackages
	KurooDBSingleton::Instance()->query("DELETE FROM package;", m_db);
	KurooDBSingleton::Instance()->insert("INSERT INTO package SELECT * FROM package_temp;", m_db);
	KurooDBSingleton::Instance()->query("DROP TABLE package_temp;", m_db);
		
	setStatus( i18n("Done.") );
	setProgress(0);
	return true;
}

#include "scaninstalledjob.moc"
