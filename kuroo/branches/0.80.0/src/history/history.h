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

#ifndef HISTORY_H
#define HISTORY_H

#include <qobject.h>
#include <qstringlist.h>
#include <qfile.h>

class KProcIO;
class PackageEmergeTime;
class QString;
class KDirWatch;

typedef QMap<QString, PackageEmergeTime> EmergeTimeMap;

/**
 * @class History
 * @short Object for the emerge history and statistics.
 */
class History : public QObject
{
Q_OBJECT
public:
	History( QObject *m_parent = 0 );
    ~History();

public slots:
	void			init( QObject *parent = 0 );
	
	/**
	 * Load emerge statistics if any.
	 * And start a watch on the emerge.log so as to parse for emerge activities.
	 */
	void			slotInit();
	
	/**
	 * Check for new entries in emerge.log.
	 */
	bool			slotRefresh();
	
	/**
	 * Load all emerge times statistics.
	 */
	void			loadTimeStatistics();
	
	/**
 	* Statistics object.
	 */
	EmergeTimeMap 	getStatisticsMap();
	void 			setStatisticsMap( const EmergeTimeMap& statisticsMap );
	
	/**
	 * Get emerge time for this package.
	 * @param package
	 * @return emergeTime		time or na
	 */
	QString 		packageTime( const QString& packageNoversion );
	
	void			slotChanged();

	void			appendEmergeInfo( const QString& einfo );
	
	void			updateStatistics();
	
	QStringList		allMergeHistory();
	
private slots:
	
	/**
	 * Launch scan to load into db.
	 */
	void			slotScanHistory( const QStringList& lines );
	
	/**
	 * Parse emerge.log when it has been changed, eg after, emerge, unmerge, sync...
 	 */
	void			slotParse();

private:
	QObject*		m_parent;
	KDirWatch		*fileWatcher;
	EmergeTimeMap	m_statisticsMap;
	QFile 			log;
	QTextStream 	stream;
	bool			userSync, isEmerging;
	
signals:
	void			signalHistoryChanged();
};

#endif
