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
#include "packageinspector.h"

#include <sys/stat.h>

#include <qpainter.h>
#include <qlayout.h>
#include <qcolor.h>
#include <qwidgetstack.h>
#include <qbitmap.h>

#include <ktextbrowser.h>
#include <kmessagebox.h>
#include <kuser.h>
#include <kcursor.h>

/**
 * @class KurooView
 * @short Main gui with icon-menu and corresponding pages (tabs).
 */
KurooView::KurooView( QWidget *parent, const char *name )
	: KurooViewBase( parent, name ),
	DCOPObject( "kurooIface" ),
	viewPortage( 0 ), viewQueue( 0 ), viewHistory( 0 ), viewLogs( 0 ), viewMerge( 0 ), packageInspector( 0 ),
	m_isHistoryRestored( false )
{
	setMinimumSize( QSize(750, 550) );
	
	viewMenu->setCursor( KCursor::handCursor() );
	
	// Create the package inspector
	packageInspector = new PackageInspector( this );
	
	// Add all pages
	viewPortage = new PortageTab( this, packageInspector );
	viewStack->addWidget( viewPortage, VIEW_PORTAGE );
	
	viewQueue = new QueueTab( this, packageInspector );
	viewStack->addWidget( viewQueue, VIEW_QUEUE );
	
	viewHistory = new HistoryTab( this );
	viewStack->addWidget( viewHistory, VIEW_HISTORY );
	
	viewMerge = new MergeTab( this );
	viewStack->addWidget( viewMerge, VIEW_MERGE );
	
	viewLogs = new LogsTab( this );
	viewStack->addWidget( viewLogs, VIEW_LOG );
	
	// Create menu-icons for the pages
	iconPackages = new IconListItem( viewMenu, ImagesSingleton::Instance()->icon( VIEW_PORTAGE ), i18n("Packages") );
	iconQueue = new IconListItem( viewMenu, ImagesSingleton::Instance()->icon( VIEW_QUEUE ), i18n("Queue") );
	iconHistory = new IconListItem( viewMenu, ImagesSingleton::Instance()->icon( VIEW_HISTORY ), i18n("History") );
	iconMerge = new IconListItem( viewMenu, ImagesSingleton::Instance()->icon( VIEW_MERGE ), i18n("Etc-update") );
	iconLog = new IconListItem( viewMenu, ImagesSingleton::Instance()->icon( VIEW_LOG ), i18n("Log") );
	
	// Connect menu-icons to the pages
	connect( viewMenu, SIGNAL( selectionChanged() ), SLOT( slotShowView() ) );
	viewMenu->setCurrentItem( 0 );
	
	// Give log access to logBrowser and checkboxes
	// Check emerge.log for new entries. (In case of cli activities outside kuroo)
	LogSingleton::Instance()->setGui( viewLogs->logBrowser, viewLogs->verboseLog, viewLogs->saveLog );
	
	// Confirm changes in views with bleue text menu
	connect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotPortageUpdated() ) );
	
	connect( QueueSingleton::Instance(), SIGNAL( signalQueueChanged(bool) ), this, SLOT( slotQueueUpdated() ) );
	connect( HistorySingleton::Instance(), SIGNAL( signalHistoryChanged() ), this, SLOT( slotHistoryUpdated() ) );
	connect( viewMerge, SIGNAL( signalMergeChanged() ), this, SLOT( slotMergeUpdated() ) );
	connect( LogSingleton::Instance(), SIGNAL( signalLogChanged() ), this, SLOT( slotLogUpdated() ) );
	
	connect( viewMenu, SIGNAL( currentChanged( QListBoxItem* ) ), this, SLOT( slotResetMenu( QListBoxItem* ) ) );
}

KurooView::~KurooView()
{
}

/**
 * Activate corresponding page when clicking on icon in menu.
 */
void KurooView::slotShowView()
{
	if ( packageInspector->isVisible() )
		packageInspector->hide();
	
	int tabIndex = viewMenu->currentItem() + 1;
	viewStack->raiseWidget( tabIndex );
}

/**
 * Check if database needs to refreshed.
 */
void KurooView::slotInit()
{
	DEBUG_LINE_INFO;
	
	connect( HistorySingleton::Instance(), SIGNAL( signalScanHistoryCompleted() ), this, SLOT( slotCheckPortage() ) );
	
	// Check is history is empty, then maybe this is also a fresh install with empty db
	if ( KurooDBSingleton::Instance()->isHistoryEmpty() ) {
		m_isHistoryRestored = true;
		
		KMessageBox::information( this, i18n( "<qt>Kuroo database is empty!<br>"
		                                      "Kuroo will now first scan your emerge log to create the emerge history.<br>"
		                                      "Next, package information in Portage will be collected.</qt>"), 
		                                i18n( "Initialiazing Kuroo") );
		HistorySingleton::Instance()->slotRefresh();
	}
	else {
		
		// Load packages if db is not empty
		if ( !KurooDBSingleton::Instance()->isPortageEmpty() ) {
			viewPortage->slotReload();
			viewQueue->slotReload( false );
		}
		
		// Check if kuroo db needs updating
		if ( !HistorySingleton::Instance()->slotRefresh() ) {
			disconnect( HistorySingleton::Instance(), SIGNAL( signalScanHistoryCompleted() ), this, SLOT( slotCheckPortage() ) );
			
			switch( KMessageBox::warningYesNo( this,
				i18n( "<qt>Kuroo database needs refreshing!<br>"
				     "Emerge log shows that your system has changed.</qt>"), i18n("Initialiazing Kuroo"), i18n("Refresh"), i18n("Skip"), 0 ) ) {

				case KMessageBox::Yes:
					SignalistSingleton::Instance()->setKurooReady( true );
					PortageSingleton::Instance()->slotRefresh();
					break;

				default:
					KurooDBSingleton::Instance()->setKurooDbMeta( "scanTimeStamp", QString::number( QDateTime::currentDateTime().toTime_t() ) );
					slotCheckPortage();
			}
		}
	}
}

/**
 * When starting kuroo, check if portage need to be rescanned.
 * Updates must be scanned afterwards.
 */
void KurooView::slotCheckPortage()
{
	DEBUG_LINE_INFO;
	
	disconnect( HistorySingleton::Instance(), SIGNAL( signalScanHistoryCompleted() ), this, SLOT( slotCheckPortage() ) );
	
	// Restore backup after db is recreated because of new version
	if ( m_isHistoryRestored ) {
		KurooDBSingleton::Instance()->restoreBackup();
		HistorySingleton::Instance()->updateStatistics();
		m_isHistoryRestored = false;
	}
	
	if ( KurooDBSingleton::Instance()->isPortageEmpty() )
		PortageSingleton::Instance()->slotRefresh();
	else {
		
		// Warn user that emerge need root permissions - many rmb actions are disabled
		if ( !KUser().isSuperUser() )
			KMessageBox::information( this, i18n("You must run Kuroo as root to emerge packages!"), i18n("Information"), "dontAskAgainNotRoot" );
		
		// Ready to roll
		SignalistSingleton::Instance()->setKurooReady( true );
	}
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
	if ( !iconPackages->isChanged() && !iconPackages->isSelected() ) {
		iconPackages->setChanged( true );
		viewMenu->triggerUpdate( true );
	}
}

/**
 * Highlight menutext in bleue.
 */
void KurooView::slotQueueUpdated()
{
	if ( !iconQueue->isChanged() && !iconQueue->isSelected() ) {
		iconQueue->setChanged( true );
		viewMenu->triggerUpdate( true );
	}
}

/**
 * Highlight menutext in bleue.
 */
void KurooView::slotHistoryUpdated()
{
	if ( !iconHistory->isChanged() && !iconHistory->isSelected() ) {
		iconHistory->setChanged( true );
		viewMenu->triggerUpdate( true );
	}
}

/**
 * Highlight menutext in bleue.
 */
void KurooView::slotMergeUpdated()
{
	if ( !iconMerge->isChanged() && !iconMerge->isSelected() ) {
		iconMerge->setChanged( true );
		viewMenu->triggerUpdate( true );
	}
}

/**
 * Highlight menutext in bleue.
 */
void KurooView::slotLogUpdated()
{
	if ( !iconLog->isChanged() && !iconLog->isSelected() ) {
		iconLog->setChanged( true );
		viewMenu->triggerUpdate( true );
	}
}

/**
 * Clear the highlighting menu text back to normal when visits the view.
 */
void KurooView::slotResetMenu( QListBoxItem* menuItem )
{
	dynamic_cast<IconListItem*>( menuItem )->setChanged( false );
	viewMenu->triggerUpdate( true );
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Create menu icons and highlight menutext when changes.
///////////////////////////////////////////////////////////////////////////////////////////////////

KurooView::IconListItem::IconListItem( QListBox *listbox, const QPixmap &pixmap, const QString &text )
	: QListBoxItem( listbox ), m_modified( false )
{
	mPixmap = pixmap;
	if ( mPixmap.isNull() )
		mPixmap = defaultPixmap();
	
	setText( text );
	mMinimumWidth = 100;
}

void KurooView::IconListItem::paint( QPainter *painter )
{
	if ( isSelected() ) {
		painter->setPen( listBox()->colorGroup().highlightedText() );
		m_modified = false;
	}
	else {
		if ( m_modified )
			painter->setPen( listBox()->colorGroup().link() );
		else
			painter->setPen( listBox()->colorGroup().text() );
	}
	
	QFontMetrics fm = painter->fontMetrics();
	int ht = fm.boundingRect( 0, 0, 0, 0, Qt::AlignCenter, text() ).height();
	int wp = mPixmap.width();
	int hp = mPixmap.height();
	
	painter->drawPixmap( ( listBox()->maxItemWidth()-wp ) / 2, 5, mPixmap );
	
	if ( !text().isEmpty() )
		painter->drawText( 0, hp + 7, listBox()->maxItemWidth(), ht, Qt::AlignCenter, text() );
}

int KurooView::IconListItem::height( const QListBox *lb ) const
{
	if ( text().isEmpty() )
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

