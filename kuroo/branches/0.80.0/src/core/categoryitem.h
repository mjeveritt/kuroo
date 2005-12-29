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


#ifndef CATEGORYITEM_H
#define CATEGORYITEM_H

#include <qlistview.h>

/**
 * @class CategoriesView::CategoryItem
 * @short Highlight category header with bold darkGray
 */
class CategoryItem : public QListViewItem
{
public:
	CategoryItem::CategoryItem( QListView* parent, const char* name, const QString &id );
	
	void 	setOn( bool on );
	QString id();
	QString name();
	
protected:
	QString	m_id, m_name;
	bool 	m_on;
	void 	paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment );
};

#endif
