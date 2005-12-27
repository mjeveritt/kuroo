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
#include "mergelistview.h"
#include "historytab.h"

#include <qcheckbox.h>
#include <qpushbutton.h>

#include <ktextbrowser.h>
#include <kmessagebox.h>
#include <klistviewsearchline.h>
#include <krun.h>

/**
 * Tabpage for emerge log browser.
 */
HistoryTab::HistoryTab( QWidget* parent )
	: HistoryBase( parent )
{
	historyFilter->setListView( historyView );
	
	connect( viewUnmerges, SIGNAL( toggled( bool ) ), this, SLOT( slotViewUnmerges( bool ) ) );
	connect( pbClearFilter, SIGNAL( clicked() ), this, SLOT( slotClearFilter() ) );
	
	// Reload view after changes.
	connect( HistorySingleton::Instance(), SIGNAL( signalHistoryChanged() ), this, SLOT( slotReload() ) );
	
	connect( EtcUpdateSingleton::Instance(), SIGNAL( signalEtcFileMerged() ), this, SLOT( slotMergeReload() ) );
	connect( mergeView, SIGNAL( executed( QListViewItem* ) ), this, SLOT( slotViewFile( QListViewItem* ) ) );
	
	slotInit();
}

/**
 * Save splitters and listview geometry.
 */
HistoryTab::~HistoryTab()
{
	// Save checkboxes state
	if ( viewUnmerges->isChecked() )
		KurooConfig::setViewUnmerges( true );
	else
		KurooConfig::setViewUnmerges( false );
}

/**
 * Initialize geometry and content.
 * Restore geometry: splitter positions, listViews width and columns width.
 */
void HistoryTab::slotInit()
{
	// Restore checkboxes state
	if ( KurooConfig::viewUnmerges() )
		viewUnmerges->setChecked( true );
	else
		viewUnmerges->setChecked( false );
	
	slotMergeReload();
}

/**
 * Reload history view.
 */
void HistoryTab::slotReload()
{
	historyView->loadFromDB();
}

/**
 * Reload history view.
 */
void HistoryTab::slotMergeReload()
{
	mergeView->loadFromDB();
}

void HistoryTab::slotClearFilter()
{
	historyFilter->clear();
}

void HistoryTab::slotViewUnmerges( bool on )
{
	kdDebug() << "HistoryTab::slotViewUnmerges" << endl;
	KurooConfig::setViewUnmerges( on );
	slotReload();
}

/**
 * Open in external browser.
 */
void HistoryTab::slotViewFile( QListViewItem *item )
{
	QString source = item->text( 0 );
	if ( !source.isEmpty() )
		new KRun( "file://" + KUROODIR + "backup/" + source );
}

#include "historytab.moc"
