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
#include <qpixmap.h>

class QPixmap;

/**
 * Creates category listview.
 */
class CategoriesListView : public KListView
{
Q_OBJECT
	public:
	CategoriesListView( QWidget *parent = 0, const char *name = 0 );
	~CategoriesListView();

public slots:
	
	/**
	 * Current category as "category-subcategory", for example "app-admin".
	 * @param package
	 */
	QString									currentCategory();
	
	/**
	 * Set category current.
	 * @param category
	 */
	void									setCurrentCategory( const QString& category );
	
	/**
 	 * Load categories.
 	 * With Jakob Petsovits technique for inserting items fast.
 	 * @param categoriesList 
	 */
	void									loadCategories( const QStringList& categoriesList );
	
private:
	QPixmap 								pxRepository, pxCategory;
	struct TreeViewCategory {
		QListViewItem * item;
		QMap<QString, QListViewItem*> 		subcategories;
	};
	QMap<QString, TreeViewCategory> 		categories;
};

#endif
