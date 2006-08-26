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

#include <kprocio.h>
#include <kmessagebox.h>
#include <kio/job.h>
#include <kinputdialog.h>

/**
 * @class EtcUpdate
 * @short Handles etc-updates.
 * 
 * Runs etc-update to collect the list of etc-files that needs merging.
 * Launches the exernal diff-tool for selected files.
 */
EtcUpdate::EtcUpdate( QObject* m_parent, const char* name )
	: QObject( m_parent, name )
{
	eProc = new KProcIO();
}

EtcUpdate::~EtcUpdate()
{
	delete eProc;
	eProc = 0;
}

void EtcUpdate::init( QObject *parent )
{
	m_parent = parent;
}

/**
 * Ask user if to run etc-update.
 * @param count
 */
// void EtcUpdate::askUpdate( const int& count )
// {
// 	switch ( KMessageBox::questionYesNoWId( GlobalSingleton::Instance()->kurooViewId(), 
// 	                                        i18n("<qt>IMPORTANT: %1 config files in /etc need updating.<br>Do you want to merge changes?</qt>")
// 	                                        .arg( QString::number( count ) ), i18n( "Kuroo" ) ) ) {
// 		
// 		case KMessageBox::Yes :
// 			slotEtcUpdate();
// 	}
// }

/**
 * Scan for new configuration files.
 */
void EtcUpdate::slotEtcUpdate()
{
	DEBUG_LINE_INFO;
	if ( KurooConfig::etcUpdateTool().isEmpty() )
		KMessageBox::informationWId( GlobalSingleton::Instance()->kurooViewId(), i18n( "Please specify merge tool in settings!" ), i18n( "Kuroo" ) );
	else {
		m_etcFilesList.clear();
		m_backupFilesList.clear();
		m_diffSource = QString::null;
		
		// First collect old merged files
		m_configProtectList = GlobalSingleton::Instance()->kurooDir() + "backup/configuration";
		
		// Then scan for new unmerged files
		m_configProtectList += QStringList::split( " ", KurooConfig::configProtectList() );
		
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
		KIO::ListJob* job = KIO::listRecursive( KURL( m_configProtectDir ), false );
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
	for ( KIO::UDSEntryList::ConstIterator entryIt = entries.begin(); entryIt != entries.end(); ++entryIt) {
		for ( KIO::UDSEntry::ConstIterator it = (*entryIt).begin(); it != (*entryIt).end(); it++ ) {
			if ( (*it).m_uds == KIO::UDS_NAME )
				configFile = (*it).m_str;
		}
		
		if ( configFile.contains( QRegExp( "\\d{8}_\\d{4}/._cfg" ) ) ) {
			m_backupFilesList += m_configProtectDir + "/" + configFile;
		}
		else {
			if ( !m_configProtectDir.startsWith( "/var/cache/kuroo" ) && configFile.contains( "._cfg" ) )
				m_etcFilesList += m_configProtectDir + "/" + configFile;
		}
	}
}

QStringList EtcUpdate::confFilesList()
{
	return m_etcFilesList;
}

QStringList EtcUpdate::backupFilesList()
{
	return m_backupFilesList;
}

/**
 * Launch diff tool with first etc-file in list.
 */
void EtcUpdate::runDiff( const QString& source, const QString& destination, bool isNew )
{
	if ( !source.isEmpty() ) {
		
		// Check for etc-files warnings
		QString etcWarning;
		const QStringList etcFilesWarningsList = QStringList::split( " ", KurooConfig::etcFiles() );
		foreach ( etcFilesWarningsList )
			if ( *it == destination )
				etcWarning = i18n("<font color=red>Warning!<br>%1 has been edited by you.</font><br>").arg( destination );

		eProc->resetAll();
		*eProc << KurooConfig::etcUpdateTool() << source << destination;
		eProc->start( KProcess::NotifyOnExit, true );
		
		m_diffSource = source;
		connect( eProc, SIGNAL( processExited( KProcess* ) ), this, SLOT( slotCleanupEtcUpdateDiff( KProcess* ) ) );
		
		if ( !eProc->isRunning() ) {
			LogSingleton::Instance()->writeLog( i18n( "%1 didn't start." ).arg( KurooConfig::etcUpdateTool() ), ERROR );
			KMessageBox::sorryWId( GlobalSingleton::Instance()->kurooViewId(), i18n( "%1 could not start!" )
									.arg( KurooConfig::etcUpdateTool() ), i18n( "Kuroo" ) );
		}
		else {
			LogSingleton::Instance()->writeLog( i18n("Merging changes in \'%1\'.").arg( destination ), KUROO );
			
			// Backup files if external to backup directory, eg first time merging them
			if ( isNew ) {
				backup( source, destination );
				emit signalEtcFileMerged();
			}
		}
	}
}

/**
 * After diff tool, delete original diff file.
 * @param proc
 */
void EtcUpdate::slotCleanupEtcUpdateDiff( KProcess* proc )
{
	DEBUG_LINE_INFO;
	disconnect( proc, SIGNAL( processExited( KProcess* ) ), this, SLOT( slotCleanupEtcUpdateDiff( KProcess* ) ) );
	m_diffSource = QString::null;
}

/**
 * Backup the original etc-files with timestamp and store in db.
 * @param source
 * @param destination
 */
void EtcUpdate::backup( const QString& source, const QString& destination )
{
	DEBUG_LINE_INFO;
	QDateTime dt = QDateTime::currentDateTime();
	QString backupPath = GlobalSingleton::Instance()->kurooDir() + "backup/configuration/" + dt.toString( "yyyyMMdd_hhmm" );
	QDir d( backupPath );
	if ( !d.exists() )
		d.mkdir( backupPath );
	
	KIO::file_copy( source, backupPath + "/" + source.section( "/", -1, -1 ), -1, true, false, false );
	KIO::file_copy( destination, backupPath + "/" + destination.section( "/", -1, -1 ), -1, true, false, false );
	KIO::file_delete( m_diffSource );
	LogSingleton::Instance()->writeLog( i18n( "Deleting \'%1\'. Backup saved in %2." ).arg( m_diffSource )
			.arg( GlobalSingleton::Instance()->kurooDir() + "backup" ), KUROO );
	
	KurooDBSingleton::Instance()->addBackup( source, destination );
}

#include "etcupdate.moc"
