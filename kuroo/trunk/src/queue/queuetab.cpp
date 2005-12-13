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
#include "emergeoptions.h"
#include "queuetab.h"
#include "queuelistview.h"

#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

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
	connect( pbClear, SIGNAL( clicked() ), QueueSingleton::Instance(), SLOT( reset() ) );
	connect( pbOptions, SIGNAL( clicked() ), this, SLOT( slotOptions() ) );
	connect( pbGo, SIGNAL( clicked() ), this, SLOT( slotGo() ) );
	connect( pbStop, SIGNAL( clicked() ), this, SLOT( slotStop() ) );
	
	// Lock/unlock if kuroo is busy.
	connect( SignalistSingleton::Instance(), SIGNAL( signalKurooBusy(bool) ), this, SLOT( slotBusy(bool) ) );
	
	connect( SignalistSingleton::Instance(), SIGNAL( signalEmergeQueue() ), this, SLOT( slotGo() ) );
	
	// Reload view after changes.
	connect( QueueSingleton::Instance(), SIGNAL( signalQueueChanged() ), this, SLOT( slotReload() ) );
	connect( SignalistSingleton::Instance(), SIGNAL( signalInstalledChanged() ), this, SLOT( slotReload() ) );
	
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
	
	slotBusy(false);
}

/**
 * Load Queue packages.
 */
void QueueTab::slotReload()
{
	queueView->loadFromDB();
	totalSizeText->setText(queueView->totalSize());
	totalTimeText->setText(queueView->totalTime());
}

/**
 * Disable/enable buttons when kuroo is busy.
 * @param b
 */
void QueueTab::slotBusy( bool b )
{
	if ( b ) {
		pbGo->setDisabled(true);
		pbOptions->setDisabled(true);
		pbClear->setDisabled(true);
		
		if ( EmergeSingleton::Instance()->isRunning() )
			pbStop->setDisabled(false);
		else
			pbStop->setDisabled(true);
	}
	else {
		if ( !KUser().isSuperUser() )
			pbGo->setDisabled(true);
		else
			pbGo->setDisabled(false);
		
		pbStop->setDisabled(true);
		pbOptions->setDisabled(false);
		pbClear->setDisabled(false);
	}
}

/**
 * Open dialog for advanced emerge options.
 */
void QueueTab::slotOptions()
{
	dial = new KDialogBase( KDialogBase::Swallow, i18n("Advanced emerge options"), KDialogBase::User1 |KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, this, i18n("Options"), true);
	
	dial->setButtonText( KDialogBase::User1, i18n("Reset") );
	connect (dial, SIGNAL(user1Clicked()), this, SLOT(slotClearOptions()));
	
	optionsDialog = new EmergeOptionsBase();
	dial->setMainWidget(optionsDialog);
	optionsDialog->show();

	// Get options
	QString options(emergeOptionsText->text());
	
	if ( !options.isEmpty() ) {
		const QStringList optionsList = QStringList::split(" ", options);
		foreach ( optionsList ) {
			options = (*it).stripWhiteSpace();
			
			if ( options == "--buildpkg" )
				optionsDialog->radioBuildpkg->toggle();
			
			if ( options == "--buildpkgonly" )
				optionsDialog->radioBuildpkgonly->toggle();
			
			if ( options == "--usepkg" )
				optionsDialog->radioUsepkg->toggle();
			
			if ( options == "--usepkgonly" )
				optionsDialog->radioUsepkgonly->toggle();
			
			if ( options == "--nodeps" )
				optionsDialog->radioNodeps->toggle();
			
			if ( options == "--onlydeps" )
				optionsDialog->radioOnlydeps->toggle();
			
			if ( options == "--verbose" )
				optionsDialog->radioVerbose->toggle();
			
			if ( options == "--quiet" )
				optionsDialog->radioQuiet->toggle();
			
			if ( options == "--pretend" )
				optionsDialog->checkPretend->toggle();
			
			if ( options == "--deep" )
				optionsDialog->checkDeep->toggle();
			
			if ( options == "--update" )
				optionsDialog->checkUpdate->toggle();
			
			if ( options == "--upgradeonly" )
				optionsDialog->checkUpgradeonly->toggle();
			
			if ( options == "--fetchonly" )
				optionsDialog->checkFetchonly->toggle();
			
			if ( options == "--emptytree" )
				optionsDialog->checkEmptytree->toggle();
			
			if ( options == "--debug" )
				optionsDialog->checkDebug->toggle();
			
			if ( options == "--noconfmem" )
				optionsDialog->checkNoconfmem->toggle();
			
			if ( options == "--oneshot" )
				optionsDialog->checkOneshot->toggle();
			
			if ( options == "--noreplace" )
				optionsDialog->checkNoreplace->toggle();
			
			if ( options == "--newuse" )
				optionsDialog->checkNewUse->toggle();
		}
	}

	// Write options back to lineedit
	if ( dial->exec() == QDialog::Accepted ) {
		QStringList optionsList;
		
		if ( optionsDialog->radioBuildpkg->isOn() ) 
			optionsList += "--buildpkg";
		else
			if ( optionsDialog->radioBuildpkgonly->isOn() ) 
				optionsList += "--buildpkgonly";
		
		if ( optionsDialog->radioUsepkg->isOn() ) 
			optionsList += "--usepkg";
		else
			if ( optionsDialog->radioUsepkgonly->isOn() ) 
				optionsList += "--usepkgonly";
		
		if ( optionsDialog->radioNodeps->isOn() ) 
			optionsList += "--nodeps";
		else
			if ( optionsDialog->radioOnlydeps->isOn() ) 
				optionsList += "--onlydeps";
		
		if ( optionsDialog->radioVerbose->isOn() ) 
			optionsList += "--verbose";
		else
			if ( optionsDialog->radioQuiet->isOn() ) 
				optionsList += "--quiet";
		
		if ( optionsDialog->checkPretend->isChecked() ) 
			optionsList += "--pretend";
		
		if ( optionsDialog->checkDeep->isChecked() ) 
			optionsList += "--deep";
		
		if ( optionsDialog->checkUpdate->isChecked() ) 
			optionsList += "--update";
		
		if ( optionsDialog->checkUpgradeonly->isChecked() ) 
			optionsList += "--upgradeonly";
		
		if ( optionsDialog->checkFetchonly->isChecked() ) 
			optionsList += "--fetchonly";
		
		if ( optionsDialog->checkEmptytree->isChecked() ) 
			optionsList += "--emptytree";
		
		if ( optionsDialog->checkDebug->isChecked() ) 
			optionsList += "--debug";
		
		if ( optionsDialog->checkNoconfmem->isChecked() ) 
			optionsList += "--noconfmem";
		
		if ( optionsDialog->checkOneshot->isChecked() ) 
			optionsList += "--oneshot";
		
		if ( optionsDialog->checkNoreplace->isChecked() ) 
			optionsList += "--noreplace";
		
		if ( optionsDialog->checkNewUse->isChecked() ) 
			optionsList += "--newuse";
		
		emergeOptionsText->setText( optionsList.join(" ") );
	}
}

void QueueTab::slotClearOptions()
{
	if ( optionsDialog->radioBuildpkg->isOn() )
		optionsDialog->radioBuildpkg->toggle();
	
	if ( optionsDialog->radioBuildpkgonly->isOn() ) 
		optionsDialog->radioBuildpkgonly->toggle();
	
	if ( optionsDialog->radioUsepkg->isOn() ) 
		optionsDialog->radioUsepkg->toggle();
	
	if ( optionsDialog->radioUsepkgonly->isOn() ) 
		optionsDialog->radioUsepkgonly->toggle();
	
	if ( optionsDialog->radioNodeps->isOn() ) 
		optionsDialog->radioNodeps->toggle();
	
	if ( optionsDialog->radioOnlydeps->isOn() ) 
		optionsDialog->radioOnlydeps->toggle();
	
	if ( optionsDialog->radioVerbose->isOn() ) 
		optionsDialog->radioVerbose->toggle();
	
	if ( optionsDialog->radioQuiet->isOn() ) 
		optionsDialog->radioQuiet->toggle();
	
	optionsDialog->checkPretend->setChecked(false);
	optionsDialog->checkDeep->setChecked(false);
	optionsDialog->checkUpdate->setChecked(false);
	optionsDialog->checkUpgradeonly->setChecked(false);
	optionsDialog->checkFetchonly->setChecked(false);
	optionsDialog->checkEmptytree->setChecked(false);
	optionsDialog->checkDebug->setChecked(false);
	optionsDialog->checkNoconfmem->setChecked(false);
	optionsDialog->checkOneshot->setChecked(false);
	optionsDialog->checkNoreplace->setChecked(false);
	optionsDialog->checkNewUse->setChecked(false);
	
	emergeOptionsText->clear();
}

/**
 * Emerge all packages in the installation queue.
 */
void QueueTab::slotGo()
{
	// Prepend emerge options
	QStringList packageList;
	QString options(emergeOptionsText->text());
	
	if ( options.isEmpty() )
		packageList = queueView->allPackages();
	else {
		packageList = QStringList::split(" ", options);
		packageList += queueView->allPackages();
	}
		
	switch( KMessageBox::questionYesNoList( this, 
		i18n("Do you want to emerge following packages?"), packageList, i18n("Emerge queue") ) ) {
			case KMessageBox::Yes: {
				QueueSingleton::Instance()->installPackageList( packageList );
				KurooStatusBar::instance()->setTotalSteps( queueView->sumTime() );
			}
	}
}

/**
 * Kill the running emerge process.
 */
void QueueTab::slotStop()
{
	switch ( KMessageBox::warningYesNo(this,
		i18n("Do you want to abort the running emerge process?"))) {
			case KMessageBox::Yes : {
				EmergeSingleton::Instance()->stop();
				KurooStatusBar::instance()->setProgressStatus( i18n("Done.") );
			}
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
	
	KPopupMenu menu(this);
	int menuItem1 = menu.insertItem(i18n("Emerge pretend"), PRETEND);
	int menuItem2 = menu.insertItem(i18n("Remove"), REMOVE );
	menu.insertItem(i18n("View Info"), GOTO );
	
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() ) {
		menu.setItemEnabled( menuItem1, false );
	}
	
	switch( menu.exec(point) ) {
		
		case PRETEND: {
			QueueSingleton::Instance()->pretendPackageList( queueView->selectedPackages() );
			break;
		}

		case REMOVE: {
			QueueSingleton::Instance()->removePackageIdList( queueView->selectedId() );
			break;
		}
		
		case GOTO: {
			SignalistSingleton::Instance()->viewPackage( queueView->currentPackage() );
			break;
		}
	}
}

#include "queuetab.moc"
