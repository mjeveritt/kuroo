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
 * @short Object handling the Gentoo emerge command.
 */
class Emerge : public QObject
{
Q_OBJECT
public:
	Emerge( QObject *parent = 0 );
    ~Emerge();

public slots:
	
	void				init( QObject *myParent = 0 );
	bool				stop();
	
	/**
 	 * Launch emerge pretend list of packages.
 	 * @param packageList	
 	 * @return true 		if emerge process started.
 	 */
	bool 				pretend( const QStringList& packageList );
	
	/**
 	 * Launch emerge packages process.
 	 * @param category
 	 * @param packageList	
 	 * @return true 		if emerge process started.
 	 */
	bool 				queue( const QStringList& packageList );
	
	/**
 	 * Launch unmerge list of packages.
 	 * @param packageList	
 	 * @return true 		if emerge process started.
 	 */
	bool 				unmerge( const QStringList& packageList );
	
	/**
 	 * Launch synchronize Portage tree.
 	 * @return true 		if emerge process started.
 	 */
	bool				sync();
	
	/**
 	 * Launch process to check for updates of world and system = "emerge -u(D)rxPortageVersion world".
 	 * @return true 		if emerge process started.
 	 */
	bool				checkUpdates();
	
	/**
 	 * @return list of packages parsed out from emerge output.
 	 */
	EmergePackageList	packageList();
	
	/**
	 * @return if true if emerging.
	 */
	bool 				isRunning();
	
private slots:
	
	/**
 	 * Parse emerge process output for messages and packages.
 	 * @param proc	
 	 */
	void 				readFromStdout( KProcIO *proc );
	
	/**
 	 * Post emerge actions.
	 * Notice user of messages, inform gui that emerge is completed...
 	 */
	void				cleanup();
	void 				cleanupQueue( KProcess *proc );
	void 				cleanupPretend( KProcess *proc );
	void 				cleanupUnmerge( KProcess *proc );
	void 				cleanupSync( KProcess *proc );
	void 				cleanupCheckUpdates( KProcess *proc );
	
	/**
 	 * count etc-files to merge.
 	 */
	bool				countEtcUpdates( const QString& line );
	
	/**
	 * Check if package is masked, if so ask to unmask.
	 */
	void				askUnmaskPackage( const QString& packageKeyword );
	
private:
	QObject				*parent;
	KProcIO 			*eProc;
	QString 			importantMessage, unmasked;
	QStringList 		blocks, lastEmergeList;
	EmergePackageList	emergePackageList;
	int					etcUpdateCount;

};

#endif
