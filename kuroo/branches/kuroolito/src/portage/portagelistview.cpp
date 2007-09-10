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
#include <ktextbrowser.h>

/**
 * @class PortageListView::PortageItem
 * @short Package item with all versions.
 */
PortageListView::PortageItem::PortageItem( QListView* parent, const char* name, const QString &id, const QString& category, 
                                           const QString& description, const int status )
	: PackageItem( parent, name, id, category, description, status ), m_parent( parent )
{}

/**
 * Set icons when package is visible.
 */
void PortageListView::PortageItem::paintCell( QPainter* painter, const QColorGroup& colorgroup, int column, int width, int alignment )
{
	if ( this->isVisible() )
		PackageItem::paintCell( painter, colorgroup, column, width, alignment );
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
	addColumn( i18n( "Package" ), 150 );
	addColumn( "" );
	addColumn( "", 25 );
	header()->setLabel( 2, ImagesSingleton::Instance()->icon( WORLD_COLUMN ), "" );
	setColumnAlignment( 2, Qt::AlignHCenter );

	addColumn( i18n( "Update" ), 80 );
	addColumn( i18n( "Description" ), 200 );
	
	setColumnWidthMode( 0, QListView::Manual );
	setColumnWidthMode( 1, QListView::Manual );
	setColumnWidthMode( 2, QListView::Manual );
	setColumnWidthMode( 3, QListView::Manual );
	
	setProperty( "selectionMode", "Extended" );
	setShowSortIndicator( true );
	setItemMargin( 1 );
	setRootIsDecorated( false );
	setFullWidth( true );
	
	if ( KuroolitoConfig::installedColumn() ) {
		header()->setLabel( 1, ImagesSingleton::Instance()->icon( INSTALLED_COLUMN ), "" );
		setColumnAlignment( 1, Qt::AlignHCenter );
		setColumnWidth( 1, 25 );
	}
	else
		hideColumn( 1 );
	
	header()->setResizeEnabled( false, 1 );
	header()->setResizeEnabled( false, 2 );
	header()->setResizeEnabled( false, 3 );
	
	// Create text-widget warning for "No packages found.."
	noHitsWarning = new KTextBrowser( this );
	noHitsWarning->setGeometry( QRect( 20, 50, 400, 300 ) );
	noHitsWarning->setFrameShape( QFrame::NoFrame );
	noHitsWarning->setText( i18n( "<font color=darkRed size=+1><b>No packages found with these filter settings</font><br>"
	                              "<font color=darkRed>Please modify the filter settings you have chosen!<br>"
	                              "Try to use more general filter options, so kuroo can find matching packages.</b></font>") );
}

PortageListView::~PortageListView()
{}

/**
 * Show warning text when package view is empty.
 * @param show/hide
 */
void PortageListView::showNoHitsWarning( bool noHits )
{
	if ( noHits )
		noHitsWarning->show();
	else
		noHitsWarning->hide();
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
	
	int packageCount = packageList.size() / 6;
	foreach ( packageList ) {
		QString id = *it++;
		QString name = *it++;
		QString category = *it++;
		QString description = *it++;
		QString status = *it++;
		QString update = *it;

		PortageItem* item = new PortageItem( this, name, id, category, description, status.toInt() );
		item->setText( 3, update );
		item->setText( 4, description );
		
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
