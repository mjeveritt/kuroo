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
#ifndef QUEUETAB_H
#define QUEUETAB_H

#include <queuebase.h>

class KListView;
class QListViewItem;
class PretendInspector;

/**
 * @class QueueTab
 * @short Tab page for the installation queue.
 */
class QueueTab : public QueueBase
{
Q_OBJECT
public:
    QueueTab( QWidget *parent = 0 );
	
	/**
	 * Save listview geometry.
	 */
    ~QueueTab();

public slots:
	
	/**
	 * Load Queue packages.
	 */
	void				slotReload( bool hasCheckedQueue );
	void				slotQueueSummary();
	
private slots:
	
	/**
	 * Initialize Queue view.
	 */
	void				slotInit();
	
	/**
	 * Disable/enable buttons when kuroo is busy.
	 * @param b
	 */
	void				slotBusy( bool busy );
	
	/**
	 * Slots for button actions.
	 */
	void				slotGo();
	void				slotPretend();
	void				slotStop();
	void				slotRemove();
	void				slotRemoveInstalled();
	
	/**
	 * Popup menu for actions like emerge.
	 * @param listView
	 * @param item
	 * @param point
	 */
	void				contextMenu( KListView *listView, QListViewItem *item, const QPoint &point );
	
	void				slotButtons();
	
private:
	bool				m_hasCheckedQueue;
	QString				initialQueueTime;
};

#endif
