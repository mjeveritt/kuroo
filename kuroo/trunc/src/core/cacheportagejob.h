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

#ifndef CACHEPORTAGEJOB_H
#define CACHEPORTAGEJOB_H

#include "threadweaver.h"

#include <qobject.h>

class DbConnection;

/**
 * @class CachePortageJob
 * @short Thread to cache package information from the Portage directory to speed up portage view refreshing.
 */
class CachePortageJob : public ThreadWeaver::DependentJob
{
Q_OBJECT
public:
    CachePortageJob( QObject *parent = 0 );
    ~CachePortageJob();

private slots:
	
	/**
	 * Count portage packages for correct progress timing.
	 */
	int							countPackages();
	
	/**
	 * Scan for package size found in digest files and store in db.
	 */
	bool 						doJob();
	void 						completeJob();
	
private:
	DbConnection* const 		m_db;
};

#endif
