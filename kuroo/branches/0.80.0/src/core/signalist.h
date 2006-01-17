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

#ifndef SIGNALIST_H
#define SIGNALIST_H

#include <qobject.h>

/**
 * @class Signalist
 * @short Object which forwards signals, so they can picked up system wide.
 * Just connect to this instance.
 */
class Signalist : public QObject
{
Q_OBJECT
public:
	Signalist( QObject *m_parent = 0 );
    ~Signalist();

public slots:
	void		init( QObject *parent = 0 );
	
	void		setKurooReady( bool isReady );
	bool		isKurooReady();
	
	/**
	 * Toggle busy flag for kuroo.
	 * @param true/false
	 */
	void		setKurooBusy( bool busy );
	
	/**
	 * Job wasn't successful.
	 */
	void		scanAborted();
	
	/**
	 * Convenience methods.
	 * Return true when kuroo is emerging or scanning.
	 */
	bool		isKurooBusy();
	
	/**
	 * For forwarding kuroo threads status.
	 */
	void		scanStarted();
	void		syncDone();
	void		cachePortageComplete();
	void 		scanPortageComplete();
	void		scanUpdatesComplete();
	void		loadUpdatesComplete();
	void		scanHistoryComplete();
	
	/**
	 * Tell queue to start emerging.
	 */
	void		startInstallQueue();
	
	/**
	 * Methods for marking packages as present in the queue.
	 */
	void		setQueued( const QString& idDB, bool b );
	void		clearQueued();
	
	/**
	 * Methods for marking packages unmasked.
	 */
	void		setUnmasked( const QString& name, bool b );
	
signals:
	void		signalKurooBusy( bool b );
	void		signalCachePortageComplete();
	void 		signalScanPortageComplete();
	void 		signalScanInstalledComplete();
	void 		signalScanUpdatesComplete();
	void 		signalLoadUpdatesComplete();
	void 		signalEmergeQueue();
	void		signalSyncDone();
	void 		signalScanHistoryComplete();
	void		signalInstalledChanged();
	void		signalViewPackage( const QString& package );
	void		signalSetQueued( const QString& idDB, bool b );
	void		signalClearQueued();
	void		signalUnmasked( const QString& name, bool b );

private:
	QObject*	m_parent;
	bool		m_busy, m_isReady;
};

#endif
