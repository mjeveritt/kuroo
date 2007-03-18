/**************************************************************************
*   Copyright (C) 2004 by                                                 *
*   karye@users.sourceforge.net                                           *
*   Stefan Bogner <bochi@online.ms>                                       *
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
#include "systemtray.h"
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
	: KConfigDialog( parent, name, config ), m_isDefault( false )
{
	setWFlags( WDestructiveClose );
	
	Options1* opt1 = new Options1( this, i18n("General") );
// 	Options2* opt2 = new Options2( this, i18n("make.conf") );
// 	Options7* opt7 = new Options7( this, i18n("Etc-update warnings") );
	
	addPage( opt1, i18n("General"), "kuroo", i18n("General preferences") );
// 	addPage( opt2, i18n("make.conf"), "kuroo_makeconf", i18n("Edit your make.conf file") );
// 	addPage( opt7, i18n("Etc-update warnings"), "messagebox_warning", i18n("Edit your etc-update warning file list") );
	
// 	connect( this, SIGNAL( settingsChanged() ), this, SLOT( slotSaveAll() ) );
// 	connect( this, SIGNAL( defaultClicked() ), this, SLOT( slotDefaults() ) );
	
// 	parseMakeConf();
}

ConfigDialog::~ConfigDialog()
{}

/**
 * Reset to defaults.
 */
void ConfigDialog::slotDefaults()
{
	DEBUG_LINE_INFO;
// 	parseMakeConf();
	show();
}

/**
 * Save settings when user press "Apply".
 */
void ConfigDialog::slotSaveAll()
{
// 	DEBUG_LINE_INFO;
// 	switch( activePageIndex() ) {
// 		
// 		// Activate the systray directly (not needing restarting kuroo)
// 		case 0: {
// 			if ( KuroolitoConfig::isSystrayEnabled() )
// 				SystemTray::instance()->activate();
// 			else
// 				SystemTray::instance()->inactivate();
// 			
// 			SignalistSingleton::Instance()->fontChanged();
// 			break;
// 		}
// 		
// 		case 1:
// 			if ( !saveMakeConf() ) {
// 				parseMakeConf();
// 				show();
// 				KMessageBox::error( this, i18n("Failed to save %1. Please run as root.").arg( KuroolitoConfig::fileMakeConf() ), i18n("Saving"));
// 			}
// 	}
}


#include "configdialog.moc"
