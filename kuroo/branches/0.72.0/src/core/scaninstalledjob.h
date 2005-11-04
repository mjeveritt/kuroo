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
#ifndef SCANINSTALLEDJOB_H
#define SCANINSTALLEDJOB_H

#include "threadweaver.h"

class DbConnection;

/**
 * Thread for scanning installed packages.
 * Thread to collect installed packages by scanning KurooConfig::dirDbPkg().
 * The packages are counted first, this to get a correct refresh progress in the gui.
 * Found packages are updated as installed in portage database by changing the "installed" flag to "1".
 * If the installed package is not found in the database it is inserted and flaged as "2".
 * 
 * @class ScanInstalledJob
 * @short Thread for scanning installed packages.
 */
class ScanInstalledJob : public ThreadWeaver::DependentJob
{
Q_OBJECT
public:
    ScanInstalledJob( QObject *parent = 0 );
    ~ScanInstalledJob();

private:
	
	/**
	 * Count portage packages for correct progress timing.
	 */
	int			countPackages();
	
	/**
	 * Scan KurooConfig::dirDbPkg() for installed packages.
	 * Update packages with the installed flag.
	 * @return bool 		true if successful.
	 */
	bool 						doJob();
	void 						completeJob();
	
private:
	DbConnection* const 		m_db;
	bool						aborted;
};

#endif
