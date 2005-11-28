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
#include "kurooview.h"
#include "installedtab.h"
#include "portagetab.h"
#include "updatestab.h"
#include "updatelistview.h"
#include "queuetab.h"
#include "queuelistview.h"
#include "resultstab.h"
#include "resultlistview.h"
#include "logstab.h"
#include "packagelistview.h"
#include "kurooviewbase.h"

#include <sys/stat.h>

#include <qpainter.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qcolor.h>
#include <qwidgetstack.h>
#include <qbitmap.h>

#include <ktabwidget.h>
#include <ktextbrowser.h>
#include <ktrader.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <kuser.h>
#include <kcursor.h>
#include <kiconloader.h>

class KurooView::IconListItem : public QListBoxItem
{
public:
	IconListItem( QListBox *listbox, const QPixmap &pixmap, const QString &text );
	virtual int height( const QListBox *lb ) const;
	virtual int width( const QListBox *lb ) const;
	int expandMinimumWidth( int width );
	
protected:
	const QPixmap &defaultPixmap();
	void paint( QPainter *painter );
	
private:
	QPixmap mPixmap;
	int mMinimumWidth;
};

/**
 * Gui content.
 * First create a tabwidget then fill it with 6 tabs for Installed, Portage, Updates...
 * Then connect signal that content of a tab is changed so the tabheader text color is activated = blue.
 * Also connect signals from all objects like Installed, Portage... to each of their corresponding tab.
 * Finally connect signal from when Portage is changed (reseted after sync) to clear all other objects.
 */
KurooView::KurooView( QWidget *parent, const char *name )
	: KurooViewBase( parent, name ),
	DCOPObject( "kurooIface" ),
	tabPortage(0), tabLogs(0)
{
	viewMenu->setCursor(KCursor::handCursor());
	
	tabPortage = new PortageTab(this);
	viewStack->addWidget( tabPortage, 1 );
	
	tabLogs = new LogsTab(this);
	viewStack->addWidget( tabLogs, 2 );
	
	KIconLoader *ldr = KGlobal::iconLoader();
	new IconListItem( viewMenu, ldr->loadIcon( "deb", KIcon::Panel ), "Packages" );
// 	new IconListItem( viewMenu, ldr->loadIcon( "run", KIcon::Panel ), "Emerge Queue" );
// 	new IconListItem( viewMenu, ldr->loadIcon( "history", KIcon::Panel ), "Emerge History" );
	new IconListItem( viewMenu, ldr->loadIcon( "history", KIcon::Panel ), "Emerge Logs" );
	
	connect( viewMenu, SIGNAL( selectionChanged() ), SLOT( slotShowView() ) );
	viewMenu->setSelected( 0, true );
	
	// Give log access to logBrowser and checkboxes
	// Check emerge.log for new entries. (Due to cli activities outside kuroo)
	LogSingleton::Instance()->setGui( tabLogs->logBrowser, tabLogs->verboseLog, tabLogs->saveLog );
	
	// View this package info by making it current.
	connect( SignalistSingleton::Instance(), SIGNAL( signalViewPackage(const QString&) ), this, SLOT( slotViewPackage(const QString&) ) );
	
	connect( tabPortage, SIGNAL( signalChanged() ), this, SLOT( slotPortageUpdated() ) );
	
	// Reset everything when a portage scan is started
	connect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotReset() ) );
}

KurooView::~KurooView()
{
}

void KurooView::slotShowView()
{
	int index( viewMenu->currentItem() + 1 );
	
	kdDebug() << "KurooView::slotShowView index=" << index << endl;
	
	switch (index) {
		case 1: {
			viewStack->raiseWidget(index);
			break;
		}
			
		case 2: {
			viewStack->raiseWidget(index);
			break;
		}
	
	}
}

KurooView::IconListItem::IconListItem( QListBox *listbox, const QPixmap &pixmap, const QString &text )
: QListBoxItem( listbox )
{
	mPixmap = pixmap;
	if( mPixmap.isNull() )
		mPixmap = defaultPixmap();
	
	setText( text );
	mMinimumWidth = 100;
}

void KurooView::IconListItem::paint( QPainter *painter )
{
	QFontMetrics fm = painter->fontMetrics();
	int ht = fm.boundingRect( 0, 0, 0, 0, Qt::AlignCenter, text() ).height();
	int wp = mPixmap.width();
	int hp = mPixmap.height();
	
	painter->drawPixmap( (mMinimumWidth-wp)/2, 5, mPixmap );
	if( !text().isEmpty() )
		painter->drawText( 0, hp+7, mMinimumWidth, ht, Qt::AlignCenter, text() );
}

int KurooView::IconListItem::height( const QListBox *lb ) const
{
	if( text().isEmpty() )
		return mPixmap.height();
	else {
		int ht = lb->fontMetrics().boundingRect( 0, 0, 0, 0, Qt::AlignCenter, text() ).height();
		return (mPixmap.height() + ht + 10);
	}
}

int KurooView::IconListItem::width( const QListBox *lb ) const
{
	int wt = lb->fontMetrics().boundingRect( 0, 0, 0, 0, Qt::AlignCenter, text() ).width() + 10;
	int wp = mPixmap.width() + 10;
	int w  = QMAX( wt, wp );
	return QMAX( w, mMinimumWidth );
}

const QPixmap &KurooView::IconListItem::defaultPixmap()
{
	static QPixmap *pix=0;
	if ( !pix ) {
		pix = new QPixmap( 32, 32 );
		QPainter p( pix );
		p.eraseRect( 0, 0, pix->width(), pix->height() );
		p.setPen( Qt::red );
		p.drawRect ( 0, 0, pix->width(), pix->height() );
		p.end();
		
		QBitmap mask( pix->width(), pix->height(), true );
		mask.fill( Qt::black );
		p.begin( &mask );
		p.setPen( Qt::white );
		p.drawRect ( 0, 0, pix->width(), pix->height() );
		p.end();
		
		pix->setMask( mask );
	}
	
	return *pix;
}

void KurooView::slotInit()
{
	kdDebug() << "KurooView::slotInit" << endl;
	
	if ( KurooDBSingleton::Instance()->isHistoryEmpty() )
		switch( KMessageBox::warningContinueCancel( this,
			i18n("<qt>Kuroo database is empty!<br><br>"
			     "Kuroo will now first scan your emerge log to create the emerge history. "
			     "Next Kuroo will refresh Portage, Installed and Updates packages view.<br>"
			     "Package information in Portage will be cached.</qt>"), i18n("Initialize Kuroo"), KStdGuiItem::cont(), "dontAskAgainInitKuroo", 0) ) {
			case KMessageBox::Continue: {
				connect( HistorySingleton::Instance(), SIGNAL( signalHistoryChanged() ), this, SLOT( slotCheckPortage() ) );
				HistorySingleton::Instance()->slotRefresh();
			}
		}
	else {
		connect( HistorySingleton::Instance(), SIGNAL( signalHistoryChanged() ), this, SLOT( slotCheckPortage() ) );
		
		// Check if kuroo database needs updating.
		if ( !HistorySingleton::Instance()->slotRefresh() ) {
			disconnect( HistorySingleton::Instance(), SIGNAL( signalHistoryChanged() ), this, SLOT( slotCheckPortage() ) );
			
			switch( KMessageBox::warningYesNo( this,
				i18n("<qt>Kuroo database needs refreshing!<br>"
				     "Emerge log shows that your system has changed.</qt>"), i18n("Initialize Kuroo"), i18n("Refresh"), i18n("Skip"), 0) ) {
				case KMessageBox::Yes: {
// 					connect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotCheckInstalled() ) );
					PortageSingleton::Instance()->slotRefresh();
					break;
				}
				default: {
					slotCheckPortage();
				}
			}
		}
	}
}

/**
 * Reset everything when a portage scan is started.
 */
void KurooView::slotReset()
{
	kdDebug() << "KurooView::slotReset" << endl;
	
// 	InstalledSingleton::Instance()->slotReset();
// 	UpdatesSingleton::Instance()->slotReset();
// 	QueueSingleton::Instance()->reset();
// 	ResultsSingleton::Instance()->reset();
	slotInit();
}

/**
 * When starting kuroo, check if portage need to be scanned.
 * Installed, and Updates must be scanned afterwards.
 */
void KurooView::slotCheckPortage()
{
	kdDebug() << "KurooView::slotCheckPortage" << endl;
	disconnect( HistorySingleton::Instance(), SIGNAL( signalHistoryChanged() ), this, SLOT( slotCheckPortage() ) );
	
	if ( PortageSingleton::Instance()->count() == "0" ) {
// 		connect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotCheckInstalled() ) );
		PortageSingleton::Instance()->slotRefresh();
	}
	else {
		tabPortage->slotReload();
// 		slotCheckInstalled();
	}
}

/**
 * Check if Installed is empty, if so scan for installed packages.
 */
void KurooView::slotCheckInstalled()
{
	kdDebug() << "KurooView::slotCheckInstalled" << endl;
	disconnect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotCheckInstalled() ) );
// 	
// 	if ( InstalledSingleton::Instance()->count() == "0" ) {
// 		connect( InstalledSingleton::Instance(), SIGNAL( signalInstalledChanged() ), this, SLOT( slotCheckUpdates() ) );
// 		InstalledSingleton::Instance()->slotRefresh();
// 	}
// 	else {
// 		tabInstalled->slotReload();
// 		slotCheckUpdates();
// 	}
}

/**
 * Check if Updates empty, if so scan for updates packages.
 */
void KurooView::slotCheckUpdates()
{
	kdDebug() << "KurooView::slotCheckUpdates" << endl;
// 	disconnect( InstalledSingleton::Instance(), SIGNAL( signalInstalledChanged() ), this, SLOT( slotCheckUpdates() ) );
// 	
// 	if ( UpdatesSingleton::Instance()->count() == "0" ) {
// 		connect( UpdatesSingleton::Instance(), SIGNAL( signalUpdatesChanged() ), this, SLOT( slotReloadQueueResults() ) );
// 		UpdatesSingleton::Instance()->slotRefresh();
// 	}
// 	else {
// 		tabUpdates->slotReload();
// 		slotReloadQueueResults();
// 	}
}

/**
 * End by loading the queue and the results packages list.
 */
void KurooView::slotReloadQueueResults()
{
	kdDebug() << "KurooView::slotReloadQueueResults" << endl;
	
// 	disconnect( UpdatesSingleton::Instance(), SIGNAL( signalUpdatesChanged() ), this, SLOT( slotReloadQueueResults() ) );
// 	
// 	tabInstalled->slotReload();
// 	tabUpdates->slotReload();
// 	tabQueue->slotReload();
// 	tabResults->slotReload();
	
	// Warn user that emerge need root permissions - many rmb actions are disabled
	if ( !KUser().isSuperUser() )
		KMessageBox::information( 0, i18n("You must run Kuroo as root to emerge packages!"), i18n("Information"), "dontAskAgainNotRoot" );
}

/**
 * Action when user click in close button
 */
void KurooView::quit()
{
}

/**
 * Change tab color back to normal.
 */
void KurooView::slotCurrentChanged( QWidget* newPage )
{
// 	mainTabs->setTabColor( newPage, black );
}

/**
 * Activate this package to view its info.
 */
void KurooView::slotViewPackage( const QString& package )
{
// 	if ( PortageSingleton::Instance()->isInstalled( package ) ) {
// 		mainTabs->showPage( tabInstalled );
// 		tabInstalled->slotViewPackage( package );
// 	}
// 	else {
// 		mainTabs->showPage( tabPortage );
// 		tabPortage->slotViewPackage( package );
// 	}
}

/**
 * Dcop interface to emerge pretend process 
 * @param packageList	list of packages to emerge
 */
void KurooView::slotEmergePretend( QString package )
{
	EmergeSingleton::Instance()->pretend( package );
}

/**
 * Count the total packages in Portage (@fixme: including installed packages not in Portage anymore)
 * @return total
 */
void KurooView::slotPortageUpdated()
{
	kdDebug() << "KurooView::slotPortageUpdated" << endl;
	static bool tabSetup(false);
	QString total = PortageSingleton::Instance()->count();

// 	mainTabs->setTabLabel( tabPortage, i18n("&Portage (%1)").arg(total) );
// 	
// 	if ( mainTabs->currentPageIndex() != 1 && tabSetup )
// 		mainTabs->setTabColor( tabPortage, blue );
	
	tabSetup = true;
}

/**
 * Installed tabpage count.
 */
void KurooView::slotInstalledUpdated()
{
// 	static bool tabSetup(false);
// 	QString total = InstalledSingleton::Instance()->count();
// 
// 	mainTabs->setTabLabel( tabInstalled, i18n("&Installed (%1)").arg( total ) );
// 	
// 	if ( mainTabs->currentPageIndex() != 0 && tabSetup )
// 		mainTabs->setTabColor( tabInstalled, blue );
// 	
// 	tabSetup = true;
}

/**
 * Update tabpage count.
 */
void KurooView::slotUpdatesUpdated()
{
// 	static bool tabSetup(false);
// 	QString total = UpdatesSingleton::Instance()->count();
// 		
// 	mainTabs->setTabLabel( tabUpdates, i18n("&Updates (%1)").arg( total ) );
// 	
// 	if ( mainTabs->currentPageIndex() != 2 && tabSetup )
// 		mainTabs->setTabColor( tabUpdates, blue );
// 	
// 	tabSetup = true;
}

/**
 * Queue tabpage count.
 */
void KurooView::slotQueueUpdated()
{
// 	static bool tabSetup(false);
// 	QString total = QueueSingleton::Instance()->count();
// 	
// 	mainTabs->setTabLabel( tabQueue, i18n("&Queue (%1)").arg( total ) );
// 	
// 	if ( mainTabs->currentPageIndex() != 3 && tabSetup )
// 		mainTabs->setTabColor( tabQueue, blue );
// 	
// 	tabSetup = true;
}

/**
 * Results tabpage count.
 */
void KurooView::slotResultsUpdated()
{
// 	static bool tabSetup(false);
// 	QString total = ResultsSingleton::Instance()->count();
// 	
// 	mainTabs->setTabLabel( tabResults, i18n("&Results (%1)").arg( total ) );
// 	
// 	if ( mainTabs->currentPageIndex() != 4 && tabSetup )
// 		mainTabs->setTabColor( tabResults, blue );
// 	
// 	tabSetup = true;
}

/**
 * Catch when log tab is updated.
 */
void KurooView::slotLogsTabUpdated()
{
// 	static bool tabSetup(false);
// 	
// 	if ( mainTabs->currentPageIndex() != 5 && tabSetup )
// 		mainTabs->setTabColor( tabLogs, blue );
// 	
// 	tabSetup = true;
}

#include "kurooview.moc"

