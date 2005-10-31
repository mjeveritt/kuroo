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
#include "historylistview.h"
#include "logstab.h"

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qsplitter.h>

#include <ktextbrowser.h>
#include <kmessagebox.h>

/**
 * Tabpage for emerge log browser, emerge history and portage directories sizes.
 */
LogsTab::LogsTab( QWidget* parent )
	: LogsBase( parent )
{
	logBrowser->setTextFormat( Qt::LogText );
	
	// Reload view after changes.
	connect( HistorySingleton::Instance(), SIGNAL( signalHistoryChanged() ), this, SLOT( slotReload() ) );
	
	slotInit();
}

/**
 * Save splitters and listview geometry.
 */
LogsTab::~LogsTab()
{
	KConfig* config = KurooConfig::self()->config();
	config->setGroup("Kuroo Geometry");
	
	QValueList<int> list = splitterV->sizes();
	config->writeEntry("splitterLogsV", list);
	
	historyView->saveLayout( KurooConfig::self()->config(), "logsViewLayout" );
	
	// Save checkboxes state
	if ( saveLog->isChecked() )
		KurooConfig::setSaveLog(true);
	else
		KurooConfig::setSaveLog(false);
	
	if ( verboseLog->isChecked() )
		KurooConfig::setVerboseLog(true);
	else
		KurooConfig::setVerboseLog(false);
	
	KurooConfig::writeConfig();
}

/**
 * Initialize geometry and content.
 * Restore geometry: splitter positions, listViews width and columns width.
 */
void LogsTab::slotInit()
{
	KConfig* config = KurooConfig::self()->config();
	config->setGroup("Kuroo Geometry");
	
	QValueList<int> sizes = config->readIntListEntry("splitterLogsV");
	splitterV->setSizes(sizes);
	
	historyView->restoreLayout( KurooConfig::self()->config(), "logsViewLayout" );
	
	// Restore checkboxes state
	if ( KurooConfig::saveLog() )
		saveLog->setChecked(true);
	else
		saveLog->setChecked(false);
	
	if ( KurooConfig::verboseLog() )
		verboseLog->setChecked(true);
	else
		verboseLog->setChecked(false);
}

/**
 * Reload history view.
 */
void LogsTab::slotReload()
{
	historyView->loadFromDB();
}

#include "logstab.moc"
