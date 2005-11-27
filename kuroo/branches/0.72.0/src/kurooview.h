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

#include <qwidget.h>

#include <kparts/part.h>
#include <kurooiface.h>

class KTabWidget;
class InstalledTab;
class PortageTab;
class UpdatesTab;
class QueueTab;
class ResultsTab;
class LogsTab;

/**
 * @class KurooView
 * @short Create the gui with tabs for "Installed", "Portage"...
 */
class KurooView : public QWidget, public kurooIface
{
Q_OBJECT
public:
	KurooView( QWidget *parent, const char *name );
	~KurooView();
	
	/**
	 * The tabwidget and the tabs accessible by Kuroo.
	 */
	KTabWidget		*mainTabs;
	InstalledTab 	*tabInstalled;
	PortageTab 		*tabPortage;
	UpdatesTab 		*tabUpdates;
	LogsTab 		*tabLogs;
	
	void 			quit();
	
	/**
	 * DCOP slot accessible by kuroo_watcher.
	 */
	bool 			slotEmergePretend( QString package );
	
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
	void 			slotCheckInstalled();
	void 			slotCheckUpdates();
	void 			slotReloadQueueResults();
	
	/**
	 * Update tab text color and package counts.
	 */
	void 			slotCurrentChanged( QWidget* newPage );
	void 			slotInstalledUpdated();
	void 			slotPortageUpdated();
	void 			slotQueueUpdated();
	void 			slotUpdatesUpdated();
	void 			slotResultsUpdated();
	void 			slotLogsTabUpdated();
	
	/**
	 * Show package summary info by making it current.
	 */
	void			slotViewPackage( const QString& package );
	
private:
	QueueTab 		*tabQueue;
	ResultsTab 		*tabResults;
};

#endif // _KUROOVIEW_H_
