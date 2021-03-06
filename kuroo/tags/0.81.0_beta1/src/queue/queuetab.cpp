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
#include "statusbar.h"
#include "queuetab.h"
#include "queuelistview.h"
#include "packageinspector.h"
#include "versionview.h"
#include "packageversion.h"

#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qtooltip.h>

#include <kpushbutton.h>
#include <ktextbrowser.h>
#include <kdialogbase.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kuser.h>
#include <kaccel.h>
#include <kiconloader.h>

/**
 * @class QueueTab
 * @short Page for the installation queue.
 */
QueueTab::QueueTab( QWidget* parent, PackageInspector *packageInspector )
	: QueueBase( parent ), m_packageInspector( packageInspector ), m_hasCheckedQueue( false ), m_initialQueueTime( QString::null )
{
	// Rmb actions.
	connect( queueView, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ), 
	         this, SLOT( contextMenu( KListView*, QListViewItem*, const QPoint& ) ) );
	
	// Button actions.
	connect( pbCheck, SIGNAL( clicked() ), this, SLOT( slotCheck() ) );
	connect( pbClear, SIGNAL( clicked() ), this, SLOT( slotClear() ) );
	connect( pbRemove, SIGNAL( clicked() ), this, SLOT( slotRemove() ) );
	connect( pbAdvanced, SIGNAL( clicked() ), this, SLOT( slotAdvanced() ) );
	connect( queueView, SIGNAL( doubleClicked( QListViewItem*, const QPoint&, int ) ), this, SLOT( slotAdvanced() ) );
	
	connect( cbRemove, SIGNAL( clicked() ), this, SLOT( slotRemoveInstalled() ) );
	
	connect( queueView, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotPackage() ) );
	connect( queueView, SIGNAL( selectionChanged() ), this, SLOT( slotButtons() ) );
	
	// Lock/unlock if kuroo is busy
	connect( SignalistSingleton::Instance(), SIGNAL( signalKurooBusy( bool ) ), this, SLOT( slotBusy() ) );
	
	// Reload view after changes in queue.
	connect( QueueSingleton::Instance(), SIGNAL( signalQueueChanged( bool ) ), this, SLOT( slotReload( bool ) ) );
	connect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotRefresh() ) );
	
	// Forward emerge start/stop/completed to package progressbar.
	connect( QueueSingleton::Instance(), SIGNAL( signalPackageStart( const QString& ) ), queueView, SLOT( slotPackageStart( const QString& ) ) );
	connect( QueueSingleton::Instance(), SIGNAL( signalPackageComplete( const QString& ) ), queueView, SLOT( slotPackageComplete( const QString& ) ) );
	connect( QueueSingleton::Instance(), SIGNAL( signalPackageAdvance() ), queueView, SLOT( slotPackageProgress() ) );

	// Update Queue summary timer
	connect( QueueSingleton::Instance(), SIGNAL( signalPackageAdvance() ), this, SLOT( slotQueueSummary() ) );
	connect( QueueSingleton::Instance(), SIGNAL( signalPackageComplete( const QString& ) ), this, SLOT( slotQueueSummary() ) );
	
	// Recalculate package when user change settings in Inspector
	connect( m_packageInspector, SIGNAL( signalPackageChanged() ), this, SLOT( slotPackage() ) );
	connect( m_packageInspector, SIGNAL( signalNextPackage( bool ) ), this, SLOT( slotNextPackage( bool ) ) );
	connect( m_packageInspector, SIGNAL( hidden() ), this, SLOT( slotButtons() ) );
	
	slotInit();
}

/**
 * Save listview geometry.
 */
QueueTab::~QueueTab()
{
	if ( cbForceConf->isChecked() )
		KurooConfig::setForceConf( true );
	else
		KurooConfig::setForceConf( false );
	
	if ( cbDownload->isChecked() )
		KurooConfig::setDownload( true );
	else
		KurooConfig::setDownload( false );
	
	if ( cbNoWorld->isChecked() )
		KurooConfig::setNoWorld( true );
	else
		KurooConfig::setNoWorld( false );
	
	if ( cbRemove->isChecked() )
		KurooConfig::setRemove( true );
	else
		KurooConfig::setRemove( false );
}

/**
 * Initialize Queue view.
 */
void QueueTab::slotInit()
{
	queueFrame->setPaletteBackgroundColor( colorGroup().base() );
	
	if ( KurooConfig::forceConf() )
		cbForceConf->setChecked( true );
	else
		cbForceConf->setChecked( false );
	
	if ( KurooConfig::download() )
		cbDownload->setChecked( true );
	else
		cbDownload->setChecked( false );
	
	if ( KurooConfig::noWorld() )
		cbNoWorld->setChecked( true );
	else
		cbNoWorld->setChecked( false );
	
	if ( KurooConfig::remove() )
		cbRemove->setChecked( true );
	else
		cbRemove->setChecked( false );
	
	slotRemoveInstalled();
	
	QToolTip::add( cbDownload, i18n(  "<qt><table width=300><tr><td>Instead of doing any package building, "
	                                  "just perform fetches for all packages (the main package as well as all dependencies), "
	                                  "grabbing all potential files.</td></tr></table></qt>" ) );
	
	QToolTip::add( cbNoWorld, i18n(   "<qt><table width=300><tr><td>Emerge as normal, "
	                                  "but do not add the packages to the world profile for later updating.</td></tr></table></qt>" ) );
	
	QToolTip::add( cbForceConf, i18n( "<qt><table width=300><tr><td>Causes portage to disregard merge records indicating that a config file"
	                                  "inside of a CONFIG_PROTECT directory has been merged already. "
	                                  "Portage will normally merge those files only once to prevent the user"
	                                  "from dealing with the same config multiple times. "
	                                  "This flag will cause the file to always be merged.</td></tr></table></qt>" ) );
	
	// Keyboard shortcuts
	KAccel* pAccel = new KAccel( this );
	pAccel->insert( "View package details...", i18n("View package details..."), i18n("View package details..."),
	                Qt::Key_Return, this, SLOT( slotAdvanced() ) );
	
	pbRemove->setIconSet( SmallIconSet("remove") );
	pbClear->setIconSet( SmallIconSet("remove_all") );
	pbAdvanced->setIconSet( SmallIconSet("options") );
	pbCheck->setIconSet( SmallIconSet("gear") );
	pbGo->setIconSet( SmallIconSet("launch") );
}

/**
 * Forward signal from next-buttons.
 * @param isNext
 */
void QueueTab::slotNextPackage( bool isNext )
{
	if ( !m_packageInspector->isParentView( VIEW_QUEUE ) )
		return;
	
	queueView->nextPackage( isNext );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Queue view slots
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Reload queue when package view is changed, fex when package is removed.
 */
void QueueTab::slotRefresh()
{
	if ( !QueueSingleton::Instance()->isQueueBusy() )
		queueView->insertPackageList( m_hasCheckedQueue );
}

/**
 * Load Queue packages.
 */
void QueueTab::slotReload( bool hasCheckedQueue )
{
	// Reenable the inspector after queue changes
// 	m_packageInspector->setDisabled( true );
	pbAdvanced->setDisabled( true );
	
	// If user is not su emerge pretend will not set packages as checked
	m_hasCheckedQueue = hasCheckedQueue;
	if ( m_hasCheckedQueue && !KUser().isSuperUser() )
		m_hasCheckedQueue = false;
	
	// Load all packages
	slotRefresh();
	
	// Enable the gui
	slotBusy();
	
	m_initialQueueTime = GlobalSingleton::Instance()->formatTime( queueView->totalDuration() );
    KurooStatusBar::instance()->clearElapsedTime();
	slotQueueSummary();
}

/**
 * View current queue summary.
 */
void QueueTab::slotQueueSummary()
{
	queueBrowser->clear();
	QString queueBrowserLines(   i18n( "<table width=100% border=0 cellpadding=0><tr><td colspan=2><b>Summary</b></td></tr>" ) );
			queueBrowserLines += i18n( "<tr><td width=10%>Number&nbsp;of&nbsp;packages:</td><td> %1</td></tr>" ).arg( queueView->count() );
			queueBrowserLines += i18n( "<tr><td width=10%>Initial&nbsp;estimated&nbsp;time:</td><td> %1</td></tr>" ).arg( m_initialQueueTime );
			queueBrowserLines += i18n( "<tr><td width=10%>Elapsed&nbsp;time:</td><td> %1</td></tr>" )
		.arg( GlobalSingleton::Instance()->formatTime( KurooStatusBar::instance()->elapsedTime() ) );
			queueBrowserLines += i18n( "<tr><td width=10%>Estimated&nbsp;time&nbsp;remaining:</td><td> %1</td></tr></table>" )
		.arg( GlobalSingleton::Instance()->formatTime( queueView->totalDuration() ) );
	queueBrowser->setText( queueBrowserLines );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Toggle button slots
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Disable/enable buttons when kuroo busy signal is received.
 */
void QueueTab::slotBusy()
{
	// No db or queue is empty - no fun!
	if ( !SignalistSingleton::Instance()->isKurooReady() || KurooDBSingleton::Instance()->isQueueEmpty() ) {
		pbRemove->setDisabled( true );
		pbAdvanced->setDisabled( true );
		pbClear->setDisabled( true );
		cbDownload->setDisabled( true );
		cbForceConf->setDisabled( true );
		cbNoWorld->setDisabled( true );
		pbCheck->setDisabled( true );
		pbGo->setDisabled( true );
	}
	else
		slotButtons();
}

/**
 * Disable buttons if no package is selected or kuroo is busy emerging.
 */
void QueueTab::slotButtons()
{
	if ( m_packageInspector->isVisible() )
		return;
	
	// Kuroo is busy emerging toggle to "abort"
	if ( EmergeSingleton::Instance()->isRunning() ) {
		pbGo->setText( i18n( "Abort Installation" ) );
		disconnect( pbGo, SIGNAL( clicked() ), this, SLOT( slotGo() ) );
		disconnect( pbGo, SIGNAL( clicked() ), this, SLOT( slotStop() ) );
		connect( pbGo, SIGNAL( clicked() ), this, SLOT( slotStop() ) );
	}
	else {
		pbGo->setText( i18n( "Step &2: Start Installation" ) );
		disconnect( pbGo, SIGNAL( clicked() ), this, SLOT( slotGo() ) );
		disconnect( pbGo, SIGNAL( clicked() ), this, SLOT( slotStop() ) );
		connect( pbGo, SIGNAL( clicked() ), this, SLOT( slotGo() ) );
	}
	
	// No package selected, disable all buttons
	if ( queueView->selectedId().isEmpty() ) {
		pbRemove->setDisabled( true );
		pbAdvanced->setDisabled( true );
		return;
	}
	
	// Queue is not empty - enable button "Remove all" and "Check Installation"
	cbDownload->setDisabled( false );
	
	// When emerging packages do not allow user to change the queue
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() ) {
		pbRemove->setDisabled( true );
		pbClear->setDisabled( true );
		pbCheck->setDisabled( true );
	}
	else {
		pbRemove->setDisabled( false );
		pbClear->setDisabled( false );
		pbCheck->setDisabled( false );
	}
	
	// User is su and packages in queue are "checked" - enable checkboxes
	if ( m_hasCheckedQueue && KUser().isSuperUser() ) {
		pbGo->setDisabled( false );
		cbForceConf->setDisabled( false );
		cbNoWorld->setDisabled( false );
		cbRemove->setDisabled( false );
	}
	else {
		pbGo->setDisabled( true );
		cbForceConf->setDisabled( true );
		cbNoWorld->setDisabled( true );
		cbRemove->setDisabled( true );
	}
	
	m_packageInspector->setDisabled( false );
	pbAdvanced->setDisabled( false );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Package slots
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Emerge all packages in the installation queue.
 */
void QueueTab::slotCheck()
{
	// Only user-end packages not the dependencies
	QStringList packageList = queueView->allPackagesNoChildren();
	EmergeSingleton::Instance()->pretend( packageList );
}

/**
 * Emerge all packages in the installation queue.
 */
void QueueTab::slotGo()
{
	// If emerge is running I'm the abort function
	if ( EmergeSingleton::Instance()->isRunning() )
		slotStop();
	
	// Only user-end packages not the dependencies
	QStringList packageList = queueView->allPackagesNoChildren();
	
	// Only download? prepend --fetch-all-uri
	// Else, let's install the user-end packages
	if ( cbDownload->isChecked() ) {
		switch( KMessageBox::questionYesNoList( this, 
			i18n("Do you want to Download following packages?"), packageList, i18n("Installation queue"),
			KStdGuiItem::yes(), KStdGuiItem::no(), "dontAskAgainDownload", KMessageBox::Dangerous ) ) {
				
				case KMessageBox::Yes:
					packageList.prepend( "--fetch-all-uri" );
					QueueSingleton::Instance()->installQueue( packageList );
					KurooStatusBar::instance()->setTotalSteps( queueView->totalDuration() );
			}
	}
	else {
		switch( KMessageBox::questionYesNoList( this, 
			i18n("Do you want to install following packages?"), packageList, i18n("Installation queue"),
			KStdGuiItem::yes(), KStdGuiItem::no(), "dontAskAgainInstall", KMessageBox::Dangerous ) ) {
				
				case KMessageBox::Yes: {
					
					// Force portage to reinstall files protected in CONFIG_PROTECT
					if ( cbForceConf->isChecked() )
						packageList.prepend( "--noconfmem" );
					
					// Emerge as normal, but do not add the packages to the world profile for later updating.
					if ( cbNoWorld->isChecked() )
						packageList.prepend( "--oneshot" );
						
					QueueSingleton::Instance()->installQueue( packageList );
					KurooStatusBar::instance()->setTotalSteps( queueView->totalDuration() );
				}
				
			}
	}
}

/**
 * Kill the running emerge process.
 */
void QueueTab::slotStop()
{
	switch ( KMessageBox::warningYesNo( this,
		i18n( "Do you want to abort the running installation?" ) ) ) {
			
			case KMessageBox::Yes : 
				EmergeSingleton::Instance()->stop();
				KurooStatusBar::instance()->setProgressStatus( QString::null, i18n("Done.") );
		}
}

/**
 * Launch emerge pretend of packages in queue.
 */
void QueueTab::slotPretend()
{
	PortageSingleton::Instance()->pretendPackageList( queueView->allId() );
}

/**
 * Remove package from Queue.
 */
void QueueTab::slotRemove()
{
	if ( isVisible() )
		m_packageInspector->hide();
	
	QueueSingleton::Instance()->removePackageIdList( queueView->selectedId() );
}

/**
 * Remove package from Queue.
 */
void QueueTab::slotClear()
{
	if ( isVisible() )
		m_packageInspector->hide();
	
	QueueSingleton::Instance()->reset();
}

/**
 * Remove package from Queue.
 */
void QueueTab::slotRemoveInstalled()
{
	QueueSingleton::Instance()->setRemoveInstalled( cbRemove->isChecked() );
}

/**
 * Open advanced dialog with: ebuild, versions, use flags...
 */
void QueueTab::slotAdvanced()
{
	DEBUG_LINE_INFO;
	pbRemove->setDisabled( true );
	pbClear->setDisabled( true );
	pbCheck->setDisabled( true );
	pbAdvanced->setDisabled( true );
	pbGo->setDisabled( true );
	
	if ( queueView->currentPackage() )
	     processPackage( true );
}

void QueueTab::slotPackage()
{
	if ( m_packageInspector->isVisible() )
		processPackage( true );
	else
		processPackage( false );
}

/**
 * Process package and view in Inspector.
 */
void QueueTab::processPackage( bool viewInspector )
{
	// Queue view is hidden don't update
	if ( m_packageInspector->isVisible() && !m_packageInspector->isParentView( VIEW_QUEUE ) )
		return;
	
	// Initialize the portage package object with package and it's versions data
	queueView->currentPackage()->parsePackageVersions();

	// Refresh inspector if visible
	if ( viewInspector )
		m_packageInspector->edit( queueView->currentPackage(), VIEW_QUEUE );
}

/**
 * Popup menu for current package.
 * @param item
 * @param point
 */
void QueueTab::contextMenu( KListView*, QListViewItem *item, const QPoint& point )
{
	if ( !item )
		return;
	
	const QStringList selectedIdList = queueView->selectedId();
	
	enum Actions { ADDWORLD, DELWORLD };
	
	KPopupMenu menu( this );
	
	int menuItem1 = menu.insertItem( ImagesSingleton::Instance()->icon( REMOVE ), i18n( "Remove" ), REMOVE );
	int menuItem2 = menu.insertItem( ImagesSingleton::Instance()->icon( DETAILS ), i18n( "Details..." ), DETAILS );
	
	int menuItem3;
	if ( !dynamic_cast<PackageItem*>( item )->isInWorld() )
		menuItem3 = menu.insertItem( ImagesSingleton::Instance()->icon( WORLD ), i18n( "Add to world" ), ADDWORLD );
	else
		menuItem3 = menu.insertItem( ImagesSingleton::Instance()->icon( WORLD ), i18n( "Remove from world" ), DELWORLD );
	menu.setItemEnabled( menuItem3, false );
	
	// No change to Queue when busy
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() )
		menu.setItemEnabled( menuItem1, false );
	
	// Allow editing of World when superuser
	if ( KUser().isSuperUser() )
		menu.setItemEnabled( menuItem3, true );
	
	if ( m_packageInspector->isVisible() ) {
		menu.setItemEnabled( menuItem1, false );
		menu.setItemEnabled( menuItem2, false );
		menu.setItemEnabled( menuItem3, false );
	}
	
	switch( menu.exec( point ) ) {
			
		case REMOVE:
			QueueSingleton::Instance()->removePackageIdList( queueView->selectedId() );
			break;
		
		case DETAILS:
			slotAdvanced();
			break;
		
		case ADDWORLD: {
			QStringList packageList;
			foreach ( selectedIdList )
				packageList += queueView->packageItemById( *it )->category() + "/" + queueView->packageItemById( *it )->name();
			PortageSingleton::Instance()->appendWorld( packageList );
			break;
		}
		
		case DELWORLD: {
			QStringList packageList;
			foreach ( selectedIdList )
				packageList += queueView->packageItemById( *it )->category() + "/" + queueView->packageItemById( *it )->name();
			PortageSingleton::Instance()->removeFromWorld( packageList );
		}
	}
}

#include "queuetab.moc"
