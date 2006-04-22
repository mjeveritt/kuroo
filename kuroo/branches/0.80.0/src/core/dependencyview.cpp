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
#include "dependencyview.h"

/**
 * @class DependencyItem
 * @short Subclass for formating text.
 */
class DependencyView::DependencyItem : public KListViewItem
{
public:
	DependencyItem( QListView* parent, const char* name, int index );
	
protected:
	void 	paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment );
	
private:
	int		m_index;
};

DependencyView::DependencyItem::DependencyItem( QListView* parent, const char* name, int index )
	: KListViewItem( parent ), m_index( index )
{
}

/**
 * Paint the installed version in dark green.
 */
void DependencyView::DependencyItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment )
{
// 	QColorGroup m_cg( cg );
// 	QFont font( p->font() );
// 	
// 	if ( m_isInstalled )
// 		font.setBold( true );
// 	
// 	switch ( m_stability ) {
// 	case ( TESTING ) :
// 		font.setItalic( true );
// 		break;
// 		
// 	case ( HARDMASKED ) :
// 		font.setItalic( true );
// 		m_cg.setColor( QColorGroup::Text, Qt::darkRed );
// 	}
// 	
// 	p->setFont( font );
// 	KListViewItem::paintCell( p, m_cg, column, width, alignment );
}

DependencyView::DependencyView( QWidget *parent, const char *name )
	: KListView( parent, name ), m_index( 0 )
{
	addColumn( i18n( "Dependency" ) );
	setResizeMode( QListView::LastColumn );
	setSorting( -1 );
}

DependencyView::~DependencyView()
{
}

void DependencyView::insertItem( const char* name )
{
	new DependencyItem( this, name, m_index );
}

#include "dependencyview.moc"
