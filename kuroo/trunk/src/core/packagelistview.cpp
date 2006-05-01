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

#include "common.h"
#include "packagelistview.h"
#include "packageitem.h"
#include "tooltip.h"

#include <qlistview.h>
#include <qheader.h>

/**
 * @class PackageListView
 * @short Base class for listViews containing packages.
 */
PackageListView::PackageListView( QWidget* parent, const char* name )
	: KListView( parent, name ), lastItem( 0 )
{
	setFrameShape( QFrame::NoFrame );
	setSelectionModeExt( FileManager );
	header()->setStretchEnabled( false );
	
	// Update visible items when world is changed
	connect( PortageSingleton::Instance(), SIGNAL( signalWorldChanged() ), this, SLOT( triggerUpdate() ) );
	
// 	connect( this, SIGNAL( onItem( QListViewItem* ) ), this, SLOT( rollOver( QListViewItem* ) ) );
	
	new ToolTip( this );
}

PackageListView::~PackageListView()
{}

/**
 * Create mouse-over effect.
 */
void PackageListView::rollOver( QListViewItem* item )
{
	dynamic_cast<PackageItem*>( item )->setRollOver( true );
	if ( lastItem )
		dynamic_cast<PackageItem*>( lastItem )->setRollOver( false );
	lastItem = item;
}

/**
 * Clear this listView and package index.
 */
void PackageListView::resetListView()
{
	clear();
	m_packageIndex.clear();
	lastItem = 0;
}

/**
 * Current package status.
 * @return status
 */
int PackageListView::currentItemStatus()
{
	return currentPackage()->status();
}

/**
 * Return PackageItem from listview index.
 * @param id
 */
PackageItem* PackageListView::packageItemById( const QString& id )
{
	if ( id.isEmpty() || !m_packageIndex[id] )
		return NULL;
	else
		return m_packageIndex[id];
}

/**
 * Current package id.
 * @return id
 */
const QString PackageListView::currentId()
{
	PackageItem* item = currentPackage();
	if ( item )
		return item->id();
	else
		return QString::null;
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
const QStringList PackageListView::selectedId()
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
const QStringList PackageListView::selectedPackages()
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
const QStringList PackageListView::allId()
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
const QStringList PackageListView::allPackages()
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
 * Set focus in listview on this package.
 * @param id
 */
void PackageListView::setPackageFocus( const QString& id )
{
	if ( id.isEmpty() || !m_packageIndex[id] ) {
		setCurrentItem( firstChild() );
		setSelected( firstChild(), true );
	}
	else {
		PackageItem* item = m_packageIndex[id];
		setCurrentItem( item );
		setSelected( item, true );
		ensureItemVisible( item );
	}
}

/**
 * Register package in index and check if in the queue.
 * @param id
 * @param item
 */
void PackageListView::indexPackage( const QString& id, PackageItem *item )
{
	if ( id.isEmpty() )
		return;
	
	m_packageIndex.insert( id, item );
	m_packageIndex[id]->setPackageIndex( m_packageIndex.count() );
}

/**
 * Total number of packages in listview.
 * @return QString
 */
const QString PackageListView::count()
{
	return QString::number( m_packageIndex.count() );
}

/**
 * Move to next package in listview.
 * @param isUp true is previous, false is next
 */
void PackageListView::nextPackage( bool isPrevious )
{
	if ( isVisible() ) {
		QListViewItem* item = currentItem();
		if ( isPrevious ) {
			if ( item->itemAbove() ) {
				item = item->itemAbove();
				ensureItemVisible( item );
				setCurrentItem( item );
				selectAll( false );
				item->setSelected( true );
			}
		}
		else {
			if ( item->itemBelow() ) {
				item = item->itemBelow();
				ensureItemVisible( item );
				setCurrentItem( item );
				selectAll( false );
				item->setSelected( true );
			}
		}
	}
}

#include "packagelistview.moc"
