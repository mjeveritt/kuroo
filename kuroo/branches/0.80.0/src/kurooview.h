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

#ifndef _KUROOVIEW_H_
#define _KUROOVIEW_H_

#include "kurooviewbase.h"

#include <qwidget.h>

#include <kparts/part.h>
#include <kurooiface.h>

class PortageTab;
class QueueTab;
class HistoryTab;
class LogsTab;
class MergeTab;

/**
 * @class KurooView
 * @short Create the gui 
 */
class KurooView : public KurooViewBase, public kurooIface
{
Q_OBJECT
public:
	KurooView( QWidget *parent, const char *name );
	~KurooView();
	
	/**
	 * The tabwidget and the tabs accessible by Kuroo.
	 */
	PortageTab*		tabPortage;
	HistoryTab*		tabHistory;
	QueueTab*		tabQueue;
	LogsTab* 		tabLogs;
	MergeTab*		tabMerge;
	
	void 			quit();
	
	/**
	 * DCOP slot accessible by kuroo_watcher.
	 */
	void 			slotEmergePretend( QString package );

public:
	class 			IconListItem;
	
public slots:
	/**
	 * Check the validity of the database. Update history if emerge.log is changed.
	 * Reset everything after a sync.
	 */
	void 			slotInit();
	void			slotReset();

private slots:
	/**
	 * Methods for checking kuroo integrity when starting and after a sync.
	 * Chain of events for refreshing "Portage", "Installed".
	 * slotCheckPortage -> slotCheckInstalled -> slotCheckUpdates
	 */
	void 			slotCheckPortage();
	
	/**
	 * Update tab text color and package counts.
	 */
	void 			slotPortageUpdated();
	void 			slotQueueUpdated();
	void			slotHistoryUpdated();
	void			slotMergeUpdated();
	void 			slotLogUpdated();
	void			slotResetMenu( QListBoxItem* menuItem );
	
	void 			slotShowView();
	
private:
	IconListItem	*iconPackages, *iconQueue, *iconHistory, *iconLog, *iconMerge;
	bool			hasHistoryRestored;
};

class KurooView::IconListItem : public QListBoxItem
{
public:
	IconListItem( QListBox *listbox, const QPixmap &pixmap, const QString &text );
	virtual int height( const QListBox *lb ) const;
	virtual int width( const QListBox *lb ) const;
	int expandMinimumWidth( int width );
	void setChanged( bool modified );
	bool isChanged();
	
protected:
	const QPixmap &defaultPixmap();
	void paint( QPainter *painter );
	
private:
	bool m_modified;
	QPixmap mPixmap;
	int mMinimumWidth;
};

#endif // _KUROOVIEW_H_
