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

#ifndef PACKAGEITEM_H
#define PACKAGEITEM_H

#include <klistview.h>

#include <qpixmap.h>

/**
 * @class PackageItem
 * @short KListViewItem subclass to implement sorting, tooltip, color...
 */
class PackageItem : public KListViewItem
{
public:
	PackageItem( QListView *parent, const QString& id, const char* name, const QString& description, const QString& status );
	PackageItem( QListViewItem *parent, const QString& id, const char* name, const QString& description, const QString& status );
	~PackageItem();
	
	QString			status();
	QString 		id();
	QString			name();
	QString			description();
	
	/**
 	 * Is the listViewItem category, package or ebuild.
 	 * Set icon and tooltip text.
 	 * @param status
 	 */
	void			setStatus( int status );
	
protected:
	
	/**
 	 * Insert package meta text into the right column.
 	 */
	void			init();
	
	/**
 	 * Convenience method.
 	 * @return true if package is in the queue.
 	 */
	virtual bool	isQueued();
	
	/**
 	 * Mark package as "masked"/"unmasked"/"present in queue" with text formating.
	 * @param p
	 * @param cq
	 * @param column
	 * @param width
	 * @param alignment
	 */
	virtual void 	paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment );
	
private:
	int				meta;
	QString			m_id, m_name, m_status, m_packageTip, m_description;
	QListView		*m_parent;
	QPixmap 		pxPackage, pxInstalled, pxStable, pxTesting, pxStableUnmasked, pxQueued;
	bool			queued;

};

#endif
