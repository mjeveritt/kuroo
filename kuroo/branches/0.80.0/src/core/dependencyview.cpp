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

#include <qpainter.h>

enum Format {
		DEPENDENCY_PACKAGE,
		DEPENDENCY_USE,
		DEPENDENCY_HEADER,
		DEPENDENCY_EITHER
};

/**
 * @class DependencyItem
 * @short Subclass for formating text.
 */
class DependencyView::DependencyItem : public KListViewItem
{
public:
	DependencyItem( KListView* parent, const char* name, int index, int format );
	DependencyItem( KListViewItem* parent, const char* name, int index, int format );
	~DependencyItem();
	
protected:
	void 			paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment );
	virtual int     compare( QListViewItem* i, int col, bool ascending ) const;
	int				index() { return m_index; };
	
private:
	int				m_index, m_format;
};

DependencyView::DependencyItem::DependencyItem( KListView* parent, const char* name, int index, int format )
	: KListViewItem( parent, name ), m_index( index ), m_format( format )
{}

DependencyView::DependencyItem::DependencyItem( KListViewItem* parent, const char* name, int index, int format )
	: KListViewItem( parent, name ), m_index( index ), m_format( format )
{}

DependencyView::DependencyItem::~DependencyItem()
{}

/**
 * Order items first inserted as top-item.
 */
int DependencyView::DependencyItem::compare( QListViewItem* item, int col, bool ascending ) const
{
	int a = m_index;
	int b = dynamic_cast<DependencyItem*>( item )->index();
	
	if ( a == b )
		return 0;
	
	if ( ascending )
		return a > b ? 1 : -1;
	else
		return a < b ? -1 : 1;
}

/**
 * 
 */
void DependencyView::DependencyItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment )
{
	QColorGroup m_cg( cg );
	QFont font( p->font() );
	
	switch ( m_format ) {
		case ( DEPENDENCY_HEADER ) :
			font.setBold( true );
			break;
		
		case ( DEPENDENCY_EITHER ) :
			font.setItalic( true );
			m_cg.setColor( QColorGroup::Text, m_cg.dark() );
			break;
		
		case ( DEPENDENCY_USE ) :
			font.setItalic( true );
			m_cg.setColor( QColorGroup::Text, m_cg.dark() );
			break;
	}
	
	p->setFont( font );
	KListViewItem::paintCell( p, m_cg, column, width, alignment );
}

DependencyView::DependencyView( QWidget *parent, const char *name )
	: KListView( parent, name ), m_index( 0 ), m_parent( 0 ), m_lastDepend( 0 )
{
// 	setShowSortIndicator( true );
	addColumn( i18n( "Dependency" ) );
	setResizeMode( QListView::LastColumn );
	setSorting( -1 );
}

DependencyView::~DependencyView()
{
}

void DependencyView::clear()
{
	m_index = 0;
	KListView::clear();
}

void DependencyView::insertItem( const char* name )
{
	QString word( name );
	m_index++;
	
// 	kdDebug() << "word=" << word << "." << LINE_INFO;
	
	// Insert Depend-header
	if ( word.contains( "DEPEND=" ) ) {
		word.remove( '=' );
		m_parent = new DependencyItem( this, word, m_index, DEPENDENCY_HEADER );
		m_parent->setOpen( true );
		return;
	}
	
	// Indent one step 
	if ( word == "(" ) {
		m_parent = m_lastDepend;
		m_parent->setOpen( true );
		return;
	}
	
	// Remove one indent step
	if ( word == ")" ) {
		if ( m_parent->parent() )
			m_parent = dynamic_cast<DependencyItem*>( m_parent->parent() );
		return;
	}
	
	// OR-header
	if ( word == "||" ) {
		m_lastDepend = new DependencyItem( m_parent, i18n("Depend on either:"), m_index, DEPENDENCY_EITHER );
		m_lastDepend->setOpen( true );
		return;
	}
	
	// Insert package
	if ( word.contains( "/" ) ) {
		m_lastDepend = new DependencyItem( m_parent, word, m_index, DEPENDENCY_PACKAGE );
		return;
	}
	
	// Insert use
	word.remove( '?' );
	if ( word.startsWith("!") ) {
		word.remove( '!' );
		m_lastDepend = new DependencyItem( m_parent, i18n("With Use %1 unset:").arg( word ), m_index, DEPENDENCY_USE );
	}
	else
		m_lastDepend = new DependencyItem( m_parent, i18n("With Use %1 set:").arg( word ), m_index, DEPENDENCY_USE );
}

#include "dependencyview.moc"
