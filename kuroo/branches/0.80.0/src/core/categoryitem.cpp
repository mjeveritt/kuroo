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

#include "categoryitem.h"

#include <qpainter.h>

CategoryItem::CategoryItem( QListView* parent, const char* name, const QString &id )
	: QListViewItem( parent, name ), m_on( false ), m_id( id ), m_name( name )
{
}

void CategoryItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment )
{
	QColorGroup m_cg( cg );
	QFont font( p->font() );
	
	if ( !m_on ) {
		font.setItalic( true );
		p->setFont( font );
		m_cg.setColor( QColorGroup::Text, Qt::gray );
	}
	else {
		font.setItalic( false );
		p->setFont( font );
		m_cg.setColor( QColorGroup::Text, Qt::black );
	}
	QListViewItem::paintCell( p, m_cg, column, width, alignment );
}

void CategoryItem::setOn( bool on )
{
	m_on = on;
	repaint();
}

QString CategoryItem::id()
{
	return m_id;
}

QString CategoryItem::name()
{
	return m_name;
}
