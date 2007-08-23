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

#ifndef SCANHISTORYJOB_H
#define SCANHISTORYJOB_H

#include "threadweaver.h"

#include <qobject.h>

class DbConnection;
class QStringList;
class PackageEmergeTime;

typedef QMap<QString, PackageEmergeTime> EmergeTimeMap;

/**
 * @class ScanHistoryJob
 * @short Thread for parsing emerge/unmerge entries found in emerge.log.
 */
class ScanHistoryJob : public ThreadWeaver::DependentJob
{
Q_OBJECT
public:
	ScanHistoryJob( QObject *parent = 0, const QStringList& logLines = QStringList::QStringList() );
	~ScanHistoryJob();

private:
	bool 						doJob();
	void 						completeJob();
	
	inline QString 					escapeString( QString string ) const { return string.replace('\'', "''"); }
	
private:
	DbConnection* const			m_db;
	
	// Log lines to parse
	QStringList 				m_logLines;
};

#endif
