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

#include "tooltip.h"
#include "packageitem.h"

#include <qheader.h>

#include <klistview.h>
#include <kdebug.h>
#include <klocale.h>

/**
 * Creates tooltip for packages in views.
 */
ToolTip::ToolTip( KListView* pWidget, QToolTipGroup* group )
	: QToolTip( pWidget->viewport(), group ), m_pParent( pWidget )
{
}

ToolTip::~ToolTip()
{}

/**
 * View Tooltip for item.
 * Also expand "actions" with text explanation.
 * @param pos	mouse position
 */
void ToolTip::maybeTip( const QPoint& pos )
{
	PackageItem* packageItem = dynamic_cast<PackageItem*>(m_pParent->itemAt(pos));
	
	if ( packageItem )
	{
      	// Get the section the mouse is in
		int section = m_pParent->header()->sectionAt(pos.x());
      	// Get the rect of the whole item (the row for the tip)
		QRect itemRect = m_pParent->itemRect(packageItem);
      	// Get the rect of the whole section (the column for the tip)
		QRect headerRect = m_pParent->header()->sectionRect(section);
      	// "Intersect" row and column to get exact rect for the tip
		QRect destRect(headerRect.left(), itemRect.top(), headerRect.width(), itemRect.height());
		
		if ( section == m_pParent->tooltipColumn() ) {
			
			// Get meta data about this package
			Meta packageMeta = packageItem->getMeta();
			
			if ( packageMeta.size() > 1 ) {
				QString tipText("<qt><table>");
				for ( Meta::Iterator itMeta = packageMeta.begin(); itMeta != packageMeta.end(); ++itMeta ) {
					
					QString key = itMeta.key();
					QString data = itMeta.data();
					tipText += "<tr><td>" + key.right(key.length()-1) + "</td><td>";
					
					// Include explaination for USE flags
					if ( key.contains(i18n("Action")) ) {
						
						if ( data.contains("N"))
							tipText += i18n("New, (not yet installed)<br>");
						
						if ( data.contains("S"))
							tipText += i18n("New slot installation (side-by-side versions)<br>");
						
						if ( data.contains("U"))
							tipText += i18n("Update, (changing versions)<br>");
						
						if ( data.contains("D"))
							tipText += i18n("Downgrade, (Best version seems lower)<br>");
						
						if ( data.contains("R"))
							tipText += i18n("Replacing, (Remerging same version)<br>");
						
						if ( data.contains("F"))
							tipText += i18n("Fetch restricted, (Manual download)<br>");
						
						if ( data.contains("f"))
							tipText += i18n("fetch restricted, (Already downloaded)<br>");
						
						if ( data.contains("B"))
							tipText += i18n("Blocked by an already installed package<br>");
						
						// Remove ending <br>
						tipText.truncate( tipText.length() - 4 );
						tipText += "</td></tr>";
						}
					else
						tipText += data + "</td></tr>";
				}
				tipText += "</table></qt>";
				tip( destRect, tipText );
			}
		}
		
	}
}


