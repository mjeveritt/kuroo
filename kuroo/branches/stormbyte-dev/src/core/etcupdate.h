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


#ifndef ETCUPDATE_H
#define ETCUPDATE_H

#include <qobject.h>

#include <kio/job.h>

class KProcIO;
class KDirWatch;

/**
 * @class EtcUpdate
 * @short Object for handling etc-updates.
 */
class EtcUpdate : public QObject
{
Q_OBJECT
public:
    EtcUpdate( QObject *m_parent = 0, const char *name = 0 );
    ~EtcUpdate();

	void				init( QObject *parent = 0 );
	/**
	 * Return found configuration files.
	 */
	inline QStringList		confFilesList() const { return m_etcFilesList; }
	/**
	 * Return backup files found in /var/cache/kuroo/backup/configuration.
	 */
	inline QStringList		backupFilesList() const { return m_backupFilesList; }
	void				runDiff( const QString& source, const QString& destination, const bool& isNew );
	
public slots:
	void				slotEtcUpdate();
	
private slots:
	void				slotChanged();
	void				slotFinished();
	void 				slotListFiles( KIO::Job*, const KIO::UDSEntryList& lst );
	void				slotCleanupDiff( KProcess* );
	
signals:
	void				signalEtcFileMerged();
	void				signalScanCompleted();
	
private:
	QObject*			m_parent;
	KProcIO*			eProc;
	
	QStringList 		m_configProtectList;
	QString				m_configProtectDir;
	
	// List of etc-files for merging
	QStringList			m_backupFilesList;
	
	// List of etc-files for merging
	QStringList			m_etcFilesList;
	
	QString				m_source, m_destination;
	
	bool				m_changed;
	int				m_mergedMode;
	
	KDirWatch			*m_mergingFile;
};

#endif
