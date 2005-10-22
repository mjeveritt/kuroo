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

#include "emergelogdlg.h"
#include "settings.h"

#include <qfile.h>

#include <klocale.h>
#include <ktextbrowser.h>
#include <kdebug.h>

/**
 * Widget to show emerge log.
 */
EmergeLogDlg::EmergeLogDlg(QWidget* parent, const char* name, bool modal, WFlags fl)
	: EmergeLogBase(parent, name, modal, fl)
{
	loadLog();
}

EmergeLogDlg::~EmergeLogDlg()
{
}

void EmergeLogDlg::loadLog()
{
	QFile file(KurooConfig::dirHome() + "/kuroo.log");
	
	if (file.open(IO_ReadOnly)) {
		QTextStream stream(&file);
		emergeLogText->clear();
		emergeLogText->setText(stream.read());
		file.close();
	}
	else
		kdDebug() << i18n("Error reading ") << KurooConfig::dirHome() << "/kuroo.log" << endl;
}

#include "emergelogdlg.moc"

