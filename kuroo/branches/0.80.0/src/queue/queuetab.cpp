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
#include "emergeinspector.h"
#include "pretendinspector.h"
#include "queuetab.h"
#include "queuelistview.h"

#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

#include <ktextbrowser.h>
#include <kdialogbase.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kpopupmenu.h>
#include <kuser.h>

/**
 * Tab page for the installation queue.
 */
QueueTab::QueueTab( QWidget* parent )
	: QueueBase( parent )
{
	// Rmb actions.
	connect( queueView, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ), 
	         this, SLOT( contextMenu( KListView*, QListViewItem*, const QPoint& ) ) );
	
	// Button actions.
	connect( pbUp, SIGNAL( clicked() ), queueView, SLOT( slotPackageUp() ) );
	connect( pbDown, SIGNAL( clicked() ), queueView, SLOT( slotPackageDown() ) );
	connect( pbClear, SIGNAL( clicked() ), QueueSingleton::Instance(), SLOT( reset() ) );
	connect( pbRemove, SIGNAL( clicked() ), this, SLOT( slotRemove() ) );
	
	connect( pbOptions, SIGNAL( clicked() ), this, SLOT( slotOptions() ) );
	connect( pbGo, SIGNAL( clicked() ), this, SLOT( slotGo() ) );
	connect( pbUninstall, SIGNAL( clicked() ), this, SLOT( slotUninstall() ) );
	connect( pbPretend, SIGNAL( clicked() ), this, SLOT( slotPretend() ) );
	
	// Lock/unlock if kuroo is busy.
	connect( SignalistSingleton::Instance(), SIGNAL( signalKurooBusy(bool) ), this, SLOT( slotBusy(bool) ) );
	
	connect( SignalistSingleton::Instance(), SIGNAL( signalEmergeQueue() ), this, SLOT( slotGo() ) );
	
	// Reload view after changes.
	connect( QueueSingleton::Instance(), SIGNAL( signalQueueChanged() ), this, SLOT( slotReload() ) );
	connect( ResultsSingleton::Instance(), SIGNAL( signalResultsChanged() ), this, SLOT( slotShowResults() ) );
	
	slotInit();
}

/**
 * Save listview geometry.
 */
QueueTab::~QueueTab()
{
	KConfig *config = KurooConfig::self()->config();
	config->setGroup("Kuroo Geometry");
	queueView->saveLayout( KurooConfig::self()->config(), "queueViewLayout" );
	KurooConfig::writeConfig();
}

/**
 * Initialize Queue view.
 */
void QueueTab::slotInit()
{
	KConfig *config = KurooConfig::self()->config();
	config->setGroup("Kuroo Geometry");
	
	if ( !KurooConfig::init() )
		queueView->restoreLayout( KurooConfig::self()->config(), "queueViewLayout" );
	
	emergeInspector = new EmergeInspector( this );
	pretendInspector = new PretendInspector( this );
	slotBusy( false );
}

/**
 * Load Queue packages.
 */
void QueueTab::slotReload()
{
	queueView->insertPackageList();
	
	QString queueBrowserLines( i18n( "<b>Summary</b><br>" ) );
			queueBrowserLines += i18n( "Number of packages: %1<br>" ).arg( QueueSingleton::Instance()->count() );
			queueBrowserLines += i18n( "Estimated time for emerge: %1<br>" ).arg( queueView->totalTime() );
			queueBrowserLines += i18n( "Estimated time remaining: <br>" );
	
	queueBrowser->clear();
	queueBrowser->setText( queueBrowserLines );
	
// 	totalSizeText->setText( queueView->totalSize() );
}

/**
 * Disable/enable buttons when kuroo is busy.
 * @param b
 */
void QueueTab::slotBusy( bool b )
{
	if ( b ) {
// 		pbGo->setDisabled( true );
		pbOptions->setDisabled( true );
		pbClear->setDisabled( true );
		
		if ( EmergeSingleton::Instance()->isRunning() )
			pbGo->setText( i18n( "Stop Installation!" ) );
		else
			pbGo->setText( i18n( "Start Installation!" ) );
	}
	else {
		if ( !KUser().isSuperUser() ) {
			pbGo->setDisabled(true);
			pbUninstall->setDisabled(true);
		}
		else {
			pbGo->setDisabled( false );
			pbUninstall->setDisabled( false );
		}
		
		pbOptions->setDisabled( false );
		pbClear->setDisabled( false );
	}
}

/**
 * Popup menu for actions like emerge.
 * @param item
 * @param point
 */
void QueueTab::contextMenu( KListView*, QListViewItem *item, const QPoint& point )
{
	if ( !item || !item->parent() )
		return;
	
	enum Actions { PRETEND, EMERGE, REMOVE, GOTO };
	
	KPopupMenu menu( this );
	int menuItem1 = menu.insertItem( i18n( "Emerge pretend" ), PRETEND );
	int menuItem2 = menu.insertItem( i18n( "Remove" ), REMOVE );
// 	menu.insertItem( i18n( "View Info" ), GOTO );
	
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() )
		menu.setItemEnabled( menuItem1, false );
	
	switch( menu.exec( point ) ) {
		
		case PRETEND:
			PortageSingleton::Instance()->pretendPackageList( queueView->selectedId() );
			break;

		case REMOVE:
			QueueSingleton::Instance()->removePackageIdList( queueView->selectedId() );
			break;
		
// 		case GOTO:
// 			SignalistSingleton::Instance()->viewPackage( queueView->currentPackage() );
	
	}
}

/**
 * Emerge all packages in the installation queue.
 */
void QueueTab::slotGo()
{
	kdDebug() << "QueueTab::slotGo" << endl;
	
	if ( EmergeSingleton::Instance()->isRunning() )
		slotStop();
	
	// Prepend emerge options
	QStringList packageList;
	QString options( emergeInspector->getOptions() );
	
	if ( options.isEmpty() )
		packageList = queueView->allPackages();
	else {
		packageList = QStringList::split( " ", options );
		packageList += queueView->allPackages();
	}
	
	switch( KMessageBox::questionYesNoList( this, 
		i18n("Do you want to emerge following packages?"), packageList, i18n("Emerge queue") ) ) {
		case KMessageBox::Yes: {
			QueueSingleton::Instance()->installPackageList( packageList );
			KurooStatusBar::instance()->setTotalSteps( queueView->sumTime() );
			pbGo->setText( i18n( "Stop Installation!" ) );
		}
	}
}

/**
 * Kill the running emerge process.
 */
void QueueTab::slotStop()
{
	switch ( KMessageBox::warningYesNo( this,
		i18n( "Do you want to abort the running emerge process?" ) ) ) {
		case KMessageBox::Yes : {
			EmergeSingleton::Instance()->stop();
			KurooStatusBar::instance()->setProgressStatus( i18n("Done.") );
		}
	}
}


/**
 * Launch unmerge of packages in queue.
 */
void QueueTab::slotUninstall()
{
	if ( !EmergeSingleton::Instance()->isRunning() || !SignalistSingleton::Instance()->isKurooBusy() /*|| !KUser().isSuperUser()*/ ) {
		QStringList packageList( queueView->allPackages() );
		switch( KMessageBox::questionYesNoList( this, 
			i18n( "<qt>Portage will not check if the package you want to remove is required by another package.<br>"
					"Do you want to unmerge following packages?</qt>" ), packageList, i18n( "Unmerge packages" ) ) ) {
				case KMessageBox::Yes:
					InstalledSingleton::Instance()->uninstallPackageList( queueView->allId() );
			}
	}
}

/**
 * Launch emerge pretend of packages in queue.
 */
void QueueTab::slotPretend()
{
	PortageSingleton::Instance()->pretendPackageList( queueView->allId() );
}

void QueueTab::slotShowResults()
{
	pretendInspector->showResults();
}

void QueueTab::slotRemove()
{
	QueueSingleton::Instance()->removePackageIdList( queueView->selectedId() );
}

void QueueTab::slotOptions()
{
	emergeInspector->edit();
}

#include "queuetab.moc"
