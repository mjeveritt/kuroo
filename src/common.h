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

#include <klocale.h>
#include <kdebug.h>

#include "settings.h"
#include "message.h"
#include "signalist.h"
#include "results.h"
#include "emerge.h"
#include "etcupdate.h"
#include "statusbar.h"
#include "log.h"
#include "queue.h"
#include "updates.h"
#include "installed.h"
#include "portage.h"
#include "portagedb.h"
#include "history.h"
#include "singleton.h"
#include "diskusage.h"

typedef Singleton<Signalist> SignalistSingleton;
typedef Singleton<Emerge> EmergeSingleton;
typedef Singleton<EtcUpdate> EtcUpdateSingleton;
typedef Singleton<Queue> QueueSingleton;
typedef Singleton<Results> ResultsSingleton;
typedef Singleton<Installed> InstalledSingleton;
typedef Singleton<Portage> PortageSingleton;
typedef Singleton<KurooDB> KurooDBSingleton;
typedef Singleton<Log> LogSingleton;
typedef Singleton<History> HistorySingleton;
typedef Singleton<Updates> UpdatesSingleton;
typedef Singleton<DiskUsage> DiskUsageSingleton;

enum LogActions { EMERGE, KUROO, ERROR, TOLOG, EMERGELOG };

enum packageStatus { NONE, CATEGORY, INSTALLED, PACKAGE, EBUILD_INSTALLED, EBUILD, MASKED, UNMASKED, QUEUED, NOTQUEUED };

static const QString KUROODIR = "/var/kuroo/";
static const int ROWLIMIT = 1000;

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

