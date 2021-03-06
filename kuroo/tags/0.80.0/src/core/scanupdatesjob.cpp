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
#include <qtextstream.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>

/**
 * @class ScanUpdatesJob
 * @short Thread for loading 'emerge -uDrxv World' output into db.
 */
ScanUpdatesJob::ScanUpdatesJob( QObject* parent, const EmergePackageList &packageList )
	: ThreadWeaver::DependentJob( parent, "DBJob" ),
	m_db( KurooDBSingleton::Instance()->getStaticDbConnection() ), m_packageList( packageList ), m_aborted( true )
{
}

ScanUpdatesJob::~ScanUpdatesJob()
{
	KurooDBSingleton::Instance()->returnStaticDbConnection( m_db );
	if ( m_aborted )
		SignalistSingleton::Instance()->scanAborted();
}

/**
 * Inform gui thread that scan is completed.
 */
void ScanUpdatesJob::completeJob()
{
	kdDebug() << k_funcinfo << endl;
	
	SignalistSingleton::Instance()->loadUpdatesComplete();
	m_aborted = false;
}

/**
 * Insert found updates into db.
 * @return success
 */
bool ScanUpdatesJob::doJob()
{
	kdDebug() << k_funcinfo << endl;
	
	if ( !m_db->isConnected() ) {
		kdDebug() << i18n("Scanning updates. Can not connect to database") << endl;
		kdDebug() << "Scanning updates. Can not connect to database" << endl;
		return false;
	}
	
	if ( m_packageList.isEmpty() ) {
		kdDebug() << i18n("Scanning updates. No update package found") << endl;
		kdDebug() << "Scanning updates. No update package found" << endl;
// 		return false;
	}
	
	setStatus( "ScanUpdates", i18n("Refreshing updates view...") );
	setProgressTotalSteps( m_packageList.count() );
	int count( 0 );

	// Temporary tables to avoid locking main table
	KurooDBSingleton::Instance()->query(" CREATE TEMP TABLE package_temp ("
	                                    " id INTEGER PRIMARY KEY AUTOINCREMENT,"
	                                    " idCategory INTEGER, "
	                                    " idSubCategory INTEGER, "
	                                    " idCatSubCategory INTEGER, "
	                                    " category VARCHAR(32), "
	                                    " name VARCHAR(32), "
	                                    " description VARCHAR(255), "
	                                    " latest VARCHAR(32), "
	                                    " date VARCHAR(32), "
	                                    " status INTEGER, "
	                                    " meta VARCHAR(255), "
	                                    " updateVersion VARCHAR(32) "
	                                    " );", m_db);
	
	KurooDBSingleton::Instance()->insert( "INSERT INTO package_temp SELECT * FROM package;", m_db );
	KurooDBSingleton::Instance()->query( "UPDATE package_temp SET updateVersion = '';", m_db );
	KurooDBSingleton::Instance()->query("BEGIN TRANSACTION;", m_db);
	
	int updatesCount( 0 );
	EmergePackageList::ConstIterator itEnd = m_packageList.end();
	for ( EmergePackageList::ConstIterator it = m_packageList.begin(); it != itEnd; ++it ) {

		// Abort the scan
		if ( isAborted() ) {
			kdDebug() << i18n("Scanning updates. Scan aborted!") << endl;
			kdDebug() << "Scanning updates. Scan aborted!" << endl;
			KurooDBSingleton::Instance()->query( "ROLLBACK TRANSACTION;", m_db );
			return false;
		}
		
		// count is also ordering number
		setProgress( count++ );
		
		// Find id for this category in db
		QString id = KurooDBSingleton::Instance()->singleQuery( 
			" SELECT id FROM package WHERE name = '" + (*it).name + "' AND idCatSubCategory = "
			" ( SELECT id from catSubCategory WHERE name = '" + (*it).category + "' ); ", m_db );
		
		if ( id.isEmpty() ) {
			kdDebug() << i18n("Scanning updates. Can not find id in database for package %1/%2.").arg( (*it).category ).arg( (*it).name ) << endl;
			kdDebug() << QString("Scanning updates. Can not find id in database for package %1/%2.").arg( (*it).category ).arg( (*it).name ) << endl;
		}
		else {
			
			// Mark as update in portage, but not for new packages
			if ( !(*it).updateFlags.contains( QRegExp("\\bN\\b") ) && !id.isEmpty() ) {
				
				// Is the package upgrade or downgrade?
				QString updateVersion;
				if ( (*it).updateFlags.contains("D") )
					updateVersion = (*it).version + " (D)";
				else
					updateVersion = (*it).version + " (U)";
				
				KurooDBSingleton::Instance()->query( QString( "UPDATE package_temp SET updateVersion = '%1' WHERE id = '%2';")
				                                     .arg( updateVersion ).arg( id ), m_db );
				
				updatesCount++;
			}
		}
	}
	KurooDBSingleton::Instance()->query("COMMIT TRANSACTION;", m_db );
	
	// Move content from temporary table
	KurooDBSingleton::Instance()->query("DELETE FROM package;", m_db );
	KurooDBSingleton::Instance()->insert("INSERT INTO package SELECT * FROM package_temp;", m_db );
	KurooDBSingleton::Instance()->query("DROP TABLE package_temp;", m_db );
	
	setKurooDbMeta( "updatesCount", QString::number( updatesCount ) );
	
	setStatus( "ScanUpdates", i18n( "Done." ) );
	setProgressTotalSteps( 0 );
	return true;
}

void ScanUpdatesJob::setKurooDbMeta( const QString& meta, const QString& data )
{
	if ( KurooDBSingleton::Instance()->singleQuery( QString("SELECT COUNT(meta) FROM dbInfo WHERE meta = '%1' LIMIT 1;").arg( meta ), m_db ) == "0" )
		KurooDBSingleton::Instance()->query( QString("INSERT INTO dbInfo (meta, data) VALUES ('%1', '%2') ;").arg( meta ).arg( data ), m_db );
	else
		KurooDBSingleton::Instance()->query( QString("UPDATE dbInfo SET data = '%2' WHERE meta = '%1';").arg( meta ).arg( data ), m_db );
}

#include "scanupdatesjob.moc"
