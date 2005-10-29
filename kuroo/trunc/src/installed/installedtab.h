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

#ifndef INSTALLEDTAB_H
#define INSTALLEDTAB_H

#include "installedbase.h"

#include <qobject.h>

class CategoriesListView;
class KTabWidget;
class KTextBrowser;
class KListView;
class QListViewItem;
class InstalledPackagesListView;

/**
 * @class InstalledTab
 * @short Tabpage containing listview and info for Installed packages.
 */
class InstalledTab : public InstalledBase
{
Q_OBJECT
public:
    InstalledTab( QWidget *parent = 0 );
    ~InstalledTab();
	
	InstalledPackagesListView 	*packagesView;
	
public slots:
	
	/**
	 * Load categories.
	 */
	void						slotReload();
	
	/**
	 * Refresh Installed packages.
	 */
	void						slotRefresh();
	
	/**
	 * Find package by name or description among installed packages.
	 * @fixme Better dialog text
	 */
	void						slotFind();
	
	/**
	 * Activate this package to view its info.
	 */
	void						slotViewPackage( const QString& package );
	
private slots:
	
	/**
	 * Initialize Installed view.
	 * Restore geometry: splitter positions, listViews width and columns width.
	 */
	void						slotInit();
	
	/**
	 * Save latest selected packages in tabs All packages, Installed packages and Updates categories.
	 */
	void						saveCurrentView();
	
	/**
	 * Unmerge selected packages.
	 */
	void						slotUnmerge();
	
	/**
	 * List packages when clicking on category.
	 */
	void						slotListPackages();
	
	/**
	 * Popup menu for actions like emerge.
	 * @todo Check dependency
	 * @param listView
	 * @param item
	 * @param point
	 */
	void						contextMenu( KListView *listView, QListViewItem *item, const QPoint &point );
	
	/**
	 * Get info for selected package in "Installed packages".
	 */
	void						slotSummary();
	
	/**
	 * Load list of installed files (CONTENTS) only if tab is active.
	 * @param page
	 */
	void						slotInstalledFiles( QWidget *page );
	
private:
	
signals:
	void						signalChanged();
};

#endif
