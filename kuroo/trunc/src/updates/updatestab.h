/***************************************************************************
 *   Copyright (C) 2005 by Karye   *
 *   karye@users.sourceforge.net   *
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

#ifndef UPDATESTAB_H
#define UPDATESTAB_H

#include "updatesbase.h"

#include <qwidget.h>

class CategoriesListView;
class UpdatePackagesListView;
class UpdateListView;
class KListView;
class QListViewItem;

/**
 * @class UpdatesTab
 * @short Tab page for update packages.
 */
class UpdatesTab : public UpdatesBase
{
Q_OBJECT
public:
    UpdatesTab( QWidget *parent = 0 );
	
	/**
	 * Save splitters and listview geometry.
	 */
    ~UpdatesTab();

private slots:
	
	/**
	 * Initialize geometry and content.
	 * Restore geometry: splitter positions, listViews width and columns width.
	 * Restore latest user view.
	 */
	void	slotInit();
	
	/**
	 * Save latest selected packages in tabs All packages, Installed packages and Updates categories.
	 */
	void	saveCurrentView();
	
	/**
	 * Disable/enable buttons when kuroo is busy.
	 * @param b
	 */
	void	slotBusy( bool b );
	void	slotListPackages();
	
	/**
	 * Launch emerge of all update packages.
	 */
	void	slotEmergeQueue();
	
	/**
	 * Popup menu for actions like emerge.
	 * @param listView
	 * @param item
	 * @param point
	 */
	void	contextMenu( KListView *listView, QListViewItem *item, const QPoint &point );
	
public slots:
	
	/**
	 * Reload update packages list.
	 */
	void	slotReload();
	
	/**
	 * Refresh update packages list.
	 */
	void	slotRefresh();
	
signals:
	void	signalChanged();
};

#endif
