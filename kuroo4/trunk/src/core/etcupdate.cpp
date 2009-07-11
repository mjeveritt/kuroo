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
#include "message.h"

#include <kmessagebox.h>
#include <kio/job.h>
#include <kinputdialog.h>
#include <kdirwatch.h>
#include <errno.h>

/**
 * @class EtcUpdate
 * @short Handles etc-updates.
 * 
 * Runs etc-update to collect the list of etc-files that needs merging.
 * Launches the exernal diff-tool for selected files.
 */
EtcUpdate::EtcUpdate( QObject* m_parent, const char* name )
    : QObject( m_parent/*, name*/ )
{}

EtcUpdate::~EtcUpdate()
{
	delete eProc;
	eProc = 0;
}

void EtcUpdate::init( QObject *parent )
{
	m_parent = parent;
	
    eProc = new KProcess();
    connect( eProc, SIGNAL( finished( int ) ), this, SLOT( slotCleanupDiff( int ) ) );
	
	m_mergingFile = new KDirWatch( this );
	connect( m_mergingFile, SIGNAL( dirty( const QString& ) ), this, SLOT( slotChanged() ) );
}

/**
 * Scan for new configuration files.
 */
void EtcUpdate::slotEtcUpdate()
{
	DEBUG_LINE_INFO;
	if ( KurooConfig::etcUpdateTool().isEmpty() )
        KMessageBox::information( 0, i18n( "Please specify merge tool in settings!" ), i18n( "Kuroo" ) );
	else {
		m_etcFilesList.clear();
		m_backupFilesList.clear();
		
		// First collect old merged files
        m_configProtectList = QStringList( kurooDir + QString("backup/configuration") );
		
		// Then scan for new unmerged files
        m_configProtectList += KurooConfig::configProtectList().split( " " );
		
		// Launch scan slave
		slotFinished();
	}
}

/**
 * Scan each confProtect dirs.
 */
void EtcUpdate::slotFinished()
{
	if ( m_configProtectList.count() > 0 ) {
		m_configProtectDir = m_configProtectList.first();
		KIO::ListJob* job = KIO::listRecursive( KUrl( m_configProtectDir ), false );
		connect( job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ), SLOT( slotListFiles( KIO::Job*, const KIO::UDSEntryList& ) ) );
		connect( job, SIGNAL( result( KIO::Job* ) ), SLOT( slotFinished() ) );
		m_configProtectList.pop_front();
	}
	else {
		emit signalScanCompleted();
	}
}

/**
 * Collect new configuration files.
 */
void EtcUpdate::slotListFiles( KIO::Job*, const KIO::UDSEntryList& entries )
{
	QString configFile;
    foreach( KIO::UDSEntry entry, entries ) {
        configFile = entry.stringValue( KIO::UDSEntry::UDS_NAME );
		
		if ( configFile.contains( QRegExp( "\\d{8}_\\d{4}/" ) ) && !configFile.endsWith( ".orig" ) ) {
			m_backupFilesList += m_configProtectDir + "/" + configFile;
        } else {
			if ( !m_configProtectDir.startsWith( "/var/cache/kuroo" ) && configFile.contains( "._cfg" ) )
				m_etcFilesList += m_configProtectDir + "/" + configFile;
		}
	}
}


/**
 * Launch diff tool with first etc-file in list.
 */
void EtcUpdate::runDiff( const QString& source, const QString& destination, const bool& isNew )
{
	struct stat st;

	if ( !source.isEmpty() ) {
		m_changed = false;
		m_source = source;
		m_destination = destination;
        QString backupPath = kurooDir + "backup/configuration/";
		
		// Check for etc-files warnings
		QString etcWarning;
        const QStringList etcFilesWarningsList = KurooConfig::etcFiles().split( " " );
        foreach( QString file, etcFilesWarningsList ) {
            if ( file == m_destination ) {
				etcWarning = i18n("<font color=red>Warning!<br>%1 has been edited by you.</font><br>").arg( m_destination );
            }
        }

        eProc->close();
// 		*eProc << KurooConfig::etcUpdateTool() << m_source << m_destination;
		*eProc << "kdiff3" << m_source << m_destination;
		
		if ( isNew )
			*eProc << "-o" << m_destination;
		else
			*eProc << "-o" << backupPath + "merging";
		
        eProc->start( /*K3Process::NotifyOnExit, true*/ );
		
        if ( eProc->state() == QProcess::NotRunning ) {
			LogSingleton::Instance()->writeLog( i18n( "%1 didn't start." ).arg( KurooConfig::etcUpdateTool() ), ERROR );
            KMessageBox::sorry( 0, i18n( "%1 could not start!" ).arg( KurooConfig::etcUpdateTool() ), i18n( "Kuroo" ) );
		}
		else {
			LogSingleton::Instance()->writeLog( i18n("Merging changes in \'%1\'.").arg( m_destination ), KUROO );
			
			// get the original file mode
			memset(&st, 0, sizeof(st));
			m_mergedMode = -1;
            if( !(stat( m_destination.toAscii(), &st ) < 0) ) {
				m_mergedMode = (int)st.st_mode;
			}

			// Watch for changes
			m_mergingFile->addFile( m_destination );
			
			// Make temporary backup of original conf file
			KIO::file_copy( m_destination, backupPath + "merging.orig" , m_mergedMode, KIO::Overwrite | KIO::HideProgressInfo );
		}
	}
}

void EtcUpdate::slotChanged()
{
	m_changed = true;
}

/**
 * After diff tool completed, close all.
 * @param proc
 */
void EtcUpdate::slotCleanupDiff( int status )
{

	//Unregister the watcher
	m_mergingFile->removeFile( m_destination );

	if ( m_changed ) {
		
		QDateTime dt = QDateTime::currentDateTime();
        QString backupPath = kurooDir + "backup/configuration/";
		QString backupPathDir = backupPath + dt.toString( "yyyyMMdd_hhmm" ) + "/";
		QDir d( backupPathDir );
		if ( !d.exists() ) {
			QDir bc( backupPath );
			if( !bc.exists() )
				bc.mkdir( backupPath );
			d.mkdir( backupPathDir );
		}
		
		// Make backup of original file
		QString destination = m_destination;
		destination.replace( "/", ":" );

		//Change this to a move instead of copy so we don't leave the temp file around
		KIO::file_move( backupPath + "merging.orig", backupPathDir + destination + ".orig", m_mergedMode, KIO::Overwrite | KIO::HideProgressInfo );
		KIO::file_copy( m_destination, backupPathDir + destination, m_mergedMode, KIO::Overwrite );

		//This is only necessary because it seems that kdiff3 rewrites the mode.
		KIO::chmod( m_destination, m_mergedMode );

		KIO::file_delete( m_source );
		
        LogSingleton::Instance()->writeLog( i18n( "Deleting \'%1\'. Backup saved in %2." ).arg( m_source ).arg( kurooDir + "backup" ), KUROO );
		
		KurooDBSingleton::Instance()->addBackup( m_source, m_destination );
		emit signalEtcFileMerged();
	}
}

#include "etcupdate.moc"
