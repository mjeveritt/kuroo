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

#ifndef SCANPORTAGEJOB_H
#define SCANPORTAGEJOB_H

#include "threadweaver.h"

#include <qobject.h>

class DbConnection;
class QRegExp;

extern QRegExp pv;

/**
 * @class ScanPortageJob
 * @short Thread for scanning local portage tree.
 */
class ScanPortageJob : public ThreadWeaver::DependentJob
{
Q_OBJECT
public:
	ScanPortageJob( QObject *parent = 0 );
	~ScanPortageJob();

private slots:
	
	/**
	 * Scan KurooConfig::dirEdbDep() + "/usr/portage" for packages in portage tree.
	 * Inserting found packages in db.
	 * @return bool 		true if successful.
	 */
	bool 						doJob();
	void 						completeJob();
	
	/**
	 * Collect info about this ebuild.
	 * @param category   	
	 * @param package  	
	 * @return  false if the file can't be opened, true otherwise.
	 */
	bool						scanInfo( const QString& path, const QString& category, const QString& package );
	
	/**
	 * Format package size nicely 
	 * @fixme: Check out KIO_EXPORT QString KIO::convertSize
	 * @param size 
	 * @return total		as "xxx kB"
	 */
	QString						kBSize( const QString& size );
	
private:
	int							totalPackages;
	bool						aborted;
	DbConnection* const 		m_db;
	struct Info {
		QString packageSlots;
		QString homepage;
		QString licenses;
		QString description;
		QString keywords;
		QString useFlags;
		QString size;
	};
	Info 						info;
};

#endif
