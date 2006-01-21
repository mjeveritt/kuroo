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

#include "common.h"
#include "packageitem.h"

#include <qpainter.h>

#include <klistview.h>

/**
 * @class PackageItem
 * @short KListViewItem subclass to implement sorting, tooltip, color...
 */
PackageItem::PackageItem( QListView* parent, const char* name, const QString& id, const QString& description, const QString& status )
	: KListViewItem( parent, name ),
	m_parent( parent ), m_isQueued( false ), m_id( id ), m_name( name ), m_status( status ), m_description( description ), meta( PACKAGE )
{
	init();
}

PackageItem::PackageItem( QListViewItem* parent, const char* name, const QString& id, const QString& description, const QString& status )
	: KListViewItem( parent, name ),
	m_parent( parent->listView() ), m_isQueued( false ), m_id( id ), m_name( name ), m_status( status ), m_description( description ), meta( PACKAGE )
{
	init();
}

PackageItem::~PackageItem()
{}

/**
 * Set package status.
 */
void PackageItem::init()
{
	if ( m_status == PACKAGE_STRING )
		setStatus( PACKAGE );
	else
		setStatus( INSTALLED );
}

/**
 * Is the listViewItem category, package or ebuild.
 * Set icon and tooltip text. @fixme!
 * @param status
 */
void PackageItem::setStatus( int status )
{
	switch ( status ) {
		
		case INSTALLED :
			setPixmap( 0, ImagesSingleton::Instance()->icon( INSTALLED ) );
			break;
			
		case PACKAGE :
			setPixmap( 0, ImagesSingleton::Instance()->icon( PACKAGE ) );
			break;
		
		case QUEUED :
			m_isQueued = true;
			setPixmap( 1, ImagesSingleton::Instance()->icon( QUEUED ) );
			break;
		
		case NOTQUEUED :
			m_isQueued = false;
			setPixmap( 1, ImagesSingleton::Instance()->icon( EMPTY ) );
		
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Accessor methods
/////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Package db id.
 * @return id
 */
QString PackageItem::id()
{
	return m_id;
}

/**
 * Package name as kuroo in app-portage/kuroo.
 * @return name
 */
QString PackageItem::name()
{
	return m_name;
}

/**
 * Package description.
 * @return description
 */
QString PackageItem::description()
{
	return m_description;
}

/**
 * Package status describing if this package is installed or not.
 * @return status
 */
QString PackageItem::status()
{
	return m_status;
}

/**
 * Is this package installed.
 * @return true/false
 */
bool PackageItem::isInstalled()
{
	return ( m_status == INSTALLED_STRING );
}

/**
 * Is this package is in the emerge queue?
 * @return true/false
 */
bool PackageItem::isQueued()
{
	return m_isQueued;
}


