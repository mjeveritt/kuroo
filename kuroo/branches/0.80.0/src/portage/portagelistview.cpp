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
#include "portagelistview.h"
#include "packageitem.h"
#include "tooltip.h"
#include "packagelistview.h"
#include "packageversion.h"
#include "dependatom.h"

#include <qheader.h>
#include <qmap.h>

#include <kconfig.h>
#include <kmessagebox.h>

static int packageCount( 0 );

/**
 * @class PortageListView::PortageItem
 * @short Package item with all versions.
 */
PortageListView::PortageItem::PortageItem( QListView* parent, const char* name, const QString &id, const QString& description, const QString& status, const QString& homepage )
	: PackageItem( parent, name, id, description, status ), m_homepage( homepage )
{
}

/**
 * Accessor for homepage.
 * @return the package homepage.
 */
QString PortageListView::PortageItem::homepage()
{
	return m_homepage;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @class PortageListView
 * @short All packages listview.
 */
PortageListView::PortageListView( QWidget* parent, const char* name )
	: PackageListView( parent, name )
{
	// Setup geometry
	addColumn( i18n( "Package" ) );
	
	addColumn( " " );
	header()->setLabel( 1, ImagesSingleton::Instance()->icon( QUEUED_COLUMN ), " " );
	
	addColumn( i18n( "Update" ) );
	addColumn( i18n( "Description" ) );
	
	setProperty( "selectionMode", "Extended" );
	setShowSortIndicator( true );
	setItemMargin( 1 );
	setRootIsDecorated( false );
	setFullWidth( true );
	
	setColumnWidthMode( 0, QListView::Manual );
	setColumnWidthMode( 1, QListView::Manual );
	setColumnWidthMode( 2, QListView::Manual );
	setResizeMode( QListView::LastColumn );
	
	setColumnWidth( 0, 150 );
	setColumnWidth( 1, 25 );
	setColumnWidth( 2, 80 );
	
	setTooltipColumn( 3 );
	
	if ( KurooConfig::installedColumn() ) {
		addColumn( " " );
		header()->setLabel( 4, ImagesSingleton::Instance()->icon( INSTALLED_COLUMN ), " " );
		setColumnAlignment( 4, Qt::AlignHCenter );
		header()->moveSection( 4, 1 );
	}
}

PortageListView::~PortageListView()
{
}

/**
 * Current package with focus.
 * @return name
 */
PortageListView::PortageItem* PortageListView::currentPortagePackage()
{
	return dynamic_cast<PortageItem*>( PackageListView::currentPackage() );
}

/**
 * View package total in package name section header.
 * @param total
 */
void PortageListView::setHeader( const QString& total )
{
	if ( !total.isEmpty() )
		header()->setLabel( 0, i18n("Package") + " (" + total + ")" );
	else
		header()->setLabel( 0, i18n("Package") );
}

/**
 * Populate listview with content of this category.
 * @param package
 */
int PortageListView::addSubCategoryPackages( const QStringList& packageList )
{
// 	clock_t start = clock();
	
	// Store package focus
	QString currentId = this->currentId();

	// Disable sorting for faster inserting. Packages are already sorted alfabetically.
	setSorting( -1 );
	resetListView();
	setHeader( QString::null );
	
	// Don't load all packages, only first ROWLIMIT
	packageCount = packageList.size() / 6;
	
	foreach ( packageList ) {
		QString id = *it++;
		QString name = *it++;
		QString description = *it++;
		QString meta = *it++;
		QString update = *it++;
		QString homepage = *it;
		
		PortageItem* item = new PortageItem( this, name, id, description, meta, homepage );
		item->setText( 2, update );
		item->setText( 3, description );
		
		if ( meta == FILTER_ALL_STRING )
			item->setStatus( PACKAGE );
		else
			item->setStatus( INSTALLED );
		
		indexPackage( id, item );
	}
	setSorting( 0 );
	setHeader( QString::number( packageCount ) );
	setPackageFocus( currentId );
	
	// Cannot have current changed for only one package so emit manually
	if ( packageCount == 1 )
		emit currentChanged( 0 );
	
// 	clock_t finish = clock();
// 	const double duration = (double) ( finish - start ) / CLOCKS_PER_SEC;
// 	kdDebug() << "PortageListView::addSubCategoryPackages SQL-query (" << duration << "s): " << endl;
	
	return packageCount;
}

#include "portagelistview.moc"
