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
	m_parent( parent ), queued( false ), m_id( id ), m_name( name ), m_status( status ), m_description( description )
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
	pxCategory = ldr->loadIcon( "kuroo_category", KIcon::Small );
	pxPackage = ldr->loadIcon( "kuroo_package", KIcon::Small );
	pxInstalled = ldr->loadIcon( "kuroo_stable", KIcon::Small );
	pxEbuild = ldr->loadIcon( "kuroo_ebuild", KIcon::Small );
	pxEbuildInstalled = ldr->loadIcon( "kuroo_ebuild_emerged", KIcon::Small );
	pxQueued = ldr->loadIcon( "kuroo_queued", KIcon::Small );
	
	if ( m_status != FILTERALL )
		setStatus( INSTALLED );
	else
		setStatus( PACKAGE );
}

/**
 * Return package db id.
 */
QString PackageItem::id()
{
	return m_id;
}

QString PackageItem::name()
{
	return m_name;
}

QString PackageItem::description()
{
	return m_description;
}

/**
 * Convenience method.
 * @return status
 */
QString PackageItem::status()
{
	return m_status;
}

/**
 * Convenience method.
 * @return meta object
 */
Meta PackageItem::getMeta()
{
	return m_meta;
}

/**
 * Convenience method.
 * @return true if package is in the queue.
 */
bool PackageItem::isQueued()
{
	return queued;
}

/**
 * Is the listViewItem category, package or ebuild.
 * Set icon and tooltip text.
 * @param status
 */
void PackageItem::setStatus( int status )
{
	switch ( status ) {
		
		case NONE :
			m_status = NONE;
			repaint();
			break;
		
		case INSTALLED :
			m_status = INSTALLED;
			setPixmap( 0, pxInstalled );
			m_meta.insert( i18n("Status"), i18n("Installed") );
			break;
			
		case PACKAGE :
			m_status = PACKAGE;
			setPixmap( 0, pxPackage );
			break;
		
		case MASKED :
			m_status = MASKED;
			m_meta.insert( i18n("Keyword"), i18n("Masked") );
			repaint();
			break;
		
		case UNMASKED :
			m_status = UNMASKED;
			m_meta.insert( i18n("Keyword"), i18n("Unmasked") );
			repaint();
			break;
		
		case QUEUED :
			queued = true;
			m_meta.insert( i18n("In Queue"), i18n("Yes") );
// 			if ( parent() )
// 				dynamic_cast<PackageItem*>( parent() )->setStatus( QUEUED );
			setPixmap( 1, pxQueued );
			break;
		
		case NOTQUEUED : {
			queued = false;
			m_meta.insert( i18n("In Queue"), i18n("No") );
// 			if ( parent() ) {
// 				PackageItem* sibling;
// 				sibling = dynamic_cast<PackageItem*>(parent()->firstChild());
// 				bool notQueued(true);
// 				while( sibling ) {
// 					if ( sibling->isQueued() )
// 						notQueued = false;
// 					sibling = dynamic_cast<PackageItem*>( sibling->nextSibling() );
// 				}
// 				if ( notQueued )
// 					dynamic_cast<PackageItem*>( parent() )->setStatus( NOTQUEUED );
// 			}
			setPixmap( 1, NULL );
			repaint();
		}
	}
}

/**
 * Change package/ebuild text color.
 * @param p
 * @param cq
 * @param column
 * @param width
 * @param alignment
 */
// void PackageItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment )
// {
// 	QColorGroup _cg( cg );
// 	QFont font( p->font() );
// 	
// 	if ( column == 0 ) {
// 		switch ( m_status ) {
// 			case NONE :
// 				font.setBold(false);
// 				p->setFont(font);
// 				_cg.setColor( QColorGroup::Text, Qt::black );
// 				break;
// 			
// 			case MASKED :
// 				font.setBold(true);
// 				p->setFont(font);
// 				_cg.setColor( QColorGroup::Text, KurooConfig::maskedColor() );
// 				break;
// 			
// 			case UNMASKED :
// 				font.setBold(true);
// 				p->setFont(font);
// 				_cg.setColor( QColorGroup::Text, KurooConfig::unmaskedColor() );
// 				break;
// 		}
// 	}
// 	KListViewItem::paintCell( p, _cg, column, width, alignment );
// }

