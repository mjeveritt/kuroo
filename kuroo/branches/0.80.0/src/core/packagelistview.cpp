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
// 	connect( SignalistSingleton::Instance(), SIGNAL( signalUnmasked(const QString&, bool) ), this, SLOT( slotSetUnmasked(const QString&, bool) ) ); @fixme

// 	new ToolTip( this ); @fixme
}

PackageListView::~PackageListView()
{
}

/**
 * Clear this listView and package index.
 */
void PackageListView::resetListView()
{
	clear();
	packageIndex.clear();
}

/**
 * Current package status.
 * @return status
 */
QString PackageListView::currentItemStatus()
{
	return currentPackage()->status();
}

PackageItem* PackageListView::itemId( const QString& id )
{
	if ( id.isEmpty() || !packageIndex[id] )
		return NULL;
	else
		return packageIndex[id];
}

/**
 * Current package id.
 * @return id
 */
QString PackageListView::currentId()
{
	PackageItem* item = currentPackage();
	if ( item )
		return item->id();
	else
		return NULL;
}

/**
 * The current package.
 * @return PackageItem*
 */
PackageItem* PackageListView::currentPackage()
{
	return dynamic_cast<PackageItem*>( this->currentItem() );
}

/**
 * All selected packages by id.
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
 * All selected packages by name.
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
 * All packages in listview by id.
 * @return idList
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
 * All packages in listview by name.
 * @return packageList
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
 * All packages in listview by name - no children
 * @return packageList
 */
QStringList PackageListView::allPackagesNoChildren()
{
	QStringList packageList;
	QListViewItem* myChild = firstChild();
	while ( myChild ) {
		packageList += dynamic_cast<PackageItem*>( myChild )->name();
		myChild = myChild->nextSibling();
	}
	return packageList;
}

/**
 * Total number of packages in listview.
 * @return QString
 */
QString PackageListView::count()
{
	return QString::number( packageIndex.count() );
}

/**
 * Set focus in listview on this package.
 * @param id
 */
void PackageListView::setPackageFocus( const QString& id )
{
	if ( id.isEmpty() || !packageIndex[id] ) {
		setCurrentItem( firstChild() );
		setSelected( firstChild(), true );
	}
	else {
		PackageItem* item = packageIndex[id];
		setCurrentItem( item );
		setSelected( item, true );
		ensureItemVisible( item );
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
 * Mark packages as queued.
 * @param idDB
 * @param true/false
 */
void PackageListView::slotSetQueued( const QString& id, bool isQueued )
{
	if ( id.isEmpty() || !packageIndex[id] )
		return;
	
	if ( isQueued )
		packageIndex[id]->setStatus( QUEUED );
	else
		packageIndex[id]->setStatus( NOTQUEUED );

}

/**
 * Register package in index and check weither it is in the queue.
 * @param id
 * @param item
 */
void PackageListView::indexPackage( const QString& id, PackageItem *item )
{
	if ( id.isEmpty() )
		return;
	
	packageIndex.insert( id, item );
	
	if ( QueueSingleton::Instance()->isQueued( id ) )
		packageIndex[id]->setStatus( QUEUED );
	else
		packageIndex[id]->setStatus( NOTQUEUED );
}

/**
 * Mark package as unmasked.
 * @param id
 * @param true/false
 */
void PackageListView::slotSetUnmasked( const QString& id, bool isUnmasked )
{
	if ( id.isEmpty() || !packageIndex[id] )
		return;

	if ( isUnmasked )
		packageIndex[id]->setStatus( UNTESTING );
	else
		packageIndex[id]->setStatus( NONE );

}

#include "packagelistview.moc"
