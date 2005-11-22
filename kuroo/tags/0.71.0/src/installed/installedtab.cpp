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
#include "installedpackagesview.h"
#include "installedpackageslistview.h"
#include "categorieslistview.h"
#include "installedtab.h"

#include <qlayout.h>
#include <qsplitter.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qregexp.h>

#include <kdialogbase.h>
#include <ktextbrowser.h>
#include <ktabwidget.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <kuser.h>

/**
 * Tabpage containing view of installed packages.
 */
InstalledTab::InstalledTab( QWidget *parent )
	: InstalledBase( parent )
{
	packagesView = packagesSearchView->packagesView;
	
	connect( categoriesView, SIGNAL( selectionChanged() ), this, SLOT( slotListPackages() ) );
	
	// Rmb actions.
	connect( packagesView, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ),
	         this, SLOT( contextMenu( KListView*, QListViewItem*, const QPoint& ) ) );
	
	// Package info actions.
	connect( packagesView, SIGNAL( selectionChanged() ), this, SLOT( slotSummary() ) );
	
	// Update file list only if this tab is open.
	connect( installedSummaryTabs, SIGNAL( currentChanged(QWidget *) ), this, SLOT( slotInstalledFiles(QWidget *) ) );
	
	// Reload view after changes.
	connect( InstalledSingleton::Instance(), SIGNAL( signalInstalledChanged() ), this, SLOT( slotReload() ) );
	connect( InstalledSingleton::Instance(), SIGNAL( signalInstalledReset() ), this, SLOT( slotReload() ) );
	
	slotInit();
}

/**
 * Save splitters and listview geometry.
 */
InstalledTab::~InstalledTab()
{
	KConfig *config = KurooConfig::self()->config();
	config->setGroup("Kuroo Geometry");
	
	QValueList<int> list = splitterH->sizes();
	config->writeEntry("splitterInstalledH", list);
	list = splitterV->sizes();
	config->writeEntry("splitterInstalledV", list);
	
	packagesView->saveLayout( KurooConfig::self()->config(), "installedViewLayout" );
	saveCurrentView();
}

/**
 * Initialize Installed view.
 * Restore geometry: splitter positions, listViews width and columns width.
 */
void InstalledTab::slotInit()
{
	KConfig *config = KurooConfig::self()->config();
	config->setGroup("Kuroo Geometry");
	
	QValueList<int> sizes = config->readIntListEntry("splitterInstalledH");
	splitterH->setSizes(sizes);
	sizes = config->readIntListEntry("splitterInstalledV");
	splitterV->setSizes(sizes);
	
	if ( !KurooConfig::init() )
		packagesView->restoreLayout( KurooConfig::self()->config(), "installedViewLayout" );
}

/**
 * Save latest selected packages in tabs All packages, Installed packages and Updates categories.
 */
void InstalledTab::saveCurrentView()
{
	QListViewItem *item = categoriesView->currentItem();
	if ( item && item->parent() )
		KurooConfig::setLatestInstalledCategory( item->parent()->text(0) + "-" + item->text(0) );
	item = packagesView->currentItem();
	if ( item )
		KurooConfig::setLatestInstalledPackage( item->text(0) );
	
	KurooConfig::writeConfig();
}

/**
 * Load categories.
 */
void InstalledTab::slotReload()
{
	saveCurrentView();
	packagesView->reset();
	categoriesView->loadCategories( InstalledSingleton::Instance()->categories() );
	slotViewPackage( KurooConfig::latestInstalledCategory() + "/" + KurooConfig::latestInstalledPackage() );
	emit signalChanged();
}

/**
 * Activate this package to view its info.
 */
void InstalledTab::slotViewPackage( const QString& package )
{
	QString category = package.section("/", 0, 0);
	QString name = package.section("/", 1, 1);
	categoriesView->setCurrentCategory( category );
	packagesView->setCurrentPackage( name );
	slotSummary();
}

/**
 * List packages when clicking on category.
 */
void InstalledTab::slotListPackages()
{
	QListViewItem *item = categoriesView->currentItem();
	
	if ( !item || !item->parent() )
		return;
	
	QString selectedCategory = item->parent()->text(0) + "-" + item->text(0);
	packagesView->addCategoryPackages( selectedCategory );
	
	// View summary info
	QString textLines = "<font size=\"+2\">" + selectedCategory + "</font><br>";
	summaryBrowser->clear();
	textLines += i18n("Total installed packages: ");
	textLines += packagesView->count();
	summaryBrowser->append( textLines );
}

/**
 * Refresh Installed packages.
 */
void InstalledTab::slotRefresh()
{
	switch( KMessageBox::questionYesNo( this, i18n("<qt>Do you want to refresh Installed view?</qt>"), i18n("Refreshing Installed"), KStdGuiItem::yes(), KStdGuiItem::no(), "dontAskAgainRefreshInstalled") ) {
		case KMessageBox::Yes: {
			saveCurrentView();
			InstalledSingleton::Instance()->slotRefresh();
		}
	}
}

/**
 * Unmerge selected packages.
 */
void InstalledTab::slotUnmerge()
{
	QStringList packageList(packagesView->selectedPackages());
	
	switch( KMessageBox::questionYesNoList( this, 
		i18n("<qt>Portage will not check if the package you want to remove is required by another package.<br>"
		     "Do you want to unmerge following packages?</qt>"), packageList, i18n("Unmerge packages") ) ) {
		case KMessageBox::Yes: {
			InstalledSingleton::Instance()->uninstallPackage( categoriesView->currentCategory(), packageList );
		}
	}
}

/**
 * Popup menu for actions like emerge.
 * @param item
 * @param point
 */
void InstalledTab::contextMenu( KListView*, QListViewItem* item, const QPoint& point )
{
	if ( !item )
		return;
		
	enum Actions { UPGRADE_PRETEND, APPEND, UPGRADE, EMERGE, UNMERGE,  DEPEND };
	
	KPopupMenu menu(this);
	int menuItem1 = menu.insertItem(i18n("&Upgrade pretend"), UPGRADE_PRETEND);
	int menuItem2 = menu.insertItem(i18n("&Append to queue"), APPEND);
	int menuItem4 = menu.insertItem(i18n("&Install now"), EMERGE);
	int menuItem5 = menu.insertItem(i18n("Unin&stall"), UNMERGE);
	
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() ) {
		menu.setItemEnabled( menuItem1, false );
		menu.setItemEnabled( menuItem2, false );
	}
	
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() || !KUser().isSuperUser() ) {
		menu.setItemEnabled( menuItem4, false );
		menu.setItemEnabled( menuItem5, false );
	}
	
	switch( menu.exec(point) ) {
		
		case UPGRADE_PRETEND: {
			InstalledSingleton::Instance()->pretendPackage( categoriesView->currentCategory(), packagesView->selectedNoVersion() );
			break;
		}

		case APPEND: {
			QueueSingleton::Instance()->addPackageIdList( packagesView->selectedId() );
			break;
		}
		
		case EMERGE: {
			QueueSingleton::Instance()->installQueue( packagesView->selectedId() );
			break;
		}
		
		case UNMERGE: {
			slotUnmerge();
		}
		
	}
}

/**
 * Find package by name or description among installed packages.
 * @fixme Better dialog text
 */
void InstalledTab::slotFind()
{
	static QString searchLine = "";
	
	KDialogBase *dial = new KDialogBase( KDialogBase::Swallow, i18n("Find packages"), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, this, i18n("Search"), true );
	SearchBase *searchDialog = new SearchBase(this);
	dial->setButtonText(KDialogBase::Ok, i18n("Search"));
	dial->setMainWidget(searchDialog);
	
	searchDialog->show();
	searchDialog->lineSearch->setFocus();
	searchDialog->lineSearch->setText(searchLine);
	
	if ( dial->exec() == QDialog::Accepted ) {
		
		// What are we searching for?
		searchLine = searchDialog->lineSearch->text();
		searchLine = searchLine.simplifyWhiteSpace();
		
		if ( searchDialog->comboSearch->currentItem() == 1 )
			InstalledSingleton::Instance()->findPackage( searchLine.lower(), true );
		else
			InstalledSingleton::Instance()->findPackage( searchLine.lower(), false );
	}
	
	delete dial;
	dial = 0;
}

/**
 * View summary info for selected package in "Installed packages".
 */
void InstalledTab::slotSummary()
{
	if ( !packagesView->currentItem() )
		return;
	
	QString summary( InstalledSingleton::Instance()->installedSummary( packagesView->currentId() ) );
	
	summaryBrowser->clear();
	if ( summary != i18n("na") )
		summaryBrowser->setText(summary);
	else
		summaryBrowser->setText(i18n("<font color=darkGrey><b>Summary could not be computed.</b></font>"));
		
	slotInstalledFiles( installedSummaryTabs->currentPage() );
}

/**
 * Load list of installed files only if tab is active.
 * @param page
 */
void InstalledTab::slotInstalledFiles( QWidget *page )
{
	if ( installedSummaryTabs->indexOf(page) == 1 ) {
		QString installedFiles( InstalledSingleton::Instance()->installedFiles( packagesView->currentId() ) );
		
		installedFilesBrowser->clear();
		if ( installedFiles != i18n("na") )
			installedFilesBrowser->setText(installedFiles);
		else
			installedFilesBrowser->setText(i18n("<font color=darkGrey><b>List of installed files not found.</b></font>"));
	}
}

#include "installedtab.moc"
