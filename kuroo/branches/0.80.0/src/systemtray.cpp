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
 */
SystemTray::SystemTray( QWidget *parent )
	: KSystemTray( parent )
{
	s_instance = this;
	QToolTip::add( this, i18n("Kuroo - Portage frontend") );
	contextMenu()->insertItem( i18n("&Configure Kuroo..."), this, SLOT( slotPreferences() ) );
	connect( SignalistSingleton::Instance(), SIGNAL( signalKurooBusy(bool) ), this, SLOT( slotBusy(bool) ) );
	
}

SystemTray::~SystemTray()
{
}

void SystemTray::activate()
{
	DEBUG_LINE_INFO;
	
	slotBusy( false );
	show();
}

void SystemTray::inactivate()
{
	DEBUG_LINE_INFO;
	
	hide();
}

void SystemTray::slotPreferences()
{
	emit signalPreferences();
}

/**
 * Show busy kuroo icon.
 * @param	kuroo state = busy or ready
 */
void SystemTray::slotBusy( bool busy )
{
	if ( busy && isVisible() )
		setPixmap( ImagesSingleton::Instance()->icon( KUROO_EMERGING ) );
	else
		setPixmap( ImagesSingleton::Instance()->icon( KUROO_READY ) );
}

#include "systemtray.moc"
