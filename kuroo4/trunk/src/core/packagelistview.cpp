/***************************************************************************
 *	Copyright (C) 2004 by karye												*
 *	karye@users.sourceforge.net												*
 *																			*
 *	This program is free software; you can redistribute it and/or modify	*
 *	it under the terms of the GNU General Public License as published by	*
 *	the Free Software Foundation; either version 2 of the License, or		*
 *	(at your option) any later version.										*
 *																			*
 *	This program is distributed in the hope that it will be useful,			*
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of			*
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the			*
 *	GNU General Public License for more details.							*
 *																			*
 *	You should have received a copy of the GNU General Public License		*
 *	along with this program; if not, write to the							*
 *	Free Software Foundation, Inc.,											*
 *	59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.				*
 ***************************************************************************/

#include <QTreeWidget>

#include "common.h"
#include "packagelistview.h"
#include "packageitem.h"
#include "tooltip.h"

/**
 * @class PackageListView
 * @short Base class for listViews containing packages.
 */
PackageListView::PackageListView( QWidget* parent, const char* name )
	: QTreeWidget( parent ), lastItem( 0 )
{
	setWindowIconText( name );
	setFrameShape( QFrame::NoFrame );
	setSelectionMode( ExtendedSelection );
	header()->setResizeMode( QHeaderView::Interactive );

	// Update visible items when world is changed
	//connect( PortageSingleton::Instance(), SIGNAL( signalWorldChanged() ), this, SLOT( undefined triggerUpdate() ) );

	connect( this, SIGNAL( onItem( QTreeWidgetItem* ) ), this, SLOT( rollOver( QTreeWidgetItem* ) ) );

	//new ToolTip( this );
}

PackageListView::~PackageListView()
{}

/**
 * Create mouse-over effect.
 */
void PackageListView::rollOver( QTreeWidgetItem* item )
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
int PackageListView::currentItemStatus() const  { return currentPackage()->status(); }

/**
 * The current package.
 * @return PackageItem*
 */
PackageItem* PackageListView::currentPackage() const  { return dynamic_cast<PackageItem*>( this->currentItem() ); }

/**
 * Return PackageItem from listview index.
 * @param id
 */
PackageItem* PackageListView::packageItemById( const QString& id ) const
{
	if ( id.isEmpty() || !m_packageIndex.contains( id ) ) {
		return NULL;
	} else {
		return m_packageIndex[id];
	}
}

/**
 * Current package id.
 * @return id
 */
const QString PackageListView::currentId() const
{
	PackageItem* item = currentPackage();
	if ( item )
		return item->id();
	else
		return QString::null;
}

/**
 * All selected packages by id.
 * @return idList
 */
const QStringList PackageListView::selectedIds() const
{
	QStringList idList;
	foreach( QTreeWidgetItem* item, selectedItems() ) {
		idList += dynamic_cast<PackageItem*>(item)->id();
	}
	return idList;
}

/**
 * All selected packages by name.
 * @return packageList
 */
const QStringList PackageListView::selectedPackages() const
{
	QStringList packageList;
	foreach( QTreeWidgetItem* item, selectedItems() ) {
		packageList += dynamic_cast<PackageItem*>(item)->name();
	}
	return packageList;
}

/**
 * All packages in listview by id.
 * @return idList
 */
const QStringList PackageListView::allId() const
{
	QStringList idList;
	for( int i = 0; i < topLevelItemCount(); i++ ) {
		idList += dynamic_cast<PackageItem*>(topLevelItem( i ))->id();
	}
	return idList;
}

/**
 * All packages in listview by name.
 * @return packageList
 */
const QStringList PackageListView::allPackages() const
{
	QStringList packageList;
	for( int i = 0; i < topLevelItemCount(); i++ ) {
		packageList += dynamic_cast<PackageItem*>(topLevelItem( i ))->name();
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
		setCurrentItem( topLevelItem(0) );
		if( NULL != currentItem() )
			currentItem()->setSelected( true );
	}
	else {
		PackageItem* item = m_packageIndex[id];
		setCurrentItem( item );
		item->setSelected( true );
		//ensureItemVisible( item );
	}
}

/**
 * Register package in index and check if in the queue.
 * @param id
 * @param item
 */
void PackageListView::indexPackage( const QString& id, PackageItem *item )
{
	if ( !id.isEmpty() ) {
		m_packageIndex.insert( id, item );
		m_packageIndex[id]->setPackageIndex( m_packageIndex.count() );
	}
}


/**
 * Move to next package in listview.
 * @param isUp true is previous, false is next
 */
void PackageListView::nextPackage( const bool& isPrevious )
{
	if ( isVisible() ) {
		QTreeWidgetItem* item = currentItem();
		if ( isPrevious ) {
			if ( itemAbove(item) ) {
				item = itemAbove(item);
				//ensureItemVisible( item ); //scrollTo
				setCurrentItem( item );
				for( int i=0; i<topLevelItemCount(); i++ ) {
					topLevelItem(i)->setSelected(false);
				}
				item->setSelected( true );
			}
		}
		else {
			if( itemBelow( item ) ) {
				item = itemBelow( item );
				//ensureItemVisible( item ); //scrollTo
				setCurrentItem( item );
				for( int i=0; i<topLevelItemCount(); i++ ) {
					topLevelItem(i)->setSelected(false);
				}
				item->setSelected( true );
			}
		}
	}
}

#include "packagelistview.moc"
