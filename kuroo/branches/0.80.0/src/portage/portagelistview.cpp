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

#include <qheader.h>
#include <qlabel.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qregexp.h>
#include <qmap.h>

#include <klistviewsearchline.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>

/**
 * Installed packages listview.
 */
PortageListView::PortageListView( QWidget* parent, const char* name )
	: PackageListView( parent, name )
{
	// Load icon
	KIconLoader *ldr = KGlobal::iconLoader();
	pxQueuedColumn = ldr->loadIcon( "kuroo_queued_column", KIcon::Small );
	
	// Setup geometry
	addColumn( i18n( "Package" ) );
	addColumn( " " );
	header()->setLabel( 1, pxQueuedColumn, " " );
	addColumn( i18n( "Update" ) );
	addColumn( i18n( "Description" ) );
	setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );
	
	setProperty( "selectionMode", "Extended" );
	setShowSortIndicator( true );	
	setItemMargin( 1 );
	setRootIsDecorated( true );
	setFullWidth( true );
	
	setColumnWidthMode( 0, QListView::Manual );
	setColumnWidthMode( 1, QListView::Manual );
	setColumnWidthMode( 2, QListView::Manual );
	setResizeMode( QListView::LastColumn );
	
	setColumnWidth( 0, 200 );
	setColumnWidth( 1, 20 );
	setColumnWidth( 2, 80 );
	
	setTooltipColumn( 4 );
}

PortageListView::~PortageListView()
{
}

/**
 * Mark package as current.
 * @param package
 */
// void PortageListView::setCurrentPackage( const QString& package )
// {
// 	clearSelection();
// 	QListViewItemIterator it( this );
// 	for ( ; it.current(); ++it )
// 		if ( package == it.current()->text(0) ) {
// 			ensureItemVisible( it.current() );
// 			setCurrentItem( it.current() );
// 			it.current()->setSelected( true );
// 			it.current()->setOpen( true );
// 			break;
// 		}
// }

/**
 * Populate listview with content of this category.
 * @param package
 */
void PortageListView::addSubCategoryPackages( const QString& categoryId, const QString& subCategoryId, int filter, const QString& text )
{
	kdDebug() << "PortageListView::addSubCategoryPackages Query-start: " << endl;
	clock_t start = clock();
	
	setSorting( -1 );
	reset();
	const QStringList packageList = PortageSingleton::Instance()->packagesInSubCategory( categoryId, subCategoryId, filter, text );
	foreach ( packageList ) {
		QString idDB = *it++;
		QString name = *it++;
		QString package = name;
		QString description = *it++;
		QString latest = *it++;
		QString meta = *it++;
		QString updateVersion = *it;
		
		Meta packageMeta;
		packageMeta.insert( i18n( "3Description" ), description );
		packageMeta.insert( i18n( "5Update" ), updateVersion );
		PackageItem *packageItem = new PackageItem( this, package, packageMeta, PACKAGE );
		
		if ( meta != FILTERALL )
			packageItem->setStatus( INSTALLED );
		
// 		if ( !keywords.contains( QRegExp("(^" + KurooConfig::arch() + "\\b)|(\\s" + KurooConfig::arch() + "\\b)") ))
// 			packageItem->setStatus(MASKED);
// 		else {
// 			if ( PortageSingleton::Instance()->isUnmasked( category + "/" + name ) )
// 				packageItem->setStatus(UNMASKED);
// 		}
		
		insertPackage( idDB, packageItem );
	}
	
	setSelected( firstChild(), true );
	setSorting( 0 );
	
	clock_t finish = clock();
	const double duration = (double) ( finish - start ) / CLOCKS_PER_SEC;
	kdDebug() << "PortageListView::addSubCategoryPackages SQL-query (" << duration << "s): " << endl;
}

/**
 * Populate listview with content of this category.
 * @param package
 */
// void PortageListView::addCategoryPackages( const QString& category, int filter  )
// {
// 	reset();
// 	const QStringList packageList = PortageSingleton::Instance()->packagesInCategory( category, filter );
// 	foreach ( packageList ) {
// 		QString idDB = *it++;
// 		QString name = *it++;
// 		QString package = name;
// 		QString description = *it++;
// 		QString latest = *it++;
// 		QString meta = *it++;
// 		QString updateVersion = *it;
// 		
// 		Meta packageMeta;
// 		packageMeta.insert( i18n( "3Description" ), description );
// 		packageMeta.insert( i18n( "5Update" ), updateVersion );
// 		PackageItem *packageItem = new PackageItem( this, package, packageMeta, PACKAGE );
// 		
// 		if ( meta != FILTERALL )
// 			packageItem->setStatus( INSTALLED );
// 		
// // 		if ( !keywords.contains( QRegExp("(^" + KurooConfig::arch() + "\\b)|(\\s" + KurooConfig::arch() + "\\b)") ))
// // 			packageItem->setStatus(MASKED);
// // 		else {
// // 			if ( PortageSingleton::Instance()->isUnmasked( category + "/" + name ) )
// // 				packageItem->setStatus(UNMASKED);
// // 		}
// 		
// 		insertPackage( idDB, packageItem );
// 	}
// 	
// 	setSelected( firstChild(), true );
// }

#include "portagelistview.moc"
