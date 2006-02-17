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
 * The external diff tool is launched for merging changes in etc config files.
 */
class EtcUpdate : public QObject
{
Q_OBJECT
public:
    EtcUpdate( QObject *m_parent = 0, const char *name = 0 );
    ~EtcUpdate();

public slots:
	void				init( QObject *parent = 0 );
	
	/**
 	 * Ask user if to run etc-update.
 	 * @param count is number of etc files to merge.
 	 */
	void				askUpdate( const int &count );

private slots:
	
	/**
 	 * Run etc-update command and parse for etc files and terminate it.
 	 */
	bool				etcUpdate();
	
	/**
 	 * Collect output from etc-update and terminate it.
 	 */
	void 				slotEtcUpdateOutput( KProcIO *proc );
	void				slotCleanupEtcUpdate( KProcess* );
	
	/**
 	 * Parse etc-update output and launch diff tool.
 	 */
	void				runDiff();
	
	/**
 	 * Post diff action, delete original diff file.
 	 * And go for next...
 	 * @param proc
  	*/
	void				slotCleanupEtcUpdateDiff( KProcess* );
	
	void				backup( const QString& source, const QString& destination );

signals:
	void				signalEtcFileMerged();
	
private:
	QObject*			m_parent;
	KProcIO*			eProc;
	QStringList			etcUpdateLines;
	QString				diffSource;
	bool				noFiles;
	
};

#endif
