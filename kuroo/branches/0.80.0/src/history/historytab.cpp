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
#include <qsplitter.h>

#include <ktextbrowser.h>
#include <kmessagebox.h>
#include <klistviewsearchline.h>

/**
 * Tabpage for emerge log browser, emerge history and portage directories sizes.
 */
HistoryTab::HistoryTab( QWidget* parent )
	: HistoryBase( parent )
{
	historyFilter->setListView( historyView );
	
	connect( pbClearFilter, SIGNAL( clicked() ), this, SLOT( slotClearFilter() ) );
	
	// Reload view after changes.
	connect( HistorySingleton::Instance(), SIGNAL( signalHistoryChanged() ), this, SLOT( slotReload() ) );
	
// 	slotInit();
}

/**
 * Save splitters and listview geometry.
 */
HistoryTab::~HistoryTab()
{
}

/**
 * Initialize geometry and content.
 * Restore geometry: splitter positions, listViews width and columns width.
 */
void HistoryTab::slotInit()
{
}

/**
 * Reload history view.
 */
void HistoryTab::slotReload()
{
	historyView->loadFromDB();
}

void HistoryTab::slotClearFilter()
{
	historyFilter->clear();
}


#include "historytab.moc"
