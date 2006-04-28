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
		DEPENDENCY_OPERATOR
};

// capture positions inside the regexp. (like m_rxAtom.cap(POS_CALLSIGN))
enum Positions {
		POS_CALLSIGN = 1,
		POS_PREFIX,
		POS_CATEGORY,
		POS_SUBCATEGORY,
		POS_PACKAGE,
		POS_VERSION
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
 * Format dependency-items nicely.
 */
void DependencyView::DependencyItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment )
{
	QColorGroup m_cg( cg );
	QFont font( p->font() );
	
	switch ( m_format ) {
		case ( DEPENDENCY_HEADER ) :
			font.setBold( true );
			break;
		
		case ( DEPENDENCY_OPERATOR ) :
			font.setItalic( true );
			m_cg.setColor( QColorGroup::Text, m_cg.dark() );
			break;
		
		case ( DEPENDENCY_USE ) :
			font.setItalic( true );
			m_cg.setColor( QColorGroup::Text, m_cg.dark() );
			break;
		
// 		case ( DEPENDENCY_PACKAGE ) :
// 			font.setUnderline( true );
// 			m_cg.setColor( QColorGroup::Text, m_cg.link() );
// 			break;
	}
	
	p->setFont( font );
	KListViewItem::paintCell( p, m_cg, column, width, alignment );
}

/**
 * @class DependencyView
 * @short Listview to build dependency-tree view.
 */
DependencyView::DependencyView( QWidget *parent, const char *name )
	: KListView( parent, name ),
	rxAtom(	
			"^"    										// Start of the string
			"(!)?" 										// "Block these packages" flag, only occurring in ebuilds
			"(~|(?:<|>|=|<=|>=))?" 						// greater-than/less-than/equal, or "all revisions" prefix
			"((?:[a-z]|[0-9])+)-((?:[a-z]|[0-9])+)/"   	// category and subcategory
			"((?:[a-z]|[A-Z]|[0-9]|-|\\+|_)+)" 			// package name
			"("           								// start of the version part
			"(?:-\\d*(?:\\.\\d+)*[a-z]?)" 				// base version number, including wildcard version matching (*)
			"(?:_(?:alpha|beta|pre|rc|p)\\d*)?" 		// version suffix
			"(?:-r\\d*)?"  								// revision
			"\\*?)?$"          							// end of the (optional) version part and the atom string
		)
{
	addColumn( i18n( "Dependency" ) );
	setResizeMode( QListView::LastColumn );
	setSorting( -1 );
	
// 	connect( this, SIGNAL( executed( QListViewItem* ) ), this, SLOT( slotPackageClicked( QListViewItem* ) ) );
}

DependencyView::~DependencyView()
{
}

/**
 * Forward signal when user click on package.
 */
void DependencyView::slotPackageClicked( QListViewItem* item )
{
	QString atom( item->text(0) );
	
	kdDebug() << "atom=" << atom << LINE_INFO;
	
	// Do the regexp match, which also prepares for text capture
	if ( !rxAtom.exactMatch( atom ) )
		return;
	
	QString package	= rxAtom.cap( POS_PACKAGE );
	QString category = rxAtom.cap( POS_CATEGORY ) + "-" + rxAtom.cap( POS_SUBCATEGORY );
	
	kdDebug() << "package=" << package << LINE_INFO;
	
	SignalistSingleton::Instance()->packageClicked( category + " " + package );
}

/**
 * Populate the tree with all dependencies.
 * @param	list of depend atoms
 */
void DependencyView::insertDependAtoms( const QStringList& dependAtomsList )
{
	kdDebug() << "dependAtomsList=" << dependAtomsList << LINE_INFO;
	int index( 0 );
	DependencyItem	*parent, *lastDepend;
	QString lastWord;
	
	foreach ( dependAtomsList ) {
		QString word( *it );
		index++;
		
		kdDebug() << "word=" << word << LINE_INFO;
		
		// Insert Depend-header
		if ( word == "DEPEND=" ) {
			parent = new DependencyItem( this, i18n("Compile-time dependencies"), index, DEPENDENCY_HEADER );
			parent->setOpen( true );
			continue;
		}
		
		if ( word == "RDEPEND=" ) {
			parent = new DependencyItem( this, i18n("Runtime dependencies"), index, DEPENDENCY_HEADER );
			parent->setOpen( true );
			continue;
		}
		
		if ( word == "PDEPEND=" ) {
			parent = new DependencyItem( this, i18n("Post-merge dependencies"), index, DEPENDENCY_HEADER );
			parent->setOpen( true );
			continue;
		}
		
		// Safety check
		if ( !parent )
			continue;
			
		// Indent one step 
		if ( word == "(" ) {
			if ( word != lastWord )
				parent = lastDepend;
			else
				parent = new DependencyItem( parent, parent->text(0), index, DEPENDENCY_OPERATOR );
			
			parent->setOpen( true );
			lastWord = word;
			continue;
		}
		lastWord = word;
		
		// Remove one indent step
		if ( word == ")" ) {
			if ( parent->parent() )
				parent = dynamic_cast<DependencyItem*>( parent->parent() );
			continue;
		}
		
		// OR-header
		if ( word == "||" ) {
			lastDepend = new DependencyItem( parent, i18n("Depend on either:"), index, DEPENDENCY_OPERATOR );
			lastDepend->setOpen( true );
			continue;
		}
		
		// Insert package
		if ( word.contains( "/" ) ) {
			lastDepend = new DependencyItem( parent, word, index, DEPENDENCY_PACKAGE );
			continue;
		}
		
		// Insert use
		word.remove( '?' );
		if ( word.startsWith("!") ) {
			word.remove( '!' );
			lastDepend = new DependencyItem( parent, i18n("With Use %1 unset:").arg( word ), index, DEPENDENCY_USE );
		}
		else
			lastDepend = new DependencyItem( parent, i18n("With Use %1 set:").arg( word ), index, DEPENDENCY_USE );
	}
	DEBUG_LINE_INFO;
	setSorting( 0, Qt::Descending );
	sort();
}

#include "dependencyview.moc"
