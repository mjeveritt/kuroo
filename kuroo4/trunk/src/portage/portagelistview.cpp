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

#include <QMap>
#include <QTreeWidget>

#include <kconfig.h>
#include <kmessagebox.h>
#include <ktextbrowser.h>

/**
 * @class PortageListView::PortageItem
 * @short Package item with all versions.
 */
PortageListView::PortageItem::PortageItem( QTreeWidget* parent, const QString& name, const QString &id, const QString& category,
										   const QString& description, const int status )
	: PackageItem( parent, name, id, category, description, status ), m_parent( parent )
{
	if ( !this->isHidden() && QueueSingleton::Instance()->isQueued( id ) )
		setQueued( true );
}

/**
 * Set icons when package is visible.
 */
void PortageListView::PortageItem::paintCell( QPainter* painter, const QPalette& palette, int column, int width, int alignment )
{
	if ( !this->isHidden() ) {

		if ( column == 3 ) {
			if ( QueueSingleton::Instance()->isQueued( id() ) ) {
				setQueued( true );
				setIcon( 3, KIcon("kuroo_queued") );
			}
			else {
				setQueued( false );
				setIcon( 3, KIcon("kuroo_empty") );
			}
		}

		PackageItem::paintCell( painter, palette, column, width, alignment );
	}
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
	//QTreeWidgetItem *header = headerItem();
	headerItem()->setText( 0, i18n( "Package" ) );
	//header.setText( 0, i18n( "Package" ) );
	headerItem()->setText( 1, "" );
	//header.setText( 2, "" );
	headerItem()->setIcon( 2, KIcon("kuroo_world_column") );
	//header.setTextAlignment( 2, Qt::AlignHCenter );
	headerItem()->setIcon( 3, KIcon("kuroo_queued_column") );
	//setColumnAlignment( 2, Qt::AlignHCenter );
	headerItem()->setText( 4, i18n( "Update" ) );
	headerItem()->setText( 5, i18n( "Description" ) );

	//setHeaderItem( &header );

	/*setColumnWidthMode( 0, QTreeWidget::Manual );
	setColumnWidthMode( 1, QTreeWidget::Manual );
	setColumnWidthMode( 2, QTreeWidget::Manual );
	setColumnWidthMode( 3, QTreeWidget::Manual );
	setColumnWidthMode( 4, QTreeWidget::Manual );*/

	setProperty( "selectionMode", "Extended" );
	//setShowSortIndicator( true );
	//setItemMargin( 1 );
	setRootIsDecorated( false );
	//setFullWidth( true );

	if ( KurooConfig::installedColumn() ) {
		headerItem()->setIcon( 1, KIcon("kuroo_installed_column") );
		//setColumnAlignment( 1, Qt::AlignHCenter );
		setColumnWidth( 1, 25 );
	}
	else
		hideColumn( 1 );

	/*header.setResizeEnabled( false, 1 );
	header.setResizeEnabled( false, 2 );
	header.setResizeEnabled( false, 3 );*/

	// Refresh packages when packages are added/removed to Queue or get installed
	//connect( QueueSingleton::Instance(), SIGNAL( signalQueueChanged( bool ) ), this, SLOT( undefined triggerUpdate() ) );

	// Create text-widget warning for "No packages found.."
	noHitsWarning = new KTextBrowser( this );
	noHitsWarning->setGeometry( QRect( 20, 50, 400, 300 ) );
	//noHitsWarning->setFrameShape( Q3Frame::NoFrame );
}

PortageListView::~PortageListView()
{}

/**
 * Show warning text when package view is empty.
 * @param show/hide
 */
void PortageListView::showNoHitsWarning( const bool& noHits, const int& number_of_terms )
{
	if ( noHits ) {
		if (number_of_terms < 0)
			noHitsWarning->setText( i18n( "<font color=darkRed size=+1><b>No updates were found</font><br><font color=darkRed>There are no updates available at the moment. Please synchronize portage if you haven't already done so and try again.</b></font>" ) );
		else
			noHitsWarning->setText( i18np( "<font color=darkRed size=+1><b>No packages were found using this filter term</font><br><font color=darkRed>Please modify the filter term you have chosen!<br>Try using a more general filter term, so kuroo can find matching packages.</b></font>",
										  "<font color=darkRed size=+1><b>No packages were found using these filter term</font><br><font color=darkRed>Please modify the filter terms you have chosen!<br>Try using more general filter terms, so kuroo can find matching packages.</b></font>", number_of_terms ) );
		noHitsWarning->show();
	}
	else {
		noHitsWarning->setText( "" );
		noHitsWarning->hide();
	}
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
		headerItem()->setText( 0, i18n("Package") + " (" + total + ")" );
	else
		headerItem()->setText( 0, i18n("Package") );
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

	// Disable sorting for faster inserting. Packages are already sorted alphabetically.
	//setSorting( -1 );
	resetListView();
	setHeader( QString::null );

	// Don't load all packages, only first ROWLIMIT
	int packageCount = packageList.size() / 6;
	QListIterator<QString> it( packageList );
	while( it.hasNext() ) {
		QString id = it.next();
		QString name = it.next();
		QString category = it.next();
		QString description = it.next();
		QString status = it.next();
		QString update = it.next();

		PortageItem* item = new PortageItem( this, name, id, category, description, status.toInt() );
		item->setText( 4, update );
		item->setText( 5, description );

		indexPackage( id, item );
	}
	//setSorting( 0 );
	setHeader( QString::number( packageCount ) );
	setPackageFocus( currentId );

	// Cannot have current changed for only one package so emit manually
	if ( packageCount == 1 ) {
		//emit currentChanged( 0 );
	}

// 	clock_t finish = clock();
// 	const double duration = (double) ( finish - start ) / CLOCKS_PER_SEC;
// 	kDebug() << "PortageListView::addSubCategoryPackages SQL-query (" << duration << "s): ";

	return packageCount;
}

#include "portagelistview.moc"
