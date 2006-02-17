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
#include "mergetab.h"

#include <qcheckbox.h>
#include <qpushbutton.h>

#include <ktextbrowser.h>
#include <kmessagebox.h>
#include <klistviewsearchline.h>
#include <krun.h>
#include <kprocio.h>
#include <kiconloader.h>

/**
 * @class MergeTab
 * @short Tabpage for emerge log browser.
 */
MergeTab::MergeTab( QWidget* parent )
	: MergeBase( parent )
{
	pbClearFilter->setIconSet( SmallIconSet("locationbar_erase") );
	
	mergeFilter->setListView( mergeView );

	connect( EtcUpdateSingleton::Instance(), SIGNAL( signalEtcFileMerged() ), this, SLOT( slotReload() ) );
	connect( mergeView, SIGNAL( executed( QListViewItem* ) ), this, SLOT( slotViewFile( QListViewItem* ) ) );
	
	slotInit();
}

/**
 * Save splitters and listview geometry.
 */
MergeTab::~MergeTab()
{
}

/**
 * Initialize geometry and content.
 */
void MergeTab::slotInit()
{
	slotReload();
}

/**
 * Reload history view.
 */
void MergeTab::slotReload()
{
	mergeView->loadFromDB();
	emit signalMergeChanged();
}

void MergeTab::slotClearFilter()
{
	mergeFilter->clear();
}

/**
 * Open in external browser.
 */
void MergeTab::slotViewFile( QListViewItem *item )
{
	if ( item->parent() ) {
		QString source = KUROODIR + "backup/" + dynamic_cast<MergeListView::MergeItem*>( item )->source();
		QString destination = KUROODIR + "backup/" + dynamic_cast<MergeListView::MergeItem*>( item )->destination();

		KProcIO* eProc = new KProcIO();
		*eProc << KurooConfig::etcUpdateTool() << source << destination;
		connect( eProc, SIGNAL( processExited( KProcess* ) ), this, SLOT( slotCleanupOpenDiff( KProcess* ) ) );
		eProc->start( KProcess::NotifyOnExit, true );
	}
}

void MergeTab::slotCleanupOpenDiff( KProcess* eProc )
{
	delete eProc;
	eProc = 0;
}

#include "mergetab.moc"
