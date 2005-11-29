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
class PortageListView;
class KTabWidget;
class KTextBrowser;
class KActionSelector;
class QPushButton;
class PortageListView;
class KPopupMenu;
class UseDialog;

/**
 * @class PortageTab
 * @short Tab page for portage packages.
 */
class PortageTab : public PortageBase
{
Q_OBJECT
public:
    PortageTab( QWidget *parent = 0 );
	
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
	
	/**
	* Activate this package to view its info.
	* @param package
	*/
	void				slotViewPackage( const QString& package );
	
private slots:
	
	/**
	* Save latest selected packages in tabs All packages, Installed packages and Updates categories.
	*/
	void				saveCurrentView();
	
	/**
	* Initialize Portage view.
	* Restore geometry: splitter positions, listViews width and columns width.
	*/
	void 				slotInit();
	
	void				slotListSubCategories();
	
	/**
	* List packages when clicking on category in installed.
	*/
	void				slotFilters( int radioFilter );
	void				slotListPackages();
	void				slotListCategoryPackages();
	
	/**
	* Popup menu for actions like emerge.
	* @param listView
	* @param item
	* @param point
	*/
	void				contextMenu( KListView* listView, QListViewItem* item, const QPoint& point );
	
	/**
	* View summary for selected package.
	*/
	void				slotSummary();
	
	/**
	* View ebuild, changelog and dependencies.
	* @param page
	*/
	void				slotPackageInfo( QWidget* page );
	
	/**
	* For editing use flags per package.
	*/
	void				useFlags();
	
private:
	int					filter;
	PortageListView 	*packagesView;
	KPopupMenu 			*menu;
	UseDialog 			*useDialog;
	
signals:
	void				signalChanged();
};

#endif
