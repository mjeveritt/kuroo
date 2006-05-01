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

#ifndef SCANUPDATESJOB_H
#define SCANUPDATESJOB_H

#include "threadweaver.h"

#include <qobject.h>

class DbConnection;

/**
 * @class ScanUpdatesJob
 * @short Thread for loading emerge -uDrxPortageVersion World output into db.
 */
class ScanUpdatesJob : public ThreadWeaver::DependentJob
{
Q_OBJECT
public:
	ScanUpdatesJob( QObject *parent, const EmergePackageList &packageList );
	~ScanUpdatesJob();

private:
	bool 						doJob();
	void 						completeJob();
	
private:
	DbConnection* const 		m_db;
	
	// Package object list
	EmergePackageList 			m_packageList;
};

#endif
