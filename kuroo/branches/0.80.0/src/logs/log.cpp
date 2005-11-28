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
#include "log.h"

#include <sys/stat.h>

#include <qdatetime.h>
#include <qcheckbox.h>

#include <ktextbrowser.h>
#include <kuser.h>
#include <kio/job.h>
#include <kmessagebox.h>

/**
 * Log output from all actions as emerge, scanning... to log window and to file.
 */
Log::Log( QObject* parent )
	: QObject( parent ), logBrowser(0), verboseLog(0), saveLog(0)
{
}

Log::~Log()
{
	logFile.close();
}

/**
 * Open persistent log.
 * @return log file name
 */
QString Log::init( QObject *myParent )
{
	parent = myParent;
	
	QString logName = KUROODIR + "kuroo.log";
	logFile.setName(logName);
	if( !logFile.open(IO_WriteOnly ) ) {
		kdDebug() << i18n("Error writing: ") << KUROODIR << "kuroo.log" << endl;
		KMessageBox::error(0, i18n("Error writing %1kuroo.log.").arg(KUROODIR), i18n("Saving"));
		return "";
	}
	else
		return logName;
}

/**
 * Backup job of "kuroo.log".
 * @return copy job
 */
KIO::Job* Log::backupLog()
{
	if ( saveLog && saveLog->isChecked() ) {
		QDateTime dt = QDateTime::currentDateTime();
		
		KIO::Job *cpjob = KIO::file_copy( KUROODIR + "kuroo.log", KUROODIR + "kuroo_" + dt.toString("yyyyMMdd_hhmm") + ".log", -1, true, false, false );
		
		return cpjob;
	}
	else
		return NULL;
}

/**
 * Connect log output browser in gui.
 * @param logBrowserGui			pointer to text browser.
 * @param checkBoxVerboseGui	pointer to verbose checkbox.
 */
void Log::setGui( KTextBrowser* logBrowserGui, QCheckBox* verboseLogGui, QCheckBox* saveLogGui )
{
	logBrowser = logBrowserGui;
	verboseLog = verboseLogGui;
	saveLog = saveLogGui;
}

/**
 * Write log lines to text browser and log file.
 * @param output		line of text.
 * @param i			type of log = EMERGE, KUROO, WARNING, TOLOG, EMERGELOG.
 */
void Log::writeLog( const QString& output, int i )
{
	QString line(output);
	line.replace('\'', "''");
	
	switch(i) {
		case EMERGE: {
			if ( verboseLog && verboseLog->isChecked() ) {
				line = "<font color=blue>" + line.replace('>', "&gt;").replace('<', "&lt;")+ "</font>";
				logBrowser->append(line);
			}
			break;
		}
		case KUROO: {
			logBrowser->append(line);
			break;
		}
		case ERROR: {
			line = "<font color=red>" + line.replace('>', "&gt;").replace('<', "&lt;") + "</font>";
			
			logBrowser->append(line);
			break;
		}
		case TOLOG: {
			QTextStream st( &logFile );
			st << line << endl;
			logFile.flush();
			break;
		}
		case EMERGELOG: {
			line = "<font color=BlueViolet>" + line.replace('>', "&gt;").replace('<', "&lt;") + "</font>";
			
			logBrowser->append(line);
		}
	}
	
	emit signalLogUpdated();
}

#include "log.moc"
