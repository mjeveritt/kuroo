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
 * @short Delivers icons in Kuroolito.
 */
Images::Images( QObject* m_parent )
	: QObject( m_parent )
{}

Images::~Images()
{}

/**
 * Load icons.
 */
void Images::init( QObject *parent )
{
	m_parent = parent;
	
	KIconLoader *ldr = KGlobal::iconLoader();
	
	pxKuroolito = ldr->loadIcon( "kuroo", KIcon::NoGroup, KIcon::SizeSmallMedium, KIcon::DefaultState, NULL, true );
	
	pxCategory = ldr->loadIcon( "kuroo_category", KIcon::Small );
	pxPackage = ldr->loadIcon( "kuroo_package", KIcon::Small );
	pxInstalled = ldr->loadIcon( "kuroo_stable", KIcon::Small );

	pxWorld = ldr->loadIcon( "kuroo_world", KIcon::Small );
	pxWorldColumn = ldr->loadIcon( "kuroo_world_column", KIcon::Small );
	
	pxEmpty = ldr->loadIcon( "kuroo_empty", KIcon::Small );
	pxWarning = ldr->loadIcon( "kuroo_warning", KIcon::Small );
	
	pxVersionInstalled = ldr->loadIcon( "kuroo_version_installed", KIcon::Small );
	pxInstalledColumn = ldr->loadIcon( "kuroo_installed_column", KIcon::Small );
}

/**
 * Deliver icons.
 * @return pointer to pixmap
 */
const QPixmap& Images::icon( int image )
{
	switch ( image ) {
		
		case KUROO_READY:
			return pxKuroolito;
			break;
	
		case EMPTY:
			return pxEmpty;
			break;
		
		case WARNING:
			return pxWarning;
			break;
			
		case INSTALLED:
			return pxInstalled;
			break;
			
		case PACKAGES:
			return pxPackage;
			break;
		
		case WORLD:
			return pxWorld;
			break;
		
		case VERSION_INSTALLED:
			return pxVersionInstalled;
			break;

		case INSTALLED_COLUMN:
			return pxInstalledColumn;
			break;
		
		case WORLD_COLUMN:
			return pxWorldColumn;
			break;
			
		default:
			pxEmpty;
	}
}

#include "images.moc"
