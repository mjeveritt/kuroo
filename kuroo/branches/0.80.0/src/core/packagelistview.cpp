/***************************************************************************
*   Copyright (C) 2004 by karye                                           *
*   karye@users.sourceforge.net                                           *
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

#include <qlistview.h>

#include "common.h"
#include "packagelistview.h"
#include "packageitem.h"
#include "tooltip.h"

/**
 * Base class for listViews containing packages.
 */
PackageListView::PackageListView( QWidget* parent, const char* name )
	: KListView( parent, name )
{
	setFrameShape( QFrame::NoFrame );
	setSelectionModeExt( FileManager );
	
	connect( SignalistSingleton::Instance(), SIGNAL( signalSetQueued(const QString&, bool) ), this, SLOT( slotSetQueued(const QString&, bool) ) );
	connect( SignalistSingleton::Instance(), SIGNAL( signalClearQueued() ), this, SLOT( slotClearQueued() ) );
// 	connect( SignalistSingleton::Instance(), SIGNAL( signalUnmasked(const QString&, bool) ), this, SLOT( slotSetUnmasked(const QString&, bool) ) );

	new ToolTip( this );
}

PackageListView::~PackageListView()
{
}

/**
 * Clear this listView and packages.
 */
void PackageListView::resetListView()
{
	clear();
	packageIndex.clear();
}

QString PackageListView::currentItemStatus()
{
	return dynamic_cast<PackageItem*>( this->currentItem() )->status();
}

/**
 * Current package idDB.
 * @return idDB
 */
QString PackageListView::currentId()
{
	PackageItem* item = dynamic_cast<PackageItem*>( this->currentItem() );
	if ( item )
		return item->id();
	else
		return NULL;
}

/**
 * Get current package.
 */
PackageItem* PackageListView::currentPackage()
{
	return dynamic_cast<PackageItem*>( this->currentItem() );
}

/**
 * Get selected packages DB id. @fixme: use QListViewItemIterator instead.
 * @return idList
 */
QStringList PackageListView::selectedId()
{
	QStringList idList;
	QListViewItemIterator it( this );
	while ( it.current() ) {
		QListViewItem *item = it.current();
		if ( item->isSelected() )
			idList += dynamic_cast<PackageItem*>( item )->id();
		++it;
	}
	return idList;
}

/**
 * Get selected packages.
 * @return packageList
 */
QStringList PackageListView::selectedPackages()
{
	QStringList packageList;
	QListViewItemIterator it( this );
	while ( it.current() ) {
		QListViewItem *item = it.current();
		if ( item->isSelected() )
			packageList += dynamic_cast<PackageItem*>( item )->name();
		++it;
	}
	return packageList;
}

/** 
 * Return all packages DB id in listview.
 * @return packageList
 */
QStringList PackageListView::allId()
{
	QStringList idList;
	QListViewItemIterator it( this );
	while ( it.current() ) {
		idList += dynamic_cast<PackageItem*>( it.current() )->id();
		++it;
	}
	return idList;
}

/** 
 * Return all packages in listview.
 * @return list of packages by name
 */
QStringList PackageListView::allPackages()
{
	QStringList packageList;
	QListViewItemIterator it( this );
	while ( it.current() ) {
		packageList += dynamic_cast<PackageItem*>( it.current() )->name();
		++it;
	}
	return packageList;
}

/**
 * Count packages in this category.
 * @return QString
 */
QString PackageListView::count()
{
	return QString::number( packageIndex.count() );
}

void PackageListView::setPackageFocus( const QString& id )
{
	if ( !id.isEmpty() && packageIndex[id] ) {
		PackageItem* item = packageIndex[id];
		setCurrentItem( item );
		setSelected( item, true );
		ensureItemVisible( item );
	}
	else {
		setCurrentItem( firstChild() );
		setSelected( firstChild(), true );
	}
}

/**
 * Clear the queued hightlighting.
 */
void PackageListView::slotClearQueued()
{
	QListViewItemIterator it( this );
	while ( it.current() ) {
		dynamic_cast<PackageItem*>( it.current() )->setStatus( NOTQUEUED );
		++it;
	}
}

/**
 * Fast method for marking packages as queued.
 * @param idDB
 * @param true/false
 */
void PackageListView::slotSetQueued( const QString& id, bool isQueued )
{
	if ( id.isEmpty() ) {
		kdDebug() << i18n("Package id is empty, skipping!") << endl;
		return;
	}
	if ( packageIndex[id] ) {
		if ( isQueued )
			packageIndex[id]->setStatus( QUEUED );
		else
			packageIndex[id]->setStatus( NOTQUEUED );
	}
}

/**
 * Register package and check weither it is in the queue.
 * @param item
 * @param idDB
 */
void PackageListView::indexPackage( const QString& id, PackageItem *item )
{
	if ( id.isEmpty() ) {
		kdDebug() << i18n("Package id is empty, skipping!") << endl;
		return;
	}
	
	packageIndex.insert( id, item );
	
	if ( QueueSingleton::Instance()->isQueued( id ) )
		packageIndex[id]->setStatus( QUEUED );
	else
		packageIndex[id]->setStatus( NOTQUEUED );
}

/**
 * Fast method for marking packages as unmasked.
 * @param idDB
 * @param true/false
 */
void PackageListView::slotSetUnmasked( const QString& id, bool isUnmasked )
{
	if ( id.isEmpty() ) {
		kdDebug() << i18n("Package id is empty, skipping!") << endl;
		return;
	}
	
	if ( packageIndex[id] ) {
		if ( isUnmasked )
			packageIndex[id]->setStatus( UNMASKED );
		else
			packageIndex[id]->setStatus( NONE );
	}
}

#include "packagelistview.moc"
