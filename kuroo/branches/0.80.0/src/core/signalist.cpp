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

#include "common.h"

#include <qapplication.h>

#include <kcursor.h>

/**
 * Object which forwards signals, so they can picked up system wide.
 * Just connect to this instance.
 */
Signalist::Signalist( QObject* parent )
	: QObject( parent ), busy( false ), busyScanning( false ), busyDiskUsage( false )
{
}

Signalist::~Signalist()
{
}

void Signalist::init( QObject* myParent )
{
	parent = myParent;
}

/**
 * Convenience flag.
 * @return busy
 */
bool Signalist::isKurooBusy()
{
	return busy;
}

/**
 * Toggle busy flag for kuroo.
 * @param busy
 */
void Signalist::setKurooBusy( bool b )
{
	static int busySession(0);
	
	if ( !b ) {
		busySession--;
		QApplication::restoreOverrideCursor();
	}
	else {
		busySession++;
		QApplication::setOverrideCursor( KCursor::workingCursor() );
	}
	
	if ( busySession == 0 ) {
		busy = false;
		emit signalKurooBusy( false );
	}
	else {
		busy = true;
		emit signalKurooBusy( true );
	}
}

/**
 * Job wasn't successful.
 */
void Signalist::scanAborted()
{
	setKurooBusy( false );
}

/**
 * Kuroo is done syncing.
 */
void Signalist::syncDone()
{
	emit signalSyncDone();
}

/**
 * Kuroo is busy scanning.
 */
void Signalist::scanStarted()
{
	setKurooBusy( true );
}

/**
 * Portage scan thread completed.
 */
void Signalist::cachePortageComplete()
{
	emit signalCachePortageComplete();
	setKurooBusy( false );
}

/**
 * Portage scan thread completed.
 */
void Signalist::scanPortageComplete()
{
	emit signalScanPortageComplete();
	setKurooBusy( false );
}

/**
 * Installed scan thread completed.
 */
void Signalist::scanInstalledComplete()
{
	emit signalScanInstalledComplete();
	setKurooBusy( false );
}

/**
 * "emerge -urxPortageVersion world" completed.
 */
void Signalist::scanUpdatesComplete()
{
	emit signalScanUpdatesComplete();
	setKurooBusy( false );
}

/**
 * Db is updated with the new updates list.
 */
void Signalist::loadUpdatesComplete()
{
	emit signalLoadUpdatesComplete();
	setKurooBusy( false );
}

/**
 * Db is updated with the new history entries.
 */
void Signalist::scanHistoryComplete()
{
	emit signalScanHistoryComplete();
	setKurooBusy( false );
}

/**
 * Tell queue to start emerging.
 */
void Signalist::startInstallQueue()
{
	emit signalEmergeQueue();
}

/**
 * Propagate signal that a package is added to the queue.
 * @param id	package db id
 * @param true/false
 */
void Signalist::setQueued( const QString& idDB, bool b )
{
	emit signalSetQueued( idDB, b );
}

void Signalist::clearQueued()
{
	emit signalClearQueued();
}

/**
 * Propagate signal that a package is unmasked.
 * @param id	package db id
 * @param true/false
 */
void Signalist::setUnmasked( const QString& name, bool b )
{
	emit signalUnmasked( name, b );
}

/**
 * Propagate signal that emerge of package started.
 * @param package
 */
void Signalist::emergePackageStart( const QString& package )
{
	emit signalEmergePackageStart( package );
}

/**
 * Propagate signal that emerge of package completed.
 * @param package
 */
void Signalist::emergePackageComplete( const QString& package )
{
	emit signalEmergePackageComplete( package );
}

#include "signalist.moc"
