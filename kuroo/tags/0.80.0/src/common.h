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

#ifndef COMMON_H
#define COMMON_H

#include "settings.h"
#include "images.h"
#include "message.h"
#include "signalist.h"
#include "emerge.h"
#include "etcupdate.h"
#include "statusbar.h"
#include "log.h"
#include "queue.h"
#include "results.h"
#include "portage.h"
#include "portagedb.h"
#include "history.h"
#include "portagefiles.h"
#include "filewatcher.h"
#include "singleton.h"

#include <qregexp.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

// Define all singleton objects
typedef Singleton<Images> ImagesSingleton;
typedef Singleton<Signalist> SignalistSingleton;
typedef Singleton<Emerge> EmergeSingleton;
typedef Singleton<EtcUpdate> EtcUpdateSingleton;
typedef Singleton<Queue> QueueSingleton;
typedef Singleton<Results> ResultsSingleton;
typedef Singleton<Portage> PortageSingleton;
typedef Singleton<KurooDB> KurooDBSingleton;
typedef Singleton<Log> LogSingleton;
typedef Singleton<History> HistorySingleton;
typedef Singleton<PortageFiles> PortageFilesSingleton;
typedef Singleton<FileWatcher> FileWatcherSingleton;

// The package states used by the filters
enum packageFilters {
		FILTER_ALL,
		FILTER_INSTALLED,
		FILTER_UPDATES,
		FILTER_OLD
};

static const QString FILTER_ALL_STRING( QString::number( FILTER_ALL ) );
static const QString FILTER_INSTALLED_STRING( QString::number( FILTER_INSTALLED ) );
static const QString FILTER_OLD_STRING( QString::number( FILTER_OLD ) );

// Log output states
enum LogActions {
		EMERGE,
		KUROO,
		ERROR,
		TOLOG,
		EMERGELOG
};

// Package and icons
enum status {
		VIEW_PORTAGE = 1,
		VIEW_QUEUE,
		VIEW_HISTORY,
		VIEW_MERGE,
		VIEW_LOG,
		PACKAGE,
		INSTALLED,
		QUEUED,
		WORLD,
		NOTQUEUED,
		UNMERGED,
		NEW,
		EMPTY,
		KUROO_READY,
		KUROO_EMERGING,
		VERSION_INSTALLED,
		QUEUED_COLUMN,
		INSTALLED_COLUMN,
		WORLD_COLUMN
};

static const QString PACKAGE_STRING( QString::number( PACKAGE ) );
static const QString INSTALLED_STRING( QString::number( INSTALLED ) );

// The "maskedness" of a package version.
enum Stability {
		STABLE,
		TESTING,
		HARDMASKED,
		NOTAVAILABLE,
		NOTARCH
};

static const QString STABLE_STRING( QString::number( STABLE ) );
static const QString TESTING_STRING( QString::number( TESTING ) );
static const QString HARDMASKED_STRING( QString::number( HARDMASKED ) );
static const QString NOTAVAILABLE_STRING( QString::number( NOTAVAILABLE ) );

// Kuroo home directory
static const QString KUROODIR = "/var/cache/kuroo/";

// Max rows in queries
static const int ROWLIMIT = 1000;

// Regexp from Portage
// static const QRegExp rxPortageVersion("(-(?:\\d+\\.)*\\d+[a-z]?)");
static QRegExp rxPortageVersion("(?:[a-z]|[A-Z]|[0-9]|-)*((-(?:\\d+\\.)*\\d+[a-z]?)(?:_(?=alpha|beta|pre|rc|p)\\d*)?(?:-r\\d*)?)");

typedef struct Info {
	QString slot;
	QString homepage;
	QString licenses;
	QString description;
	QString keywords;
	QString useFlags;
	QString size;
};

/**
 * Use this to const-iterate over QStringLists, if you like.
 * Watch out for the definition of last in the scope of your for.
 *
 *     QStringList strings;
 *     foreach( strings )
 *         debug() << *it << endl;
 */
#define foreach( x ) \
for( QStringList::ConstIterator it = x.begin(), end = x.end(); it != end; ++it )

#endif
