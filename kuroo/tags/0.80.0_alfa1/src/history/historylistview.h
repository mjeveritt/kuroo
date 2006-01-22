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

#ifndef HISTORYLISTVIEW_H
#define HISTORYLISTVIEW_H

#include <klistview.h>

class QTime;
class Package;
class PackageEmergeTime;
class KProcIO;
class KListViewItem;

typedef QMap< QString, QListViewItem* > ItemMap;
typedef QMap< QString, Package > PackageMap;
typedef QMap< QString, PackageEmergeTime > EmergeTimeMap;

/**
 * @class HistoryListView
 * @short Specialized listview for emerge history.
 */
class HistoryListView : public KListView
{
Q_OBJECT
public:
	HistoryListView( QWidget *parent = 0, const char *name = 0 );
	~HistoryListView();
	
	QString 		current();
	QStringList 	selected();
	void 			loadFromDB();
	
private:
	KLocale 		*loc;
	ItemMap			itemMap;
	
signals:
	void    	signalHistoryLoaded();
};

#endif
