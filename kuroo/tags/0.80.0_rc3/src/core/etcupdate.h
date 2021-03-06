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

class KProcIO;

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

public slots:
	void				init( QObject *parent = 0 );
	void				askUpdate( const int &count );

private slots:
	bool				etcUpdate();
	void 				slotEtcUpdateOutput( KProcIO *proc );
	void				slotCleanupEtcUpdate( KProcess* );
	void				runDiff();
	void				slotCleanupEtcUpdateDiff( KProcess* );
	void				backup( const QString& source, const QString& destination );

signals:
	void				signalEtcFileMerged();
	
private:
	QObject*			m_parent;
	KProcIO*			eProc;
	
	// Keep track of current etc-file position in the total
	int 				count, totalEtcCount;
	
	// Collected output from etc-update
	QStringList			etcUpdateLines;
	
	// List of etc-files for merging
	QStringList			etcFilesList;
	
	// Remember last source file when iterating
	QString				diffSource;
	
	// True if etc-update didn't find any files for merging
	bool				noFiles;
	
};

#endif
