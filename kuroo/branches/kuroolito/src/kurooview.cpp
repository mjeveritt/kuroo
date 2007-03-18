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
 * @class KuroolitoView
 * @short Gui content with icon-menu and pages.
 * 
 * Insert all 5 pages in a widgetStack, connects icon-menu buttons to corresponding pages (tabs).
 * Highlights icon-texts when changes are mades in the page.
 */
KuroolitoView::KuroolitoView( QWidget *parent, const char *name )
	: KuroolitoViewBase( parent, name ),
	DCOPObject( "kurooIface" ),
	viewPortage( 0 ), packageInspector( 0 ), m_isHistoryRestored( false )
{
	setMinimumSize( QSize(750, 550) );
	
// 	viewMenu->setCursor( KCursor::handCursor() );
	
	// Create the package inspector
// 	packageInspector = new PackageInspector( this );
	
	// Add all pages
// 	viewPortage = new PortageTab( this, packageInspector );
// 	parent->addWidget( viewPortage, VIEW_PORTAGE );

	
	// Create menu-icons for the pages
// 	iconPackages = new IconListItem( viewMenu, ImagesSingleton::Instance()->icon( VIEW_PORTAGE ), i18n("Packages") );

	// Connect menu-icons to the pages
// 	connect( viewMenu, SIGNAL( selectionChanged() ), SLOT( slotShowView() ) );
// 	viewMenu->setCurrentItem( 0 );
	
	// Confirm changes in views with bleue text menu
// 	connect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotPortageUpdated() ) );
// 	connect( viewMenu, SIGNAL( currentChanged( QListBoxItem* ) ), this, SLOT( slotResetMenu( QListBoxItem* ) ) );
}

KuroolitoView::~KuroolitoView()
{}

/**
 * Activate corresponding page when clicking on icon in menu.
 */
void KuroolitoView::slotShowView()
{
// 	if ( packageInspector->isVisible() )
// 		packageInspector->hide();
// 	
// 	int tabIndex = viewMenu->currentItem() + 1;
// 	viewStack->raiseWidget( tabIndex );
}

/**
 * Check if database needs to refreshed.
 */
void KuroolitoView::slotInit()
{
// 	connect( HistorySingleton::Instance(), SIGNAL( signalScanHistoryCompleted() ), this, SLOT( slotCheckPortage() ) );
	
	// Check is history is empty, then maybe this is also a fresh install with empty db
// 	if ( KuroolitoDBSingleton::Instance()->isHistoryEmpty() ) {
// 		m_isHistoryRestored = true;
// 		
// 		KMessageBox::information( this, i18n( "<qt>Kuroolito database is empty!<br>"
// 		                                      "Kuroolito will now first scan your emerge log to create the emerge history.<br>"
// 		                                      "Next, package information in Portage will be collected.</qt>"), 
// 		                                i18n( "Initialiazing Kuroolito") );
// 		HistorySingleton::Instance()->slotRefresh();
// 	}
// 	else {
		
		// Load packages if db is not empty
// 		if ( !KuroolitoDBSingleton::Instance()->isPortageEmpty() ) {
// 			viewPortage->slotReload();
// 			viewQueue->slotReload( false );
// 		}
// 		else
// 			slotCheckPortage();
		
		// Check if kuroo db needs updating
// 		if ( !HistorySingleton::Instance()->slotRefresh() ) {
// 			disconnect( HistorySingleton::Instance(), SIGNAL( signalScanHistoryCompleted() ), this, SLOT( slotCheckPortage() ) );
// 			
// 			switch( KMessageBox::warningYesNo( this,
// 				i18n( "<qt>Kuroolito database needs refreshing!<br>"
// 				     "Emerge log shows that your system has changed.</qt>"), i18n("Initialiazing Kuroolito"), i18n("Refresh"), i18n("Skip"), 0 ) ) {
// 
// 				case KMessageBox::Yes:
// 					SignalistSingleton::Instance()->setKuroolitoReady( true );
// 					PortageSingleton::Instance()->slotRefresh();
// 					break;
// 
// 				default:
// 					KuroolitoDBSingleton::Instance()->setKuroolitoDbMeta( "scanTimeStamp", QString::number( QDateTime::currentDateTime().toTime_t() ) );
// 					slotCheckPortage();
// 			}
// 		}
// 	}
}

/**
 * When starting kuroo, check if portage need to be rescanned.
 * Updates must be scanned afterwards.
 */
void KuroolitoView::slotCheckPortage()
{
	DEBUG_LINE_INFO;
// 	disconnect( HistorySingleton::Instance(), SIGNAL( signalScanHistoryCompleted() ), this, SLOT( slotCheckPortage() ) );
	
	// Restore backup after db is recreated because of new version
// // 	if ( m_isHistoryRestored ) {
// // 		HistorySingleton::Instance()->updateStatistics();
// // 		m_isHistoryRestored = false;
// // 	}
	
	DEBUG_LINE_INFO;
/*	
	if ( KuroolitoDBSingleton::Instance()->isPortageEmpty() )
		PortageSingleton::Instance()->slotRefresh();
	else {*/
		
		// Ready to roll
// 		SignalistSingleton::Instance()->setKuroolitoReady( true );
// 	}
// 	DEBUG_LINE_INFO;
	
	// Warn user that emerge need root permissions - many rmb actions are disabled
// 	if ( !KUser().isSuperUser() )
// 		KMessageBox::information( this, i18n("You must run Kuroolito as root to emerge packages!"), i18n("Information"), "dontAskAgainNotRoot" );
}

/**
 * Dcop interface to emerge pretend process 
 * @param packageList	list of packages to emerge
 */
void KuroolitoView::slotEmergePretend( QString package )
{
// 	EmergeSingleton::Instance()->pretend( package );
}

/**
 * Highlight menutext in bleue.
 */
void KuroolitoView::slotPortageUpdated()
{
// 	if ( !iconPackages->isChanged() && !iconPackages->isSelected() ) {
// 		iconPackages->setChanged( true );
// 		viewMenu->triggerUpdate( true );
// 	}
}

/**
 * Clear the highlighting menu text back to normal when visits the view.
 */
void KuroolitoView::slotResetMenu( QListBoxItem* menuItem )
{
// 	dynamic_cast<IconListItem*>( menuItem )->setChanged( false );
// 	viewMenu->triggerUpdate( true );
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Create menu icons and highlight menutext when changes.
///////////////////////////////////////////////////////////////////////////////////////////////////

KuroolitoView::IconListItem::IconListItem( QListBox *listbox, const QPixmap &pixmap, const QString &text )
	: QListBoxItem( listbox ), m_modified( false )
{
	mPixmap = pixmap;
	if ( mPixmap.isNull() )
		mPixmap = defaultPixmap();
	
	setText( text );
	mMinimumWidth = 100;
}

void KuroolitoView::IconListItem::paint( QPainter *painter )
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

int KuroolitoView::IconListItem::height( const QListBox *lb ) const
{
	if ( text().isEmpty() )
		return mPixmap.height();
	else {
		int ht = lb->fontMetrics().boundingRect( 0, 0, 0, 0, Qt::AlignCenter, text() ).height();
		return ( mPixmap.height() + ht + 10 );
	}
}

int KuroolitoView::IconListItem::width( const QListBox *lb ) const
{
	int wt = lb->fontMetrics().boundingRect( 0, 0, 0, 0, Qt::AlignCenter, text() ).width() + 10;
	int wp = mPixmap.width() + 10;
	int w  = QMAX( wt, wp );
	return QMAX( w, mMinimumWidth );
}

const QPixmap &KuroolitoView::IconListItem::defaultPixmap()
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

void KuroolitoView::IconListItem::setChanged( bool modified )
{
	m_modified = modified;
}

bool KuroolitoView::IconListItem::isChanged()
{
	return m_modified;
}

#include "kurooview.moc"

