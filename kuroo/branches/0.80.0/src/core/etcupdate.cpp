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

/**
 * @class EtcUpdate
 * @short Handles etc-updates.
 * The external diff tool is launched for merging changes in etc config files.
 */
EtcUpdate::EtcUpdate( QObject* m_parent, const char* name )
	: QObject( m_parent, name ), 
	m_noFiles( true ), m_count( 0 ), m_totalEtcCount( 0 )
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
void EtcUpdate::askUpdate( const int& count )
{
	switch ( KMessageBox::questionYesNoWId( GlobalSingleton::Instance()->kurooViewId(), 
	                                        i18n("<qt>IMPORTANT: %1 config files in /etc need updating.<br>Do you want to merge changes?</qt>")
	                                        .arg( QString::number( count ) ), i18n( "Kuroo" ) ) ) {
		
		case KMessageBox::Yes :
			slotEtcUpdate();
	}
}

/**
 * Run etc-update.
 * @return success
 */
void EtcUpdate::slotEtcUpdate()
{
	if ( KurooConfig::etcUpdateTool().isEmpty() )
		KMessageBox::informationWId( GlobalSingleton::Instance()->kurooViewId(), i18n( "Please specify merge tool in settings!" ), i18n( "Kuroo" ) );
	else {
		m_etcUpdateLines.clear();
		m_diffSource = QString::null;
		m_noFiles = true;
		m_count = 1;
		m_totalEtcCount = 0;
		
		eProc->resetAll();
		*eProc << "etc-update" ;

		if ( !eProc->start( KProcess::NotifyOnExit, KProcess::All ) ) {
			LogSingleton::Instance()->writeLog( i18n("\netc-update didn't start. "), ERROR );
			return;
		}
		else {
			connect( eProc, SIGNAL( readReady( KProcIO* ) ), this, SLOT( slotEtcUpdateOutput( KProcIO* ) ) );
			connect( eProc, SIGNAL( processExited( KProcess* ) ), this, SLOT( slotCleanupEtcUpdate( KProcess* ) ) );
			LogSingleton::Instance()->writeLog( i18n( "\nRunning etc-update..." ), KUROO );
			KurooStatusBar::instance()->setProgressStatus( "Etc-update", i18n("Checking for configuration file updates...") );
			SignalistSingleton::Instance()->setKurooBusy( true );
			return;
		}
	}
}

/**
 * Collect output from etc-update and terminate it.
 * @param process
 */
void  EtcUpdate::slotEtcUpdateOutput( KProcIO* proc )
{
	QString line;
	
	while ( proc->readln( line, true ) != -1 ) {
		m_etcUpdateLines += line;
		LogSingleton::Instance()->writeLog( line, EMERGE );

		if ( line.contains("Please select a file") ) {
			m_noFiles = false;
			proc->writeStdin( (QString)"-1", true );
			proc->closeWhenDone();
		}
	}
}

/**
 * Terminate.
 * @param process
 */
void EtcUpdate::slotCleanupEtcUpdate( KProcess* )
{
	KurooStatusBar::instance()->setProgressStatus( "Etc-update", i18n("Done.") );
	
	disconnect( eProc, SIGNAL( readReady( KProcIO* ) ), this, SLOT( slotEtcUpdateOutput( KProcIO* ) ) );
	disconnect( eProc, SIGNAL( processExited( KProcess*) ), this, SLOT( slotCleanupEtcUpdate( KProcess* ) ) );
	
	if ( m_noFiles ) {
		SignalistSingleton::Instance()->setKurooBusy( false );
		KMessageBox::sorryWId( GlobalSingleton::Instance()->kurooViewId(), 
		                       i18n("There are no files that need to be updated through etc-update."), i18n("Etc-update") );
	}
	else {
		
		// Parse out etc-files list
		QRegExp rxEtcFiles( "(?:^\\d+\\)\\s+){0,1}(/\\S*)" );
		foreach( m_etcUpdateLines )
			if ( rxEtcFiles.search( *it ) > -1 )
				m_etcFilesList += rxEtcFiles.cap(1).stripWhiteSpace();
		
		m_totalEtcCount = m_etcFilesList.size() / 2;
		runDiff();
	}
}

/**
 * Launch diff tool with first etc-file in list.
 */
void EtcUpdate::runDiff()
{
	if ( m_totalEtcCount > 0 ) {
		QString destination = m_etcFilesList.first();
		m_etcFilesList.pop_front();
		QString source = m_etcFilesList.first();
		m_etcFilesList.pop_front();
		
		// Check for etc-files warnings
		QString etcWarning;
		const QStringList etcFilesWarningsList = QStringList::split( " ", KurooConfig::etcFiles() );
		foreach ( etcFilesWarningsList )
			if ( *it == destination )
				etcWarning = i18n("<font color=red>Warning!<br>%1 has been edited by you.</font><br>").arg( destination );
		
		// Ask user what to do with this etc-file
		switch ( KMessageBox::questionYesNoWId( GlobalSingleton::Instance()->kurooViewId(), i18n("<qt>%1Do you want to merge changes in %2?</qt>")
		                                        .arg( etcWarning, destination ), 
		                                     i18n("etc-update (%1 of %2)").arg( m_count++ ).arg( m_totalEtcCount ) ) ) {
			
			// Merge this etc-file
			case KMessageBox::Yes : {
				
				eProc->resetAll();
				*eProc << KurooConfig::etcUpdateTool() << source << destination;
				eProc->start( KProcess::NotifyOnExit, true );
				
				m_diffSource = source;
				connect( eProc, SIGNAL( processExited( KProcess* ) ), this, SLOT( slotCleanupEtcUpdateDiff( KProcess* ) ) );
				
				if ( !eProc->isRunning() ) {
					LogSingleton::Instance()->writeLog( i18n( "%1 didn't start." ).arg( KurooConfig::etcUpdateTool() ), ERROR );
					KMessageBox::sorryWId( GlobalSingleton::Instance()->kurooViewId(), i18n( "%1 could not start!" )
					                       .arg( KurooConfig::etcUpdateTool() ), i18n( "Kuroo" ) );
					SignalistSingleton::Instance()->setKurooBusy( false );
				}
				else {
					LogSingleton::Instance()->writeLog( i18n("Merging changes in \'%1\'.").arg( destination ), KUROO );
					backup( source, destination );
				}
			
				break;
			}
			
			// Move to next etc-file
			case KMessageBox::No : {
				LogSingleton::Instance()->writeLog( i18n( "Skipping \'%1\'. etc-update will ask again next time." ).arg( destination ), KUROO );
				runDiff();
			}
		}
	}
	else {
		SignalistSingleton::Instance()->setKurooBusy( false );
		LogSingleton::Instance()->writeLog( i18n( "etc-update completed." ), KUROO );
	}
}

/**
 * After diff tool, delete original diff file.
 * And go for next...
 * @param proc
 */
void EtcUpdate::slotCleanupEtcUpdateDiff( KProcess* proc )
{
	disconnect( proc, SIGNAL( processExited( KProcess* ) ), this, SLOT( slotCleanupEtcUpdateDiff( KProcess* ) ) );
	KIO::file_delete( m_diffSource );
	LogSingleton::Instance()->writeLog( i18n( "Deleting \'%1\'. Backup saved in %2." ).arg( m_diffSource )
	                                    .arg( GlobalSingleton::Instance()->kurooDir() + "backup" ), KUROO );
	m_diffSource = QString::null;
	
	// Move to next etc-file
	runDiff();
}

/**
 * Backup the original etc-files with timestamp and store in db.
 * @param source
 * @param destination
 */
void EtcUpdate::backup( const QString& source, const QString& destination )
{
	QDateTime dt = QDateTime::currentDateTime();
	QString backupSource = source.section( "/", -1, -1 ) + "_" + dt.toString( "yyyyMMdd_hhmm" );
	QString backupDestination = destination.section( "/", -1, -1 ) + "_" + dt.toString( "yyyyMMdd_hhmm" );
	
	KIO::file_copy( source, GlobalSingleton::Instance()->kurooDir() + "backup/" + backupSource,
	                -1, true, false, false );
	KIO::file_copy( destination, GlobalSingleton::Instance()->kurooDir() + "backup/" + backupDestination,
	                -1, true, false, false );
	
	KurooDBSingleton::Instance()->addBackup( backupSource, backupDestination );
	emit signalEtcFileMerged();
}

#include "etcupdate.moc"
