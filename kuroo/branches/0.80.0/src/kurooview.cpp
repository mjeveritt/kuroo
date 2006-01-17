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
#include "portagetab.h"
#include "queuetab.h"
#include "queuelistview.h"
#include "logstab.h"
#include "historytab.h"
#include "mergetab.h"
#include "packagelistview.h"
#include "kurooviewbase.h"

#include <sys/stat.h>

#include <qpainter.h>
#include <qlayout.h>
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

/**
 * @class KurooView
 * @short Gui content
 */
KurooView::KurooView( QWidget *parent, const char *name )
	: KurooViewBase( parent, name ),
	DCOPObject( "kurooIface" ),
	tabPortage( 0 ), tabQueue( 0 ), tabHistory( 0 ), tabLogs( 0 ), tabMerge( 0 )
{
	viewMenu->setCursor( KCursor::handCursor() );
	
	// Add all pages
	tabPortage = new PortageTab( this );
	viewStack->addWidget( tabPortage, 1 );
	
	tabQueue = new QueueTab( this );
	viewStack->addWidget( tabQueue, 2 );
	
	tabHistory = new HistoryTab( this );
	viewStack->addWidget( tabHistory, 3 );
	
	tabMerge = new MergeTab( this );
	viewStack->addWidget( tabMerge, 4 );
	
	tabLogs = new LogsTab( this );
	viewStack->addWidget( tabLogs, 5 );
	
	// Create menu-icons for the pages
	KIconLoader *ldr = KGlobal::iconLoader();
	iconPackages = new IconListItem( viewMenu, ldr->loadIcon( "kuroo", KIcon::Panel ), i18n("Packages") );
	iconQueue = new IconListItem( viewMenu, ldr->loadIcon( "launch", KIcon::Panel ), i18n("Queue") );
	iconHistory = new IconListItem( viewMenu, ldr->loadIcon( "date", KIcon::Panel ), i18n("History") );
	iconMerge = new IconListItem( viewMenu, ldr->loadIcon( "kcmdf", KIcon::Panel ), i18n("Etc-update") );
	iconLog = new IconListItem( viewMenu, ldr->loadIcon( "document", KIcon::Panel ), i18n("Log") );
	
	// Connect menu-icons to the pages
	connect( viewMenu, SIGNAL( selectionChanged() ), SLOT( slotShowView() ) );
	viewMenu->setSelected( 0, true );
	
	// Give log access to logBrowser and checkboxes
	// Check emerge.log for new entries. (In case of cli activities outside kuroo)
	LogSingleton::Instance()->setGui( tabLogs->logBrowser, tabLogs->verboseLog, tabLogs->saveLog );
	
	// Reset everything when a portage scan is started
	connect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotReset() ) );
	
	// Confirm changes in views with bleue text menu
	connect( InstalledSingleton::Instance(), SIGNAL( signalInstalledChanged() ), this, SLOT( slotPortageUpdated() ) );
	connect( UpdatesSingleton::Instance(), SIGNAL( signalUpdatesChanged() ), this, SLOT( slotPortageUpdated() ) );
	connect( QueueSingleton::Instance(), SIGNAL( signalQueueChanged(bool) ), this, SLOT( slotQueueUpdated() ) );
	connect( HistorySingleton::Instance(), SIGNAL( signalHistoryChanged() ), this, SLOT( slotHistoryUpdated() ) );
	connect( tabMerge, SIGNAL( signalMergeChanged() ), this, SLOT( slotMergeUpdated() ) );
	connect( LogSingleton::Instance(), SIGNAL( signalLogChanged() ), this, SLOT( slotLogUpdated() ) );
	connect( viewMenu, SIGNAL( currentChanged( QListBoxItem* ) ), this, SLOT( slotResetMenu( QListBoxItem* ) ) );
}

KurooView::~KurooView()
{
}

/**
 * Activate corresponding view when clicking on icon in menu.
 */
void KurooView::slotShowView()
{
	viewStack->raiseWidget( viewMenu->currentItem() + 1 );
}

/**
 * Check if database needs to refreshed.
 */
void KurooView::slotInit()
{
	if ( KurooDBSingleton::Instance()->isHistoryEmpty() ) {
		switch( KMessageBox::warningContinueCancel( this,
			i18n( "<qt>Kuroo database is empty!<br>"
			     "Kuroo will now first scan your emerge log to create the emerge history. "
			     "Package information in Portage will be cached.</qt>"), i18n("Initialize Kuroo"), KStdGuiItem::cont(), "dontAskAgainInitKuroo", 0 ) ) {
				     
			case KMessageBox::Continue: {
				connect( HistorySingleton::Instance(), SIGNAL( signalHistoryChanged() ), this, SLOT( slotCheckPortage() ) );
				HistorySingleton::Instance()->slotRefresh();
			}
		}
	}
	else {
		connect( HistorySingleton::Instance(), SIGNAL( signalHistoryChanged() ), this, SLOT( slotCheckPortage() ) );
		
		// Check if kuroo database needs updating.
		if ( !HistorySingleton::Instance()->slotRefresh() ) {
			disconnect( HistorySingleton::Instance(), SIGNAL( signalHistoryChanged() ), this, SLOT( slotCheckPortage() ) );
			
			switch( KMessageBox::warningYesNo( this,
				i18n( "<qt>Kuroo database needs refreshing!<br>"
				     "Emerge log shows that your system has changed.</qt>"), i18n("Initialize Kuroo"), i18n("Refresh"), i18n("Skip"), 0 ) ) {

				case KMessageBox::Yes:
					PortageSingleton::Instance()->slotRefresh();
					break;

				default:
					slotCheckPortage();
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
	slotPortageUpdated();
	UpdatesSingleton::Instance()->slotReset();
	slotInit();
}

/**
 * When starting kuroo, check if portage need to be rescanned.
 * Updates must be scanned afterwards.
 */
void KurooView::slotCheckPortage()
{
	kdDebug() << "KurooView::slotCheckPortage" << endl;
	disconnect( HistorySingleton::Instance(), SIGNAL( signalHistoryChanged() ), this, SLOT( slotCheckPortage() ) );
	
	if ( KurooDBSingleton::Instance()->packageTotal() == "0" )
		PortageSingleton::Instance()->slotRefresh();
	else {
		tabPortage->slotReload();
		tabQueue->slotReload( false );
		
		if ( KurooDBSingleton::Instance()->updatesTotal() == "0" )
			UpdatesSingleton::Instance()->slotRefresh();
		
		// Warn user that emerge need root permissions - many rmb actions are disabled
		if ( !KUser().isSuperUser() )
			KMessageBox::information( 0, i18n("You must run Kuroo as root to emerge packages!"), i18n("Information"), "dontAskAgainNotRoot" );
		
		// Ready to roll
		SignalistSingleton::Instance()->setKurooReady( true );
// 		SignalistSingleton::Instance()->setKurooBusy( false );
	}
}

/**
 * Action when user click in close button
 */
void KurooView::quit()
{
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
 * Highlight menutext in bleue.
 */
void KurooView::slotPortageUpdated()
{
	if ( !iconPackages->isChanged() ) {
		iconPackages->setChanged( true );
		viewMenu->triggerUpdate( true );
	}
}

/**
 * Highlight menutext in bleue.
 */
void KurooView::slotQueueUpdated()
{
	if ( !iconQueue->isChanged() ) {
		iconQueue->setChanged( true );
		viewMenu->triggerUpdate( true );
	}
}

/**
 * Highlight menutext in bleue.
 */
void KurooView::slotHistoryUpdated()
{
	if ( !iconHistory->isChanged() ) {
		iconHistory->setChanged( true );
		viewMenu->triggerUpdate( true );
	}
}

/**
 * Highlight menutext in bleue.
 */
void KurooView::slotMergeUpdated()
{
	if ( !iconMerge->isChanged() ) {
		iconMerge->setChanged( true );
		viewMenu->triggerUpdate( true );
	}
}

/**
 * Highlight menutext in bleue.
 */
void KurooView::slotLogUpdated()
{
	if ( !iconLog->isChanged() ) {
		iconLog->setChanged( true );
		viewMenu->triggerUpdate( true );
	}
}

/**
 * Disable icon menu text, from blue to black.
 */
void KurooView::slotResetMenu( QListBoxItem* menuItem )
{
	dynamic_cast<IconListItem*>( menuItem )->setChanged( false );
	viewMenu->triggerUpdate( true );
}


///////////////////////////////////////////////////////////////////////////////////////////////////

KurooView::IconListItem::IconListItem( QListBox *listbox, const QPixmap &pixmap, const QString &text )
	: QListBoxItem( listbox ), m_modified( false )
{
	mPixmap = pixmap;
	if( mPixmap.isNull() )
		mPixmap = defaultPixmap();
	
	setText( text );
	mMinimumWidth = 100;
}

void KurooView::IconListItem::paint( QPainter *painter )
{
	if ( m_modified )
		painter->setPen( Qt::blue );
	else
		painter->setPen( Qt::black );
	
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
		return ( mPixmap.height() + ht + 10 );
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
	static QPixmap *pix = 0;
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

void KurooView::IconListItem::setChanged( bool modified )
{
	m_modified = modified;
}

bool KurooView::IconListItem::isChanged()
{
	return m_modified;
}

#include "kurooview.moc"

