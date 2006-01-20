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
 * @class QueueTab
 * @short Page for the installation queue.
 */
QueueTab::QueueTab( QWidget* parent )
	: QueueBase( parent ), m_hasCheckedQueue( false )
{
	// Rmb actions.
	connect( queueView, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ), 
	         this, SLOT( contextMenu( KListView*, QListViewItem*, const QPoint& ) ) );
	
	// Button actions.
	connect( pbClear, SIGNAL( clicked() ), QueueSingleton::Instance(), SLOT( reset() ) );
	connect( pbRemove, SIGNAL( clicked() ), this, SLOT( slotRemove() ) );
		
	// Lock/unlock if kuroo is busy.
	connect( SignalistSingleton::Instance(), SIGNAL( signalKurooBusy( bool ) ), this, SLOT( slotBusy( bool ) ) );
	
	connect( SignalistSingleton::Instance(), SIGNAL( signalEmergeQueue() ), this, SLOT( slotGo() ) );
	
	// Reload view after changes in queue.
	connect( QueueSingleton::Instance(), SIGNAL( signalQueueChanged( bool ) ), this, SLOT( slotReload( bool ) ) );
	
	// Forward emerge start/stop/completed to package progressbar.
	connect( QueueSingleton::Instance(), SIGNAL( signalPackageStart( const QString& ) ), queueView, SLOT( slotPackageStart( const QString& ) ) );
	connect( QueueSingleton::Instance(), SIGNAL( signalPackageComplete( const QString& ) ), queueView, SLOT( slotPackageComplete( const QString& ) ) );
	connect( QueueSingleton::Instance(), SIGNAL( signalPackageAdvance( const QString& ) ), queueView, SLOT( slotPackageProgress( const QString& ) ) );
	
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
	
	slotBusy( false );
}

/**
 * Load Queue packages.
 */
void QueueTab::slotReload( bool hasCheckedQueue )
{
	m_hasCheckedQueue = hasCheckedQueue;
	
	kdDebug() << "QueueTab::slotReload hasCheckedQueue=" << hasCheckedQueue << endl;
	queueView->insertPackageList();
	
	QString queueBrowserLines( i18n( "<b>Summary</b><br>" ) );
			queueBrowserLines += i18n( "Number of packages: %1<br>" ).arg( queueView->count() );
			queueBrowserLines += i18n( "Estimated time for installation: %1<br>" ).arg( queueView->totalTime() );
// 			queueBrowserLines += i18n( "Estimated time remaining: <br>" );
	
	queueBrowser->clear();
	queueBrowser->setText( queueBrowserLines );
	
	if ( m_hasCheckedQueue && !KUser().isSuperUser() )
		m_hasCheckedQueue = false;
	
	slotBusy( false );
}

/**
 * Disable/enable buttons when kuroo busy signal is received.
 * @param busy
 */
void QueueTab::slotBusy( bool busy )
{
	kdDebug() << "QueueTab::slotBusy busy=" << busy << endl;
	
	if ( EmergeSingleton::Instance()->isRunning() ) {
		pbGo->setText( i18n( "Stop!" ) );
		disconnect( pbGo, SIGNAL( clicked() ), this, SLOT( slotGo() ) );
		disconnect( pbGo, SIGNAL( clicked() ), this, SLOT( slotStop() ) );
		connect( pbGo, SIGNAL( clicked() ), this, SLOT( slotStop() ) );
	}
	else {
		disconnect( pbGo, SIGNAL( clicked() ), this, SLOT( slotGo() ) );
		disconnect( pbGo, SIGNAL( clicked() ), this, SLOT( slotStop() ) );
		connect( pbGo, SIGNAL( clicked() ), this, SLOT( slotGo() ) );
		
		if ( m_hasCheckedQueue && KUser().isSuperUser() )
			pbGo->setText( i18n( "Start Installation!" ) );
		else
			pbGo->setText( i18n("Check Installation!") );
	}
	
	// No db no fun!
	if ( !SignalistSingleton::Instance()->isKurooReady() || queueView->count() == "0" || busy ) {
		pbClear->setDisabled( true );
		pbRemove->setDisabled( true );
		cbDownload->setDisabled( true );
	}
	else {
		pbClear->setDisabled( false );
		pbRemove->setDisabled( false );
		cbDownload->setDisabled( false );
	}

	if ( !SignalistSingleton::Instance()->isKurooReady() || queueView->count() == "0" )
		pbGo->setDisabled( true );
	else
		pbGo->setDisabled( false );
}

/**
 * Emerge all packages in the installation queue.
 */
void QueueTab::slotGo()
{
	kdDebug() << "QueueTab::slotGo m_hasCheckedQueue=" << m_hasCheckedQueue << endl;
	
	// If emerge is running I'm the abort function
	if ( EmergeSingleton::Instance()->isRunning() )
		slotStop();
	
	// First we must run emerge pretend
	if ( !m_hasCheckedQueue ) {
		PortageSingleton::Instance()->pretendPackageList( queueView->allId() );
		return;
	}
	
	// Only user-end packages not the dependencies
	QStringList packageList = queueView->allPackagesNoChildren();
	
	// Only download? prepend --fetch-all-uri
	// Else, let's install the user-end packages
	if ( cbDownload->isChecked() ) {
		switch( KMessageBox::questionYesNoList( this, 
			i18n("Do you want to Download following packages?"), packageList, i18n("Installation queue") ) ) {
			case KMessageBox::Yes: {
				packageList.prepend( "--fetch-all-uri" );
				QueueSingleton::Instance()->installPackageList( packageList );
				KurooStatusBar::instance()->setTotalSteps( queueView->sumTime() );
			}
		}
	}
	else {
		switch( KMessageBox::questionYesNoList( this, 
			i18n("Do you want to install following packages?"), packageList, i18n("Installation queue") ) ) {
			case KMessageBox::Yes: {
				QueueSingleton::Instance()->installPackageList( packageList );
				KurooStatusBar::instance()->setTotalSteps( queueView->sumTime() );
				m_hasCheckedQueue = false;
			}
		}
	}
}

/**
 * Kill the running emerge process.
 */
void QueueTab::slotStop()
{
	switch ( KMessageBox::warningYesNo( this,
		i18n( "Do you want to abort the running installation?" ) ) ) {
		case KMessageBox::Yes : {
			EmergeSingleton::Instance()->stop();
			QueueSingleton::Instance()->stopTimer();
			KurooStatusBar::instance()->setProgressStatus( QString::null, i18n("Done.") );
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

/**
 * Remove package from Queue.
 */
void QueueTab::slotRemove()
{
	QueueSingleton::Instance()->removePackageIdList( queueView->selectedId() );
}

/**
 * Popup menu for actions like emerge.
 * @param item
 * @param point
 */
void QueueTab::contextMenu( KListView*, QListViewItem *item, const QPoint& point )
{
	if ( !item )
		return;
	
	enum Actions { PRETEND, EMERGE, REMOVE, GOTO };
	
	KPopupMenu menu( this );
	int menuItem1 = menu.insertItem( i18n( "Emerge pretend" ), PRETEND );
	int menuItem2 = menu.insertItem( i18n( "Remove" ), REMOVE );
	
	if ( EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy() ) {
		menu.setItemEnabled( menuItem1, false );
		menu.setItemEnabled( menuItem2, false );
	}
	
	switch( menu.exec( point ) ) {
		
		case PRETEND:
			PortageSingleton::Instance()->pretendPackageList( queueView->selectedId() );
			break;
			
		case REMOVE:
			QueueSingleton::Instance()->removePackageIdList( queueView->selectedId() );
	}
}

#include "queuetab.moc"
