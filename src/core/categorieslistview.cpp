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
CategoriesListView::CategoriesListView( QWidget *parent, const char *name )
	: KListView( parent, name )
{
	// Load icons for category, package ...
	KIconLoader *ldr = KGlobal::iconLoader();
	pxCategory = ldr->loadIcon("kuroo_category", KIcon::Small);
	pxRepository = ldr->loadIcon("kuroo_repository", KIcon::NoGroup, KIcon::SizeSmallMedium, KIcon::DefaultState, NULL, true);
		
	addColumn(i18n("Category"));
	header()->setLabel(header()->count() - 1, i18n("Category"));
	setSizePolicy(QSizePolicy((QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, 0, 0, sizePolicy().hasHeightForWidth()));
	setMinimumSize(QSize(150, 0));
	setShowSortIndicator(true);
	setRootIsDecorated(true);
	setFullWidth(true);
}

CategoriesListView::~CategoriesListView()
{
}

/**
 * Get current category.
 * @return category
 */
QString CategoriesListView::currentCategory()
{
	QListViewItem *item = this->currentItem();
	
	if ( !item || !item->parent() )
		return i18n("na");
	
	return item->parent()->text(0) + "-" + item->text(0);
}

/**
 * Set category current.
 * @param category
 */
void CategoriesListView::setCurrentCategory( const QString& category )
{
	QString categoryName = category.section("-", 0, 0);
	QString subcategoryName = category.section("-", 1, 1);
	
	if( categories.contains(categoryName) ) {
		if ( categories[categoryName].subcategories.contains(subcategoryName) ) {
			QListViewItem *item = categories[categoryName].subcategories[subcategoryName];
			ensureItemVisible(item);
			setCurrentItem(item);
			item->setSelected(true);
		}
	}
}

/**
 * Load categories.
 * From Jakob Petsovits solution for inserting items fast.
 * @param categoriesList 
 */
void CategoriesListView::loadCategories( const QStringList& categoriesList )
{
	QListViewItem *catItem, *subcatItem;
	
	categories.clear();
	clear();
	setRootIsDecorated(true);

	foreach ( categoriesList ) {
		QString categoryName = (*it).section("-", 0, 0);
		QString subcategoryName = (*it).section("-", 1, 1);
		
		if( !categories.contains(categoryName) ) {
			catItem = new KListViewItem( this, categoryName );
			catItem->setExpandable(true);
			catItem->setPixmap(0, pxCategory);
			categories[categoryName].item = catItem;
		}
		
		if( !categories[categoryName].subcategories.contains(subcategoryName) ) {
			subcatItem = new KListViewItem(categories[categoryName].item, subcategoryName);
			subcatItem->setPixmap(0, pxCategory);
			categories[categoryName].subcategories[subcategoryName] = subcatItem;
		}
	}
}

#include "categorieslistview.moc"
