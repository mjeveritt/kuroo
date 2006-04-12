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

#define DEBUG_PREFIX "PortageTab"

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
	: PortageBase( parent ), 
	m_packageInspector( packageInspector ), m_uninstallInspector( 0 ), m_delayFilters( 0 )
{
	// Connect the filters
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
	
	// Toggle Queue button between "add/remove" when after queue has been edited
	connect( QueueSingleton::Instance(), SIGNAL( signalQueueChanged( bool ) ), this, SLOT( slotInitButtons() ) );
	connect( m_packageInspector, SIGNAL( signalPackageChanged() ), this, SLOT( slotButtons() ) );
	
	// Reload view after changes.
	connect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotReload() ) );
	
	// Enable/disable this view and buttons when kuroo is busy
	connect( SignalistSingleton::Instance(), SIGNAL( signalKurooBusy( bool ) ), this, SLOT( slotBusy() ) );
	
	// Load Inspector with current package info
	connect( packagesView, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotPackage() ) );
	connect( packagesView, SIGNAL( selectionChanged() ), this, SLOT( slotButtons() ) );
	
	// Connect changes made in Inspector to this view so it gets updated
	connect( m_packageInspector, SIGNAL( signalPackageChanged() ), this, SLOT( slotPackage() ) );
	connect( m_packageInspector, SIGNAL( signalNextPackage( bool ) ), this, SLOT( slotNextPackage( bool ) ) );
	
	slotInit();
}

PortageTab::~PortageTab()
{
	delete m_uninstallInspector;
	m_uninstallInspector = 0;
}

/**
 * Initialize Portage view.
 */
void PortageTab::slotInit()
{
	// Get color theme
	portageFrame->setPaletteBackgroundColor( colorGroup().base() );
	
	// Change select-color in summaryBrowser to get contrast
	QPalette summaryPalette;
	QColorGroup summaryColorGroup( colorGroup() );
	summaryColorGroup.setColor( QColorGroup::HighlightedText, colorGroup().dark() );
	summaryPalette.setActive( summaryColorGroup );
	summaryPalette.setInactive( summaryColorGroup );
	summaryPalette.setDisabled( summaryColorGroup );
	summaryBrowser->setPalette( summaryPalette );
	
	// Initialize the uninstall dialog
	m_uninstallInspector = new UninstallInspector( this );
	
	pbClearFilter->setIconSet( SmallIconSet("locationbar_erase") );
	
	slotBusy();
}

/**
 * Forward signal from next-buttons only if this tab is visible for user.
 * @param isNext
 */
void PortageTab::slotNextPackage( bool isNext )
{
	if ( !m_packageInspector->isParentView( VIEW_PORTAGE ) )
		return;
	
	packagesView->slotNextPackage( isNext );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Toggle button slots
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Reset queue button text when queue is refreshed.
 */
void PortageTab::slotInitButtons()
{
	pbQueue->setText( i18n("Add to Queue") );
}

/**
 * Disable/enable buttons when kuroo is busy.
 */
void PortageTab::slotBusy()
{
	// If no db no fun!
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
	// No package selected, disable all buttons
	if ( packagesView->selectedId().isEmpty() ) {
		pbQueue->setDisabled( true );
		pbAdvanced->setDisabled( true );
		pbUninstall->setDisabled( true );
		return;
	}
	
	m_packageInspector->setDisabled( false );
	pbAdvanced->setDisabled( false );

	// When kuroo is busy disable queue and uninstall button
	if ( SignalistSingleton::Instance()->isKurooBusy() ) {
		pbQueue->setDisabled( true );
		pbUninstall->setDisabled( true );
		return;
	}
	else
		pbQueue->setDisabled( false );

	// Toggle queue button between add/remove
	if ( packagesView->currentPackage()->isInPortage() ) {
		pbQueue->setDisabled( false );
		if ( packagesView->currentPackage()->isQueued() )
			pbQueue->setText( i18n("Remove from Queue") );
		else
			pbQueue->setText( i18n("Add to Queue") );
	}
	else {
		pbQueue->setText( i18n("Add to Queue") );
		pbQueue->setDisabled( true );
	}
	
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
 * Initialize category and subcategory views with the available categories and subcategories.
 */
void PortageTab::slotReload()
{
	DEBUG_LINE_INFO;
	
	m_packageInspector->setDisabled( true );
	pbAdvanced->setDisabled( true );
	
	disconnect( categoriesView, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotListSubCategories() ) );
	disconnect( subcategoriesView, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotListPackages() ) );
	
	categoriesView->init();
	subcategoriesView->init();
	
	connect( categoriesView, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotListSubCategories() ) );
	connect( subcategoriesView, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotListPackages() ) );
	
	categoriesView->loadCategories( KurooDBSingleton::Instance()->portageCategories( filterGroup->selectedId(), searchFilter->text() ), false );
}

/**
 * Execute query based on filter and text. Add a delay of 250ms.
 */
void PortageTab::slotFilters()
{
	m_delayFilters++;
	QTimer::singleShot( 250, this, SLOT( slotActivateFilters() ) );
}

/**
 * Execute query based on filter and text.
 */
void PortageTab::slotActivateFilters()
{
	--m_delayFilters;
	if ( m_delayFilters == 0 )
		categoriesView->loadCategories( KurooDBSingleton::Instance()->portageCategories( filterGroup->selectedId(), searchFilter->text() ), true );
}

/**
 * List packages when clicking on category in installed.
 */
void PortageTab::slotListSubCategories()
{
	subcategoriesView->loadCategories( KurooDBSingleton::Instance()->portageSubCategories( categoriesView->currentCategoryId(), 
		filterGroup->selectedId(), searchFilter->text() ) );
}

/**
 * List packages when clicking on subcategory.
 */
void PortageTab::slotListPackages()
{
	// Disable all buttons if query result is empty
	if ( packagesView->addSubCategoryPackages( KurooDBSingleton::Instance()->portagePackagesBySubCategory( categoriesView->currentCategoryId(),
		subcategoriesView->currentCategoryId(), filterGroup->selectedId(), searchFilter->text() ) ) == 0 ) {
		
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
 * Reset text filter when clicking on clear button.
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
	DEBUG_LINE_INFO;
	
	switch( KMessageBox::questionYesNo( this,
		i18n( "<qt>Do you want to refresh the Packages view?<br>"
		      "This will take a couple of minutes...</qt>"), i18n( "Refreshing Packages" ), 
	                                    KStdGuiItem::yes(), KStdGuiItem::no(), "dontAskAgainRefreshPortage" ) ) {
		case KMessageBox::Yes:
			PortageSingleton::Instance()->slotRefresh();
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
		else {
			const QStringList selectedIdList = packagesView->selectedId();
			QStringList packageIdList;
			foreach( selectedIdList )
				if ( packagesView->packageItemById( *it )->isInPortage() )
					packageIdList += *it;
			QueueSingleton::Instance()->addPackageIdList( packageIdList );
		}
	}
}

/**
 * Uninstall selected package.
 */
void PortageTab::slotUninstall()
{
	if ( !EmergeSingleton::Instance()->isRunning() || !SignalistSingleton::Instance()->isKurooBusy() || !KUser().isSuperUser() ) {
		const QStringList selectedIdList = packagesView->selectedId();
		
		// Pick only installed packages
		QStringList packageList;
		foreach ( selectedIdList ) {
			if ( packagesView->packageItemById( *it )->isInstalled() ) {
				packageList += *it;
				packageList += KurooDBSingleton::Instance()->category( *it ) + "/" + packagesView->packageItemById( *it )->name();
			}
		}
		
		m_uninstallInspector->view( packageList );
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
	if ( packagesView->currentPackage() )
		processPackage( true );
}

void PortageTab::slotPackage()
{
	if ( m_packageInspector->isVisible() )
		processPackage( true );
	else	
		processPackage( false );
}

/**
 * Process package and all it's versions.
 * Update summary and Inspector.
 */
void PortageTab::processPackage( bool viewInspector )
{
	DEBUG_LINE_INFO;
	
	if ( m_packageInspector->isVisible() && !m_packageInspector->isParentView( VIEW_PORTAGE ) )
		return;
	
	// Clear summary and Inspector text browsers and dropdown menus
	summaryBrowser->clear();
	m_packageInspector->dialog->versionsView->clear();
	m_packageInspector->dialog->cbVersionsEbuild->clear();
	m_packageInspector->dialog->cbVersionsDependencies->clear();
	m_packageInspector->dialog->cbVersionsInstalled->clear();
	m_packageInspector->dialog->cbVersionsUse->clear();
	m_packageInspector->dialog->cbVersionsSpecific->clear();
	
	// Initialize the portage package object with the current package and it's versions data
	packagesView->currentPackage()->initVersions();
	QString package( packagesView->currentPackage()->name() );
	QString category( packagesView->currentPackage()->category() );
	
	// Now parse sorted list of versions for current package
	QString version, emergeVersion, linesAvailable, linesInstalled, linesEmergeVersion, description, homepage;
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
		
// 		kdDebug() << "version="<< (*sortedVersionIterator)->version() << " isInstalled=" << (*sortedVersionIterator)->isInstalled() << endl;
		
		// Insert version in Inspector version view
		m_packageInspector->dialog->versionsView->insertItem( 
			(*sortedVersionIterator)->version(), stability, (*sortedVersionIterator)->size(), (*sortedVersionIterator)->isInstalled() );
		
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
		
		description = (*sortedVersionIterator)->description();
		homepage = (*sortedVersionIterator)->homepage();
	}
	
	// Update current package with description from latest version
	packagesView->currentPackage()->setDescription( description );

	// Remove trailing commas
	linesInstalled.truncate( linesInstalled.length() - 2 );
	linesAvailable.truncate( linesAvailable.length() - 2 );
	
	// Build summary html-view
	QString lines =  "<table width=100% border=0 cellpadding=0>";
	lines += "<tr><td bgcolor=#" + GlobalSingleton::Instance()->bgHexColor() + " colspan=2><b><font color=#"
		+ GlobalSingleton::Instance()->fgHexColor() + "><font size=\"+1\">" + package + "</font> ";
	lines += "(" + category.section( "-", 0, 0 ) + "/";
	lines += category.section( "-", 1, 1 ) + ")</b></font></td></tr>";
	
	if ( packagesView->currentPackage()->isInPortage() ) {
		lines += "<tr><td colspan=2>" + description + "</td></tr>";
		lines += "<tr><td colspan=2>" + i18n("<b>Homepage: </b>") + "<a href=\"" + homepage;
		lines += "\">" + homepage + "</a></td></tr>";
	}
	else
		lines += i18n("%1Package not available in Portage tree anymore!%2")
					.arg("<tr><td colspan=2><font color=darkRed><b>")
					.arg("</b></font></td></tr>");
	
	// Construct installed verions line
	if ( !linesInstalled.isEmpty() )
		linesInstalled = i18n("%1Installed&nbsp;version:%2%3%4")
						.arg("<tr><td width=10%><b>")
						.arg("</b></font></td><td width=90%>")
						.arg( linesInstalled )
						.arg("</td></tr>");
	else
		linesInstalled = i18n("%1Installed&nbsp;version:%2%3%4")
						.arg("<tr><td width=10%><b>")
						.arg("</b></font></td><td width=90%>")
						.arg("Not installed")
						.arg("</td></tr>");
	if ( packagesView->currentPackage()->isInPortage() ) {
	
		// Construct emerge version line
		if ( !linesEmergeVersion.isEmpty() ) {
			
			// Set active version in Inspector dropdown menus
			m_packageInspector->dialog->cbVersionsEbuild->setCurrentText( emergeVersion );
			m_packageInspector->dialog->cbVersionsDependencies->setCurrentText( emergeVersion );
			m_packageInspector->dialog->cbVersionsUse->setCurrentText( emergeVersion );
			m_packageInspector->dialog->versionsView->usedForInstallation( emergeVersion );
			
			linesEmergeVersion = i18n("%1Emerge&nbsp;version:%2%3%4")
				.arg("<tr><td width=10%><b>")
				.arg("</b></td><td width=90%>")
				.arg( linesEmergeVersion )
				.arg("</td></tr>");
		}
		else {
			if ( versionNotInArchitecture && linesAvailable.isEmpty() )
				linesEmergeVersion = i18n("%1Emerge&nbsp;version:%2No version available on %3%4")
				.arg("<tr><td width=10%><b>")
				.arg("</font></td><td width=90%><font color=darkRed>")
				.arg( KurooConfig::arch() )
				.arg("</b></td></tr>");
			else
				linesEmergeVersion = i18n("%1Emerge&nbsp;version:%2No version available - please check package details%3")
				.arg("<tr><td width=10%><b>")
				.arg("</font></td><td width=90%><font color=darkRed>")
				.arg("</font></b></td></tr>");
		}
		
		// Construct available versions line
		if ( !linesAvailable.isEmpty() )
			linesAvailable = i18n("%1Available&nbsp;versions:%2%3%4")
			.arg("<tr><td width=10%><b>")
			.arg("</b></td><td width=90%>")
			.arg( linesAvailable )
			.arg("</b></td></tr>");
		else
			linesAvailable = i18n("%1Available&nbsp;versions:%2No versions available on %3%4")
			.arg("<tr><td width=10%><b>")
			.arg("</td><td width=90%><font color=darkRed>")
			.arg( KurooConfig::arch() )
			.arg("</font></b></td></tr>");
		
		summaryBrowser->setText( lines + linesInstalled + linesEmergeVersion + linesAvailable + "</table>");
	}
	else
		summaryBrowser->setText( lines + linesInstalled + "</table>");
	
	kdDebug() << "viewInspector=" << viewInspector << LINE_INFO;
	
	// Refresh inspector if visible
	if ( viewInspector )
		m_packageInspector->edit( packagesView->currentPackage(), emergeVersion, VIEW_PORTAGE );
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
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() 
	     || !packagesView->currentPackage()->isInPortage() )
		menu.setItemEnabled( menuItem1, false );
	
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() 
	     || !packagesView->currentPackage()->isInstalled() || !KUser().isSuperUser() )
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

