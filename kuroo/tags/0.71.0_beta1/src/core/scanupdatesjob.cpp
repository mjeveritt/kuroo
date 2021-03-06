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
#include "scanupdatesjob.h"

#include <qdir.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qheader.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qtextstream.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>

/**
 * Thread for loading emerge -uDpv World output into db.
 */
ScanUpdatesJob::ScanUpdatesJob( QObject* parent, const EmergePackageList &packageList )
	: ThreadWeaver::DependentJob( parent, "DBJob" ),
	m_db( KurooDBSingleton::Instance()->getStaticDbConnection() ), m_packageList( packageList ), aborted(true)
{
}

ScanUpdatesJob::~ScanUpdatesJob()
{
	KurooDBSingleton::Instance()->returnStaticDbConnection(m_db);
	if ( aborted )
		SignalistSingleton::Instance()->scanAborted();
}

/**
 * Inform gui thread that scan is completed.
 */
void ScanUpdatesJob::completeJob()
{
	SignalistSingleton::Instance()->loadUpdatesComplete();
	aborted = false;
}

/**
 * Insert found updates into db.
 * @return success
 */
bool ScanUpdatesJob::doJob()
{
	if ( !m_db->isConnected() ) {
		kdDebug() << i18n("Can not connect to database") << endl;
		return false;
	}
	
	if ( m_packageList.isEmpty() ) {
		kdDebug() << i18n("No update package found") << endl;
		return false;
	}
	
	setStatus( i18n("Refreshing updates view...") );
	setProgressTotalSteps( m_packageList.count() );
	int count(0);
		
	// Temporary tables to avoid locking main table
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
	
	KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE updates_temp ("
	                                    " id INTEGER PRIMARY KEY AUTOINCREMENT, "
	                                    " idPackage INTEGER UNIQUE, "
	                                    " installedVersion VARCHAR(32),"
	                                    " updateFlags VARCHAR(32),"
	                                    " useFlags VARCHAR(32)"
	                                    " );", m_db);
	
	KurooDBSingleton::Instance()->insert("INSERT INTO package_temp SELECT * FROM package;", m_db);
	KurooDBSingleton::Instance()->query("UPDATE package_temp SET updateVersion = '';", m_db);
	
	KurooDBSingleton::Instance()->query("BEGIN TRANSACTION;", m_db);
	
	EmergePackageList::ConstIterator itEnd = m_packageList.end();
	for ( EmergePackageList::ConstIterator it = m_packageList.begin(); it != itEnd; ++it ) {

		// Abort the scan
		if ( isAborted() ) {
			kdDebug() << i18n("Updates scan aborted") << endl;
			KurooDBSingleton::Instance()->query("ROLLBACK TRANSACTION;", m_db);
			return false;
		}
		
		// count is also ordering number
		setProgress( count++ );
		
		// Find id for this category in db
		QString idCategory = KurooDBSingleton::Instance()->query(QString("SELECT id FROM category WHERE name = '%1';").arg((*it).category), m_db).first();
		
		// Mark as update in portage
		if ( !(*it).updateFlags.contains("N") ) {
			QString idPackage = KurooDBSingleton::Instance()->query(QString("SELECT id FROM package_temp WHERE name = '%1' AND idCategory = '%2' AND version = '%3';").arg((*it).name).arg(idCategory).arg((*it).installedVersion), m_db).first();
			
			KurooDBSingleton::Instance()->query(QString("UPDATE package_temp SET updateVersion = '%1' WHERE id = '%2';").arg((*it).version).arg(idPackage), m_db);
		}
		
		QString idPackage = KurooDBSingleton::Instance()->query(QString("SELECT id FROM package_temp WHERE name = '%1' AND idCategory = '%2' AND version = '%3';").arg((*it).name).arg(idCategory).arg((*it).version), m_db).first();
		
		KurooDBSingleton::Instance()->insert(QString("INSERT INTO updates_temp (idPackage, installedVersion, updateFlags, useFlags ) VALUES ('%1', '%2', '%3', '%4');").arg(idPackage).arg((*it).installedVersion).arg((*it).updateFlags).arg((*it).useFlags), m_db);
	}
	KurooDBSingleton::Instance()->query("COMMIT TRANSACTION;", m_db);
	
	// Move content from temporary table 
	KurooDBSingleton::Instance()->query("DELETE FROM package;", m_db);
	KurooDBSingleton::Instance()->insert("INSERT INTO package SELECT * FROM package_temp;", m_db);
	KurooDBSingleton::Instance()->query("DROP TABLE package_temp;", m_db);
	
	KurooDBSingleton::Instance()->query("DELETE FROM updates;", m_db);
	KurooDBSingleton::Instance()->insert("INSERT INTO updates SELECT * FROM updates_temp;", m_db);
	KurooDBSingleton::Instance()->query("DROP TABLE updates_temp;", m_db);
	
	setStatus(i18n("Done."));
	setProgress(0);
	return true;
}

#include "scanupdatesjob.moc"
