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

#ifndef UPDATELISTVIEW_H
#define UPDATELISTVIEW_H

#include "packagelistview.h"

#include <qpixmap.h>
#include <klistview.h>

class QPixmap;
class Package;
class QRegExp;
class PackageItem;

extern QRegExp pv;

/**
 * @class UpdateListView
 * @short Specialized listview for alfabetical list of update packages.
 */
class UpdateListView : public PackageListView
{
Q_OBJECT
public:
	UpdateListView( QWidget *parent = 0, const char *name = 0 );
	~UpdateListView();
	
public slots:
	void 			loadFromDB();
	
private:
	QPixmap 		pxQueuedColumn;
	
signals:
	void	signalUpdatesLoaded();
};

#endif
