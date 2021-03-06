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

#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>

#include <ktextbrowser.h>
#include <kdialogbase.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kuser.h>

/**
 * @class QueueTab
 * @short Page for the installation queue.
 */
QueueTab::QueueTab( QWidget* parent, PackageInspector *packageInspector )
	: QueueBase( parent ), m_packageInspector( packageInspector ), m_hasCheckedQueue( false ), initialQueueTime( QString::null )
{
	// Rmb actions.
	connect( queueView, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ), 
	         this, SLOT( contextMenu( KListView*, QListViewItem*, const QPoint& ) ) );
	
	// Button actions.
	connect( pbClear, SIGNAL( clicked() ), QueueSingleton::Instance(), SLOT( reset() ) );
	connect( pbRemove, SIGNAL( clicked() ), this, SLOT( slotRemove() ) );
	connect( pbAdvanced, SIGNAL( clicked() ), this, SLOT( slotAdvanced() ) );
	
	connect( cbRemove, SIGNAL( clicked() ), this, SLOT( slotRemoveInstalled() ) );
	
	connect( queueView, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotPackage() ) );
	connect( queueView, SIGNAL( selectionChanged() ), this, SLOT( slotButtons() ) );
	
	connect( m_packageInspector, SIGNAL( signalPackageChanged() ), this, SLOT( slotPackage() ) );
	connect( m_packageInspector, SIGNAL( signalUseChanged() ), this, SLOT( slotPackageUseChanged() ) );
	
	// Lock/unlock if kuroo is busy.
	connect( SignalistSingleton::Instance(), SIGNAL( signalKurooBusy( bool ) ), this, SLOT( slotBusy( bool ) ) );
	
	connect( SignalistSingleton::Instance(), SIGNAL( signalEmergeQueue() ), this, SLOT( slotGo() ) );
	
	// Reload view after changes in queue.
	connect( QueueSingleton::Instance(), SIGNAL( signalQueueChanged( bool ) ), this, SLOT( slotReload( bool ) ) );
	
	// Forward emerge start/stop/completed to package progressbar.
	connect( QueueSingleton::Instance(), SIGNAL( signalPackageStart( const QString& ) ), queueView, SLOT( slotPackageStart( const QString& ) ) );
	connect( QueueSingleton::Instance(), SIGNAL( signalPackageComplete( const QString&, bool ) ), queueView, SLOT( slotPackageComplete( const QString&, bool ) ) );
	connect( QueueSingleton::Instance(), SIGNAL( signalPackageAdvance() ), queueView, SLOT( slotPackageProgress() ) );
	connect( QueueSingleton::Instance(), SIGNAL( signalPackageStart( const QString& ) ), this, SLOT( slotQueueSummary() ) );
	
	slotInit();
}

/**
 * Save listview geometry.
 */
QueueTab::~QueueTab()
{
}

/**
 * Initialize Queue view.
 */
void QueueTab::slotInit()
{
	pbRemove->setDisabled( true );
	pbAdvanced->setDisabled( true );
	cbForce->setDisabled( true );
	cbRemove->setDisabled( true );
	
	connect( m_packageInspector, SIGNAL( signalNextPackage( bool ) ), queueView, SLOT( slotNextPackage( bool ) ) );
	slotBusy( false );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Queue view slots
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Load Queue packages.
 */
void QueueTab::slotReload( bool hasCheckedQueue )
{
	m_hasCheckedQueue = hasCheckedQueue;
	
	queueView->insertPackageList( m_hasCheckedQueue );
	
	if ( m_hasCheckedQueue && !KUser().isSuperUser() )
		m_hasCheckedQueue = false;
	
	slotBusy( false );
	
	initialQueueTime = queueView->totalTimeFormatted();
	slotQueueSummary();
}

/**
 * After package use has changed, clear use column and go back to "Check Installation".
 */
void QueueTab::slotPackageUseChanged()
{
	m_hasCheckedQueue = false;
	KurooDBSingleton::Instance()->clearQueuePackageUse();
	queueView->clearQueuePackageUse();
	slotBusy( false );
}

/**
 * View current queue summary.
 */
void QueueTab::slotQueueSummary()
{
	QString queueBrowserLines( i18n( "<b>Summary</b><br>" ) );
			queueBrowserLines += i18n( "Number of packages: %1<br>" ).arg( queueView->count() );
			queueBrowserLines += i18n( "Estimated time for installation: %1<br>" ).arg( initialQueueTime );
			queueBrowserLines += i18n( "Estimated time remaining: %1<br>" ).arg( queueView->totalTimeFormatted() );
	
	queueBrowser->clear();
	queueBrowser->setText( queueBrowserLines );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Toggle button slots
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Disable/enable buttons when kuroo busy signal is received.
 * @param busy
 */
void QueueTab::slotBusy( bool busy )
{
// 	kdDebug() << "QueueTab::slotBusy busy=" << busy << endl;
	
	if ( EmergeSingleton::Instance()->isRunning() ) {
		pbGo->setText( i18n( "Abort" ) );
		disconnect( pbGo, SIGNAL( clicked() ), this, SLOT( slotGo() ) );
		disconnect( pbGo, SIGNAL( clicked() ), this, SLOT( slotStop() ) );
		connect( pbGo, SIGNAL( clicked() ), this, SLOT( slotStop() ) );
	}
	else {
		disconnect( pbGo, SIGNAL( clicked() ), this, SLOT( slotGo() ) );
		disconnect( pbGo, SIGNAL( clicked() ), this, SLOT( slotStop() ) );
		connect( pbGo, SIGNAL( clicked() ), this, SLOT( slotGo() ) );
		
		if ( m_hasCheckedQueue && KUser().isSuperUser() ) {
			pbGo->setText( i18n( "Start Installation" ) );
			cbForce->setDisabled( false );
			cbRemove->setDisabled( false );
		}
		else {
			pbGo->setText( i18n("Check Installation") );
			cbForce->setDisabled( true );
			cbRemove->setDisabled( true );
		}
	}
	
	// No db no fun!
	if ( !SignalistSingleton::Instance()->isKurooReady() || queueView->count() == "0" || busy ) {
		pbRemove->setDisabled( true );
		pbAdvanced->setDisabled( true );
		pbClear->setDisabled( true );
		cbDownload->setDisabled( true );
		cbForce->setDisabled( true );
		cbRemove->setDisabled( true );
	}
	else {
		pbRemove->setDisabled( false );
		pbAdvanced->setDisabled( false );
		pbClear->setDisabled( false );
		cbDownload->setDisabled( false );
	}
	
	if ( !SignalistSingleton::Instance()->isKurooReady() || queueView->count() == "0" )
		pbGo->setDisabled( true );
	else
		pbGo->setDisabled( false );
}

/**
 * Disable buttons if no package is selected or kuroo is busy emerging.
 */
void QueueTab::slotButtons()
{
	if ( queueView->selectedId().isEmpty() ) {
		pbRemove->setDisabled( true );
		pbAdvanced->setDisabled( true );
		return;
	}
	
	if ( !EmergeSingleton::Instance()->isRunning() ) {
		pbRemove->setDisabled( false );
		pbAdvanced->setDisabled( false );
	}
	else {
		pbRemove->setDisabled( true );
		pbAdvanced->setDisabled( true );
	}
	
	pbAdvanced->setDisabled( false );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Package slots
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Emerge all packages in the installation queue.
 */
void QueueTab::slotGo()
{
// 	kdDebug() << "QueueTab::slotGo m_hasCheckedQueue=" << m_hasCheckedQueue << endl;
	
	// If emerge is running I'm the abort function
	if ( EmergeSingleton::Instance()->isRunning() )
		slotStop();
	
	// First we must run emerge pretend
	if ( !m_hasCheckedQueue ) {
		PortageSingleton::Instance()->pretendPackageList( queueView->allId() );
		return;
	}
	
	// Only user-end packages not the dependencies
	QStringList packageList = queueView->allPackagesNoChildren();
	
	// Only download? prepend --fetch-all-uri
	// Else, let's install the user-end packages
	if ( cbDownload->isChecked() ) {
		switch( KMessageBox::questionYesNoList( this, 
		                                        i18n("Do you want to Download following packages?"), packageList, i18n("Installation queue"),
		                                        KStdGuiItem::yes(), KStdGuiItem::no(), "dontAskAgainInstall", KMessageBox::Dangerous ) ) {
			                                        
													case KMessageBox::Yes:
														packageList.prepend( "--fetch-all-uri" );
														QueueSingleton::Instance()->installPackageList( packageList );
														KurooStatusBar::instance()->setTotalSteps( queueView->sumTime() );
		                                        
		                              			}
	}
	else {
		switch( KMessageBox::questionYesNoList( this, 
		                                        i18n("Do you want to install following packages?"), packageList, i18n("Installation queue"),
		                                        KStdGuiItem::yes(), KStdGuiItem::no(), "dontAskAgainDownload", KMessageBox::Dangerous ) ) {
			                                        
													case KMessageBox::Yes: {
														
														// Force portage to reinstall files protected in CONFIG_PROTECT
														if ( cbForce->isChecked() )
															packageList.prepend( "--noconfmem" );
														
														QueueSingleton::Instance()->installPackageList( packageList );
														KurooStatusBar::instance()->setTotalSteps( queueView->sumTime() );
														m_hasCheckedQueue = false;
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
												QueueSingleton::Instance()->stopTimer();
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
	QueueSingleton::Instance()->removePackageIdList( queueView->selectedId() );
}

/**
 * Remove package from Queue.
 */
void QueueTab::slotRemoveInstalled()
{
	if ( cbRemove->isChecked() )
		QueueSingleton::Instance()->setRemoveInstalled( true );
	else
		QueueSingleton::Instance()->setRemoveInstalled( false );
}

/**
 * Open advanced dialog with: ebuild, versions, use flags...
 */
void QueueTab::slotAdvanced()
{
	if ( queueView->currentPackage() ) {
		slotPackage();
		m_packageInspector->edit( queueView->currentPackage() );
	}
}

/**
 * Load package item with all version data...
 */
void QueueTab::slotPackage()
{
	if ( !isVisible() )
		return;
	
	// clear text browsers and dropdown menus
	m_packageInspector->dialog->versionsView->clear();
	m_packageInspector->dialog->cbVersionsEbuild->clear();
	m_packageInspector->dialog->cbVersionsDependencies->clear();
	m_packageInspector->dialog->cbVersionsInstalled->clear();
	m_packageInspector->dialog->cbVersionsUse->clear();
	m_packageInspector->dialog->cbVersionsSpecific->clear();
	
	// Initialize the portage package object with package and it's versions data
	queueView->currentPackage()->initVersions();

	// Now parse sorted list of versions for current package
	QValueList<PackageVersion*> sortedVersions = queueView->currentPackage()->sortedVersionList();
	bool versionNotInArchitecture = false;
	QValueList<PackageVersion*>::iterator sortedVersionIterator;
	QString latestVersion;
	for ( sortedVersionIterator = sortedVersions.begin(); sortedVersionIterator != sortedVersions.end(); sortedVersionIterator++ ) {
		
		// Load all dropdown menus in the inspector with relevant versions
		m_packageInspector->dialog->cbVersionsEbuild->insertItem( (*sortedVersionIterator)->version() );
		m_packageInspector->dialog->cbVersionsDependencies->insertItem( (*sortedVersionIterator)->version() );
		m_packageInspector->dialog->cbVersionsUse->insertItem( (*sortedVersionIterator)->version() );
		m_packageInspector->dialog->cbVersionsSpecific->insertItem( (*sortedVersionIterator)->version() );
		
		// Mark official version stability for version listview
		QString stability;
		if ( (*sortedVersionIterator)->isOriginalHardMasked() )
			stability = i18n("Hardmasked");
		else
			if ( (*sortedVersionIterator)->isOriginalTesting() )
				stability = i18n("Testing");
			else
				if ( (*sortedVersionIterator)->isAvailable() )
					stability = i18n("Stable");
				else
					if ( (*sortedVersionIterator)->isNotArch() )
						stability = i18n("Not on %1").arg( KurooConfig::arch() );
					else
						stability = i18n("Not available");
		
		// Insert version in Inspector version view
		m_packageInspector->dialog->versionsView->insertItem( (*sortedVersionIterator)->version(), stability, (*sortedVersionIterator)->size(), (*sortedVersionIterator)->isInstalled() );
		
		// Mark installed version
		if ( (*sortedVersionIterator)->isInstalled() )
			m_packageInspector->dialog->cbVersionsInstalled->insertItem( (*sortedVersionIterator)->version() );
		
		// Collect latest available version
		if ( (*sortedVersionIterator)->isAvailable() )
			latestVersion = (*sortedVersionIterator)->version();
	}
	
	// Set active version in Inspector dropdown menus
	if ( !latestVersion.isEmpty() ) {
		m_packageInspector->dialog->cbVersionsEbuild->setCurrentText( latestVersion );
		m_packageInspector->dialog->cbVersionsDependencies->setCurrentText( latestVersion );
		m_packageInspector->dialog->cbVersionsUse->setCurrentText( latestVersion );
		m_packageInspector->dialog->versionsView->usedForInstallation( latestVersion );
	}
	
	// Refresh inspector if visible
	if ( m_packageInspector->isVisible() )
		m_packageInspector->edit( queueView->currentPackage() );
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
	
	enum Actions { REMOVE, OPTIONS, TOWORLD };
	
	KPopupMenu menu( this );
	int menuItem1 = menu.insertItem( i18n( "Remove" ), REMOVE );
	int menuItem2 = menu.insertItem( i18n( "Options..." ), OPTIONS );
	int menuItem3 = menu.insertItem( i18n( "Add to world" ), TOWORLD );
	
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() ) {
		menu.setItemEnabled( menuItem1, false );
		
		if ( !KUser().isSuperUser() )
			menu.setItemEnabled( menuItem3, false );
	}
	
	switch( menu.exec( point ) ) {
			
		case REMOVE:
			QueueSingleton::Instance()->removePackageIdList( queueView->selectedId() );
			break;
		
		case OPTIONS:
			slotAdvanced();
			break;
		
		case TOWORLD:
			PortageSingleton::Instance()->appendWorld( queueView->currentPackage()->category(), queueView->currentPackage()->name() );
		
	}
}

#include "queuetab.moc"
