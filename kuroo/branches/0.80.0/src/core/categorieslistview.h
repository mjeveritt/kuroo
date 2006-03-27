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


#ifndef CATEGORIESLISTVIEW_H
#define CATEGORIESLISTVIEW_H

#include <klistview.h>
#include <qvaluevector.h>

/**
 * @class CategoriesListView
 * @short Base class for category listview.
 */
class CategoriesView : public KListView
{
Q_OBJECT
public:
	CategoriesView( QWidget *parent = 0, const char *name = 0 );
	~CategoriesView();

	class CategoryItem;
	
public slots:
	CategoryItem*							currentCategory();
	QString									currentCategoryId();
	
protected slots:
	void									storeFocus( QListViewItem* item );
	
protected:
	void									restoreFocus( bool isFiltered );
	
protected:
	
	// Category that has focus
	QString									m_focus;
	
	// Index of categoris in listview
	QDict<CategoryItem>						m_categoryIndex;

	// Vector containing all categories
	typedef QValueVector<CategoryItem*>		Categories;
	Categories			 					categories;
	
signals:
	void									categoriesChanged();
};

/**
 * @class CategoriesListView
 * @short Categories listview.
 */
class CategoriesListView : public CategoriesView
{
Q_OBJECT
public:
	CategoriesListView( QWidget *parent = 0, const char *name = 0 );
	~CategoriesListView();
	
	void									init();
	void 									loadCategories( const QStringList& categoriesList, bool isFiltered );
};

/**
 * @class SubCategoriesListView
 * @short Subcategories listview.
 */
class SubCategoriesListView : public CategoriesView
{
Q_OBJECT
public:
	SubCategoriesListView( QWidget *parent = 0, const char *name = 0 );
	~SubCategoriesListView();
	
	void									init();
	void 									loadCategories( const QStringList& categoriesList );
	
private:
	
	// Vector containing all sub-categories
	typedef std::map<int, QString>			SubCategory;
	typedef std::vector<SubCategory>		AllSubCategories;
	AllSubCategories			 			allSubCategories;
};

#endif
