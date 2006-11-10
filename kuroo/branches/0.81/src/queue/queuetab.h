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
class PackageInspector;

/**
 * @class QueueTab
 * @short Tab page for the installation queue.
 */
class QueueTab : public QueueBase
{
Q_OBJECT
public:
	QueueTab( QWidget *parent = 0, PackageInspector *packageInspector = 0 );
    ~QueueTab();

public slots:
	void				slotReload( bool hasCheckedQueue );
	void				slotQueueSummary();
	
private:
	void				processPackage( bool viewInspector );
	
private slots:
	void				slotInit();
	
	void				slotWhatsThis();
	
	void				slotRefresh();
	void				slotNextPackage( bool isNext );
	void				slotBusy();
	void				slotButtons();
	void				slotCheck();
	void				slotGo();
	void				slotPretend();
	void				slotStop();
	void				slotRemove();
	void				slotClear();
	void				slotRemoveInstalled();
	void				slotAdvanced();
	void				slotPackage();
	void				contextMenu( KListView *listView, QListViewItem *item, const QPoint &point );
	
private:
	
	// Packages loaded in Queue are a pretend result
	bool				m_hasCheckedQueue;
	
	// Remember the initial emerge duration before installation started
	QString				m_initialQueueTime;
	
	// The package inspector
	PackageInspector	*m_packageInspector;
};

#endif
