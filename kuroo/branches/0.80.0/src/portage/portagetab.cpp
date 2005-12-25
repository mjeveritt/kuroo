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
#include "packageitem.h"

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
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kdialogbase.h>
#include <kmessagebox.h>
#include <kuser.h>
#include <klineedit.h>

/**
 * Page for portage packages.
 */
PortageTab::PortageTab( QWidget* parent )
	: PortageBase( parent )
{
	categoriesView->init();
	subcategoriesView->init();
	
	connect( filterGroup, SIGNAL( released( int ) ), this, SLOT( slotFilters() ) );
	connect( packagesView, SIGNAL( selectionChanged() ), this, SLOT( slotPackage() ) );
	connect( searchFilter, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotFilters() ));
	
	// Rmb actions.
	connect( packagesView, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ),
	         this, SLOT( contextMenu( KListView*, QListViewItem*, const QPoint& ) ) );
	
	// Button actions.
	connect( pbAddQueue, SIGNAL( clicked() ), this, SLOT( slotAddQueue() ) );
	connect( pbUninstall, SIGNAL( clicked() ), this, SLOT( slotUninstall() ) );
	connect( pbAdvanced, SIGNAL( clicked() ), this, SLOT( slotAdvanced() ) );
	connect( pbClearFilter, SIGNAL( clicked() ), this, SLOT( slotClearFilter() ) );
	
	// Reload view after changes.
	connect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotReload() ) );
	
	// Lock/unlock actions when kuroo is busy.
	connect( SignalistSingleton::Instance(), SIGNAL( signalKurooBusy( bool ) ), this, SLOT( slotBusy( bool ) ) );
	
	slotInit();
}

/**
 * Save splitters and listview geometry.
 */
PortageTab::~PortageTab()
{
// 	KConfig *config = KurooConfig::self()->config();
// 	config->setGroup("Kuroo Geometry");
// 	
// 	QValueList<int> list = splitterH->sizes();
// 	config->writeEntry("splitterPortageH", list);
// 	list = splitterV->sizes();
// 	config->writeEntry("splitterPortageV", list);
	
	packagesView->saveLayout( KurooConfig::self()->config(), "portageViewLayout" );
	
	// Save latest selected packages in tabs All packages, Installed packages and Updates categories
// 	saveCurrentView();
}

/**
 * Save latest selected packages in tabs All packages, Installed packages and Updates categories.
 */
void PortageTab::saveCurrentView()
{
// 	QListViewItem *item = categoriesView->currentItem();
// 	if ( item && item->parent() )
// 		KurooConfig::setLatestPortageCategory( item->parent()->text(0) + "-" + item->text(0) );
// 	
// 	item = packagesView->currentItem();
// 	if ( item )
// 		KurooConfig::setLatestPortagePackage( item->text(0) );
// 	
// 	KurooConfig::writeConfig();
}

/**
 * Initialize Portage view.
 * Restore geometry: splitter positions, listViews width and columns width.
 */
void PortageTab::slotInit()
{
	kdDebug() << "PortageTab::slotInit" << endl;
// 	KConfig *config = KurooConfig::self()->config();
// 	config->setGroup("Kuroo Geometry");
// 	
// 	// @fixme: portage splitters are bugging! using installed splitters instead
// 	QValueList<int> sizes = config->readIntListEntry("splitterInstalledH");
// 	splitterH->setSizes(sizes);
// 	sizes = config->readIntListEntry("splitterInstalledV");
// 	splitterV->setSizes(sizes);
	
	if ( !KurooConfig::init() )
		packagesView->restoreLayout( KurooConfig::self()->config(), "portageViewLayout" );
	
	packageInspector = new PackageInspector( this );
	slotBusy( false );
}

/**
 * Populate view with portage packages.
 * Then load the emerge history.
 */
void PortageTab::slotReload()
{
	kdDebug() << "PortageTab::slotReload" << endl;
	
	// Prepare categories by loading index @fixme: what if category-subcategory is changed in db?
	disconnect( categoriesView, SIGNAL( selectionChanged() ), this, SLOT( slotListSubCategories() ) );
	disconnect( subcategoriesView, SIGNAL( selectionChanged() ), this, SLOT( slotListPackages() ) );
	categoriesView->init();
	subcategoriesView->init();
	connect( categoriesView, SIGNAL( selectionChanged() ), this, SLOT( slotListSubCategories() ) );
	connect( subcategoriesView, SIGNAL( selectionChanged() ), this, SLOT( slotListPackages() ) );
	
	saveCurrentView();
	slotFilters();
}

/**
 * Toggle between package filter: All, Installed or updates
 */
void PortageTab::slotFilters()
{
	kdDebug() << "PortageTab::slotFilters" << endl;
	
	packagesView->reset();
	categoriesView->loadCategories( PortageSingleton::Instance()->categories( filterGroup->selectedId(), searchFilter->text() ) );
}

/**
 * List packages when clicking on category in installed.
 */
void PortageTab::slotListSubCategories()
{
	kdDebug() << "PortageTab::slotListSubCategories" << endl;
	
	QString categoryId = categoriesView->currentCategoryId();
	subcategoriesView->loadCategories( PortageSingleton::Instance()->subCategories( categoryId, filterGroup->selectedId(), searchFilter->text() ) );
}

/**
 * List packages when clicking on category in installed.
 */
void PortageTab::slotListPackages()
{
	kdDebug() << "PortageTab::slotListPackages" << endl;
	
	QString categoryId = categoriesView->currentCategoryId();
	QString subCategoryId = subcategoriesView->currentCategoryId();
	int count = packagesView->addSubCategoryPackages( PortageSingleton::Instance()->packagesInSubCategory( categoryId, subCategoryId, filterGroup->selectedId(), searchFilter->text() ) );
	
	packagesView->setHeader( QString::number( count ) );
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
		case KMessageBox::Yes: {
			saveCurrentView();
			PortageSingleton::Instance()->slotRefresh();
		}
	}
}

/**
 * Disable/enable buttons when kuroo is busy.
 * @param b
 */
void PortageTab::slotBusy( bool b )
{
	if ( b ) {
		pbUninstall->setDisabled( true );
		pbAddQueue->setDisabled( true );
	}
	else {
		if ( !KUser().isSuperUser() )
			pbUninstall->setDisabled( true );
		else
			pbUninstall->setDisabled( false );
		
		pbAddQueue->setDisabled( false );
	}
}

/**
 * View summary for selected package.
 */
void PortageTab::slotPackage()
{
	kdDebug() << "PortageTab::slotPackage" << endl;
		
	pbUninstall->setDisabled( true );
	if ( packagesView->currentItemStatus() == INSTALLED && KUser().isSuperUser() )
		pbUninstall->setDisabled( false );
	
	summaryBrowser->clear();
	QString package( PortageSingleton::Instance()->package( packagesView->currentId() ) );
	QString category( PortageSingleton::Instance()->category( packagesView->currentId() ) );
	Info info( PortageSingleton::Instance()->packageInfo( packagesView->currentId() ) );
	
	QString textLines = "<font size=\"+2\">" + package + "</font> ";
			textLines += "(" + category.section( "-", 0, 0 ) + " / " + category.section( "-", 1, 1 ) + ") <br>";
			textLines += info.description + "<br>";
			textLines += i18n("<b>Homepage: </b>") + "<a href=\"" + info.homepage + "\">" + info.homepage + "</a><br>";
	
	
	QString textLinesAvailable = i18n("<b>Versions available:</b> ");
	QString textLinesInstalled;
	const QStringList versionList = PortageSingleton::Instance()->packageVersions( packagesView->currentId() );
	foreach ( versionList ) {
		QString version = *it++;
		QString meta = *it;
		
		if ( meta == FILTERINSTALLED )
			textLinesInstalled += "<font color=darkGreen><b>" + version + "</b></font>, ";
		
		textLinesAvailable += version + ", ";
	}
	textLinesAvailable.truncate( textLinesAvailable.length() - 2 );
	textLinesInstalled.truncate( textLinesInstalled.length() - 2 );
	
	if ( !textLinesInstalled.isEmpty() )
		textLinesInstalled = i18n("<b>Versions installed:</b> ") + textLinesInstalled + "<br>";
	else
		textLinesInstalled = i18n("<b>Versions installed:</b> Not installed<br>");
	
	summaryBrowser->setText( textLines + textLinesInstalled + textLinesAvailable );
	
	if ( packageInspector->isVisible() )
		slotAdvanced();
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
	
	enum Actions { PRETEND, APPEND, EMERGE, DEPEND, UNMASK, CLEARUNMASK, USEFLAGS };
	
	KPopupMenu menu(this);
	int menuItem1 = menu.insertItem(i18n("&Pretend"), PRETEND);
	int menuItem2 = menu.insertItem(i18n("&Append to queue"), APPEND);
	int menuItem3 = menu.insertItem(i18n("&Install now"), EMERGE);
	int menuItem4 = menu.insertItem(i18n("&Unmask"), UNMASK);
	int menuItem5 = menu.insertItem(i18n("&Clear Unmasking"), CLEARUNMASK);
	
	// No access when kuroo is busy.
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() ) {
		menu.setItemEnabled( menuItem1, false );
		menu.setItemEnabled( menuItem2, false );
		menu.setItemEnabled( menuItem3, false );
	}
	
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() || !KUser().isSuperUser() )
		menu.setItemEnabled( menuItem3, false );
	
	if ( SignalistSingleton::Instance()->isKurooBusy() || !KUser().isSuperUser() ) {
		menu.setItemEnabled( menuItem4, false );
		menu.setItemEnabled( menuItem5, false );
	}
	
	switch( menu.exec( point ) ) {
		
		case PRETEND:
			PortageSingleton::Instance()->pretendPackageList( packagesView->selectedId() );
			break;
			
		case APPEND:
			QueueSingleton::Instance()->addPackageIdList( packagesView->selectedId() );
			break;
			
		case EMERGE:
			QueueSingleton::Instance()->installQueue( packagesView->selectedId() );
			break;
			
		case UNMASK:
			PortageSingleton::Instance()->unmaskPackageList( packagesView->selectedId() );
			break;
			
		case CLEARUNMASK:
			PortageSingleton::Instance()->clearUnmaskPackageList( packagesView->selectedId() );
	}
}

/**
 * Append package to the queue.
 */
void PortageTab::slotAddQueue()
{
	if ( !EmergeSingleton::Instance()->isRunning() || !SignalistSingleton::Instance()->isKurooBusy() )
		QueueSingleton::Instance()->addPackageIdList( packagesView->selectedId() );
}

/**
 * Uninstall selected package.
 */
void PortageTab::slotUninstall()
{
	if ( !EmergeSingleton::Instance()->isRunning() || !SignalistSingleton::Instance()->isKurooBusy() /*|| !KUser().isSuperUser()*/ ) {
		QStringList packageList( packagesView->selectedPackages() );
		switch( KMessageBox::questionYesNoList( this, 
				i18n( "<qt>Portage will not check if the package you want to remove is required by another package.<br>"
				      "Do you want to unmerge following packages?</qt>" ), packageList, i18n( "Unmerge packages" ) ) ) {
				case KMessageBox::Yes:
					InstalledSingleton::Instance()->uninstallPackageList( packagesView->selectedId() );
			}
	}
}

/**
 * Open advanced dialog with: ebuild, versions, use flags...
 */
void PortageTab::slotAdvanced()
{
	packageInspector->edit( packagesView->selectedId().first() );
}

#include "portagetab.moc"
