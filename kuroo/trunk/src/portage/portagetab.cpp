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
#include "portagepackagesview.h"
#include "portagetab.h"
#include "usedialog.h"

#include <qregexp.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qregexp.h>

#include <ktextbrowser.h>
#include <ktabwidget.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kdialogbase.h>
#include <kmessagebox.h>
#include <kuser.h>
#include <klineedit.h>

/**
 * Tab page for portage packages.
 */
PortageTab::PortageTab( QWidget* parent )
	: PortageBase( parent )
{
	packagesView = packagesSearchView->packagesView;
	
	connect( categoriesView, SIGNAL( selectionChanged() ), this, SLOT( slotListPackages() ) );
	
	// Rmb actions.
	connect( packagesView, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ), 
	         this, SLOT( contextMenu( KListView*, QListViewItem*, const QPoint& ) ) );
	
	// Package info actions.
	connect( packagesView, SIGNAL( selectionChanged() ), this, SLOT( slotSummary() ) );
	
	// Update file list only if this tab is open
	connect( portageSummaryTabs, SIGNAL( currentChanged(QWidget *) ), this, SLOT( slotPackageInfo(QWidget *) ) );
	
	// Reload view after changes.
	connect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotReload() ) );
	connect( InstalledSingleton::Instance(), SIGNAL( signalInstalledChanged() ), this, SLOT( slotReload() ) );
	
	slotInit();
}

/**
 * Save splitters and listview geometry.
 */
PortageTab::~PortageTab()
{
	KConfig *config = KurooConfig::self()->config();
	config->setGroup("Kuroo Geometry");
	
	QValueList<int> list = splitterH->sizes();
	config->writeEntry("splitterPortageH", list);
	list = splitterV->sizes();
	config->writeEntry("splitterPortageV", list);
	
	packagesView->saveLayout( KurooConfig::self()->config(), "portageViewLayout" );
	
	// Save latest selected packages in tabs All packages, Installed packages and Updates categories
	saveCurrentView();
}

/**
 * Save latest selected packages in tabs All packages, Installed packages and Updates categories.
 */
void PortageTab::saveCurrentView()
{
	kdDebug() << "PortageTab::saveCurrentView" << endl;
	QListViewItem *item = categoriesView->currentItem();
	if ( item && item->parent() )
		KurooConfig::setLatestPortageCategory( item->parent()->text(0) + "-" + item->text(0) );
	
	item = packagesView->currentItem();
	if ( item )
		KurooConfig::setLatestPortagePackage( item->text(0) );
	
	KurooConfig::writeConfig();
}

/**
 * Initialize Portage view.
 * Restore geometry: splitter positions, listViews width and columns width.
 */
void PortageTab::slotInit()
{
	KConfig *config = KurooConfig::self()->config();
	config->setGroup("Kuroo Geometry");
	
	// @fixme: portage splitters are bugging! using installed splitters instead
	QValueList<int> sizes = config->readIntListEntry("splitterInstalledH");
	splitterH->setSizes(sizes);
	sizes = config->readIntListEntry("splitterInstalledV");
	splitterV->setSizes(sizes);
	
	if ( !KurooConfig::init() )
		packagesView->restoreLayout( KurooConfig::self()->config(), "portageViewLayout" );
	
	useDialog = new UseDialog(this);
}

/**
 * Populate view with portage packages.
 * Then load the emerge history.
 */
void PortageTab::slotReload()
{
	kdDebug() << "PortageTab::slotReload" << endl;
	saveCurrentView();
	packagesView->reset();
	categoriesView->loadCategories( PortageSingleton::Instance()->categories() );
	slotViewPackage( KurooConfig::latestPortageCategory() + "/" + KurooConfig::latestPortagePackage() );
	emit signalChanged();
}

/**
 * Activate this package to view its info.
 * @param package
 */
void PortageTab::slotViewPackage( const QString& package )
{
	QString category = package.section("/", 0, 0);
	QString name = package.section("/", 1, 1);
	categoriesView->setCurrentCategory( category );
	packagesView->setCurrentPackage( name );
	slotSummary();
}

/**
 * List packages when clicking on category in installed.
 */
void PortageTab::slotListPackages()
{
	QString category = categoriesView->currentCategory();
	if ( category == i18n("na") )
		return;
	
	packagesView->addCategoryPackages( category );
	
	// View summary info
	QString textLines = "<font size=\"+2\">" + category + "</font><br>";
	summaryBrowser->clear();
	textLines += i18n("Total available packages: ");
	textLines += packagesView->count();
	summaryBrowser->append( textLines );
}

/**
 * Refresh installed packages list.
 */
void PortageTab::slotRefresh()
{
	switch( KMessageBox::questionYesNo( this, 
		i18n("<qt>Do you want to refresh Portage view?<br><br>"
		     "Installed and Updates view will be refreshed automatically afterwards. "
		     "Queue and Results view will be cleared.<br>"
		     "This will take a couple of minutes...</qt>"), i18n("Refreshing Portage"), KStdGuiItem::yes(), KStdGuiItem::no(), "dontAskAgainRefreshPortage") ) {
		case KMessageBox::Yes: {
			saveCurrentView();
			PortageSingleton::Instance()->slotRefresh();
		}
	}
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
	int menuItem6 = menu.insertItem(i18n("&Edit Use Flags"), USEFLAGS);
	
	// No access when kuroo is busy.
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() ) {
		menu.setItemEnabled( menuItem1, false );
		menu.setItemEnabled( menuItem2, false );
		menu.setItemEnabled( menuItem3, false );
	}
	
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() || !KUser().isSuperUser() )
		menu.setItemEnabled( menuItem3, false );
	
	// Kuroo needs specific package to work on.
	if ( !item->parent() ) {
		menu.setItemEnabled( menuItem2, false );
		menu.setItemEnabled( menuItem3, false );
	}
	
	if ( item->parent() || SignalistSingleton::Instance()->isKurooBusy() || !KUser().isSuperUser() ) {
		menu.setItemEnabled( menuItem4, false );
		menu.setItemEnabled( menuItem5, false );
	}
	
	if ( item->parent() || SignalistSingleton::Instance()->isKurooBusy() || !KUser().isSuperUser() ) {
		menu.setItemEnabled( menuItem6, false );
	}
	
	switch( menu.exec(point) ) {
		
		case PRETEND: {
			PortageSingleton::Instance()->pretendPackage( categoriesView->currentCategory(), packagesView->selectedPackages() );
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
		
		case UNMASK: {
			PortageSingleton::Instance()->unmaskPackageList( categoriesView->currentCategory(), packagesView->selectedPackages() );
			break;
		}
		
		case CLEARUNMASK: {
			PortageSingleton::Instance()->clearUnmaskPackageList( categoriesView->currentCategory(), packagesView->selectedPackages() );
			break;
		}
		
		case USEFLAGS: {
			useFlags();
			break;
		}
	}
}

/**
 * Find package by name or description among portage packages.
 */
void PortageTab::slotFind()
{
	static QString searchLine = "";
	
	KDialogBase *dial = new KDialogBase(KDialogBase::Swallow, i18n("Find packages"), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, this, i18n("Search"), true);
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
			PortageSingleton::Instance()->findPackage( searchLine.lower(), true );
		else
			PortageSingleton::Instance()->findPackage( searchLine.lower(), false );
	}
	
	delete dial;
	dial = 0;
}

/**
 * View summary for selected package.
 */
void PortageTab::slotSummary()
{
	kdDebug() << "PortageTab::slotSummary" << endl;
	summaryBrowser->clear();
	
	if ( !packagesView->currentItem() )
		return;
	
	// Is it a package or ebuild?
	if ( !packagesView->currentItem()->parent() ) {
		QString summary( PortageSingleton::Instance()->packageSummary( packagesView->currentId() ) );
		summaryBrowser->setText(summary);
	}
	else {
		QString summary( PortageSingleton::Instance()->versionSummary( packagesView->currentId() ) );
		summaryBrowser->setText(summary);
	}
	
	slotPackageInfo( portageSummaryTabs->currentPage() );
}

/**
 * View ebuild, changelog and dependencies.
 * @param page
 */
void PortageTab::slotPackageInfo( QWidget *page )
{
	enum summaryTab { EBUILD = 1, CHANGELOG, DEPENDENCIES };
	
	// Get selected item
	QString package = packagesView->currentPackage();
	QString category = PortageSingleton::Instance()->category( packagesView->currentId() );

	switch ( portageSummaryTabs->indexOf(page) ) {
		case EBUILD: {
			ebuildBrowser->clear();
			QString ebuild( PortageSingleton::Instance()->ebuild( packagesView->currentId() ) );
			
			if ( ebuild != i18n("na") )
				ebuildBrowser->setText( ebuild );
			else 
				ebuildBrowser->setText( i18n("<font color=darkGrey><b>Ebuild not found.</b></font>") );
			
			break;
		}
		
		case CHANGELOG: {
			changelogBrowser->clear();
			QString changelog( PortageSingleton::Instance()->changelog( packagesView->currentId() ) );
			
			if ( changelog != i18n("na") )
				changelogBrowser->setText( changelog );
			else 
				changelogBrowser->setText( i18n("<font color=darkGrey><b>Changelog not found.</b></font>") );

			break;
		}
		
		case DEPENDENCIES: {
			dependencyBrowser->clear();
			QString dependencies( PortageSingleton::Instance()->dependencies( packagesView->currentId() ) );
			
			if ( dependencies != i18n("na") )
				dependencyBrowser->setText( dependencies );
			else 
				dependencyBrowser->setText( i18n("<font color=darkGrey><b>Dependencies not found.</b></font>") );

			break;
		}
	}
}

/**
 * Open use flags dialog.
 */
void PortageTab::useFlags()
{
	useDialog->edit( PortageSingleton::Instance()->category( packagesView->currentId() ) + "/" + packagesView->currentPackage() );
}

#include "portagetab.moc"
