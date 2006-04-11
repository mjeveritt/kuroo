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
class UninstallInspector;

/**
 * @class PortageTab
 * @short Package view with filters.
 */
class PortageTab : public PortageBase
{
Q_OBJECT
public:
	PortageTab( QWidget *parent = 0, PackageInspector *packageInspector = 0 );
    ~PortageTab();
	
public slots:
	void				slotReload();
	void				slotRefresh();
	
private slots:
	void 				slotInit();
	void				slotNextPackage( bool isNext );
	void				slotBusy();
	void				slotInitButtons();
	void				slotButtons();
	void				slotListSubCategories();
	void				slotFilters();
	void				slotActivateFilters();
	void				slotClearFilter();
	void				slotListPackages();
	void				slotQueue();
	void				slotUninstall();
	
	void				slotAdvanced();
	void				slotPackage();
	void				processPackage( bool viewInspector );
	
	void				contextMenu( KListView* listView, QListViewItem* item, const QPoint& point );
	
private:
	
	// Delay package view until all text in entered in the text-filter
	int					m_delayFilters;
	
	// rmb
	KPopupMenu 			*menu;
	
	// The package inspector
	PackageInspector	*m_packageInspector;
	
	// Lists all package versions for uninstalling
	UninstallInspector	*m_uninstallInspector;
	
signals:
	void				signalChanged();
};

#endif
