/***************************************************************************
 *	Copyright (C) 2004 by karye												*
 *	karye@users.sourceforge.net												*
 *																			*
 *	This program is free software; you can redistribute it and/or modify	*
 *	it under the terms of the GNU General Public License as published by	*
 *	the Free Software Foundation; either version 2 of the License, or		*
 *	(at your option) any later version.										*
 *																			*
 *	This program is distributed in the hope that it will be useful,			*
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of			*
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the			*
 *	GNU General Public License for more details.							*
 *																			*
 *	You should have received a copy of the GNU General Public License		*
 *	along with this program; if not, write to the							*
 *	Free Software Foundation, Inc.,											*
 *	59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.				*
 ***************************************************************************/

#include "common.h"
#include "categorieslistview.h"

#include <qpainter.h>
#include <QTreeWidget>

#include <kglobal.h>

CategoriesView::CategoryItem::CategoryItem( QTreeWidget* parent, const QString& name, const QString &id )
: QTreeWidgetItem( parent, QStringList(name) ), m_id( id ), m_name( name ), m_on( false )
{}

/**
 * Paint empty categories in italic and grey when empty.
 */
void CategoriesView::CategoryItem::paintCell( QPainter *p, int column, int width, int alignment )
{
	QFont font( p->font() );

	if ( !m_on ) {
		font.setItalic( true );
		p->setFont( font );
		//m_cg.setColor( QColorGroup::Text, m_cg.dark() );
	}
	else {
		font.setItalic( false );
		p->setFont( font );
		//m_cg.setColor( QColorGroup::Text, m_cg.text() );
	}

	setTextAlignment( 0, alignment );
	//QTreeWidgetItem::paintCell( p, m_cg, column, width, alignment );
}

/**
 * @class CategoriesListView
 * @short Base class for category listview.
 */
CategoriesView::CategoriesView( QWidget *parent, const char *name )
: QTreeWidget( parent/*, name */), m_focus( i18n("All") ), categories( 0 )
{
	//setFullWidth( true );
	setFrameShape( QFrame::NoFrame );
	setSortingEnabled( false );
	//kdDebug() << "CategoriesView.Constructor minimumWidth=" << minimumWidth()
	//			<< "actual width" << width() << LINE_INFO;

	connect( this, SIGNAL( currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*) ), this, SLOT( slotStoreFocus( QTreeWidgetItem*,QTreeWidgetItem* ) ) );
}

CategoriesView::~CategoriesView()
{}

/**
 * Retreive focus category when a new category is made current.
 * @categoryItem*
 */
void CategoriesView::slotStoreFocus( QTreeWidgetItem* current, QTreeWidgetItem* previous )
{
	//WARN: This shouldn't ever be null, need to fix
	if (NULL != current)
	{
		m_focus = current->text(0);
	}
}

/**
 * Retreive last focus category from internal index, and making that category current.
 * bool is this triggered by filter action?
 */
void CategoriesView::restoreFocus( const bool& isFiltered )
{
	QMultiHash<QString, CategoryItem>::iterator focusCategory = m_categoryIndex.find( m_focus );
	if( focusCategory == m_categoryIndex.end() ) {
		focusCategory = m_categoryIndex.begin(); //firstChild()
	}

	setCurrentItem( &focusCategory.value() );
	setItemSelected( &focusCategory.value(), true );

	// Emit manually 'currentChanged' if triggered by filter
	/*if ( isFiltered ) {
		emit currentChanged( &focusCategory.value() );
	}*/

}

/**
 * Get current category idDB.
 * @return category
 */
const QString CategoriesView::currentCategoryId() const
{
	CategoryItem* item = currentCategory();
	if ( item )
		return item->id();
	else
		return "0";
}


/////////////////////////////////////////////////////////////////////////////////
// ListView stuff
/////////////////////////////////////////////////////////////////////////////////

/**
 * @class CategoriesListView
 * @short Categories listview.
 *
 * Specialized listview for viewing categories.
 * First all available categories are inserted.
 * When a category has no subcategory it is marked in gray and italic = off.
 */
CategoriesListView::CategoriesListView( QWidget *parent, const char *name )
	: CategoriesView( parent, name )
{
	setHeaderLabels( QStringList( i18n( "Category" ) ) );
	//header()->setLabel( header()->count() - 1, i18n("Category") );
	//kdDebug() << "CategoriesListView.constructor minimumWidth=" << minimumWidth()
	//			<< "actual width" << width() << LINE_INFO;
}

CategoriesListView::~CategoriesListView()
{}

/**
 * Create index of all categories name by db id.
 * Insert them in listview.
 */
void CategoriesListView::init()
{
	kdDebug() << "CategoriesListView.init minimumWidth=" << minimumWidth()
				<< "actual width=" << width() << LINE_INFO;

	categories.clear();
	clear();

	// Get all available categories (including "All" meta category)
	const QStringList allCategoriesList = KurooDBSingleton::Instance()->allCategories();
	categories.resize( allCategoriesList.size() );

	// Insert the meta-category All first as id = 0
	m_categoryIndex.clear();
	CategoryItem* item = new CategoryItem( this, i18n("All"), "0" );
	m_categoryIndex.insert( i18n("All"), *item );
	item->setOn( true );
	categories[0] = item;

	int i = 1;
	// We have to skip the first since it's the "All" meta category, and we've already added it manually
	for( QStringList::ConstIterator it = allCategoriesList.begin() + 1, end = allCategoriesList.end(); it != end; ++it ) {
		item = new CategoryItem( this, *it, QString::number( i ) );
		categories[i] = item;
		m_categoryIndex.insert( *it, *item );
		i++;
	}

	//TODO: these don't work we need a better solution
	//resizeColumnToContents( 0 );
	//setColumnWidth( 0, 75 );
}

/**
 * View available categories.
 * @param categoriesList list category id
 */
void CategoriesListView::loadCategories( const QStringList& categoriesList, bool isFiltered )
{
	// Set all categories off = empty
	for ( Categories::iterator it = categories.begin() + 1; it != categories.end(); ++it )
		(*it)->setOn( false );

	// Enable found categories from query
	foreach ( QString cat, categoriesList )
		categories[ cat.toInt() ]->setOn( true );

	// After all categories are loaded try restoring last known focus-category
	restoreFocus( isFiltered );
}

/**
 * @class SubCategoriesListView
 * @short Subcategories listview.
 *
 * Specialized listview to view available subcategories and marking empty as off.
 */
SubCategoriesListView::SubCategoriesListView( QWidget *parent, const char *name )
	: CategoriesView( parent, name )
{
	setHeaderLabels( QStringList( i18n( "Subcategory" ) ) );
}

SubCategoriesListView::~SubCategoriesListView()
{}

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
	allSubCategories[0].insert( 0, i18n("All") );

	// Insert all in matrix
	for(QStringList::const_iterator it = allCategoriesList.constBegin(); it != allCategoriesList.constEnd(); ++it) {
		int idCategory = (*it++).toInt();
		int idSubCategory = (*it++).toInt();
		QString name = *it;
		allSubCategories[idCategory].insert( idSubCategory, name );
	}
}

/**
 * View available subcategories.
 * @param categoriesList list category id
 */
void SubCategoriesListView::loadCategories( const QStringList& categoriesList )
{
	// Get the category id
	static int idCategory( -1 );
	if ( !categoriesList.isEmpty() && idCategory != categoriesList.first().toInt() )
		idCategory = categoriesList.first().toInt();

	clear(); // @warning: categoryItem cannot be used anymore
	m_categoryIndex.clear();

	// Insert meta-subcategory
	CategoryItem* item = new CategoryItem( this, i18n("All"), "0" );
	m_categoryIndex.insert( i18n("All"), *item );

	// When meta-category is selected skip to show only meta-subcategory
	if ( idCategory != 0 ) {
		categories[0] = item;	//Meta sub-category
		QMapIterator<int, QString> it(allSubCategories[idCategory]);
		while (it.hasNext()) {
			it.next();
			QString id = QString::number( it.key() );
			QString name = it.value();

			// Skip empty subcategory
			if ( !name.isEmpty() ) {
				CategoryItem* item = new CategoryItem( this, name, id );
				categories[it.key()] = item;
				m_categoryIndex.insert( name, *item );
			}
		}

		// Enable subcategories from query. Skip first which is the category
		for( QStringList::ConstIterator it = ++( categoriesList.begin() ), end = categoriesList.end(); it != end; ++it ) {
			if ( categories[(*it).toInt()] )
				categories[(*it).toInt()]->setOn( true );
		}
	}
	else {
		item->setOn( true );
	}

	//TODO: this doesn't work, we need to find a better solution
	//resizeColumnToContents( 0 );

	// After all categories are loaded try restoring last known focus-category
	restoreFocus( false );
}

#include "categorieslistview.moc"
