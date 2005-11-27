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
#include "kurooview.h"
#include "installedtab.h"
#include "portagetab.h"
#include "updatestab.h"
#include "updatelistview.h"
#include "queuetab.h"
#include "queuelistview.h"
#include "resultstab.h"
#include "resultlistview.h"
#include "logstab.h"
#include "packagelistview.h"

#include <sys/stat.h>

#include <qpainter.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qcolor.h>

#include <ktabwidget.h>
#include <ktextbrowser.h>
#include <ktrader.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <kuser.h>

/**
 * Gui content.
 * First create a tabwidget then fill it with 6 tabs for Installed, Portage, Updates...
 * Then connect signal that content of a tab is changed so the tabheader text color is activated = blue.
 * Also connect signals from all objects like Installed, Portage... to each of their corresponding tab.
 * Finally connect signal from when Portage is changed (reseted after sync) to clear all other objects.
 */
KurooView::KurooView( QWidget *parent, const char *name )
	: QWidget( parent, name ),
	DCOPObject( "KurooIface" ),
	mainTabs(0), tabInstalled(0), tabPortage(0), tabUpdates(0), tabQueue(0), tabResults(0), tabLogs(0)
{
    // setup our layout manager to automatically add our widgets
	QHBoxLayout *top_layout = new QHBoxLayout(this);
	top_layout->setAutoAdd(true);
	
	mainTabs = new KTabWidget( this, "mainTabs" );
	mainTabs->setEnabled( true );
	mainTabs->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, mainTabs->sizePolicy().hasHeightForWidth() ) );
	mainTabs->setMinimumSize( QSize( 650, 500 ) );
	mainTabs->setTabShape( QTabWidget::Rounded );
	mainTabs->setMargin( 0 );
 
	tabInstalled = new InstalledTab( this );
	mainTabs->insertTab( tabInstalled, i18n("Installed") );

	tabPortage = new PortageTab( this );
	mainTabs->insertTab( tabPortage, i18n("Portage") );
	
	tabUpdates = new UpdatesTab( this );
	mainTabs->insertTab( tabUpdates, i18n("Updates") );
	
	tabQueue = new QueueTab( this );
	mainTabs->insertTab( tabQueue, i18n("Queue") );
	
	tabResults = new ResultsTab( this );
	mainTabs->insertTab( tabResults, i18n("Results") );
	
	tabLogs = new LogsTab( this );
	mainTabs->insertTab( tabLogs, i18n("Logs") );
	
	// Give log access to logBrowser and checkboxes
	// Check emerge.log for new entries. (Due to cli activities outside kuroo)
	LogSingleton::Instance()->setGui( tabLogs->logBrowser, tabLogs->verboseLog, tabLogs->saveLog );
	
	connect( mainTabs, SIGNAL( currentChanged( QWidget* ) ), this, SLOT( slotCurrentChanged( QWidget* ) ) );

	// View this package info by making it current.
	connect( SignalistSingleton::Instance(), SIGNAL( signalViewPackage(const QString&) ), this, SLOT( slotViewPackage(const QString&) ) );
	
	connect( tabPortage, SIGNAL( signalChanged() ), this, SLOT( slotPortageUpdated() ) );
	connect( tabInstalled, SIGNAL( signalChanged() ), this, SLOT( slotInstalledUpdated() ) );
	connect( tabUpdates, SIGNAL( signalChanged() ), this, SLOT( slotUpdatesUpdated() ) );
	connect( tabQueue->queueView, SIGNAL( signalQueueLoaded() ), this, SLOT( slotQueueUpdated() ) );
	connect( tabResults->resultView, SIGNAL( signalResultsLoaded() ), this, SLOT( slotResultsUpdated() ) );
	connect( tabLogs->logBrowser, SIGNAL( textChanged() ), this, SLOT( slotLogsTabUpdated() ) );
	
	// Reset everything when a portage scan is started
	connect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotReset() ) );
}

KurooView::~KurooView()
{
}

/**
 * When gui drawing is completed on screen, start checking that database is uptodate.
 * First check if database is completely empty, then do a full portage scan.
 * Else check if emerge log shows emerge activity outside kuroo, then we need to update database.
 */
void KurooView::slotInit()
{
	if ( KurooDBSingleton::Instance()->isHistoryEmpty() )
		switch( KMessageBox::warningContinueCancel( this, 
			i18n("<qt>Kuroo database is empty!<br><br>"
			     "Kuroo will now first scan your emerge log to create the emerge history. "
			     "Next Kuroo will refresh Portage, Installed and Updates packages view.<br>"
			     "Package information in Portage will be cached.</qt>"), i18n("Initialize Kuroo"), KStdGuiItem::cont(), "dontAskAgainInitKuroo", 0) ) {
			case KMessageBox::Continue: {
				connect( HistorySingleton::Instance(), SIGNAL( signalHistoryChanged() ), this, SLOT( slotCheckPortage() ) );
				HistorySingleton::Instance()->slotRefresh();
			}
		}
	else {
		connect( HistorySingleton::Instance(), SIGNAL( signalHistoryChanged() ), this, SLOT( slotCheckPortage() ) );
		
		// Check if emerge log shows emerge activity outside kuroo, then we need to update database
		if ( !HistorySingleton::Instance()->slotRefresh() ) {
			disconnect( HistorySingleton::Instance(), SIGNAL( signalHistoryChanged() ), this, SLOT( slotCheckPortage() ) );
			
			switch( KMessageBox::warningYesNo( this, 
				i18n("<qt>Kuroo database needs refreshing!<br>"
				     "Emerge log shows that your system has changed.</qt>"), i18n("Initialize Kuroo"), i18n("Refresh"), i18n("Skip"), 0) ) {
				case KMessageBox::Yes: {
					connect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotCheckInstalled() ) );
					PortageSingleton::Instance()->slotRefresh();
					break;
				}
				default: {
					slotCheckPortage();
				}
			}
		}
	}
}

/**
 * Reset all views when a portage scan is started.
 */
void KurooView::slotReset()
{
	InstalledSingleton::Instance()->slotReset();
	UpdatesSingleton::Instance()->slotReset();
	QueueSingleton::Instance()->reset();
	ResultsSingleton::Instance()->reset();
	slotInit();
}

/**
 * Is Portage view empty?
 */
void KurooView::slotCheckPortage()
{
	disconnect( HistorySingleton::Instance(), SIGNAL( signalHistoryChanged() ), this, SLOT( slotCheckPortage() ) );
	
	if ( PortageSingleton::Instance()->count() == "0" ) {
		connect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotCheckInstalled() ) );
		PortageSingleton::Instance()->slotRefresh();
	}
	else {
		tabPortage->slotReload();
		slotCheckInstalled();
	}
}

/**
 * Is Installed view empty?
 */
void KurooView::slotCheckInstalled()
{
	disconnect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotCheckInstalled() ) );
	
	if ( InstalledSingleton::Instance()->count() == "0" ) {
		connect( InstalledSingleton::Instance(), SIGNAL( signalInstalledChanged() ), this, SLOT( slotCheckUpdates() ) );
		InstalledSingleton::Instance()->slotRefresh();
	}
	else {
		tabInstalled->slotReload();
		slotCheckUpdates();
	}
}

/**
 * Is Updates view empty?
 */
void KurooView::slotCheckUpdates()
{
	disconnect( InstalledSingleton::Instance(), SIGNAL( signalInstalledChanged() ), this, SLOT( slotCheckUpdates() ) );
	
	if ( UpdatesSingleton::Instance()->count() == "0" ) {
		connect( UpdatesSingleton::Instance(), SIGNAL( signalUpdatesChanged() ), this, SLOT( slotReloadQueueResults() ) );
		UpdatesSingleton::Instance()->slotRefresh();
	}
	else {
		slotReloadQueueResults();
	}
}

/**
 * Finish kuroo startup by loading the queue and the results packages list.
 */
void KurooView::slotReloadQueueResults()
{
	disconnect( UpdatesSingleton::Instance(), SIGNAL( signalUpdatesChanged() ), this, SLOT( slotReloadQueueResults() ) );
	
	tabInstalled->slotReload();
	tabUpdates->slotReload();
	tabQueue->slotReload();
	tabResults->slotReload();
	
	// Warn user that emerge need root permissions - many rmb actions are disabled
	if ( !KUser().isSuperUser() )
		KMessageBox::information( 0, i18n("You must run Kuroo as root to emerge packages!"), i18n("Information"), "dontAskAgainNotRoot" );
}

/**
 * Action when user click in close button
 */
void KurooView::quit()
{
}

/**
 * Change tab color back to normal.
 */
void KurooView::slotCurrentChanged( QWidget* newPage )
{
	mainTabs->setTabColor( newPage, black );
}

/**
 * When user choose "View package", go to corresponding tab and select the package for info.
 */
void KurooView::slotViewPackage( const QString& package )
{
	if ( PortageSingleton::Instance()->isInstalled( package ) ) {
		mainTabs->showPage( tabInstalled );
		tabInstalled->slotViewPackage( package );
	}
	else {
		mainTabs->showPage( tabPortage );
		tabPortage->slotViewPackage( package );
	}
}

/**
 * Dcop interface to emerge pretend process.
 * @param packageList	list of packages to emerge
 */
bool KurooView::slotEmergePretend( QString package )
{
	kdDebug() << "KurooView::slotEmergePretend package=" << package << endl;
	if ( EmergeSingleton::Instance()->pretend( package ) )
		return true;
	else
		return false;
}

/**
 * Count the total packages in Portage.
 */
void KurooView::slotPortageUpdated()
{
	static bool tabSetup(false);
	QString total = PortageSingleton::Instance()->count();
		
	mainTabs->setTabLabel( tabPortage, i18n("&Portage (%1)").arg( total ) );
	
	if ( mainTabs->currentPageIndex() != 1 && tabSetup )
		mainTabs->setTabColor( tabPortage, blue );
	
	tabSetup = true;
}

/**
 * Installed view package count.
 */
void KurooView::slotInstalledUpdated()
{
	static bool tabSetup(false);
	QString total = InstalledSingleton::Instance()->count();

	mainTabs->setTabLabel( tabInstalled, i18n("&Installed (%1)").arg( total ) );
	
	if ( mainTabs->currentPageIndex() != 0 && tabSetup )
		mainTabs->setTabColor( tabInstalled, blue );
	
	tabSetup = true;
}

/**
 * Update view package count.
 */
void KurooView::slotUpdatesUpdated()
{
	static bool tabSetup(false);
	QString total = UpdatesSingleton::Instance()->count();
		
	mainTabs->setTabLabel( tabUpdates, i18n("&Updates (%1)").arg( total ) );
	
	if ( mainTabs->currentPageIndex() != 2 && tabSetup )
		mainTabs->setTabColor( tabUpdates, blue );
	
	tabSetup = true;
}

/**
 * Queue view package count.
 */
void KurooView::slotQueueUpdated()
{
	static bool tabSetup(false);
	QString total = QueueSingleton::Instance()->count();
	
	mainTabs->setTabLabel( tabQueue, i18n("&Queue (%1)").arg( total ) );
	
	if ( mainTabs->currentPageIndex() != 3 && tabSetup )
		mainTabs->setTabColor( tabQueue, blue );
	
	tabSetup = true;
}

/**
 * Results view package count.
 */
void KurooView::slotResultsUpdated()
{
	static bool tabSetup(false);
	QString total = ResultsSingleton::Instance()->count();
	
	mainTabs->setTabLabel( tabResults, i18n("&Results (%1)").arg( total ) );
	
	if ( mainTabs->currentPageIndex() != 4 && tabSetup )
		mainTabs->setTabColor( tabResults, blue );
	
	tabSetup = true;
}

/**
 * Catch when log tab is updated.
 */
void KurooView::slotLogsTabUpdated()
{
	static bool tabSetup(false);
	
	if ( mainTabs->currentPageIndex() != 5 && tabSetup )
		mainTabs->setTabColor( tabLogs, blue );
	
	tabSetup = true;
}

#include "kurooview.moc"

