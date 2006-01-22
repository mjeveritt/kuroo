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

#include <qpixmap.h>

#include <kiconloader.h>

/**
 * @class Images
 * @short Initialize images in Kuroo.
 */
Images::Images( QObject* m_parent )
	: QObject( m_parent )
{
}

Images::~Images()
{
}

/**
 * Load icons.
 */
void Images::init( QObject *parent )
{
	m_parent = parent;
	
	KIconLoader *ldr = KGlobal::iconLoader();
	
	pxKuroo = ldr->loadIcon( "kuroo", KIcon::NoGroup, KIcon::SizeSmallMedium, KIcon::DefaultState, NULL, true );
	pxEmerging = ldr->loadIcon( "kuroo1", KIcon::NoGroup, KIcon::SizeSmallMedium, KIcon::DefaultState, NULL, true );
	
	pxCategory = ldr->loadIcon( "kuroo_category", KIcon::Small );
	pxNew = ldr->loadIcon( "kuroo_new", KIcon::Small );
	pxUnmerged = ldr->loadIcon( "kuroo_unmerged", KIcon::Small );
	
	pxPackage = ldr->loadIcon( "kuroo_package", KIcon::Small );
	pxInstalled = ldr->loadIcon( "kuroo_stable", KIcon::Small );
	
	pxQueued = ldr->loadIcon( "kuroo_queued", KIcon::Small );
	pxQueuedColumn = ldr->loadIcon( "kuroo_queued_column", KIcon::Small );
	pxEmpty = ldr->loadIcon( "kuroo_empty", KIcon::Small );
	pxVersionInstalled = ldr->loadIcon( "kuroo_version_installed", KIcon::Small );
}

/**
 * Deliver icons.
 * @return pointer to pixmap
 */
QPixmap& Images::icon( int state )
{
	switch ( state ) {
		
		case KUROO_READY:
			return pxKuroo;
			break;
			
		case KUROO_EMERGING:
			return pxEmerging;
			break;
	
		case EMPTY:
			return pxEmpty;
			break;
			
		case INSTALLED:
			return pxInstalled;
			break;
			
		case PACKAGE:
			return pxPackage;
			break;
			
		case QUEUED:
			return pxQueued;
			break;
	
		case UNMERGED:
			return pxUnmerged;
			break;
			
		case NEW:
			return pxNew;
			break;
		
		case VERSION_INSTALLED:
			return pxVersionInstalled;
			break;
		
		case QUEUED_COLUMN:
			return pxQueuedColumn;
	}
}

#include "images.moc"
