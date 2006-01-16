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

#ifndef MERGETAB_H
#define MERGETAB_H

#include "mergebase.h"

/**
 * @class MergeTab
 * @short Tabpage for emerge log browser, emerge history and portage directories sizes.
 */
class MergeTab : public MergeBase
{
Q_OBJECT
public:
    MergeTab( QWidget *parent = 0 );
	
	/**
	 * Save splitters and listview geometry.
	 */
    ~MergeTab();

	
private slots:
	
	/**
	 * Initialize geometry and content.
	 * Restore geometry: splitter positions, listViews width and columns width.
	 */
	void 		slotInit();
	
	/**
	 * Reload history view.
	 */
	void		slotReload();
	
	void		slotClearFilter();
	
	void 		slotViewFile( QListViewItem* item );
};

#endif
