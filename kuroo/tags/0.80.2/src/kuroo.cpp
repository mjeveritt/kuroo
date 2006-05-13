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
#include "threadweaver.h"
#include "systemtray.h"
#include "kurooinit.h"
#include "kuroo.h"
#include "statusbar.h"
#include "introdlg.h"
#include "portagetab.h"
#include "logstab.h"

#include <unistd.h>

#include <qdragobject.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qtimer.h>
#include <qcheckbox.h>

#include <kdeversion.h>
#include <kstatusbar.h>
#include <kaccel.h>
#include <kfiledialog.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kmessagebox.h>
#include <ktabwidget.h>
#include <kuser.h>
#include <kio/job.h>

/**
 * @class Kuroo
 * @short Main kde window with menus, system tray icon and statusbar.
 */
Kuroo::Kuroo()
	: MainWindow( 0, "Kuroo" ),
	kurooInit( new KurooInit( this, "KurooInit" ) ),
	kurooMessage( new Message( this ) ),
	m_view( new KurooView( this, "KurooView" ) ), systemTray( new SystemTray( this ) ),
	prefDialog( 0 ), wizardDialog( 0 ), m_shuttingDown( false )
{
	// Get pointer so MessageBox's can be made modal to kuroo windown and more...
	GlobalSingleton::Instance()->setKurooView( m_view );
	
	setCentralWidget( m_view );
	setupActions();
	statusBar();
	setupGUI();
		
	// Add system tray icon
	if ( KurooConfig::isSystrayEnabled() )
		systemTray->activate();
	
	connect( systemTray, SIGNAL( quitSelected() ), this, SLOT( slotQuit() ) );
	connect( systemTray, SIGNAL( signalPreferences() ), this, SLOT( slotPreferences() ) );
	
	// Lock/unlock if kuroo is busy.
	connect( SignalistSingleton::Instance(), SIGNAL( signalKurooBusy( bool ) ), this, SLOT( slotBusy() ) );
	
	// when the last window is closed, the application should quit
	connect( kapp, SIGNAL( lastWindowClosed() ), kapp, SLOT( quit() ) );
	
	// Kuroo must initialize with db first
	SignalistSingleton::Instance()->setKurooReady( false );
	
	// Initialize with settings in make.conf
	prefDialog = KConfigDialog::exists( i18n( "settings" ) );
	if ( !prefDialog )
		prefDialog = new ConfigDialog( m_view, i18n( "settings" ), KurooConfig::self() );
	
	// Zack Rusin's delayed initialization technique
	QTimer::singleShot( 0, m_view, SLOT( slotInit() ) );
}

/**
 * If necessary wait for job to finish before terminating.
 */
Kuroo::~Kuroo()
{
	int maxLoops( 99 );
	while ( true ) {
		if ( ThreadWeaver::instance()->isJobPending( "DBJob" ) || ThreadWeaver::instance()->isJobPending( "CachePortageJob" ) )
			::usleep( 100000 ); // Sleep 100 msec
		else
			break;
		
		if ( maxLoops-- == 0 ) {
			KMessageBox::error( 0, i18n("Kuroo is not responding. Attempting to terminate kuroo!"), i18n("Terminating") );
			break;
		}
	}
}

/**
 * Build mainwindow menus and toolbar.
 */
void Kuroo::setupActions()
{
	KStdAction::quit( this, SLOT( slotQuit() ), actionCollection() );
	KStdAction::preferences( this, SLOT( slotPreferences() ), actionCollection() );
	
	(void) new KAction( i18n("&Information"), 0, KShortcut( CTRL + Key_W ),
	                    				this, SLOT( introWizard() ), actionCollection(), "information" );
	
	actionRefreshPortage = new KAction( i18n("&Refresh Packages"), 0, KShortcut( CTRL + Key_P ),
	                                    PortageSingleton::Instance() , SLOT( slotRefresh() ), actionCollection(), "refresh_portage" );
	
	actionRefreshUpdates = new KAction( i18n("&Refresh Updates"), 0, KShortcut( CTRL + Key_U ),
	                                    PortageSingleton::Instance() , SLOT( slotRefreshUpdates() ), actionCollection(), "refresh_updates" );
	
	actionSyncPortage = new KAction( i18n("&Sync Portage"), 0, KShortcut( CTRL + Key_S ),
	                          			this, SLOT( slotSync() ), actionCollection(), "sync_portage" );
	
	actionEtcUpdate = new KAction( i18n("&Run etc-update"), 0, KShortcut( CTRL + Key_E ),
	                               		EtcUpdateSingleton::Instance(), SLOT( slotEtcUpdate() ), actionCollection(), "etc_update" );
	
	createGUI();
}

/**
 * Disable buttons when kuroo is busy.
 */
void Kuroo::slotBusy()
{
	if ( SignalistSingleton::Instance()->isKurooBusy() || EmergeSingleton::Instance()->isRunning() ) {
		actionRefreshPortage->setEnabled( false );
		actionRefreshUpdates->setEnabled( false );
	}
	else {
		actionRefreshPortage->setEnabled( true );
		actionRefreshUpdates->setEnabled( true );
		
		// Make sure progressbar is stopped!
		KurooStatusBar::instance()->stopTimer();
	}
	
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() || 
	     !KUser().isSuperUser() || KurooDBSingleton::Instance()->isPortageEmpty() ) {
		actionSyncPortage->setEnabled( false );
		actionEtcUpdate->setEnabled( false );
	}
	else {
		actionSyncPortage->setEnabled( true );
		actionEtcUpdate->setEnabled( true );
	}
	
	// No db no fun!
	if ( !SignalistSingleton::Instance()->isKurooReady() ) {
		actionRefreshPortage->setEnabled( false );
		actionRefreshUpdates->setEnabled( false );
		actionSyncPortage->setEnabled( false );
	}
}

/**
 * Launch emerge portage sync.
 */
void Kuroo::slotSync()
{
	KLocale *loc = KGlobal::locale();
	QDateTime t;
	QString timeStamp( KurooDBSingleton::Instance()->getKurooDbMeta( "syncTimeStamp" ) );
	QString lastSyncDate( QString::null );
	
	if ( !timeStamp.isEmpty() ) {
		t.setTime_t( timeStamp.toUInt() );
		lastSyncDate = loc->formatDateTime(t);
	}
	
	switch( KMessageBox::questionYesNo( this, 
		i18n( "<qt>Do you want to synchronize portage?<br>"
		      "This will take a couple of minutes...</qt>" ), i18n( "Last sync: %1" ).arg( lastSyncDate ) ) ) {
			     
		case KMessageBox::Yes:
			PortageSingleton::Instance()->slotSync();
	}
}

/**
 * Open kuroo preferences window.
 */
void Kuroo::slotPreferences()
{
	prefDialog = KConfigDialog::exists( i18n( "settings" ) );
	if ( !prefDialog )
		prefDialog = new ConfigDialog( m_view, i18n( "settings" ), KurooConfig::self() );
	prefDialog->show();
	prefDialog->raise();
	prefDialog->setActiveWindow();
}

/**
 * Show the wizard.
 */
void Kuroo::introWizard()
{
	if ( !wizardDialog )
		wizardDialog = new IntroDlg( this );
	
	wizardDialog->show();
}

/**
 * Hide or minimize kuroo window when clicking in close button.
 */
bool Kuroo::queryClose()
{
	if ( !m_shuttingDown ) {
		if ( !KurooConfig::isSystrayEnabled() )
			slotQuit();
		else {
			hide();
			return false;
		}
	}
	
	return true;
}

/**
 * Bye, bye!
 */
bool Kuroo::queryExit()
{
	return true;
}

/**
 * Backup emerge and merge history entries to text file.
 * Wait for the backup of the log is completed before terminating.
 */
void Kuroo::slotQuit()
{
	KurooDBSingleton::Instance()->backupDb();
	KIO::Job *backupLogJob = LogSingleton::Instance()->backupLog();
	if ( backupLogJob != NULL )
		connect( backupLogJob, SIGNAL( result( KIO::Job* ) ), SLOT( slotWait() ) );
	else
		slotWait();
}

/**
 * Abort any running threads.
 */
void Kuroo::slotWait()
{
	if ( SignalistSingleton::Instance()->isKurooBusy() ) {
		switch( KMessageBox::questionYesNo( this, 
			i18n("<qt>Kuroo is busy<br><br>"
			     "Do you want to quit?<br>"
			     "All jobs will be aborted.</qt>"), i18n("Quit") ) ) {
				     
			case KMessageBox::Yes: {
				ThreadWeaver::instance()->abortAllJobsNamed( "DBJob" );
				ThreadWeaver::instance()->abortAllJobsNamed( "CachePortageJob" );
				QTimer::singleShot( 500, this, SLOT( slotTerminate() ) );
			}
		}
	}
	else
		slotTerminate();
}

/**
 * Terminate kuroo.
 */
void Kuroo::slotTerminate()
{
	m_shuttingDown = true;
	close();
}

#include "kuroo.moc"
