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
#include <kinputdialog.h>


/**
 * @class LogsTab
 * @short Tabpage for emerge log browser, emerge history and portage directories sizes.
 */
LogsTab::LogsTab( QWidget* parent )
	: LogsBase( parent )
{
	connect( pbEnter, SIGNAL( clicked() ), this, SLOT( slotUserInput() ) );
	
// 	logBrowser->setTextFormat( Qt::LogText ); // Text doesn't wrap in log mode!
	init();
}

/**
 * Save checkboxes state.
 */
LogsTab::~LogsTab()
{
	if ( saveLog->isChecked() )
		KurooConfig::setSaveLog( true );
	else
		KurooConfig::setSaveLog( false );
	
	if ( verboseLog->isChecked() )
		KurooConfig::setVerboseLog( true );
	else
		KurooConfig::setVerboseLog( false );
	
	KurooConfig::writeConfig();
}

/**
 * Restore checkboxes state
 */
void LogsTab::init()
{
	if ( KurooConfig::saveLog() )
		saveLog->setChecked( true );
	else
		saveLog->setChecked( false );
	
	if ( KurooConfig::verboseLog() )
		verboseLog->setChecked( true );
	else
		verboseLog->setChecked( false );
}

/**
 * Open dialog for manually sending text to running emerge process.
 */
void LogsTab::slotUserInput()
{
	QString input = KInputDialog::getText( i18n("User Input"), i18n("Enter text:"), QString::null, 0, this, 0, 0, QString::null );
	EmergeSingleton::Instance()->inputText( input );
}

#include "logstab.moc"
