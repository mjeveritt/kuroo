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
#include "systemtray.h"

#include <qtooltip.h>

#include <kpopupmenu.h>

SystemTray* SystemTray::s_instance = 0;

/**
 * @class SystemTray
 * @short Singleton object that creates the kuroo systemtray icon and actions.
 * 
 * Displays kuroo icon in systemtray and switches to "busy" icon when kuroo is busy.
 * The user can disable the systemtray icon in settings.
 */
SystemTray::SystemTray( QWidget *parent )
	: KSystemTray( parent )
{
	s_instance = this;
	QToolTip::add( this, i18n("Kuroolito - Portage frontend") );
}

SystemTray::~SystemTray()
{
}

#include "systemtray.moc"
