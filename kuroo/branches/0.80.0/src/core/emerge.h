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

public slots:
	void				init( QObject *parent = 0 );
	bool				stop();
	bool 				pretend( const QStringList& packageList );
	bool 				queue( const QStringList& packageList );
	bool 				unmerge( const QStringList& packageList );
	bool				sync();
	bool				checkUpdates();
	EmergePackageList	packageList();
	bool 				isRunning();
	QString				packageMessage();
	
private slots:
	void 				slotEmergeOutput( KProcIO *proc );
	void				cleanup();
	void 				slotCleanupQueue( KProcess *proc );
	void 				slotCleanupPretend( KProcess *proc );
	void 				slotCleanupUnmerge( KProcess *proc );
	void 				slotCleanupSync( KProcess *proc );
	void 				slotCleanupCheckUpdates( KProcess *proc );
	bool				countEtcUpdates( const QString& line );
	void				askUnmaskPackage( const QString& packageKeyword );
	void				slotTryEmerge();
	
signals:
	void				signalEmergeComplete();
	
private:
	QObject*			m_parent;
	KProcIO*			eProc;
	
	// Collects messages from emerge, like masked errors, ewarn and einfos
	QString 			importantMessage;
	
	// The current package
	QString				m_packageMessage;
	
	// The parsed package emerge says need unmasking
	QString				unmasked;
	
	// Collect all blocking packages
	QStringList 		blocks;
	
	// Remember packages emerge started with, used when auto-unmasking
	QStringList			lastEmergeList;
	
	// List of parsed packages
	EmergePackageList	emergePackageList;
	
	// Count of etc-updates files to merge
	int					etcUpdateCount;
};

#endif
