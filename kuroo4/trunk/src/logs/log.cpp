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

#include <sys/stat.h>

#include <qdatetime.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <Q3TextStream>

#include <ktextbrowser.h>
#include <kuser.h>
#include <kio/job.h>
#include <kmessagebox.h>
#include "settings.h"

uint Log::buffer_MaxLines = 2000;

/**
 * @class Log
 * @short Log output from all actions as emerge, scanning... to log window and to file.
 * 
 * @todo: save all output to log file
 */
Log::Log( QObject* m_parent )
	: QObject( m_parent ), m_verboseLog( 0 ), m_saveLog( 0 ), m_logBrowser( 0 ), numLines( 0 )
{}

Log::~Log()
{
	m_logFile.close();
}

/**
 * Open persistent log.
 * @return log file name
 */
const QString Log::init( QObject *parent )
{
	m_parent = parent;
	
	QString logName = GlobalSingleton::Instance()->kurooDir() + "kuroo.log";
	kDebug(0) << logName << "\n";
    m_logFile.setFileName( logName );
	if( !m_logFile.open( QIODevice::WriteOnly ) ) {
		kError(0) << "Writing: " << GlobalSingleton::Instance()->kurooDir() << "kuroo.log" << LINE_INFO;
		KMessageBox::error(0, i18n("Writing %1kuroo.log.").arg( GlobalSingleton::Instance()->kurooDir() ), i18n("Saving"));
		return QString::null;
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
	if ( m_saveLog && m_saveLog->isChecked() ) {
		QDateTime dt = QDateTime::currentDateTime();
		KIO::Job *cpjob = KIO::file_copy( GlobalSingleton::Instance()->kurooDir() + "kuroo.log", 
                                          GlobalSingleton::Instance()->kurooDir() + "kuroo_" + dt.toString("yyyyMMdd_hhmm") + ".log");
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
	m_logBrowser = logBrowserGui;
	m_verboseLog = verboseLogGui;
	m_saveLog = saveLogGui;
}

/**
 * Write log lines to text browser and log file.
 * @param output		line of text.
 * @param logType			type of log = EMERGE, KUROO, WARNING, TOLOG, EMERGELOG.
 */
void Log::writeLog( const QString& output, int logType )
{
	QString line(output);
    line.toUtf8().replace( '\'', "''" );
	
	switch ( logType ) {
		
		case EMERGE: {
			if ( m_verboseLog && m_verboseLog->isChecked() ) {
				//line = line.replace('>', "&gt;").replace('<', "&lt;");
				addText( line );
				emit signalLogChanged();
			}
			break;
		}
		
		case KUROO: {
			addText( line );
			break;
		}
		
		case ERROR: {
			line = line.replace('>', "&gt;").replace('<', "&lt;");
			addText( line );
			break;
		}
		
		case TOLOG: {
			Q3TextStream st( &m_logFile );
			st << line << "\n";
			m_logFile.flush();
			break;
		}
		
		case EMERGELOG: {
			line = line.replace('>', "&gt;").replace('<', "&lt;");
			addText( line );
		}
	}
}

void Log::addText(const QString& str) {
	//kDebug(0) << "Max Buffer Lines: " << buffer_MaxLines << "\n";
	if (numLines > buffer_MaxLines) {
		m_logBrowser->clear();
		numLines=0;
	}
	m_logBrowser->append(str);
	numLines++;
}


#include "log.moc"
