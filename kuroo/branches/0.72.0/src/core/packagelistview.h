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

#ifndef PACKAGELISTVIEW_H
#define PACKAGELISTVIEW_H

#include <klistview.h>

#include <qdict.h>

class PackageItem;

/**
 * @class PackageListView
 * @short Base class for packages listviews.
 */
class PackageListView : public KListView
{
Q_OBJECT
public:
    PackageListView( QWidget *parent = 0, const char *name = 0 );
    ~PackageListView();

public:
	virtual	void			reset();
	
	/**
 	 * Current package idDB.
 	 * @return idDB
 	 */
	virtual QString			currentId();
	
	/**
 	 * Get current package.
  	 * @return package name
 	 */
	virtual QString 		currentPackage();
	
	/**
 	 * Get selected packages DB id.
 	 * @return idList
 	 */
	virtual QStringList		selectedId();
	
	/**
 	 * Get selected packages by name as "metalog-0.10.0" or "metalog".
 	 * @return packageList
 	 */
	virtual QStringList		selectedPackages();
	
	/** 
  	 * Return all packages by DB id in listview.
 	 * @return packageList
 	 */
	virtual QStringList		allId();
	
	/** 
 	 * Return all packages in listview.
 	 * @return list of packages by name
 	 */
	virtual QStringList		allPackages();
	
	/**
 	 * Count packages.
 	 * @return QString
 	 */
	virtual QString			count();
	
public slots:
	
	/**
 	 * Clear the queued hightlighting.
 	 */
	virtual void			slotClearQueued();
	
protected slots:
	
	/**
 	 * Fast method for marking packages as queued.
 	 * @param idDB
 	 * @param true/false
 	 */
	virtual void 			setQueued( const QString& idDB, bool b );
	
	/**
 	 * Insert package and check if in the queue.
 	 * @param item
 	 * @param idDB
 	 */
	virtual void 			insertPackage( const QString& idDB, PackageItem *item );
	
	/**
	* Fast method for marking packages as unmasked.
	* @param idDB
	* @param true/false
	*/
	virtual void 			setUnmasked( const QString& name, bool b );
	
protected:
	
	/**
	 * Internal index of packages in listView.
	 */
	QDict<PackageItem>		packages;
	
};

#endif
