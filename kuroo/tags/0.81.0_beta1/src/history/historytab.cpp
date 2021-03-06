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
#include "historylistview.h"
#include "historytab.h"

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>

#include <ktextbrowser.h>
#include <kmessagebox.h>
#include <klistviewsearchline.h>
#include <kiconloader.h>
#include <kpushbutton.h>

/**
 * @class HistoryTab
 * @short Tabpage for emerge log browser.
 */
HistoryTab::HistoryTab( QWidget* parent )
	: HistoryBase( parent )
{
	pbClearFilter->setIconSet( SmallIconSet("locationbar_erase") );
	
	historyFilter->setListView( historyView );
	
	// Connect button and checkbox
	connect( viewUnmerges, SIGNAL( toggled( bool ) ), this, SLOT( slotViewUnmerges( bool ) ) );
	connect( pbClearFilter, SIGNAL( clicked() ), this, SLOT( slotClearFilter() ) );
	
	// Reload view after changes.
	connect( HistorySingleton::Instance(), SIGNAL( signalHistoryChanged() ), this, SLOT( slotReload() ) );
	connect( cbDays, SIGNAL( activated( int ) ), this, SLOT( slotReload( int ) ) );
	
	// Load history view after scan completed
	connect( HistorySingleton::Instance(), SIGNAL( signalScanHistoryCompleted() ), this, SLOT( slotReload() ) );
	
	connect( historyView, SIGNAL( selectionChanged() ), this, SLOT( slotButtonView() ) );
	connect( pbView, SIGNAL( clicked() ), this, SLOT( slotViewInfo() ) );
	
	slotInit();
}

/**
 * Save checkboxes state
 */
HistoryTab::~HistoryTab()
{
	if ( viewUnmerges->isChecked() )
		KurooConfig::setViewUnmerges( true );
	else
		KurooConfig::setViewUnmerges( false );
}

/**
 * Initialize geometry and content.
 * Restore checkboxes state
 */
void HistoryTab::slotInit()
{
	pbView->setDisabled( true );
	
	if ( KurooConfig::viewUnmerges() )
		viewUnmerges->setChecked( true );
	else
		viewUnmerges->setChecked( false );
}

/**
 * Reload history view.
 */
void HistoryTab::slotReload()
{
	slotReload( cbDays->currentItem() );
}

void HistoryTab::slotReload( int limit )
{
	int days;
	
	switch ( limit ) {
		case 0 :
			days = 7;
			break;
			
		case 1:
			days = 30;
			break;
			
		case 2:
			days = 180;
			break;
			
		case 3:
			days = 365;
			break;
			
		case 4:
			days = 10000;
	}
	
	historyView->loadFromDB( days );
}

void HistoryTab::slotClearFilter()
{
	historyFilter->clear();
}

void HistoryTab::slotViewUnmerges( bool on )
{
	KurooConfig::setViewUnmerges( on );
	slotReload();
}

/**
 * Activate button only when any file is selected.
 */
void HistoryTab::slotButtonView()
{
	QListViewItem *item = historyView->currentItem();
	if ( item && item->parent() )
		pbView->setDisabled( false );
	else
		pbView->setDisabled( true );
}

/**
 * Open in external browser.
 */
void HistoryTab::slotViewInfo()
{
	QListViewItem *item = historyView->currentItem();
	if ( !item )
		return;
	
	if ( !( item->text(2) ).isEmpty() ) {
		
		QString logText;
		QString eLogFile( KurooConfig::dirELog() + "/" + dynamic_cast<HistoryListView::HistoryItem*>( item )->einfo() );
		QFile logFile( eLogFile );
		if ( logFile.open( IO_ReadOnly ) ) {
			QTextStream stream( &logFile );
			while ( !stream.atEnd() )
				logText += stream.readLine() + "<br>";
			logFile.close();
		
			Message::instance()->prompt( i18n("Emerge log"), i18n("Installation message for %1:").arg( item->text(0) ), logText );
		}
		else {
			kdError(0) << "Reading: " << eLogFile << LINE_INFO;
			KMessageBox::error( this, i18n("Can not find elog for this emerge. Please check elog settings in /etc/make.conf.\n"
										   "Error reading:\n" + eLogFile
										  ), i18n("emerge elog") );
		}
	}
}

#include "historytab.moc"
