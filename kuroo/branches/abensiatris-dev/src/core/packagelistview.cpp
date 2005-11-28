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

/**
 * Base class for listViews containing packages.
 */
PackageListView::PackageListView( QWidget* parent, const char* name )
	: KListView( parent, name )
{
	connect( SignalistSingleton::Instance(), SIGNAL( signalSetQueued(const QString&, bool) ), this, SLOT( setQueued(const QString&, bool) ) );
	connect( SignalistSingleton::Instance(), SIGNAL( signalClearQueued() ), this, SLOT( slotClearQueued() ) );
	
	connect( SignalistSingleton::Instance(), SIGNAL( signalUnmasked(const QString&, bool) ), this, SLOT( setUnmasked(const QString&, bool) ) );
	connect( SignalistSingleton::Instance(), SIGNAL( signalInWorld(const QString&, bool) ), this, SLOT( setInWorld(const QString&, bool) ) );
	
	setSelectionModeExt(FileManager);
	
	new ToolTip(this);
}

PackageListView::~PackageListView()
{
}

/**
 * Clear this listView and packages.
 */
void PackageListView::reset()
{
	clear();
	packages.clear();
}

/**
 * Current package idDB.
 * @return idDB
 */
QString PackageListView::currentId()
{
	if ( !packages.isEmpty() ) {
		for ( QDictIterator<PackageItem> it(packages); it.current(); ++it ) {
			if ( it.current()->text(0) == this->currentItem()->text(0) )
				return it.currentKey();
		}
	}
	else
		return i18n("na");
}

/**
 * Get current package.
 * @return package name
 */
QString PackageListView::currentPackage()
{
	QListViewItem *item = this->currentItem();
	
	if ( item )
		return item->text(0);
	else
		return i18n("na");
}

/**
 * Get selected packages DB id.
 * @return idList
 */
QStringList PackageListView::selectedId()
{
	QStringList idDBList;
	for ( QDictIterator<PackageItem> it(packages); it.current(); ++it ) {
		if ( it.current()->parent() ) {
			if ( it.current()->isSelected() && it.current()->isVisible() )
				idDBList += it.currentKey();
		}
		else {
			if ( it.current()->isSelected() && it.current()->isVisible() )
				idDBList += it.currentKey();
		}
	}
	return idDBList;
}

/**
 * Get selected packages.
 * @return packageList
 */
QStringList PackageListView::selectedPackages()
{
	QStringList packageList;
	for ( QDictIterator<PackageItem> it(packages); it.current(); ++it ) {
		if ( it.current()->isSelected() && it.current()->isVisible() )
			packageList += it.current()->text(0);
	}
	return packageList;
}

/** 
 * Return all packages DB id in listview.
 * @return packageList
 */
QStringList PackageListView::allId()
{
	QStringList idDBList;
	for ( QDictIterator<PackageItem> it(packages); it.current(); ++it )
		idDBList += it.currentKey();
	
	return idDBList;
}

/** 
 * Return all packages in listview.
 * @return list of packages by name
 */
QStringList PackageListView::allPackages()
{
	QStringList idDBList;
	for ( QDictIterator<PackageItem> it(packages); it.current(); ++it )
		idDBList += it.current()->text(0);
	
	return idDBList;
}

/**
 * Count packages in this category.
 * @return QString
 */
QString PackageListView::count()
{
	return QString::number( packages.count() );
}

/**
 * Clear the queued hightlighting.
 */
void PackageListView::slotClearQueued()
{
	QString name = this->name();
	for ( QDictIterator<PackageItem> it(packages); it.current(); ++it ) {
		packages[it.currentKey()]->setStatus(NOTQUEUED);
	}
}

/**
 * Fast method for marking packages as queued.
 * @param idDB
 * @param true/false
 */
void PackageListView::setQueued( const QString& idDB, bool b )
{
	if ( packages[idDB] ) {
		if ( b )
			packages[idDB]->setStatus(QUEUED);
		else
			packages[idDB]->setStatus(NOTQUEUED);
	}
}

/**
 * Register package and check weither it is in the queue.
 * @param item
 * @param idDB
 */
void PackageListView::insertPackage( const QString& idDB, PackageItem *item )
{
	packages.insert( idDB, item );
	
	if ( QueueSingleton::Instance()->isQueued(idDB) )
		packages[idDB]->setStatus(QUEUED);
	else
		packages[idDB]->setStatus(NOTQUEUED);
}

/**
 * Fast method for marking packages as unmasked.
 * @param idDB
 * @param true/false
 */
void PackageListView::setUnmasked( const QString& name, bool b )
{
	kdDebug() << "PackageListView::setUnmasked name=" << name << endl;
	
	PackageItem *myChild = dynamic_cast<PackageItem*>( this->firstChild() );
	while ( myChild ) {
		if ( myChild->text(0) == name ) {
			if ( b )
				myChild->setStatus(UNMASKED);
			else
				myChild->setStatus(NONE);
			break;
		}
		myChild = dynamic_cast<PackageItem*>( myChild->nextSibling() );
	}
}

/**
 * Fast method for marking packages in world.
 * @param idDB
 * @param true/false
 */
void PackageListView::setInWorld( const QString& name, bool b )
{
	kdDebug() << "PackageListView::setInWorld name=" << name << " b=" << b << endl;
	
	PackageItem *myChild = dynamic_cast<PackageItem*>( this->firstChild() );
	while ( myChild ) {
		if ( myChild->text(0).section(pv, 0, 0) == name ) {
			if ( b )
				myChild->setStatus(INSTALLED_WORLD);
			else
				myChild->setStatus(INSTALLED);
			break;
		}
		myChild = dynamic_cast<PackageItem*>( myChild->nextSibling() );
	}
}

#include "packagelistview.moc"
