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
#include "categoryitem.h"

#include <qheader.h>
#include <qlabel.h>
#include <qimage.h>
#include <qpixmap.h>

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
// QString CategoriesView::currentCategory()
// {
// 	QListViewItem *item = this->currentItem();
// 	if ( !item )
// 		return i18n( "na" );
// 	
// 	return item->text( 0 );
// }

CategoryItem* CategoriesView::currentCategory()
{
	return dynamic_cast<CategoryItem*>( this->currentItem() );
}


/**
 * Get current category idDB.
 * @return category
 */
QString CategoriesView::currentCategoryId()
{
	CategoryItem* item = dynamic_cast<CategoryItem*>( this->currentItem() );
	if ( item )
		return item->id();
	else
		return "0";
}

/**
 * Mark package as current.
 * @param package
 */
void CategoriesView::setCurrentCategoryId( const QString& id )
{
	setCurrentItem( categories[id.toInt()] );
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

/**
 * Create index of all categories name by db id.
 * Insert them in listview.
 */
void CategoriesListView::init()
{
	categories.clear();
	clear();
	
	const QStringList allCategoriesList = KurooDBSingleton::Instance()->allCategories();
	int i = allCategoriesList.size() - 1;
	categories.resize( i + 1 );
	
	// Insert categories in reverse order to get them in alfabetical order
	CategoryItem* item;
	for( QStringList::ConstIterator it = --( allCategoriesList.end() ), end = allCategoriesList.begin(); it != end; --it ) {
		item = new CategoryItem( this, *it, QString::number( i ) );
		categories[i] = item;
		i--;
	}
	
	// Insert the meta-category All on top as id = 0
	item = new CategoryItem( this, "All", "0" );
	item->setOn( true );
	categories[0] = item;
	setSelected( firstChild(), true );
}

/**
 * View available categories.
 * @param categoriesList list category id
 */
void CategoriesListView::loadCategories( const QStringList& categoriesList )
{
	// Set all categories off = empty
	for ( Categories::iterator it = categories.begin() + 1; it != categories.end(); ++it ) {
		(*it)->setOn( false );
	}
	
	// Enable found categories from query
	foreach ( categoriesList ) {
		categories[(*it).toInt()]->setOn( true );
	}
	
	emit selectionChanged();
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

/**
 * Create index of all subcategories name by db id.
 */
void SubCategoriesListView::init()
{
	categories.clear();
	allSubCategories.clear();
	
	const QStringList allCategoriesList = KurooDBSingleton::Instance()->allSubCategories();
	int size = allCategoriesList.size() / 3 + 1;

	// Prepend the meta-category All at id = 0
	allSubCategories.resize( size );
	categories.resize( size );
	allSubCategories[0].insert( std::pair<int, QString>(0, "All") );
	
	// Insert all in matrix
	foreach ( allCategoriesList ) {
		int idCategory = (*it++).toInt();
		int idSubCategory = (*it++).toInt();
		QString name = *it;
		allSubCategories[idCategory].insert( std::pair<int, QString>(idSubCategory, name) );
	}
}

/**
 * View available subcategories.
 * @param categoriesList list category id
 */
void SubCategoriesListView::loadCategories( const QStringList& categoriesList )
{
	// Retreive focus
	QString currentId = currentCategoryId();
	bool isSameCategory( false );
	
	// Get the category id
	static int idCategory( -1 );
	if ( idCategory != categoriesList.first().toInt() ) {
		idCategory = categoriesList.first().toInt();
		isSameCategory = false;
	}
	else
		isSameCategory = true;
	
	clear(); // @warning: categoryItem cannot be used anymore
	CategoryItem* item;
	item = new CategoryItem( this, "", "0" ); // Insert empty item to get focus to work on last before last item
	
	// When meta-category is selected skip to show only meta-subcategory
	if ( idCategory != 0 ) {
	
		// Insert all subcategories in reverse order to get them alfabetically listed, and set them off = empty
		for ( SubCategory::reverse_iterator it = allSubCategories[idCategory].rbegin(); it != allSubCategories[idCategory].rend(); ++it ) {
			QString id = QString::number( (*it).first );
			QString name = (*it).second;

			// Skip empty subcategory
			if ( !name.isEmpty() ) {
				item = new CategoryItem( this, name, id );
				categories[(*it).first] = item;
			}
		}
		
		// Insert meta-subcategory
		item = new CategoryItem( this, "All", "0" );
		categories[0] = item;
		
		// Enable subcategories from query. Skip first which is the category
		for( QStringList::ConstIterator it = ++( categoriesList.begin() ), end = categoriesList.end(); it != end; ++it ) {
			if ( categories[(*it).toInt()] )
				categories[(*it).toInt()]->setOn( true );
		}
	}
	else {
	
		// Insert meta-subcategory
		item = new CategoryItem( this, "All", "0" );
		item->setOn( true );
	}
	
	// Restore focus
	if ( isSameCategory && currentId != "0" )
		setCurrentCategoryId( currentId );
	else
		setCurrentItem( firstChild() );
}

#include "categorieslistview.moc"
