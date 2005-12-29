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
#include <qmap.h>

#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>

static int packageCount( 0 );

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
	
	setTooltipColumn( 3 );
	
	// @fixme: How to check if bottom of listview is reached
// 	connect( this, SIGNAL( verticalSliderPressed() ), this, SLOT( slotLastPackage() ) );
	
// 	connect( this, SIGNAL( contentsMoving( int, int ) ), this, SLOT( slotNewItem( int, int ) ) );
}

PortageListView::~PortageListView()
{
}

void PortageListView::setHeader( const QString& text )
{
	if ( !text.isEmpty() )
		header()->setLabel( 0, i18n("Package") + " (" + text + ")" );
	else
		header()->setLabel( 0, i18n("Package") );
}

/**
 * Populate listview with content of this category.
 * @param package
 */
void PortageListView::addSubCategoryPackages( const QStringList& packageList )
{
// 	clock_t start = clock();
	
// 	kdDebug() << "PortageListView::addSubCategoryPackages packageList=" << packageList << endl;
	
	// Disable sorting for faster inserting. Packages are already sorted alfabetically.
	setSorting( -1 );
	reset();
	setHeader( "" );
	
	// Don't load all packages, only first ROWLIMIT
	packageCount = packageList.size() / 6;
	int max( ( packageCount - ROWLIMIT ) * 6 ) ;
	
	foreach ( packageList ) {
		
		// Since packages are loaded in reverse alfabetical order, skip all except last ROWLIMIT
		if ( max-- > 0 )
			continue;
		
		QString id = *it++;
		QString name = *it++;
		QString description = *it++;
		QString meta = *it++;
		QString updateVersion = *it++;
		QString homepage = *it;
		
		PackageItem *packageItem = new PackageItem( this, id, name, description, homepage, meta );
	}
	setSorting( 0 );
	setCurrentItem( firstChild() );
	setSelected( firstChild(), true );
	setHeader( QString::number( packageCount ) );
	
// 	clock_t finish = clock();
// 	const double duration = (double) ( finish - start ) / CLOCKS_PER_SEC;
// 	kdDebug() << "PortageListView::addSubCategoryPackages SQL-query (" << duration << "s): " << endl;
}

/**
 * Check if last package is visible and inform that not all packages are listed.
 */
// void PortageListView::slotLastPackage()
// {
// 	if ( packageCount > ROWLIMIT && lastItem()->isVisible() ) {
// 		KMessageBox::information( 0, i18n("Only %1 packages are visible. Please use filters to browse for packages!").arg( QString::number( ROWLIMIT ) ), i18n("Information"), "dontAskAgainOverflow" );
// 	}
// }

// void PortageListView::slotNewItem( int x, int y )
// {
// 	QListViewItem* item = itemAt( QPoint( 0, visibleHeight() ) );
// 	if ( item )
// 		kdDebug() << "PortageListView::slotNewItem name=" << item->text(0) << endl;
// }

#include "portagelistview.moc"
