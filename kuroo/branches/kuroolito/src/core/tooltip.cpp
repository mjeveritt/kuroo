/***************************************************************************
*   Copyright (C) 2007 by Karye                                           *
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

#include "tooltip.h"
#include "packageitem.h"
#include "common.h"

#include <qheader.h>

#include <klistview.h>

/**
 * @class ToolTip
 * @short Creates tooltip for icons in views.
 */
ToolTip::ToolTip( KListView* pWidget, QToolTipGroup* group )
	: QToolTip( pWidget->viewport(), group ), m_pParent( pWidget )
{}

ToolTip::~ToolTip()
{}

/**
 * View Tooltip for the icons.
 * @param pos	mouse position
 */
void ToolTip::maybeTip( const QPoint& pos )
{
	PackageItem* packageItem = dynamic_cast<PackageItem*>( m_pParent->itemAt( pos ) );
	
	if ( packageItem ) {
		QString package = packageItem->text( 0 );
		
      	// Get the section the mouse is in
		int section = m_pParent->header()->sectionAt( pos.x() );
		
      	// Get the rect of the whole item (the row for the tip)
		QRect itemRect = m_pParent->itemRect( packageItem );
		
      	// Get the rect of the whole section (the column for the tip)
		QRect headerRect = m_pParent->header()->sectionRect( section );
		
      	// "Intersect" row and column to get exact rect for the tip
		QRect destRect( headerRect.left(), itemRect.top(), headerRect.width(), itemRect.height() );
		
		QString tipText;
		
		switch ( section ) {
		
			case 0 : {
				if ( packageItem->isInstalled() )
					tipText = i18n( "%1 is installed" ).arg( package );
				else {
						if ( packageItem->isInPortage() )
							tipText = i18n( "%1 is not installed" ).arg( package );
						else
							tipText = i18n( "%1 not in Portage" ).arg( package );
				}
				break;
			}
			
			case 1 :
				if ( packageItem->isInstalled() )
					tipText = i18n( "%1 is installed" ).arg( package );
				break;
				
			case 2 :
				if ( packageItem->isInWorld() )
					tipText = i18n( "%1 is in World list" ).arg( package );
				break;
		}
		
		tip( destRect, tipText );
	}
}


