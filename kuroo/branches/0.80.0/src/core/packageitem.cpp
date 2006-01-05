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
#include <kiconloader.h>

/**
 * KListViewItem subclass to implement sorting, tooltip, color...
 */
PackageItem::PackageItem( QListView* parent, const QString& id, const char* name, const QString& description, const QString& status )
	: KListViewItem( parent, name ),
	m_parent( parent ), queued( false ), m_id( id ), m_name( name ), m_status( status ), m_description( description ), meta( PACKAGE )
{
	init();
}

PackageItem::PackageItem( QListViewItem* parent, const QString& id, const char* name, const QString& description, const QString& status )
	: KListViewItem( parent, name ),
	m_parent( parent->listView() ), queued( false ), m_id( id ), m_name( name ), m_status( status ), m_description( description ), meta( PACKAGE )
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
	// Load icons for category, package ...
	KIconLoader *ldr = KGlobal::iconLoader();
	pxPackage = ldr->loadIcon( "kuroo_package", KIcon::Small );
	pxInstalled = ldr->loadIcon( "kuroo_stable", KIcon::Small );
	pxQueued = ldr->loadIcon( "kuroo_queued", KIcon::Small );
	
	if ( m_status != FILTERALL_STRING )
		setStatus( INSTALLED );
	else
		setStatus( PACKAGE );
}

/**
 * Is the listViewItem category, package or ebuild.
 * Set icon and tooltip text. @fixme!
 * @param status
 */
void PackageItem::setStatus( int status )
{
	switch ( status ) {
		
		case NONE :
			meta = NONE;
			repaint();
			break;
		
		case INSTALLED :
			meta = INSTALLED;
			setPixmap( 0, pxInstalled );
// 			m_meta.insert( i18n("Status"), i18n("Installed") );
			break;
			
		case PACKAGE :
			meta = PACKAGE;
			setPixmap( 0, pxPackage );
			break;
		
		case MASKED :
			meta = MASKED;
// 			m_meta.insert( i18n("Keyword"), i18n("Masked") );
			repaint();
			break;
		
		case UNMASKED :
			meta = UNMASKED;
// 			m_meta.insert( i18n("Keyword"), i18n("Unmasked") );
			repaint();
			break;
		
		case QUEUED :
			queued = true;
// 			m_meta.insert( i18n("In Queue"), i18n("Yes") );
			setPixmap( 1, pxQueued );
			break;
		
		case NOTQUEUED : {
			queued = false;
// 			m_meta.insert( i18n("In Queue"), i18n("No") );
			setPixmap( 1, NULL );
			repaint();
		}
	}
}

/**
 * View package masked status by changing name text color.
 * @param p
 * @param cq
 * @param column
 * @param width
 * @param alignment
 */
void PackageItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment )
{
	QColorGroup _cg( cg );
	QFont font( p->font() );
	
	if ( column == 0 ) {
		switch ( meta ) {
			case NONE :
				font.setBold(false);
				p->setFont(font);
				_cg.setColor( QColorGroup::Text, Qt::black );
				break;
			
			case MASKED :
				font.setBold(true);
				p->setFont(font);
				_cg.setColor( QColorGroup::Text, KurooConfig::maskedColor() );
				break;
			
			case UNMASKED : {
				font.setBold(true);
				p->setFont(font);
				_cg.setColor( QColorGroup::Text, KurooConfig::unmaskedColor() );
			}
		}
	}
	KListViewItem::paintCell( p, _cg, column, width, alignment );
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
 * Wether this package is in the emerge queue.
 * @return true/false
 */
bool PackageItem::isQueued()
{
	return queued;
}
