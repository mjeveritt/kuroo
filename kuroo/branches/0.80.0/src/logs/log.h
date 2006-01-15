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
#ifndef LOG_H
#define LOG_H

#include <qobject.h>
#include <qfile.h>

#include <kio/job.h>

class QCheckBox;
class KTextBrowser;

/**
 * @class Log
 * @short Log output from all actions as emerge, scanning... to log window and to file.
 */
class Log : public QObject
{
Q_OBJECT
public:
	Log( QObject *m_parent = 0 );
    ~Log();
	
	/**
	 * Connect log output browser in gui.
	 * @param logBrowserGui			pointer to text browser.
	 * @param checkBoxVerboseGui	pointer to verbose checkbox.
	 */
	void 			setGui( KTextBrowser *logBrowserGui, QCheckBox *verboseLogGui, QCheckBox *saveLogGui );
	
	/**
	 * Write log lines to text browser and log file.
	 * @param output		line of text.
	 * @param i			type of log = EMERGE, KUROO, WARNING, TOLOG, EMERGELOG.
	 */
	void 			writeLog( const QString& output, int i );

public slots:
	QString			init( QObject *parent = 0 );
	
	/**
	 * Backup job of "kuroo.log".
	 */
	KIO::Job*		backupLog();
	
private:
	QObject*		m_parent;
	QCheckBox 		*verboseLog, *saveLog;
	KTextBrowser 	*logBrowser;
	QFile 			logFile;
	
signals:
	void			signalLogChanged();
};

#endif
