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
#include "packageinspector.h"

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
 * @class Kuroolito
 * @short Main kde window with menus, system tray icon and statusbar.
 */
Kuroolito::Kuroolito()
	: MainWindow( 0, "Kuroolitolito" ),
	kurooInit( new KuroolitoInit( this, "KuroolitoInit" ) ),
	kurooMessage( new Message( this ) ),
// 	m_view( new KuroolitoView( this, "KuroolitoView" ) ), 
	systemTray( new SystemTray( this ) ),
	prefDialog( 0 ), wizardDialog( 0 ), m_shuttingDown( false )
{
	GlobalSingleton::Instance()->setColorTheme();
	
	// Create the package inspector
	PackageInspector *packageInspector = new PackageInspector( this );
	
	// Add all pages
	viewPortage = new PortageTab( this, packageInspector );
	
	setCentralWidget( viewPortage );
	setupActions();
	statusBar();
	setupGUI();
		
	// Add system tray icon
	if ( KuroolitoConfig::isSystrayEnabled() )
		systemTray->activate();
	
	connect( systemTray, SIGNAL( quitSelected() ), this, SLOT( slotQuit() ) );
	connect( systemTray, SIGNAL( signalPreferences() ), this, SLOT( slotPreferences() ) );
	
	// Lock/unlock if kuroo is busy.
	connect( SignalistSingleton::Instance(), SIGNAL( signalKuroolitoBusy( bool ) ), this, SLOT( slotBusy() ) );
	
	// when the last window is closed, the application should quit
	connect( kapp, SIGNAL( lastWindowClosed() ), kapp, SLOT( quit() ) );
	
	// Kuroolito must initialize with db first
	SignalistSingleton::Instance()->setKuroolitoReady( true );
	
	// Initialize with settings in make.conf
	prefDialog = KConfigDialog::exists( i18n( "settings" ) );
	if ( !prefDialog )
		prefDialog = new ConfigDialog( viewPortage, i18n( "settings" ), KuroolitoConfig::self() );
	
	// Zack Rusin's delayed initialization technique
// 	QTimer::singleShot( 0, m_view, SLOT( slotInit() ) );
	
	if ( !KuroolitoDBSingleton::Instance()->isPortageEmpty() )
		viewPortage->slotReload();
	else
		PortageSingleton::Instance()->slotRefresh();
}

/**
 * If necessary wait for job to finish before terminating.
 */
Kuroolito::~Kuroolito()
{
	int maxLoops( 99 );
	while ( true ) {
		if ( ThreadWeaver::instance()->isJobPending( "DBJob" ) || ThreadWeaver::instance()->isJobPending( "CachePortageJob" ) )
			::usleep( 100000 ); // Sleep 100 msec
		else
			break;
		
		if ( maxLoops-- == 0 ) {
			KMessageBox::error( 0, i18n("Kuroolito is not responding. Attempting to terminate kuroo!"), i18n("Terminating") );
			break;
		}
	}
}

void Kuroolito::slotWhatsThis( int tabIndex )
{
	if ( tabIndex == 5 )
		whatsThis();
}


/**
 * Build mainwindow menus and toolbar.
 */
void Kuroolito::setupActions()
{
	KStdAction::quit( this, SLOT( slotQuit() ), actionCollection() );
	KStdAction::preferences( this, SLOT( slotPreferences() ), actionCollection() );
	
	(void) new KAction( i18n("&Release information"), 0, KShortcut( CTRL + Key_W ),
	                    				this, SLOT( introWizard() ), actionCollection(), "information" );
	
	actionRefreshPortage = new KAction( i18n("&Refresh Packages"), 0, KShortcut( CTRL + Key_P ),
	                                    PortageSingleton::Instance() , SLOT( slotRefresh() ), actionCollection(), "refresh_portage" );
	
	actionRefreshUpdates = new KAction( i18n("&Refresh Updates"), 0, KShortcut( CTRL + Key_U ),
	                                    PortageSingleton::Instance() , SLOT( slotRefreshUpdates() ), actionCollection(), "refresh_updates" );
	
	actionSyncPortage = new KAction( i18n("&Sync Portage"), 0, KShortcut( CTRL + Key_S ),
	                          			this, SLOT( slotSync() ), actionCollection(), "sync_portage" );
	
	createGUI();
}

/**
 * Disable buttons when kuroo is busy.
 */
void Kuroolito::slotBusy()
{
// 	if ( SignalistSingleton::Instance()->isKuroolitoBusy() || EmergeSingleton::Instance()->isRunning() ) {
// 		actionRefreshPortage->setEnabled( false );
// 		actionRefreshUpdates->setEnabled( false );
// 	}
// 	else {
// 		actionRefreshPortage->setEnabled( true );
// 		actionRefreshUpdates->setEnabled( true );
// 		
// 		// Make sure progressbar is stopped!
// 		KuroolitoStatusBar::instance()->stopTimer();
// 	}
	
// 	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKuroolitoBusy() || 
// 	     !KUser().isSuperUser() || KuroolitoDBSingleton::Instance()->isPortageEmpty() ) {
// 		actionSyncPortage->setEnabled( false );
// 	}
// 	else {
// 		actionSyncPortage->setEnabled( true );
// 	}
// 	
	// No db no fun!
	if ( !SignalistSingleton::Instance()->isKuroolitoReady() ) {
		actionRefreshPortage->setEnabled( false );
		actionRefreshUpdates->setEnabled( false );
		actionSyncPortage->setEnabled( false );
	}
}

/**
 * Launch emerge portage sync.
 */
void Kuroolito::slotSync()
{
	KLocale *loc = KGlobal::locale();
	QDateTime t;
	QString timeStamp( KuroolitoDBSingleton::Instance()->getKuroolitoDbMeta( "syncTimeStamp" ) );
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
void Kuroolito::slotPreferences()
{
	prefDialog = KConfigDialog::exists( i18n( "settings" ) );
	if ( !prefDialog )
		prefDialog = new ConfigDialog( viewPortage, i18n( "settings" ), KuroolitoConfig::self() );
	prefDialog->show();
	prefDialog->raise();
	prefDialog->setActiveWindow();
}

/**
 * Show the wizard.
 */
void Kuroolito::introWizard()
{
	if ( !wizardDialog )
		wizardDialog = new IntroDlg( this );
	
	wizardDialog->show();
}

/**
 * Hide or minimize kuroo window when clicking in close button.
 */
bool Kuroolito::queryClose()
{
	if ( !m_shuttingDown ) {
		if ( !KuroolitoConfig::isSystrayEnabled() )
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
bool Kuroolito::queryExit()
{
	return true;
}

/**
 * Backup emerge and merge history entries to text file.
 * Wait for the backup of the log is completed before terminating.
 */
void Kuroolito::slotQuit()
{
/*	KuroolitoDBSingleton::Instance()->backupDb();*/
// 	KIO::Job *backupLogJob = LogSingleton::Instance()->backupLog();
// 	if ( backupLogJob != NULL )
// 		connect( backupLogJob, SIGNAL( result( KIO::Job* ) ), SLOT( slotWait() ) );
// 	else
		slotWait();
}

/**
 * Abort any running threads.
 */
void Kuroolito::slotWait()
{
	if ( SignalistSingleton::Instance()->isKuroolitoBusy() ) {
		switch( KMessageBox::questionYesNo( this, 
			i18n("<qt>Kuroolitolito is busy<br><br>"
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
void Kuroolito::slotTerminate()
{
	m_shuttingDown = true;
	close();
}

#include "kuroo.moc"
