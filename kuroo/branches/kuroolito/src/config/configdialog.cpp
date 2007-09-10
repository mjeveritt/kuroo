/**************************************************************************
*   Copyright (C) 2004 by Karye                                           *
*   info@kuroo.org                                                        *
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
#include "configdialog.h"
#include "options1.h"

#include <qtextstream.h>
#include <qdir.h>
#include <qfile.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlineedit.h>

#include <kconfigdialog.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <ktextedit.h>
#include <kuser.h>

/**
 * @class ConfigDialog
 * @short Kuroolito preferences.
 * 
 * Build settings widget for kuroo and make.conf.
 * Parses make.conf and tries to keep user-format when saving back settings.
 */
ConfigDialog::ConfigDialog( QWidget *parent, const char* name, KConfigSkeleton *config )
	: KConfigDialog( parent, name, config )
{
	setWFlags( WDestructiveClose );
	
	Options1* opt1 = new Options1( this, i18n("General") );
	addPage( opt1, i18n("General"), "kuroo", i18n("General preferences") );
}

ConfigDialog::~ConfigDialog()
{}

#include "configdialog.moc"
