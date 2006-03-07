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
#include "uninstallinspector.h"

#include <qlayout.h>
#include <qsplitter.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qtimer.h>

#include <kpushbutton.h>
#include <ktextbrowser.h>
#include <ktabwidget.h>
#include <kpopupmenu.h>
#include <kdialogbase.h>
#include <kmessagebox.h>
#include <kuser.h>
#include <klineedit.h>
#include <kiconloader.h>

/**
 * @class PortageTab
 * @short Package view with filters.
 */
PortageTab::PortageTab( QWidget* parent, PackageInspector *packageInspector )
	: PortageBase( parent ), m_packageInspector( packageInspector ), uninstallInspector( 0 ), queuedFilters( 0 )
{
	pbClearFilter->setIconSet( SmallIconSet("locationbar_erase") );
	
	// Initialize category and subcategory views with all available data
	categoriesView->init();
	subcategoriesView->init();
	
	uninstallInspector = new UninstallInspector( this );
	
	connect( filterGroup, SIGNAL( released( int ) ), this, SLOT( slotFilters() ) );
	connect( searchFilter, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotFilters() ));
	
	// Rmb actions.
	connect( packagesView, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ),
	         this, SLOT( contextMenu( KListView*, QListViewItem*, const QPoint& ) ) );
	
	// Button actions.
	connect( pbQueue, SIGNAL( clicked() ), this, SLOT( slotQueue() ) );
	connect( pbUninstall, SIGNAL( clicked() ), this, SLOT( slotUninstall() ) );
	connect( packagesView, SIGNAL( doubleClicked( QListViewItem*, const QPoint&, int ) ), this, SLOT( slotAdvanced() ) );
	connect( pbAdvanced, SIGNAL( clicked() ), this, SLOT( slotAdvanced() ) );
	connect( pbClearFilter, SIGNAL( clicked() ), this, SLOT( slotClearFilter() ) );
	
	// Toggle Queue button
	connect( QueueSingleton::Instance(), SIGNAL( signalQueueChanged( bool ) ), this, SLOT( slotInitButtons() ) );
	connect( SignalistSingleton::Instance(), SIGNAL( signalPackageChanged() ), this, SLOT( slotButtons() ) );
	
	// Reload view after changes.
	connect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotReload() ) );
	
	// Lock/unlock actions when kuroo is busy.
	connect( SignalistSingleton::Instance(), SIGNAL( signalKurooBusy( bool ) ), this, SLOT( slotBusy() ) );
	
	// Load Inspector with current package info
	connect( packagesView, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotPackage() ) );
	connect( packagesView, SIGNAL( selectionChanged() ), this, SLOT( slotButtons() ) );
	
	connect( m_packageInspector, SIGNAL( signalPackageChanged() ), this, SLOT( slotPackage() ) );
	connect( m_packageInspector, SIGNAL( signalNextPackage( bool ) ), this, SLOT( slotNextPackage( bool ) ) );
	
// 	slotInit();
}

PortageTab::~PortageTab()
{
	delete uninstallInspector;
	uninstallInspector = 0;
}

/**
 * Initialize Portage view.
 */
void PortageTab::slotInit()
{
	slotBusy();
}

/**
 * Forward signal from next-buttons only if this tab is visible for user.
 * @param isNext
 */
void PortageTab::slotNextPackage( bool isNext )
{
	if ( !isVisible() )
		return;
	
	packagesView->slotNextPackage( isNext );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Toggle button slots
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PortageTab::slotInitButtons()
{
	pbQueue->setText( i18n("Add to Queue") );
}

/**
 * Disable/enable buttons when kuroo is busy.
 */
void PortageTab::slotBusy()
{
	kdDebug() << "PortageTab::slotBusy" << endl;
	
	// If kuroo busy or no db no fun!
	if ( !SignalistSingleton::Instance()->isKurooReady() ) {
		pbUninstall->setDisabled( true );
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
		slotButtons();
	}
}

/**
 * Toggle buttons states.
 */
void PortageTab::slotButtons()
{
	kdDebug() << "PortageTab::slotButtons" << endl;
	
	// No package selected, disable all buttons
	if ( packagesView->selectedId().isEmpty() ) {
		pbQueue->setDisabled( true );
		pbAdvanced->setDisabled( true );
		pbUninstall->setDisabled( true );
		return;
	}
	
	m_packageInspector->setDisabled( false );
	pbAdvanced->setDisabled( false );
	
	// Toggle queue button between add/remove
	if ( packagesView->currentPackage()->isInPortage() ) {
		if ( packagesView->currentPackage()->isQueued() )
			pbQueue->setText( i18n("Remove from Queue") );
		else
			pbQueue->setText( i18n("Add to Queue") );
	}

	// When kuroo is busy disable queue and uninstall button
	if ( SignalistSingleton::Instance()->isKurooBusy() ) {
		pbQueue->setDisabled( true );
		pbUninstall->setDisabled( true );
		return;
	}
	else
		pbQueue->setDisabled( false );
	
	// If user is su enable uninstall
	if ( packagesView->currentPackage()->isInstalled() && KUser().isSuperUser() )
		pbUninstall->setDisabled( false );
	else
		pbUninstall->setDisabled( true );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Package view slots
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Populate view with portage packages.
 * Then load the emerge history.
 */
void PortageTab::slotReload()
{
	kdDebug() << "PortageTab::slotReload" << endl;
	
	m_packageInspector->setDisabled( true );
	pbAdvanced->setDisabled( true );
	
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
	kdDebug() << "PortageTab::slotListPackages" << endl;
	
	QString categoryId = categoriesView->currentCategoryId();
	QString subCategoryId = subcategoriesView->currentCategoryId();
	
	// Disable all buttons if query result is empty
	if ( packagesView->addSubCategoryPackages( KurooDBSingleton::Instance()->portagePackagesBySubCategory( categoryId, subCategoryId, filterGroup->selectedId(), searchFilter->text() ) ) == 0 ) {
		
		m_packageInspector->hide();
		slotButtons();
		summaryBrowser->clear();
		summaryBrowser->setText( i18n("<font color=darkRed size=+1><b>No package found with these filter settings</font><br>"
		                              "<font color=darkRed>Please modify the filter settings you have chosen!<br>"
		                              "Try to use more general filter options, so kuroo can find matching packages.</b></font>") );
		
		// Highlight text filter background in red if query failed
		if ( !searchFilter->text().isEmpty() )
			searchFilter->setPaletteBackgroundColor( QColor( KurooConfig::noMatchColor() ) );
		else
			searchFilter->setPaletteBackgroundColor( Qt::white );
	}
	else {
		
		// Highlight text filter background in green if query successful
		if ( !searchFilter->text().isEmpty() )
			searchFilter->setPaletteBackgroundColor( QColor( KurooConfig::matchColor() ) );
		else
			searchFilter->setPaletteBackgroundColor( Qt::white );
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
 * Refresh packages list.
 */
void PortageTab::slotRefresh()
{
	kdDebug() << "PortageTab::slotRefresh" << endl;
	
	switch( KMessageBox::questionYesNo( this,
		i18n( "<qt>Do you want to refresh the Packages view?<br>"
		      "This will take a couple of minutes...</qt>"), i18n( "Refreshing Packages" ), KStdGuiItem::yes(), KStdGuiItem::no(), "dontAskAgainRefreshPortage" ) ) {
		case KMessageBox::Yes:
			PortageSingleton::Instance()->slotRefresh();
	}
}

/**
 * Append or remove package to the queue. @fixme: What if package not in portage is in list?
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
		QStringList packageList;
		foreach ( selectedList ) {
			if ( packagesView->itemId( *it )->isInstalled() ) {
				packageList += *it;
				packageList += KurooDBSingleton::Instance()->category( *it ) + "/" + packagesView->itemId( *it )->name();
			}
		}
		
		uninstallInspector->view( packageList );
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Package slots
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Open advanced dialog with: ebuild, versions, use flags...
 */
void PortageTab::slotAdvanced()
{
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
	kdDebug() << "PortageTab::slotPackage" << endl;
	
	// Packages view is hidden don't update
	if ( !isVisible() )
		return;
	
	// clear text browsers and dropdown menus
	summaryBrowser->clear();
	m_packageInspector->dialog->versionsView->clear();
	m_packageInspector->dialog->cbVersionsEbuild->clear();
	m_packageInspector->dialog->cbVersionsDependencies->clear();
	m_packageInspector->dialog->cbVersionsInstalled->clear();
	m_packageInspector->dialog->cbVersionsUse->clear();
	m_packageInspector->dialog->cbVersionsSpecific->clear();
	
	// Initialize the portage package object with package and it's versions data
	packagesView->currentPackage()->initVersions();
	QString package( packagesView->currentPackage()->name() );
	QString category( packagesView->currentPackage()->category() );
	
	QString lines =  "<table width=100% border=0 cellpadding=0>";
			lines += "<tr><td bgcolor=#88bb22 colspan=2><b><font color=white><font size=\"+1\">" + package + "</font> ";
			lines += "(" + category.section( "-", 0, 0 ) + "/";
			lines += category.section( "-", 1, 1 ) + ")</b></font></td></tr>";
	
	if ( packagesView->currentPackage()->isInPortage() ) {
			lines += "<tr><td colspan=2>" + packagesView->currentPackage()->description() + "</td></tr>";
			lines += "<tr><td colspan=2>" + i18n("<b>Homepage: </b>") + "<a href=\"" + packagesView->currentPortagePackage()->homepage();
			lines += "\">" + packagesView->currentPortagePackage()->homepage() + "</a></td></tr>";
	}
	else
		lines += i18n("<tr><td colspan=2><font color=darkRed><b>Package not available in Portage tree anymore!</b></font></td></tr>");
	
	// Now parse sorted list of versions for current package
	QString version, emergeVersion, linesAvailable, linesInstalled, linesEmergeVersion;
	QValueList<PackageVersion*> sortedVersions = packagesView->currentPackage()->sortedVersionList();
	bool versionNotInArchitecture( false );
	QValueList<PackageVersion*>::iterator sortedVersionIterator;
	for ( sortedVersionIterator = sortedVersions.begin(); sortedVersionIterator != sortedVersions.end(); sortedVersionIterator++ ) {
		
		// Load all dropdown menus in the inspector with relevant versions
		m_packageInspector->dialog->cbVersionsEbuild->insertItem( (*sortedVersionIterator)->version() );
		m_packageInspector->dialog->cbVersionsDependencies->insertItem( (*sortedVersionIterator)->version() );
		m_packageInspector->dialog->cbVersionsUse->insertItem( (*sortedVersionIterator)->version() );
		m_packageInspector->dialog->cbVersionsSpecific->insertItem( (*sortedVersionIterator)->version() );
		
		version = (*sortedVersionIterator)->version();
		
		// Mark official version stability for version listview
		QString stability;
		if ( (*sortedVersionIterator)->isOriginalHardMasked() ) {
			stability = i18n("Hardmasked");
			version = "<font color=darkRed><i>" + version + "</i></font>";
		}
		else
			if ( (*sortedVersionIterator)->isOriginalTesting() ) {
				stability = i18n("Testing");
				version = "<i>" + version + "</i>";
			}
			else
				if ( (*sortedVersionIterator)->isAvailable() )
					stability = i18n("Stable");
				else
					if ( (*sortedVersionIterator)->isNotArch() )
						stability = i18n("Not on %1").arg( KurooConfig::arch() );
					else
						stability = i18n("Not available");
		
// 		kdDebug() << "version="<< (*sortedVersionIterator)->version() << " stability=" << stability << endl;
		
		// Insert version in Inspector version view
		m_packageInspector->dialog->versionsView->insertItem( (*sortedVersionIterator)->version(), stability, (*sortedVersionIterator)->size(), (*sortedVersionIterator)->isInstalled() );
		
		// Create nice summary showing installed packages
		if ( (*sortedVersionIterator)->isInstalled() ) {
			version = "<b>" + version + "</b>";
			linesInstalled.prepend( version + " (" + stability + "), " );
			m_packageInspector->dialog->cbVersionsInstalled->insertItem( (*sortedVersionIterator)->version() );
		}
		
		// Collect all available packages except those not in users arch
		if ( (*sortedVersionIterator)->isAvailable() ) {
			emergeVersion = (*sortedVersionIterator)->version();
			linesEmergeVersion = version + " (" + stability + ")";
			linesAvailable.prepend( version + ", " );
		}
		else {
			if ( (*sortedVersionIterator)->isNotArch() )
				versionNotInArchitecture = true;
			else
				linesAvailable.prepend( version + ", " );
		}
	}
	
	// Remove trailing commas
	linesInstalled.truncate( linesInstalled.length() - 2 );
	linesAvailable.truncate( linesAvailable.length() - 2 );
	
	// Construct installed summary
	if ( !linesInstalled.isEmpty() )
		linesInstalled = i18n("<tr><td width=10%><b>Installed&nbsp;version:</b></font></td><td width=90%>%1</td></tr>").arg( linesInstalled );
	else
		linesInstalled = i18n("<tr><td width=10%><b>Installed&nbsp;version:</b></font></td><td width=90%>Not installed</td></tr>");
	
	if ( packagesView->currentPackage()->isInPortage() ) {
	
		// Construct emerge summary
		if ( !linesEmergeVersion.isEmpty() ) {
			
			// Set active version in Inspector dropdown menus
			m_packageInspector->dialog->cbVersionsEbuild->setCurrentText( emergeVersion );
			m_packageInspector->dialog->cbVersionsDependencies->setCurrentText( emergeVersion );
			m_packageInspector->dialog->cbVersionsUse->setCurrentText( emergeVersion );
			m_packageInspector->dialog->versionsView->usedForInstallation( emergeVersion );
			
			linesEmergeVersion = i18n("<tr><td width=10%><b>Emerge&nbsp;version:</b></td><td width=90%>%1</td></tr>").arg( linesEmergeVersion );
		}
		else {
			if ( versionNotInArchitecture && linesAvailable.isEmpty() )
				linesEmergeVersion = i18n("<tr><td width=10%><b>Emerge&nbsp;version:</font></td>"
				                          "<td width=90%><font color=darkRed>No version available on %1</b></td></tr>").arg( KurooConfig::arch() );
			else
				linesEmergeVersion = i18n("<tr><td width=10%><b>Emerge&nbsp;version:</font></td>"
				                          "<td width=90%><font color=darkRed>No version available - please check package details</font></b></td></tr>");
		}
		
		// Construct available versions summary
		if ( !linesAvailable.isEmpty() )
			linesAvailable = i18n("<tr><td width=10%><b>Available&nbsp;versions:</b></td><td width=90%>%1</b></td></tr>").arg( linesAvailable );
		else
			linesAvailable = i18n("<tr><td width=10%><b>Available&nbsp;versions:</td>"
			                      "<td width=90%><font color=darkRed>No versions available on %1</font></b></td></tr>").arg( KurooConfig::arch() );
		
		summaryBrowser->setText( lines + linesInstalled + linesEmergeVersion + linesAvailable + "</table>");
	}
	else
		summaryBrowser->setText( lines + linesInstalled + "</table>");
	
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
	
	enum Actions { APPEND, UNINSTALL, OPTIONS, ADDWORLD, DELWORLD };
	
	KPopupMenu menu( this );
	int menuItem1;
	
	if ( !packagesView->currentPackage()->isQueued() )
		menuItem1 = menu.insertItem( i18n("&Add to queue"), APPEND );
	else
		menuItem1 = menu.insertItem( i18n("&Remove from queue"), APPEND );
	
	int menuItem3 = menu.insertItem( i18n( "Details..." ), OPTIONS );
	
	int menuItem4;
	if ( !dynamic_cast<PackageItem*>( item )->isInWorld() )
		menuItem4 = menu.insertItem( i18n( "Add to world" ), ADDWORLD );
	else
		menuItem4 = menu.insertItem( i18n( "Remove from world" ), DELWORLD );
	menu.setItemEnabled( menuItem4, false );
	
	int menuItem2 = menu.insertItem( i18n("&Uninstall"), UNINSTALL );
	
	// No access when kuroo is busy
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() || !packagesView->currentPackage()->isInPortage() )
		menu.setItemEnabled( menuItem1, false );
	
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() || !packagesView->currentPackage()->isInstalled() || !KUser().isSuperUser() )
			menu.setItemEnabled( menuItem2, false );
	
	if ( KUser().isSuperUser() )
		menu.setItemEnabled( menuItem4, true );
	
	switch( menu.exec( point ) ) {

		case APPEND:
			slotQueue();
			break;
			
		case UNINSTALL:
			slotUninstall();
			break;
		
		case OPTIONS:
			slotAdvanced();
			break;
		
		case ADDWORLD:
			PortageSingleton::Instance()->appendWorld( packagesView->currentPackage()->category() + "/" + packagesView->currentPackage()->name() );
			break;
			
		case DELWORLD:
			PortageSingleton::Instance()->removeFromWorld( packagesView->currentPackage()->category() + "/" + packagesView->currentPackage()->name() );
		
	}
}

#include "portagetab.moc"
