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

#ifndef PORTAGELISTVIEW_H
#define PORTAGELISTVIEW_H

#include <qpixmap.h>

#include "packagelistview.h"

class Package;
class QRegExp;

extern QRegExp pv;

/**
 * @class PortageListView
 * @short Installed packages listview.
 */
class PortageListView : public PackageListView
{
Q_OBJECT
public:
	PortageListView( QWidget *parent = 0, const char *name = 0 );
	~PortageListView();
	
public slots:
	
	/**
	 * Mark package as current.
	 * @param package
	 */
// 	void 			setCurrentPackage( const QString& package );
	
	/**
	 * Populate listview with content of this category..
	 * @param package
	 */
	void 			addCategoryPackages( const QString& category, int filter );
	void 			addSubCategoryPackages( const QString& category, const QString& subCategory, int filter );
	
private:
	QStringList		unmaskedList;
	QPixmap 		pxQueuedColumn;
};

#endif
