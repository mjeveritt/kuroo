/***************************************************************************
 *   Copyright (C) 2007 by Karye                                           *
 *   info@kuroo.org                                                        *
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
	PortageTab( QWidget *parent = 0 );
    ~PortageTab();
	
public slots:
	void				slotReload();
	
private slots:
	void 				slotInit();
	void				slotListSubCategories();
	void				slotFillFilter( const QString& text );
	void				slotFilters();
	void				slotActivateFilters();
	void				slotClearFilter();
	void				slotListPackages();
	void				slotPackage();
	
private:

	// Delay package view until all text in entered in the text-filter
	int					m_delayFilters;
	
signals:
	void				signalChanged();
};

#endif
