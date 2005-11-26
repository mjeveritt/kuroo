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

#ifndef _SYSTRAY_H_
#define _SYSTRAY_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qmap.h>
#include <qptrlist.h>

#include <kapplication.h>
#include <kmainwindow.h>
#include <ksystemtray.h>
#include <dcopobject.h>
#include <dcopref.h>
#include <kconfigdialog.h>

#include "gentooagent.h"

class DCOPRef;
class KSystemTray;
class KConfigDialog;
class QPixmap;
class QTimer;
class KProcIO;
class KURLLabel;

typedef QMap<QString, QString> dbMap;
typedef QValueList< QPair<QString, QString> > ArticleMap;

typedef struct {
	DCOPRef ref;
	QString title;
	QString url;
	QString description;
	ArticleMap map;
} Feed;

typedef QValueList<Feed> FeedList;

extern QRegExp pv;

/**
 * @class GentooWatcher
 * @short GentooWatcher is a system tray class checking for latest packages in the Portage tree and Gentoo GLSA.
 * Gentoo Watcher fetches the xml documents every x hour.
 * The documents are then parsed and packages inserted to the listviews of m_window.
 */
class GentooWatcher : public KSystemTray, public DCOPObject
{
Q_OBJECT
K_DCOP
public:
	GentooWatcher();
	virtual ~GentooWatcher();

k_dcop:
	
	/**
	 * Catch signal from rssservice that articles are fetched and ready for display.
	 */
	void documentUpdated( DCOPRef feedRef );
	
	/**
	 * Catch signal from rssservice that articles are fetched and ready for display.
	 */
	void documentAdded( QString );

protected:
	void mousePressEvent( QMouseEvent* ev );

private slots:
	
	/**
	 * Add rss urls to rssservice and get feeds.
	 */
	void initDocuments();
	
	/**
	 * Start timer to fetch rss every x hour.
	 */
	void updateDocuments();
	void checkUpdates();
	
	/**
	 * Add packages to listViews.
	 */
	void updateView();
	
	/**
	 * Set column width for nice gui and more.
	 */
	void setupGui();
	
	/**
	 * Gentoo watcher settings dialog.
	 */
	void configureAgent();
	
	/**
	 * Reset Gentoo Watcher icon.
	 */
	void slotSysTrayView();
	
	bool queryClose();
	void checkGlsa();
	void readFromStdout(KProcIO *proc);
	
signals:
	void tryConnectRss();

private:
	int checkGlsaInterval;
	FeedList mFeeds;
	QStringList glsaList;
	QTimer *internalTimer, *checkGlsaTimer;
	KConfigDialog *dialog;
	GentooAgent *m_window;
	KSystemTray *m_systemTray;
	QPixmap pxGlsa0, pxGlsa1, pxGlsa2, pxGlsa3, pxInstalled, pxTesting, pxPackage;
	
};

#endif // _SYSTRAY_H_
