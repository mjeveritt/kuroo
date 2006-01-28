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

#ifndef PORTAGETAB_H
#define PORTAGETAB_H

#include <qobject.h>

#include "portagebase.h"
#include "scanportagejob.h"

class CategoriesListView;
class KPopupMenu;
class PackageInspector;

/**
 * @class PortageTab
 * @short Tab page for portage packages.
 */
class PortageTab : public PortageBase
{
Q_OBJECT
public:
	PortageTab( QWidget *parent = 0, PackageInspector *packageInspector = 0 );
	
	/**
	* Save splitters and listview geometry.
	*/
    ~PortageTab();
	
public slots:
	
	/**
	* Populate view with portage packages.
	* Then load the emerge history.
	*/
	void				slotReload();

	/**
	* Refresh installed packages list.
	*/
	void				slotRefresh();
	
	
private slots:
	void				slotBusy( bool b );
	
	void				slotInitButtons();
	void				slotButtons( bool isQueued );
	
	/**
	* Initialize Portage view.
	* Restore geometry: splitter positions, listViews width and columns width.
	*/
	void 				slotInit();
	
	void				slotListSubCategories();
	
	/**
	* List packages when clicking on category in installed.
	*/
	void				slotFilters();
	void				slotActivateFilters();
	void				slotListPackages();
	
	/**
	* Popup menu for actions like emerge.
	* @param listView
	* @param item
	* @param point
	*/
	void				contextMenu( KListView* listView, QListViewItem* item, const QPoint& point );
	
	void				slotQueue();
	void				slotUninstall();
	void				slotAdvanced();
	void				slotClearFilter();
	
	/**
	* View summary for selected package.
	*/
	void				slotPackage();
	
private:
	int					queuedFilters;
	KPopupMenu 			*menu;
	PackageInspector	*m_packageInspector;
	
signals:
	void				signalChanged();
};

#endif
