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

#include <klistview.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kdebug.h>

/**
 * @class CategoriesListView
 * @short Creates category listview.
 */
CategoriesView::CategoriesView( QWidget *parent, const char *name )
	: KListView( parent, name )
{
	setSizePolicy( QSizePolicy((QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, 0, 0, sizePolicy().hasHeightForWidth()) );
	setFullWidth( true );
	setFrameShape( QFrame::NoFrame );
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
		return i18n("na");
	
	return item->text(0);
}

/**
 * Get current category idDB.
 * @return category
 */
QString CategoriesView::currentCategoryId()
{
	QListViewItem *item = this->currentItem();
	
	QMap<QString, QString>::iterator itMap = categories.find( item->text( 0 ) ) ;
	if ( itMap != categories.end() )
		return itMap.data();
	else
		return i18n( "na" );
}

/**
 * Load categories.
 * @param categoriesList
 */
void CategoriesView::loadCategories( const QStringList& categoriesList, const QStringList& allCategoriesList )
{
	QValueVector<QString> allCategories( 200 );
	int i( 1 );
	foreach ( allCategoriesList ) {
		allCategories[ i++ ] = *it;
	}
	
	categories.clear();
	foreach ( categoriesList ) {
		QString idDB( *it );
		QString name( allCategories[ idDB.toInt() ] );
		new QListViewItem( this, name );
		categories.insert( name, idDB );
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

void CategoriesListView::loadCategories( const QStringList& categoriesList )
{
	CategoriesView::loadCategories( categoriesList, KurooDBSingleton::Instance()->allCategories() );
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

void SubCategoriesListView::loadCategories( const QStringList& categoriesList )
{
	CategoriesView::loadCategories( categoriesList, KurooDBSingleton::Instance()->allSubCategories() );
	setSelected( firstChild(), true );
}

#include "categorieslistview.moc"
