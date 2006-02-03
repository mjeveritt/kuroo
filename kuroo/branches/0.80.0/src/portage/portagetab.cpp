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
#include "search.h"
#include "categorieslistview.h"
#include "portagelistview.h"
#include "portagetab.h"
#include "packageinspector.h"
#include "packageversion.h"
#include "versionview.h"

#include <qlayout.h>
#include <qsplitter.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qtimer.h>

#include <ktextbrowser.h>
#include <ktabwidget.h>
#include <kpopupmenu.h>
#include <kdialogbase.h>
#include <kmessagebox.h>
#include <kuser.h>
#include <klineedit.h>

/**
 * @class PortageTab
 * @short Package view with filters.
 */
PortageTab::PortageTab( QWidget* parent, PackageInspector *packageInspector )
	: PortageBase( parent ), m_packageInspector( packageInspector ), queuedFilters( 0 )
{
	// Initialize category and subcategory views with all available data
	categoriesView->init();
	subcategoriesView->init();
	
	connect( filterGroup, SIGNAL( released( int ) ), this, SLOT( slotFilters() ) );
	connect( searchFilter, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotFilters() ));
	
	// Rmb actions.
	connect( packagesView, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ),
	         this, SLOT( contextMenu( KListView*, QListViewItem*, const QPoint& ) ) );
	
	// Button actions.
	connect( pbQueue, SIGNAL( clicked() ), this, SLOT( slotQueue() ) );
	connect( pbUninstall, SIGNAL( clicked() ), this, SLOT( slotUninstall() ) );
	connect( pbAdvanced, SIGNAL( clicked() ), this, SLOT( slotAdvanced() ) );
	connect( pbClearFilter, SIGNAL( clicked() ), this, SLOT( slotClearFilter() ) );
	
	// Toggle Queue button
	connect( QueueSingleton::Instance(), SIGNAL( signalQueueChanged( bool ) ), this, SLOT( slotInitButtons() ) );
	connect( packagesView, SIGNAL( signalStatusChanged( bool ) ), this, SLOT( slotButtons( bool ) ) );
	
	// Reload view after changes.
	connect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotReload() ) );
	connect( InstalledSingleton::Instance(), SIGNAL( signalInstalledChanged() ), this, SLOT( slotReload() ) );
	connect( UpdatesSingleton::Instance(), SIGNAL( signalUpdatesChanged() ), this, SLOT( slotReload() ) );
	
	// Lock/unlock actions when kuroo is busy.
	connect( SignalistSingleton::Instance(), SIGNAL( signalKurooBusy( bool ) ), this, SLOT( slotBusy( bool ) ) );
	
	// Load Inspector with current package info
	connect( packagesView, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotPackage() ) );
	
	slotInit();
}

PortageTab::~PortageTab()
{
}

/**
 * Initialize Portage view.
 */
void PortageTab::slotInit()
{
	connect( m_packageInspector, SIGNAL( signalNextPackage( bool ) ), packagesView, SLOT( slotNextPackage( bool ) ) );
	slotBusy( false );
}

/**
 * Populate view with portage packages.
 * Then load the emerge history.
 */
void PortageTab::slotReload()
{
	// Prepare categories by loading index
	disconnect( categoriesView, SIGNAL( selectionChanged() ), this, SLOT( slotListSubCategories() ) );
	disconnect( subcategoriesView, SIGNAL( selectionChanged() ), this, SLOT( slotListPackages() ) );
	categoriesView->init();
	subcategoriesView->init();
	connect( categoriesView, SIGNAL( selectionChanged() ), this, SLOT( slotListSubCategories() ) );
	connect( subcategoriesView, SIGNAL( selectionChanged() ), this, SLOT( slotListPackages() ) );
	
	slotFilters();
}

/**
 * Execute query based on filter and text. Add a delay of 250ms.
 */
void PortageTab::slotFilters()
{
	queuedFilters++;
	QTimer::singleShot( 250, this, SLOT( slotActivateFilters() ) );
}

/**
 * Execute query based on filter and text.
 */
void PortageTab::slotActivateFilters()
{
	--queuedFilters;
	if ( queuedFilters == 0 )
		categoriesView->loadCategories( KurooDBSingleton::Instance()->portageCategories( filterGroup->selectedId(),searchFilter->text()  ) );
}

/**
 * List packages when clicking on category in installed.
 */
void PortageTab::slotListSubCategories()
{
	QString categoryId = categoriesView->currentCategoryId();
	subcategoriesView->loadCategories( KurooDBSingleton::Instance()->portageSubCategories( categoryId, filterGroup->selectedId(), searchFilter->text() ) );
}

/**
 * List packages when clicking on subcategory.
 */
void PortageTab::slotListPackages()
{
	QString categoryId = categoriesView->currentCategoryId();
	QString subCategoryId = subcategoriesView->currentCategoryId();
	
	// Disable all buttons if query result is empty
	if ( packagesView->addSubCategoryPackages( KurooDBSingleton::Instance()->portagePackagesBySubCategory( categoryId, subCategoryId, filterGroup->selectedId(), searchFilter->text() ) ) == 0 ) {
		pbAdvanced->setDisabled( true );
		m_packageInspector->setDisabled( true );
		pbQueue->setDisabled( true );
		
		// Highlight text filter background in red if query failed
		if ( !searchFilter->text().isEmpty() )
			searchFilter->setPaletteBackgroundColor( QColor( KurooConfig::noMatchColor() ) );
		else
			searchFilter->setPaletteBackgroundColor( Qt::white );
		
		// User has edited package, reload the package
		disconnect( m_packageInspector, SIGNAL( signalPackageChanged() ), this, SLOT( slotPackage() ) );
	}
	else {
		pbAdvanced->setDisabled( false );
		m_packageInspector->setDisabled( false );
		if ( !EmergeSingleton::Instance()->isRunning() )
			pbQueue->setDisabled( false );
		
		// Highlight text filter background in green if query successful
		if ( !searchFilter->text().isEmpty() )
			searchFilter->setPaletteBackgroundColor( QColor( KurooConfig::matchColor() ) );
		else
			searchFilter->setPaletteBackgroundColor( Qt::white );
		
		connect( m_packageInspector, SIGNAL( signalPackageChanged() ), this, SLOT( slotPackage() ) );
	}
}

/**
 * Reset text filter.
 */
void PortageTab::slotClearFilter()
{
	searchFilter->clear();
}

/**
 * Refresh installed packages list.
 */
void PortageTab::slotRefresh()
{
	switch( KMessageBox::questionYesNo( this,
		i18n( "<qt>Do you want to refresh the Packages view?<br>"
		      "This will take a couple of minutes...</qt>"), i18n( "Refreshing Packages" ), KStdGuiItem::yes(), KStdGuiItem::no(), "dontAskAgainRefreshPortage" ) ) {
		case KMessageBox::Yes:
			PortageSingleton::Instance()->slotRefresh();
		
	}
}

/**
 * Disable/enable buttons when kuroo is busy.
 * @param busy
 */
void PortageTab::slotBusy( bool busy )
{
	if ( busy ) {
		pbUninstall->setDisabled( true );
		pbQueue->setDisabled( true );
	}
	else {
		if ( !KUser().isSuperUser() )
			pbUninstall->setDisabled( true );
		else
			pbUninstall->setDisabled( false );
		
		pbQueue->setDisabled( false );
	}
	
	// No db no fun!
	if ( !SignalistSingleton::Instance()->isKurooReady() ) {
		pbAdvanced->setDisabled( true );
		pbQueue->setDisabled( true );
		filterGroup->setDisabled( true );
		searchFilter->setDisabled( true );
		pbClearFilter->setDisabled( true );
	}
	else {
		filterGroup->setDisabled( false );
		searchFilter->setDisabled( false );
		pbClearFilter->setDisabled( false );
	}
}

void PortageTab::slotInitButtons()
{
	pbQueue->setText( i18n("Add to Queue") );
}

/**
 * Toggle Add/Remove to Queue button.
 * @param isQueued
 */
void PortageTab::slotButtons( bool isQueued )
{
	if ( isQueued )
		pbQueue->setText( i18n("Remove from Queue") );
	else
		pbQueue->setText( i18n("Add to Queue") );
}

/**
 * Open advanced dialog with: ebuild, versions, use flags...
 */
void PortageTab::slotAdvanced()
{
// 	kdDebug() << "PortageTab::slotAdvanced" << endl;
	
	if ( packagesView->currentPackage() ) {
		slotPackage();
		m_packageInspector->edit( packagesView->currentPackage() );
	}
}

/**
 * View summary for selected package.
 */
void PortageTab::slotPackage()
{
// 	kdDebug() << "PortageTab::slotPackage" << endl;
	
	if ( !isVisible() )
		return;
	
	if ( packagesView->currentPackage()->isInstalled() && KUser().isSuperUser() && !EmergeSingleton::Instance()->isRunning() ) {
		pbUninstall->setDisabled( false );
		pbQueue->setDisabled( false );
		m_packageInspector->setDisabled( false );
		pbAdvanced->setDisabled( false );
	}
	else {
		pbUninstall->setDisabled( true );
	}
	
	if ( packagesView->currentPackage()->isQueued() )
		pbQueue->setText( i18n("Remove from Queue") );
	else
		pbQueue->setText( i18n("Add to Queue") );
	
	// clear text browsers and dropdown menus
	summaryBrowser->clear();
	m_packageInspector->dialog->versionsView->clear();
	m_packageInspector->dialog->cbVersionsEbuild->clear();
	m_packageInspector->dialog->cbVersionsDependencies->clear();
	m_packageInspector->dialog->cbVersionsInstalled->clear();
	m_packageInspector->dialog->cbVersionsUse->clear();
	m_packageInspector->dialog->cbVersionsSpecific->clear();
	m_packageInspector->dialog->groupAdvanced->setDisabled( true );
	
	// Initialize the portage package object with package and it's versions data
	packagesView->currentPackage()->initVersions();
	QString package( packagesView->currentPackage()->name() );
	QString category( packagesView->currentPackage()->category() );
	
	QString lines =  "<table width=100%><tr>";
			lines += "<td bgcolor=#7a5ada><b><font color=white><font size=\"+1\">" + package + "</font> ";
			lines += "(" + category.section( "-", 0, 0 ) + "/";
			lines += category.section( "-", 1, 1 ) + ")</b></font></td></tr><tr><td>";
			lines += packagesView->currentPackage()->description() + "</td></tr><tr><td>";
			lines += i18n("<b>Homepage: </b>") + "<a href=\"" + packagesView->currentPortagePackage()->homepage();
			lines += "\">" + packagesView->currentPortagePackage()->homepage() + "</a></td></tr><tr><td>";
	
	QString linesAvailable;
	QString linesInstalled;
	QString linesEmergeVersion;
	
	// Now parse sorted list of versions for current package
	QValueList<PackageVersion*> sortedVersions = packagesView->currentPackage()->sortedVersionList();
	bool versionNotInArchitecture = false;
	QValueList<PackageVersion*>::iterator sortedVersionIterator;
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
					else {
						stability = i18n("Not available");
						m_packageInspector->dialog->groupAdvanced->setDisabled( false );
					}
		
		// Insert version in Inspector version view
		m_packageInspector->dialog->versionsView->insertItem( (*sortedVersionIterator)->version(), stability, (*sortedVersionIterator)->size(), (*sortedVersionIterator)->isInstalled() );
		
		// Create nice summary showing installed packages in green and unavailable as red
		if ( (*sortedVersionIterator)->isInstalled() ) {
			linesInstalled += "<font color=darkGreen><b>" + (*sortedVersionIterator)->version() + "</b></font>, ";
			m_packageInspector->dialog->cbVersionsInstalled->insertItem( (*sortedVersionIterator)->version() );
		}
		
		// Collect all available packages except those not in users arch
		if ( (*sortedVersionIterator)->isAvailable() ) {
			linesEmergeVersion = (*sortedVersionIterator)->version();
			linesAvailable += (*sortedVersionIterator)->version() + ", ";
		}
		else
			if ( (*sortedVersionIterator)->isNotArch() )
				versionNotInArchitecture = true;
			else
				linesAvailable += "<font color=darkRed><b>" + (*sortedVersionIterator)->version() + "</b></font>, ";
		
	}
	
	// Remove trailing commas
	linesInstalled.truncate( linesInstalled.length() - 2 );
	linesAvailable.truncate( linesAvailable.length() - 2 );
	
	// Construct installed summary
	if ( !linesInstalled.isEmpty() )
		linesInstalled = i18n("<b>Versions installed:</b></font> ") + linesInstalled + "<br>";
	else
		linesInstalled = i18n("<b>Versions installed:</b></font> Not installed<br>");
	
	// Construct installation summary
	if ( !linesEmergeVersion.isEmpty() ) {
		
		// Set active version in Inspector dropdown menus
		m_packageInspector->dialog->cbVersionsEbuild->setCurrentText( linesEmergeVersion );
		m_packageInspector->dialog->cbVersionsDependencies->setCurrentText( linesEmergeVersion );
		m_packageInspector->dialog->cbVersionsUse->setCurrentText( linesEmergeVersion );
		m_packageInspector->dialog->versionsView->usedForInstallation( linesEmergeVersion );
		
		linesEmergeVersion = i18n("<b>Version used for installation:</b> ") + linesEmergeVersion;
	}
	else {
		if ( versionNotInArchitecture && linesAvailable.isEmpty() )
			linesEmergeVersion = i18n("<b>Version used for installation: "
			                          "<font color=darkRed>No version available on %1</b>").arg( KurooConfig::arch() );
		else
			linesEmergeVersion = i18n("<b>Version used for installation:</font> "
			                          "<font color=darkRed>No version available - please check advanced options</font></b>");
	}
	
	// Construct available versions summary
	if ( !linesAvailable.isEmpty() )
		linesAvailable = i18n("<b>Versions available:</b> ") + linesAvailable + "<br>";
	else
		linesAvailable = i18n("<b>Versions available: "
		                      "<font color=darkRed>No version available on %1</font></b><br>").arg( KurooConfig::arch() );
	
	summaryBrowser->setText( lines + linesInstalled + linesAvailable + linesEmergeVersion + "</td></tr></table>");
	
	// Refresh inspector if visible
	if ( m_packageInspector->isVisible() )
		m_packageInspector->edit( packagesView->currentPackage() );
}

/**
 * Popup menu for actions like emerge.
 * @param item
 * @param point
 */
void PortageTab::contextMenu( KListView*, QListViewItem* item, const QPoint& point )
{
	if ( !item )
		return;
	
	enum Actions { PRETEND, APPEND, EMERGE };
	
	KPopupMenu menu( this );
	int menuItem1 = menu.insertItem(i18n("&Pretend"), PRETEND);
	int menuItem2;
	
	if ( !packagesView->currentPackage()->isQueued() )
		menuItem2 = menu.insertItem(i18n("&Add to queue"), APPEND);
	else
		menuItem2 = menu.insertItem(i18n("&Remove from queue"), APPEND);
	
	int menuItem3 = menu.insertItem(i18n("&Install now"), EMERGE);
	
	// No access when kuroo is busy.
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() ) {
		menu.setItemEnabled( menuItem1, false );
		menu.setItemEnabled( menuItem2, false );
		menu.setItemEnabled( menuItem3, false );
	}
	
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() || !KUser().isSuperUser() )
		menu.setItemEnabled( menuItem3, false );
	
	switch( menu.exec( point ) ) {
		
		case PRETEND:
			PortageSingleton::Instance()->pretendPackageList( packagesView->selectedId() );
			break;
			
		case APPEND:
			slotQueue();
			break;
			
		case EMERGE:
			QueueSingleton::Instance()->installQueue( packagesView->selectedId() );
	}
}

/**
 * Append or remove package to the queue.
 */
void PortageTab::slotQueue()
{
	if ( !EmergeSingleton::Instance()->isRunning() || !SignalistSingleton::Instance()->isKurooBusy() ) {
		if ( packagesView->currentPackage()->isQueued() )
			QueueSingleton::Instance()->removePackageIdList( packagesView->selectedId() );
		else
			QueueSingleton::Instance()->addPackageIdList( packagesView->selectedId() );
	}
}

/**
 * Uninstall selected package.
 */
void PortageTab::slotUninstall()
{
	if ( !EmergeSingleton::Instance()->isRunning() || !SignalistSingleton::Instance()->isKurooBusy() || !KUser().isSuperUser() ) {
		const QStringList selectedList = packagesView->selectedId();
		
		// Pick only installed packages
		QStringList packageList, idList;
		foreach ( selectedList ) {
			if ( packagesView->itemId( *it )->isInstalled() ) {
				packageList += packagesView->itemId( *it )->name();
				idList += *it;
			}
		}
		
		switch( KMessageBox::questionYesNoList( this, 
				i18n( "<qt>Portage will not check if the package you want to remove is required by another package.<br>"
				      "Do you want to uninstall following packages?</qt>" ), packageList, i18n( "Uninstall packages" ) ) ) {
				case KMessageBox::Yes:
					InstalledSingleton::Instance()->uninstallPackageList( packagesView->selectedId() );
			}
	}
}

#include "portagetab.moc"
