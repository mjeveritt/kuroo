/***************************************************************************
*   Copyright (C) 2004 by karye                                           *
*   karye@users.sourceforge.net                                           *
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

#ifndef GENTOOAGENT_H
#define GENTOOAGENT_H

#include <dcopclient.h>
#include <klistview.h>
#include <kprocio.h>

#include "gentoowatcherbase.h"

class KPopupMenu;
class KListView;

/**
 * @class GentooAgent
 * @short GentooAgent systray widget.
 */
class GentooAgent: public GentooWatcherBase {
Q_OBJECT
public:
	GentooAgent(QWidget *parent = 0, const char *name = 0);
	~GentooAgent();

private slots:
	void closeEvent( QCloseEvent *ce );
	
	/**
	 * Launch emerge pretend in kuroo via dcop.
	 */
	void sendToKuroo();
	
	/**
	 * Stop trying after 25 sec = 5 times.
	 */
	void stopTimer();
	
	/**
	 * Start kuroo.
	 * @param item
	 */
	void addPackageKuroo( QListViewItem *item );
	void contextMenu( KListView * listView, QListViewItem * item, const QPoint & point );
	
	/**
	 * Open page in browser.
	 * @param item
	 */
	void sentToBrowser( QListViewItem* item );
	
	/**
 	 * Lauch glsa-check.
 	 * @param item
	 */
	void viewGlsa( QListViewItem* item );
	
	/**
 	 * Collect output from "glsa-check".
 	 * @param proc
 	 */
	void readFromStdout( KProcIO *proc );
	
	/**
 	 * Open dialog for "glsa-check".
 	 */
	void popupGlsa( KProcess* );
	void openGlsa( QListViewItem* item );

private:
	QTimer *internalTimer;
	DCOPClient *client;
	QString glsaText, package;
	KPopupMenu * menu;
};

#endif
