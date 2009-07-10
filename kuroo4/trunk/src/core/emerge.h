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

class K3ProcIO;
class K3Process;

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
	void						inputText( const QString& text );
	bool						stop();
	bool 						isRunning() const;
	
	bool 						pretend( const QStringList& packageList );
	bool 						queue( const QStringList& packageList );
	bool 						unmerge( const QStringList& packageList );
	bool						quickpkg( const QStringList& packageList );
	bool						sync();
	bool						checkUpdates();
	/**
	 * Are we paused?
	 * @return bool
	 */
	inline bool					isPaused() const { return m_isPaused; }
	/**
	 * Can we pasue?
	 * @return bool
	 */
	inline bool					canPause() const { return m_pausable; }
	
	/**
	 * @return list of packages parsed out from emerge output.
	 */
	inline const 	EmergePackageList 		packageList() const { return m_emergePackageList; }
	const 	QString					packageMessage();
	/**
	 * Set Skip Housekeeping
	 */
	inline void					setSkipHousekeeping(const bool& x) { m_skipHousekeeping = x;}

public slots:
	void						slotPause();
	void						slotUnpause();
	
private:
	void						cleanup();
	bool						countEtcUpdates( const QString& line );
	void						askUnmaskPackage( const QString& packageKeyword );
	/**
	 * Do we Skip housekeeping?
	 * @return bool
	 */
	inline bool					skipHousekeeping() const { return m_skipHousekeeping; }
	
private slots:
	void 						slotEmergeOutput( K3ProcIO *proc );
	void 						slotCleanupQueue( K3Process *proc );
	void 						slotCleanupPretend( K3Process *proc );
	void 						slotCleanupUnmerge( K3Process *proc );
	void 						slotCleanupSync( K3Process *proc );
	void 						slotCleanupCheckUpdates( K3Process *proc );
	void						slotTryEmerge();
	void						slotBackupComplete( K3Process *proc );
	void						slotEmergeDistfilesComplete( K3Process* proc );
        void						slotEClean2Complete( K3Process* proc );
        void						slotRevdepRebuildComplete( K3Process* proc );

	
signals:
	void						signalEmergeComplete();
	
private:
	QObject*					m_parent;
	K3ProcIO*					eProc;
	K3ProcIO*                                        eClean1;
        K3ProcIO*                                        eClean2;
        K3ProcIO*                                        ioRevdepRebuild;
        
	// Used to collect ewarn and einfo messages spaning multiple lines
	bool						m_completedFlag;

	// Used to track a quickpkg backup
	bool						m_backupComplete;
	bool						m_backingUp;

	// Can we pause this eProc?
	bool						m_pausable;
	bool						m_isPaused;

	// should we be ecleaning?
	bool						m_doeclean;
        bool                                            m_dorevdeprebuild;
	
	// Package with the important message
	QString						m_importantMessagePackage;
	
	// Collects messages from emerge, like masked errors, ewarn and einfos
	QString 					m_importantMessage;
	
	// The current package
	QString						m_packageMessage;
	
	// The parsed package emerge says need unmasking
	QString						m_unmasked;
	
	// Collect all blocking packages
	QStringList 				m_blocks;
	
	// Remember packages emerge started with, used when auto-unmasking
	QStringList					m_lastEmergeList;
	
	// List of parsed packages
	EmergePackageList			m_emergePackageList;
	
	// Count of etc-updates files to merge
	int							m_etcUpdateCount;
        
        bool                                            m_skipHousekeeping;
};

#endif
