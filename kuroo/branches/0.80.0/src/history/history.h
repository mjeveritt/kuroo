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

	void			init( QObject *parent = 0 );
	
public slots:
	void			slotInit();
	void			slotScanHistoryCompleted();
	bool			slotRefresh();
	void			loadTimeStatistics();
	EmergeTimeMap 	getStatisticsMap();
	void 			setStatisticsMap( const EmergeTimeMap& statisticsMap );
	QString 		packageTime( const QString& packageNoversion );
	void			appendEmergeInfo();
	void			updateStatistics();
	QStringList		allMergeHistory();
	
private slots:
	void			slotScanHistory( const QStringList& lines );
	void			slotParse();

private:
	QObject*		m_parent;
	KDirWatch		*logWatcher;
	EmergeTimeMap	m_statisticsMap;
	QFile 			log;
	QTextStream 	stream;
	bool			userSync, isEmerging;
	
signals:
	void			signalScanHistoryCompleted();
	void			signalHistoryChanged();
};

#endif
