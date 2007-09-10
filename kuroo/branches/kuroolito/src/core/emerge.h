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

#ifndef EMERGE_H
#define EMERGE_H

#include "package.h"

#include <qobject.h>
#include <qstringlist.h>
#include <qfile.h>

class KProcIO;
class KProcess;

/**
 * @class Emerge
 * @short The Gentoo emerge command.
 */
class Emerge : public QObject
{
Q_OBJECT
public:
	Emerge( QObject *m_parent = 0 );
    ~Emerge();

	void						init( QObject *parent = 0 );
	bool 						isRunning();
	
	bool						checkUpdates();
	
	const EmergePackageList		packageList();
	
private slots:
	void 						slotEmergeOutput( KProcIO *proc );
	void 						slotCleanupCheckUpdates( KProcess *proc );
	
signals:
	void						signalEmergeComplete();
	
private:
	QObject*					m_parent;
	KProcIO*					eProc;

	// List of parsed packages
	EmergePackageList			m_emergePackageList;

};

#endif
