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

#ifndef UPDATEPACKAGESLISTVIEW_H
#define UPDATEPACKAGESLISTVIEW_H

#include "packagelistview.h"

#include <klistview.h>

#include <qpixmap.h>

class QPixmap;
class QRegExp;
class PackageItem;

extern QRegExp pv;

/**
 * @class UpdatePackagesListView
 * @short Specialized listview update packages sorted by category.
 */
class UpdatePackagesListView : public PackageListView
{
Q_OBJECT
public:
	UpdatePackagesListView( QWidget *parent = 0, const char *name = 0 );
	~UpdatePackagesListView();
	
public slots:
	void 			addCategoryPackages( const QString& category );
	
private:
	QPixmap 		pxQueuedColumn;
};

#endif
