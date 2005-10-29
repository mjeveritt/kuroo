/**************************************************************************
*   Copyright (C) 2004 by karye                                           *
*   karye@users.sourceforge.net                                           *
*   Copyright (C) 2005 by Jakob Petsovits                                 *
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

#ifndef PORTAGELISTVIEW_H
#define PORTAGELISTVIEW_H

#include "packagelistview.h"

#include <qpixmap.h>

class QPixmap;
class Package;
class QRegExp;
class PackageItem;

extern QRegExp pv;

/**
 * @class PortageListView
 * @short Specialized listview for viewing all portage packages in selected category.
 */
class PortageListView : public PackageListView
{
Q_OBJECT
public:
	PortageListView( QWidget *parent = 0, const char *name = 0 );
	~PortageListView();
	
public slots:
	
	/**
	* Current package id.
	* If Package is selected return ebuild id.
	* @param id
	*/
	QString 							currentId();
	
	/**
	* Mark package as selected in view
	* @param package	
	*/
	QStringList 						selectedPackages();
	
	/**
	* Get selected packages.
	* @param packageList
	*/
	void 								setCurrentPackage( const QString& package );
	
	/**
	* Populate listview with packages from selected category.
	* Jakob Petsovits technique for fast item loading.
	* @param category package clicked on in categories listview.
	*/
	void 								addCategoryPackages( const QString& category );
	
private:
	QPixmap 							pxQueuedColumn;
	struct TreeViewPackage {
		PackageItem* item;
		QMap< QString, PackageItem* > 	versionItems;
	};
	QMap< QString, TreeViewPackage > 	packageItems;
};

#endif
