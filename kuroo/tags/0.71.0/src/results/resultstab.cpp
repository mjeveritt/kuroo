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
#include "resultstab.h"
#include "resultlistview.h"

#include <qpushbutton.h>

#include <kpopupmenu.h>

/**
 * Tabpage for emerge and search results.
 */
ResultsTab::ResultsTab( QWidget *parent )
 : ResultsBase( parent )
{
	// Rmb actions.
	connect( resultView, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ), 
	         this, SLOT( contextMenu( KListView*, QListViewItem*, const QPoint& ) ) );
	
	// Button actions.
	connect( pbClear, SIGNAL( clicked() ), ResultsSingleton::Instance(), SLOT( reset() ) );
	connect( pbAppendAll, SIGNAL( clicked() ), this, SLOT( slotAppendAll() ) );
	
	// Lock/unlock if kuroo is busy.
	connect( SignalistSingleton::Instance(), SIGNAL( signalKurooBusy(bool) ), this, SLOT( slotBusy(bool) ) );
	
	// Reload view after changes.
	connect( ResultsSingleton::Instance(), SIGNAL( signalResultsChanged() ), this, SLOT( slotReload() ) );
	connect( SignalistSingleton::Instance(), SIGNAL( signalInstalledChanged() ), this, SLOT( slotReload() ) );
	
	slotInit();
}

/**
 * Save listview geometry.
 */
ResultsTab::~ResultsTab()
{
	KConfig *config = KurooConfig::self()->config();
	config->setGroup("Kuroo Geometry");
	resultView->saveLayout( KurooConfig::self()->config(), "resultsViewLayout" );
	KurooConfig::writeConfig();
}

/**
 * Initialize Results view.
 */
void ResultsTab::slotInit()
{
	KConfig *config = KurooConfig::self()->config();
	config->setGroup("Kuroo Geometry");
	
	if ( !KurooConfig::init() )
		resultView->restoreLayout( KurooConfig::self()->config(), "resultsViewLayout" );
}

void ResultsTab::slotReload()
{
	resultView->loadFromDB();
}

/**
 * Disable/enable buttons when kuroo is busy.
 * @param b
 */
void ResultsTab::slotBusy( bool b )
{
	if ( b )
		pbAppendAll->setDisabled(true);
	else
		pbAppendAll->setDisabled(false);
}

/**
 * Append all packages to the installation queue.
 */
void ResultsTab::slotAppendAll()
{
	QueueSingleton::Instance()->addPackageIdList( resultView->allId() );
}

/**
 * Popup menu for actions like emerge.
 * @param item
 * @param point
 */
void ResultsTab::contextMenu( KListView*, QListViewItem* item, const QPoint& point )
{
	if ( !item )
		return;
	
	enum Actions { PRETEND, APPEND, GOTO };
	
	KPopupMenu menu(this);
	int menuItem1 = menu.insertItem( i18n("&Emerge pretend"), PRETEND );
	int menuItem2 = menu.insertItem( i18n("&Append to queue"), APPEND );
	menu.insertItem( i18n("View Info"), GOTO );
	
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() ) {
		menu.setItemEnabled( menuItem1, false );
		menu.setItemEnabled( menuItem2, false );
	}
	
	switch( menu.exec(point) ) {
		
		case PRETEND: {
			ResultsSingleton::Instance()->pretendPackageList( resultView->selectedPackages() );
			break;
		}
		
		case APPEND: {
			QueueSingleton::Instance()->addPackageIdList( resultView->selectedId() );
			break;
		}
		
		case GOTO: {
			SignalistSingleton::Instance()->viewPackage( resultView->currentPackage() );
		}
	}
}

#include "resultstab.moc"
