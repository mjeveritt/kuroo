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
#include "categorieslistview.h"

#include <qheader.h>
#include <qlabel.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qregexp.h>
#include <qpainter.h>

#include <klistview.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kdebug.h>

/**
 * @class CategoriesView::CategoryItem
 * @short Highlight category header with bold darkGray
 */
class CategoriesView::CategoryItem : public QListViewItem
{
public:
	CategoryItem::CategoryItem( QListView* parent, const char* name, const QString &id );
	
	void 	setOn( bool on );
	QString id();
	
protected:
	QString	m_id;
	bool 	m_on;
	void 	paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment );
};

CategoriesView::CategoryItem::CategoryItem( QListView* parent, const char* name, const QString &id )
	: QListViewItem( parent, name ), m_on( true ), m_id( id )
{
}

void CategoriesView::CategoryItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment )
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

void CategoriesView::CategoryItem::setOn( bool on )
{
	m_on = on;
	repaint();
}

QString CategoriesView::CategoryItem::id()
{
	return m_id;
}

/**
 * @class CategoriesListView
 * @short Creates category listview.
 */
CategoriesView::CategoriesView( QWidget *parent, const char *name )
	: KListView( parent, name ), categories( 0 )
{
	setSizePolicy( QSizePolicy((QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, 0, 0, sizePolicy().hasHeightForWidth()) );
	setFullWidth( true );
	setFrameShape( QFrame::NoFrame );
	setSorting( -1 );
}

CategoriesView::~CategoriesView()
{
}

/**
 * Get current category.
 * @return category
 */
QString CategoriesView::currentCategory()
{
	QListViewItem *item = this->currentItem();
	if ( !item )
		return i18n( "na" );
	
	return item->text( 0 );
}

/**
 * Get current category idDB. @fixme: use listview key
 * @return category
 */
QString CategoriesView::currentCategoryId()
{
	return dynamic_cast<CategoryItem*>( this->currentItem() )->id();
}

/**
 * Mark package as current. @fixme: optimze with QMap
 * @param package
 */
void CategoriesView::setCurrentCategory( const QString& package )
{
	clearSelection();
	QListViewItemIterator it( this );
	for ( ; it.current(); ++it )
		if ( package == it.current()->text(0) ) {
			ensureItemVisible( it.current() );
			setCurrentItem( it.current() );
			it.current()->setSelected( true );
			break;
		}
}


/////////////////////////////////////////////////////////////////////////////////

/**
 * @class CategoriesListView
 * @short Creates category listview.
 */
CategoriesListView::CategoriesListView( QWidget *parent, const char *name )
	: CategoriesView( parent, name )
{
	addColumn( i18n( "Category" ) );
	header()->setLabel( header()->count() - 1, i18n("Category") );
}

CategoriesListView::~CategoriesListView()
{
}

void CategoriesListView::init()
{
	kdDebug() << "CategoriesListView::init" << endl;
	
	CategoryItem* item( new CategoryItem( this, "All", "0" ) );
	categories.push_back( item );
	
	const QStringList allCategoriesList = KurooDBSingleton::Instance()->allCategories();
	int i( 1 );
	foreach ( allCategoriesList ) {
		item = new CategoryItem( this, *it, QString::number( i++ ) );
		categories.push_back( item );
	}
	setSorting( 0 );
}

void CategoriesListView::loadCategories( const QStringList& categoriesList )
{
	// Set all categories off = empty
	for ( Categories::iterator it = categories.begin() + 1; it != categories.end(); ++it ) {
		(*it)->setOn( false );
	}
	
	// Enable categories from query
	foreach ( categoriesList ) {
		int id = (*it).toInt();
		categories[id]->setOn( true );
	}

	setSelected( firstChild(), true );
}


/**
 * @class SubCategoriesListView
 * @short Creates category listview.
 */
SubCategoriesListView::SubCategoriesListView( QWidget *parent, const char *name )
	: CategoriesView( parent, name )
{
	addColumn( i18n( "Subcategory" ) );
	header()->setLabel( header()->count() - 1, i18n( "Subcategory" ) );
}

SubCategoriesListView::~SubCategoriesListView()
{
}

void SubCategoriesListView::init()
{
	allSubCategories.push_back( "All" );
	
	const QStringList allCategoriesList = KurooDBSingleton::Instance()->allSubCategories();
	foreach ( allCategoriesList ) {
		allSubCategories.push_back( *it );
	}
}

void SubCategoriesListView::loadCategories( const QStringList& categoriesList )
{
	categories.clear();
	clear();
	CategoryItem* item;
	
	foreach ( categoriesList ) {
		QString idDB( *it );
		QString name( allSubCategories[ idDB.toInt() ] );
		item = new CategoryItem( this, name, idDB );
		categories.push_back( item );
	}
	
	setSelected( firstChild(), true );
}

#include "categorieslistview.moc"
