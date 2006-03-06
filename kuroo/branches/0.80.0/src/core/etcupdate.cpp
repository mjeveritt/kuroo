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
	noFiles( true ), count( 0 ), totalEtcCount( 0 )
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
	switch ( KMessageBox::questionYesNo( 0, i18n("<qt>IMPORTANT: %1 config files in /etc need updating.<br>Do you want to merge changes?</qt>")
										 .arg( QString::number( count ) ), i18n( "Kuroo" ) ) ) {
		
		case KMessageBox::Yes :
			etcUpdate();
	}
}

/**
 * Run etc-update.
 * @return success
 */
bool EtcUpdate::etcUpdate()
{
	if ( KurooConfig::etcUpdateTool().isEmpty() )
		KMessageBox::information( 0, i18n( "Please specify merge tool in settings!" ), i18n( "Kuroo" ) );
	else {
		etcUpdateLines.clear();
		diffSource = QString::null;
		noFiles = true;
		count = 1;
		totalEtcCount = 0;
		
		eProc->resetAll();
		*eProc << "etc-update" ;

		if ( !eProc->start( KProcess::NotifyOnExit, KProcess::All ) ) {
			LogSingleton::Instance()->writeLog( i18n("\netc-update didn't start. "), ERROR );
			return false;
		}
		else {
			connect( eProc, SIGNAL( readReady( KProcIO* ) ), this, SLOT( slotEtcUpdateOutput( KProcIO* ) ) );
			connect( eProc, SIGNAL( processExited( KProcess* ) ), this, SLOT( slotCleanupEtcUpdate( KProcess* ) ) );
			LogSingleton::Instance()->writeLog( i18n( "\nRunning etc-update..." ), KUROO );
			KurooStatusBar::instance()->setProgressStatus( "Etc-update", i18n("Checking for configuration file updates...") );
			SignalistSingleton::Instance()->setKurooBusy( true );
			return true;
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
		etcUpdateLines += line;
		LogSingleton::Instance()->writeLog( line, EMERGE );

		if ( line.contains("Please select a file") ) {
			noFiles = false;
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
	
	if ( noFiles ) {
		KMessageBox::sorry( 0, i18n("There are no files that need to be updated through etc-update."), i18n("Etc-update") );
		SignalistSingleton::Instance()->setKurooBusy( false );
	}
	else {
		
		// Parse out etc-files
		QRegExp rxEtcFiles( "(?:^\\d+\\))*\\s+(/\\s*\\S*)" );
		foreach( etcUpdateLines )
			if ( rxEtcFiles.search( *it ) > -1 )
				etcFilesList += rxEtcFiles.cap(1).stripWhiteSpace();
		
		runDiff();
	}
}

/**
 * Launch diff tool with first etc-file in list.
 */
void EtcUpdate::runDiff()
{
	totalEtcCount = etcFilesList.size() / 2;
	if ( totalEtcCount > 0 ) {
		QString destination = etcFilesList.first();
		etcFilesList.pop_front();
		QString source = etcFilesList.first();
		etcFilesList.pop_front();
		
		// Check for etc warning
		QString etcWarning;
		const QStringList etcFilesList = QStringList::split( "\n", KurooConfig::etcFiles() );
		foreach ( etcFilesList )
			if ( *it == destination )
				etcWarning = i18n("<font color=red>Warning!<br>%1 has been edited by you.</font><br>").arg( destination );
		
		switch ( KMessageBox::questionYesNo( 0, i18n("<qt>%1Do you want to merge changes in %2?</qt>").arg( etcWarning, destination ), i18n("etc-update (%1 of %2)").arg( count++ ).arg( totalEtcCount ) ) ) {
			
			// Merge this etc-file
			case KMessageBox::Yes : {
				
				eProc->resetAll();
				*eProc << KurooConfig::etcUpdateTool() << source << destination;
				eProc->start( KProcess::NotifyOnExit, true );
				
				diffSource = source;
				connect( eProc, SIGNAL( processExited( KProcess* ) ), this, SLOT( slotCleanupEtcUpdateDiff( KProcess* ) ) );
				
				if ( !eProc->isRunning() ) {
					LogSingleton::Instance()->writeLog( i18n( "%1 didn't start." ).arg( KurooConfig::etcUpdateTool() ), ERROR );
					KMessageBox::sorry( 0, i18n( "%1 could not start!" ).arg( KurooConfig::etcUpdateTool() ), i18n( "Kuroo" ) );
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
	KIO::file_delete( diffSource );
	LogSingleton::Instance()->writeLog( i18n( "Deleting \'%1\'. Backup saved in %2." ).arg( diffSource ).arg( KUROODIR + "backup" ), KUROO );
	diffSource = QString::null;
	
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
	
	KIO::file_copy( source, KUROODIR + "backup/" + backupSource );
	KIO::file_copy( destination, KUROODIR + "backup/" + backupDestination );
	
	KurooDBSingleton::Instance()->addBackup( backupSource, backupDestination );
	
	emit signalEtcFileMerged();
}

#include "etcupdate.moc"
