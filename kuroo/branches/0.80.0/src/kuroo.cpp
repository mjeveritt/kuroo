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
#include "emergelogdlg.h"
#include "installedtab.h"
#include "portagetab.h"
#include "updatestab.h"
#include "logstab.h"

#include <unistd.h>

#include <qdragobject.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qregexp.h>
#include <qtimer.h>

#include <kglobal.h>
#include <kiconloader.h>
#include <kdeversion.h>
#include <kstatusbar.h>
#include <kaccel.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kmessagebox.h>
#include <ktabwidget.h>
#include <kuser.h>
#include <kio/job.h>

/**
 * Create main window with menus, system tray icon and statusbar.
 * First launch KurooInit to check the integrity of kuroo setup.
 */
Kuroo::Kuroo()
	: MainWindow( 0, "Kuroo" ),
	kurooInit( new KurooInit(this, "KurooInit") ),
	kurooMessage( new Message(this, "Message") ),
	m_view( new KurooView(this, "KurooView") ),
	prefDialog(0), wizardDialog(0), emergeLogDialog(0), m_shuttingDown(false)
{
	setCentralWidget(m_view);
	setupActions();
	statusBar();
	setupGUI();
	
	// Add system tray icon
	SystemTray *systemTray = new SystemTray( this );
	systemTray->show();
	
	connect( systemTray, SIGNAL( quitSelected() ), this, SLOT( slotQuit() ) );
	connect( systemTray, SIGNAL( signalPreferences() ), this, SLOT( slotPreferences() ) );
	
	// Change toolbar buttons meaning
	connect( m_view->mainTabs, SIGNAL( currentChanged(QWidget*) ), this, SLOT( activateToolbar(QWidget*) ) );
	
	// Lock/unlock if kuroo is busy.
	connect( SignalistSingleton::Instance(), SIGNAL( signalKurooBusy(bool) ), this, SLOT( slotBusy(bool) ) );
	
	// when the last window is closed, the application should quit
	connect( qApp, SIGNAL( lastWindowClosed() ), qApp, SLOT( quit() ) );
	
	actionRefresh->setToolTip( i18n("Refresh Portage view") );
	actionFind->setToolTip( i18n("Find packages in Portage") );
	actionSync->setToolTip( i18n("Synchronize Portage with Gentoo mirrors") );
	connect( actionFind, SIGNAL( activated() ), m_view->tabPortage, SLOT( slotFind() ) );
	connect( actionRefresh, SIGNAL( activated() ), m_view->tabPortage, SLOT( slotRefresh() ) );
	
	if ( SignalistSingleton::Instance()->isKurooBusy() || EmergeSingleton::Instance()->isRunning() ) {
		actionRefresh->setEnabled(false);
	}
	else {
		actionRefresh->setEnabled(true);
	}
	
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() || !KUser().isSuperUser() || KurooDBSingleton::Instance()->isPortageEmpty() ) {
		actionSync->setEnabled(false);
	}
	else
		actionSync->setEnabled(true);
	
	if ( SignalistSingleton::Instance()->isKurooBusy() )
		actionFind->setEnabled(false);
	else
		actionFind->setEnabled(true);
	
	// Zack Rusin's delayed initialization technique
	QTimer::singleShot( 0, m_view, SLOT(slotInit()) );
}

Kuroo::~Kuroo()
{
}

/**
 * Build mainwindow menus and toolbar.
 */
void Kuroo::setupActions()
{
	KStdAction::quit( this, SLOT(slotQuit()), actionCollection() );
	KStdAction::preferences( this, SLOT(slotPreferences()), actionCollection() );
	
	(void) new KAction( i18n("&Wizard"), 0, KShortcut( CTRL + Key_W ),
	                    this, SLOT(introWizard()), actionCollection(), "wizard" );
	
	(void) new KAction( i18n("&Emerge Log"), 0, KShortcut( CTRL + Key_L ),
	                    this, SLOT(emergeLog()), actionCollection(), "emergeLog" );
	
	actionRefresh = new KAction( i18n("&Refresh"), "kuroo_refresh_view", KShortcut( CTRL + Key_R ),
	                             this, SLOT(slotNull()), actionCollection(), "refresh" );
	
	actionSync = new KAction( i18n("&Sync"), "kuroo_sync_portage", KShortcut( CTRL + Key_S ),
	                          this, SLOT(slotSync()), actionCollection(), "sync" );
	
	actionFind = new KAction( i18n("&Find"), "kuroo_find_packages", KShortcut( CTRL + Key_F ),
	                          this, SLOT(slotNull()), actionCollection(), "find" );
	
	createGUI();
}

/**
 * Disable buttons when kuroo is busy.
 */
void Kuroo::slotBusy( bool b )
{
	kdDebug() << "Kuroo::slotBusy" << endl;
// 	activateToolbar( m_view->mainTabs->currentPage() );
}

/**
 * Connect toolbar buttons to the right slot for each tab.
 * @param page
 */
void Kuroo::activateToolbar( QWidget *page )
{
// 	enum Actions { INSTALLED = 0, PORTAGE, UPDATES, QUEUE, RESULTS, LOGS };
// 	
// 	switch ( m_view->mainTabs->indexOf(page) ) {
// 		
// 		case INSTALLED: {
// 			if ( SignalistSingleton::Instance()->isKurooBusy() || EmergeSingleton::Instance()->isRunning() ||  KurooDBSingleton::Instance()->isPortageEmpty() )
// 				actionRefresh->setEnabled(false);
// 			else
// 				actionRefresh->setEnabled(true);
// 			
// 			if ( SignalistSingleton::Instance()->isKurooBusy() )
// 				actionFind->setEnabled(false);
// 			else
// 				actionFind->setEnabled(true);
// 			
// 			actionSync->setEnabled(false);
// 			
// 			actionRefresh->setToolTip( i18n("Refresh Installed view") );
// 			actionFind->setToolTip( i18n("Find packages in Installed") );
// 			actionSync->setToolTip( i18n("na") );
// 			
// 			disconnect( actionFind, SIGNAL( activated() ), m_view->tabPortage, SLOT( slotFind() ) );
// 			disconnect( actionFind, SIGNAL( activated() ), m_view->tabInstalled, SLOT( slotFind() ) );
// 			connect( actionFind, SIGNAL( activated() ), m_view->tabInstalled, SLOT( slotFind() ) );
// 			
// 			disconnect( actionRefresh, SIGNAL( activated() ), m_view->tabPortage, SLOT( slotRefresh() ) );
// 			disconnect( actionRefresh, SIGNAL( activated() ), m_view->tabUpdates, SLOT( slotRefresh() ) );
// 			disconnect( actionRefresh, SIGNAL( activated() ), m_view->tabInstalled, SLOT( slotRefresh() ) );
// 			connect( actionRefresh, SIGNAL( activated() ), m_view->tabInstalled, SLOT( slotRefresh() ) );
// 			
// 			break;
// 		}
// 			
// 		case PORTAGE: {
// 			if ( SignalistSingleton::Instance()->isKurooBusy() || EmergeSingleton::Instance()->isRunning() ) {
// 				actionRefresh->setEnabled(false);
// 			}
// 			else {
// 				actionRefresh->setEnabled(true);
// 			}
// 			
// 			if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() || !KUser().isSuperUser() || KurooDBSingleton::Instance()->isPortageEmpty() ) {
// 				actionSync->setEnabled(false);
// 			}
// 			else
// 				actionSync->setEnabled(true);
// 			
// 			if ( SignalistSingleton::Instance()->isKurooBusy() )
// 				actionFind->setEnabled(false);
// 			else
// 				actionFind->setEnabled(true);
// 			
// 			actionRefresh->setToolTip( i18n("Refresh Portage view") );
// 			actionFind->setToolTip( i18n("Find packages in Portage") );
// 			actionSync->setToolTip( i18n("Synchronize Portage with Gentoo mirrors") );
// 			
// 			disconnect( actionFind, SIGNAL( activated() ), m_view->tabPortage, SLOT( slotFind() ) );
// 			disconnect( actionFind, SIGNAL( activated() ), m_view->tabInstalled, SLOT( slotFind() ) );
// 			connect( actionFind, SIGNAL( activated() ), m_view->tabPortage, SLOT( slotFind() ) );
// 			
// 			disconnect( actionRefresh, SIGNAL( activated() ), m_view->tabInstalled, SLOT( slotRefresh() ) );
// 			disconnect( actionRefresh, SIGNAL( activated() ), m_view->tabUpdates, SLOT( slotRefresh() ) );
// 			disconnect( actionRefresh, SIGNAL( activated() ), m_view->tabPortage, SLOT( slotRefresh() ) );
// 			connect( actionRefresh, SIGNAL( activated() ), m_view->tabPortage, SLOT( slotRefresh() ) );
// 			
// 			break;
// 		}
// 		
// 		case UPDATES: {
// 			if ( SignalistSingleton::Instance()->isKurooBusy() || EmergeSingleton::Instance()->isRunning() ||  KurooDBSingleton::Instance()->isPortageEmpty() )
// 				actionRefresh->setEnabled(false);
// 			else
// 				actionRefresh->setEnabled(true);
// 			
// 			actionFind->setEnabled(false);
// 			actionSync->setEnabled(false);
// 		
// 			actionRefresh->setToolTip( i18n("Refresh Updates view") );
// 			actionFind->setToolTip( i18n("na") );
// 			actionSync->setToolTip( i18n("na") );
// 			
// 			disconnect( actionRefresh, SIGNAL( activated() ), m_view->tabInstalled, SLOT( slotRefresh() ) );
// 			disconnect( actionRefresh, SIGNAL( activated() ), m_view->tabPortage, SLOT( slotRefresh() ) );
// 			disconnect( actionRefresh, SIGNAL( activated() ), m_view->tabUpdates, SLOT( slotRefresh() ) );
// 			connect( actionRefresh, SIGNAL( activated() ), m_view->tabUpdates, SLOT( slotRefresh() ) );
// 			
// 			break;
// 		}
// 		
// 		case QUEUE: {
// 			actionRefresh->setEnabled(false);
// 			actionFind->setEnabled(false);
// 			actionSync->setEnabled(false);
// 			
// 			actionRefresh->setToolTip( i18n("na") );
// 			actionFind->setToolTip( i18n("na") );
// 			actionSync->setToolTip( i18n("na") );
// 			
// 			break;
// 		}
// 			
// 		case RESULTS: {
// 			actionRefresh->setEnabled(false);
// 			actionFind->setEnabled(false);
// 			actionSync->setEnabled(false);
// 			
// 			actionRefresh->setToolTip( i18n("na") );
// 			actionFind->setToolTip( i18n("na") );
// 			actionSync->setToolTip( i18n("na") );
// 			
// 			break;
// 		}
// 		
// 		case LOGS: {
// 			actionRefresh->setEnabled(false);
// 			actionFind->setEnabled(false);
// 			actionSync->setEnabled(false);
// 			
// 			actionRefresh->setToolTip( i18n("na") );
// 			actionFind->setToolTip( i18n("na") );
// 			actionSync->setToolTip( i18n("na") );
// 			
// 			disconnect( actionRefresh, SIGNAL( activated() ), m_view->tabInstalled, SLOT( slotRefresh() ) );
// 			disconnect( actionRefresh, SIGNAL( activated() ), m_view->tabPortage, SLOT( slotRefresh() ) );
// 			disconnect( actionRefresh, SIGNAL( activated() ), m_view->tabUpdates, SLOT( slotRefresh() ) );
// 		}
// 	}
}

/**
 * Needed to initialize setupActions.
 */
void Kuroo::slotNull()
{
}

/**
 * Launch emerge portage sync.
 */
void Kuroo::slotSync()
{
	QString lastSyncDate( KurooDBSingleton::Instance()->getLastSync().first() );
	
	if ( lastSyncDate.isEmpty() )
		lastSyncDate = i18n("na");
	
	actionSync->setEnabled(false);
	switch( KMessageBox::questionYesNo( this, 
		i18n("<qt>Do you want to synchronize portage?<br><br>"
		     "Portage, Installed and Updates view will be refreshed automatically afterwards. "
		     "Queue and Results view will be cleared.<br>"
		     "This will take a couple of minutes...</qt>"), i18n("Last sync: %1").arg(lastSyncDate) ) ) {
		case KMessageBox::Yes: {
			PortageSingleton::Instance()->slotSync();
			break;
		}
		case KMessageBox::No: {
			actionSync->setEnabled(true);
		}
	}
}

/**
 * Open kuroo preferences window.
 */
void Kuroo::slotPreferences()
{
	prefDialog = KConfigDialog::exists(i18n("settings"));
	if ( !prefDialog )
		prefDialog = new ConfigDialog( m_view, i18n("settings"), KurooConfig::self() );
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
		wizardDialog = new IntroDlg(this);
	
	wizardDialog->show();
}

/**
 * Show the full emerge log 
 */
void Kuroo::emergeLog()
{
	if ( !emergeLogDialog )
		emergeLogDialog = new EmergeLogDlg(this);
	else
		emergeLogDialog->loadLog();
	emergeLogDialog->show();
}

/**
 * Hide kuroo window when clicking in close button.
 */
bool Kuroo::queryClose()
{
	if( !m_shuttingDown ) {
		hide();
		return false;
	}
	else
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
 * Wait for the backup of the log is completed before terminating.
 */
void Kuroo::slotQuit()
{
	KIO::Job *backupLogJob = LogSingleton::Instance()->backupLog();
	if ( backupLogJob != NULL )
		connect( backupLogJob, SIGNAL( result( KIO::Job* ) ), SLOT( slotWait() ) );
	else
		slotWait();
}

/**
 * Aborted any running threads.
 * Allow time to abort before db connection is closed.
 * @fixme Wait for threads to terminate.
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
				QTimer::singleShot( 500, this, SLOT( slotTerminate()) );
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
	qApp->closeAllWindows();
}

#include "kuroo.moc"
