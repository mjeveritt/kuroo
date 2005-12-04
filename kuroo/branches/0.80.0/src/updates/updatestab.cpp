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
#include "updatestab.h"
#include "categorieslistview.h"
#include "updatelistview.h"
#include "updatepackageslistview.h"

#include <qlayout.h>
#include <qsplitter.h>
#include <qgroupbox.h>
#include <qpushbutton.h>

#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kuser.h>
#include <kmessagebox.h>

/**
 * Tab page for update packages.
 */
UpdatesTab::UpdatesTab( QWidget *parent )
	: UpdatesBase( parent )
{
// 	connect( categoriesView, SIGNAL( selectionChanged() ), this, SLOT( slotListPackages() ) );
	
	// Rmb actions.
// 	connect( packagesView, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ), 
// 	         this, SLOT( contextMenu( KListView*, QListViewItem*, const QPoint& ) ) );
// 	connect( updatesList, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ), 
// 	         this, SLOT( contextMenu( KListView*, QListViewItem*, const QPoint& ) ) );
	
	// Button actions.
// 	connect( pbEmergeQueue, SIGNAL( clicked() ), this, SLOT( slotEmergeQueue() ) );
	
	// Lock/unlock if kuroo is busy.
// 	connect( SignalistSingleton::Instance(), SIGNAL( signalKurooBusy(bool) ), this, SLOT( slotBusy(bool) ) );
	
	// Reload views after changes.
// 	connect( UpdatesSingleton::Instance(), SIGNAL( signalUpdatesChanged() ), this, SLOT( slotReload() ) );
	
	slotInit();
}

/**
 * Save splitters and listview geometry.
 */
UpdatesTab::~UpdatesTab()
{
	KConfig *config = KurooConfig::self()->config();
	config->setGroup("Kuroo Geometry");
	
	QValueList<int> list = splitterH->sizes();
	config->writeEntry("splitterUpdatesH", list);
	list = splitterV->sizes();
	config->writeEntry("splitterUpdatesV", list);
	
	packagesView->saveLayout( KurooConfig::self()->config(), "updatesViewLayout" );
	updatesList->saveLayout( KurooConfig::self()->config(), "updatesListLayout" );
	
	saveCurrentView();
}

/**
 * Save latest selected packages in tabs All packages, Installed packages and Updates categories.
 */
void UpdatesTab::saveCurrentView()
{
	QListViewItem *item = categoriesView->currentItem();
	if ( item && item->parent() )
		KurooConfig::setLatestUpdatesCategory( item->parent()->text(0) + "-" + item->text(0) );
	
	KurooConfig::writeConfig();
}

/**
 * Initialize geometry and content.
 * Restore geometry: splitter positions, listViews width and columns width.
 * Restore latest user view.
 */
void UpdatesTab::slotInit()
{
	KConfig *config = KurooConfig::self()->config();
	config->setGroup("Kuroo Geometry");
	
	QValueList<int> sizes = config->readIntListEntry("splitterUpdatesH");
	splitterH->setSizes(sizes);
	sizes = config->readIntListEntry("splitterUpdatesV");
	splitterV->setSizes(sizes);
	
	if ( !KurooConfig::init() ) {
		packagesView->restoreLayout( KurooConfig::self()->config(), "updatesViewLayout" );
		updatesList->restoreLayout( KurooConfig::self()->config(), "updatesListLayout" );
	}
	
	slotBusy(false);
}

/**
 * Reload update packages list.
 */
void UpdatesTab::slotReload()
{
	kdDebug() << "UpdatesTab::slotReload" << endl;
	
	saveCurrentView();
	packagesView->reset();
	updatesList->reset();
	
	categoriesView->loadCategories( UpdatesSingleton::Instance()->categories() );
	categoriesView->setCurrentCategory( KurooConfig::latestUpdatesCategory() );
	updatesList->loadFromDB();
	emit signalChanged();
}

/**
 * Refresh update packages list.
 */
void UpdatesTab::slotRefresh()
{
	switch( KMessageBox::questionYesNo( this, i18n("<qt>Do you want to refresh Updates view?</qt>"), i18n("Refreshing Updates"), KStdGuiItem::yes(), KStdGuiItem::no(), "dontAskAgainRefreshUpdates") ) {
		case KMessageBox::Yes: {
			saveCurrentView();
			UpdatesSingleton::Instance()->slotRefresh();
		}
	}
}

/**
 * Disable/enable buttons when kuroo is busy.
 * @param b
 */
void UpdatesTab::slotBusy( bool b )
{
	if ( b )
		pbEmergeQueue->setDisabled(true);
	else {
		if ( !KUser().isSuperUser() )
			pbEmergeQueue->setDisabled(true);
		else
			pbEmergeQueue->setDisabled(false);
	}
}

/**
 * Fill the right listView with all upgradable packages.
 * Restore updates packages view.
 */
void UpdatesTab::slotListPackages()
{
	QListViewItem *item = categoriesView->currentItem();
	if ( !item || !item->parent() )
		return;
	
	QString selectedCategory = item->parent()->text(0) + "-" + item->text(0);
	packagesView->addCategoryPackages( selectedCategory );
}

/**
 * Launch emerge of all update packages.
 */
void UpdatesTab::slotEmergeQueue()
{
	QueueSingleton::Instance()->installQueue( updatesList->allId() );
}

/**
 * Popup menu for actions like emerge.
 * @param listView
 * @param item
 * @param point
 */
void UpdatesTab::contextMenu( KListView *listView, QListViewItem *item, const QPoint &point )
{
	QString lv(listView->name());
	
	if ( !item )
		return;
	
	enum Actions {
		PRETEND, APPEND, EMERGE,
		PRETEND_LIST, APPEND_LIST, EMERGE_LIST, GOTO, GOTOLIST };
	
	KPopupMenu menu(this);
	
	if ( lv.endsWith("View") ) {
		int menuItem1 = menu.insertItem( i18n("&Pretend"), PRETEND );
		int menuItem2 = menu.insertItem( i18n("&Append to queue"), APPEND );
		int menuItem3 = menu.insertItem( i18n("&Install now"), EMERGE );
		menu.insertItem( i18n("View Info"), GOTO );
		
		if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() ) {
			menu.setItemEnabled( menuItem1, false );
			menu.setItemEnabled( menuItem2, false );
			menu.setItemEnabled( menuItem3, false );
		}
		
		if ( !KUser().isSuperUser() )
			menu.setItemEnabled( menuItem3, false );
	}
	else {
		int menuItem1 = menu.insertItem(i18n("&Pretend"), PRETEND_LIST);
		int menuItem2 = menu.insertItem(i18n("&Append to queue"), APPEND_LIST);
		int menuItem3 = menu.insertItem(i18n("&Install now"), EMERGE_LIST);
		menu.insertItem(i18n("View Info"), GOTOLIST );
		
		if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() ) {
			menu.setItemEnabled( menuItem1, false );
			menu.setItemEnabled( menuItem2, false );
			menu.setItemEnabled( menuItem3, false );
		}
		
		if ( !KUser().isSuperUser() )
			menu.setItemEnabled( menuItem3, false );
	}
	
	switch( menu.exec(point) ) {
		
		case PRETEND: {
			UpdatesSingleton::Instance()->pretendPackage( categoriesView->currentCategory(), packagesView->selectedPackages() );
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
		
		case PRETEND_LIST: {
			UpdatesSingleton::Instance()->pretendPackage( updatesList->selectedPackages() );
			break;
		}
		
		case APPEND_LIST: {
			QueueSingleton::Instance()->addPackageIdList( updatesList->selectedId() );
			break;
		}
		
		case EMERGE_LIST: {
			QueueSingleton::Instance()->installQueue( updatesList->selectedId() );
			break;
		}
		
		case GOTO: {
			SignalistSingleton::Instance()->viewPackage( categoriesView->currentCategory() + "/" + packagesView->currentPackage() ); 
			break;
		}
		
		case GOTOLIST: {
			SignalistSingleton::Instance()->viewPackage( updatesList->currentPackage() );
			break;
		}
	}
}

#include "updatestab.moc"
