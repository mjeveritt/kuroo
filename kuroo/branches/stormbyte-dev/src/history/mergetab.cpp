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
#include <qheader.h>
#include <qwhatsthis.h>

#include <ktextbrowser.h>
#include <kmessagebox.h>
#include <klistviewsearchline.h>
#include <kiconloader.h>
#include <kpushbutton.h>

/**
 * @class MergeTab
 * @short Tabpage for emerge log browser.
 */
MergeTab::MergeTab( QWidget* parent )
	: MergeBase( parent )
{
	// Connect What's this button
	connect( pbWhatsThis, SIGNAL( clicked() ), this, SLOT( slotWhatsThis() ) );
	
	pbClearFilter->setIconSet( SmallIconSet("locationbar_erase") );
	
	mergeFilter->setListView( mergeView );

	connect( EtcUpdateSingleton::Instance(), SIGNAL( signalScanCompleted() ), this, SLOT( slotLoadConfFiles() ) );
	connect( EtcUpdateSingleton::Instance(), SIGNAL( signalEtcFileMerged() ), this, SLOT( slotReload() ) );
	
	// When all packages are emerged...
	connect( EmergeSingleton::Instance(), SIGNAL( signalEmergeComplete() ), this, SLOT( slotReload() ) );
		
	connect( unmergeView, SIGNAL( selectionChanged() ), this, SLOT( slotButtonMerge() ) );
	connect( mergeView, SIGNAL( selectionChanged() ), this, SLOT( slotButtonView() ) );
	
	connect( pbMerge, SIGNAL( clicked() ), this, SLOT( slotMergeFile() ) );
	connect( pbView, SIGNAL( clicked() ), this, SLOT( slotViewFile() ) );
	
	slotInit();
}

/**
 * Save splitters and listview geometry.
 */
MergeTab::~MergeTab()
{}

/**
 * Initialize geometry and content.
 */
void MergeTab::slotInit()
{
	pbWhatsThis->setIconSet( SmallIconSet("info") );
	unmergeView->header()->setLabel( 0, i18n("New Configuration file") );
	mergeView->header()->setLabel( 0, i18n("Merged Configuration file") );
	pbMerge->setDisabled( true );
	pbView->setDisabled( true );
	slotReload();
}

/**
 * What's this info explaning this tabs functionality.
 */
void MergeTab::slotWhatsThis()
{
	QWhatsThis::display( i18n( "<qt>"
			"This tab keeps track of all configuration files that need to be merged.<br>"
			"Your system is scanned automatically for configuration files after completed installation.<br>"
			"Select a file to merge and press 'Merge changes'. KDiff3 will then open with old and new files. "
			"After saving changes in KDiff3 the configuration file will be removed. "
			"Old merged changes can then be reviewed in the right list.</qt>" )
			, QCursor::pos(), this );
}

/**
 * Reload history view.
 */
void MergeTab::slotReload()
{
	DEBUG_LINE_INFO;
	EtcUpdateSingleton::Instance()->slotEtcUpdate();
}

/**
 * List new configuration files in mergeView.
 */
void MergeTab::slotLoadConfFiles()
{
	DEBUG_LINE_INFO;
	QStringList confFilesList = EtcUpdateSingleton::Instance()->confFilesList();
	unmergeView->loadConfFiles( confFilesList );
	
	QStringList backupFilesList = EtcUpdateSingleton::Instance()->backupFilesList();
	mergeView->loadBackupFiles( backupFilesList );
	
	emit signalMergeChanged();
}

void MergeTab::slotClearFilter()
{
	mergeFilter->clear();
}

/**
 * Activate buttons only when file is selected.
 */
void MergeTab::slotButtonView()
{
	QListViewItem *item = mergeView->currentItem();
	if ( item && item->parent() ) {
		unmergeView->clearSelection();
		pbView->setDisabled( false );
		pbMerge->setDisabled( true );
	}
	else {
		pbMerge->setDisabled( true );
		pbView->setDisabled( true );
	}
}

/**
 * Activate buttons only when file is selected.
 */
void MergeTab::slotButtonMerge()
{
	QListViewItem *item = unmergeView->currentItem();
	if ( item ) {
		mergeView->clearSelection();
		pbMerge->setDisabled( false );
		pbView->setDisabled( true );
	}
	else {
		pbMerge->setDisabled( true );
		pbView->setDisabled( true );
	}
}

/**
 * View merged changes in diff tool.
 */
void MergeTab::slotViewFile()
{
	QListViewItem *item = mergeView->currentItem();
	if ( !item || !item->parent() )
		return;
	
	QString source = dynamic_cast<MergeListView::MergeItem*>( item )->source();
	QString destination = dynamic_cast<MergeListView::MergeItem*>( item )->destination();
	
	EtcUpdateSingleton::Instance()->runDiff( source, destination, false );
}

/**
 * Launch diff tool to merge changes.
 */
void MergeTab::slotMergeFile()
{
	QListViewItem *item = unmergeView->currentItem();
	if ( !item && item->parent() )
		return;
	
	QString source = dynamic_cast<MergeListView::MergeItem*>( item )->source();
	QString destination = dynamic_cast<MergeListView::MergeItem*>( item )->destination();
	
	EtcUpdateSingleton::Instance()->runDiff( source, destination, true );
}

#include "mergetab.moc"
