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

#include <kmenu.h>

SystemTray* SystemTray::s_instance = 0;

/**
 * @class SystemTray
 * @short Singleton object that creates the kuroo systemtray icon and actions.
 * 
 * Displays kuroo icon in systemtray and switches to "busy" icon when kuroo is busy.
 * The user can disable the systemtray icon in settings.
 */
SystemTray::SystemTray( QWidget *parent )
    : KSystemTrayIcon( parent )
{
	s_instance = this;
    setToolTip( i18n("Kuroo - Portage frontend") );

    /*contextMenu()->insertItem( i18n("&Configure Kuroo..."), this, SLOT( slotPreferences() ) );
	m_menuPause = contextMenu()->insertItem( i18n("Pause Emerge"), this, SLOT( slotPause() ) );
	m_menuUnpause = contextMenu()->insertItem( i18n("Unpause Emerge"), this, SLOT( slotUnpause() ) );

	contextMenu()->setItemEnabled( m_menuPause, false );
    contextMenu()->setItemEnabled( m_menuUnpause, false );*/
	
	connect( SignalistSingleton::Instance(), SIGNAL( signalKurooBusy(bool) ), this, SLOT( slotBusy(bool) ) );
}

SystemTray::~SystemTray()
{
}

void SystemTray::activate()
{
	slotBusy( false );
	show();
}

void SystemTray::inactivate()
{
	hide();
}

void SystemTray::slotPreferences()
{
	emit signalPreferences();
}

void SystemTray::slotPause()
{
	if( EmergeSingleton::Instance()->canPause() ){
        /*contextMenu()->setItemEnabled( m_menuPause, false );
        contextMenu()->setItemEnabled( m_menuUnpause, true );*/
		EmergeSingleton::Instance()->slotPause();
	}
}

void SystemTray::slotUnpause()
{
	if( EmergeSingleton::Instance()->canPause() ){
        /*contextMenu()->setItemEnabled( m_menuPause, true );
        contextMenu()->setItemEnabled( m_menuUnpause, false );*/
		EmergeSingleton::Instance()->slotUnpause();
	}
}

/**
 * Show busy kuroo icon.
 * @param	kuroo state = busy or ready
 */
void SystemTray::slotBusy( bool busy )
{
	if ( busy && isVisible() ) {
        setIcon( ImagesSingleton::Instance()->icon( KUROO_EMERGING ) );
        if( EmergeSingleton::Instance()->isPaused() && EmergeSingleton::Instance()->canPause() ) {
            //contextMenu()->setItemEnabled( m_menuUnpause, true );
        } else if( !EmergeSingleton::Instance()->isPaused() && EmergeSingleton::Instance()->canPause() ) {
            //contextMenu()->setItemEnabled( m_menuPause, true );
        }
	}
	else {
        setIcon( ImagesSingleton::Instance()->icon( KUROO_READY ) );
        if( EmergeSingleton::Instance()->canPause() && EmergeSingleton::Instance()->isPaused() ) {
            //contextMenu()->setItemEnabled( m_menuUnpause, true );
        } else {
            /*contextMenu()->setItemEnabled( m_menuPause, false );
            contextMenu()->setItemEnabled( m_menuUnpause, false );	*/
		}
	}
}

#include "systemtray.moc"
