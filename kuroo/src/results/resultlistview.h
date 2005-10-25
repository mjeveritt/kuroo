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

#ifndef RESULTLISTVIEW_H
#define RESULTLISTVIEW_H

#include "packagelistview.h"

#include <klistview.h>

#include <qpixmap.h>

class QPixmap;
class Package;
class QRegExp;
class PackageItem;

extern QRegExp pv;

/**
 * @class ResultListView
 * @short Specialized listview for results packages.
 */
class ResultListView : public PackageListView
{
Q_OBJECT
public:
	ResultListView( QWidget *parent = 0, const char *name = 0 );
	~ResultListView();
	
public slots:
	
	/** 
	 * Populate listview with content of table resultPackages 
	 */
	void 									loadFromDB();
	
signals:
	void									signalResultsLoaded();
	
private:
	QPixmap 								pxQueuedColumn;
	struct TreeViewCategory {
		QListViewItem * item;
		QMap<QString, QListViewItem *> 		packages;
	};
	QMap<QString, TreeViewCategory> 		categories;
	
};

#endif
