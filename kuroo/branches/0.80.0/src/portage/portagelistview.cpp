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
 * @class PortageListView::PortageItem
 * @short Package item with all versions.
 */
PortageListView::PortageItem::PortageItem( QListView* parent, const char* name, const QString &id, const QString& description, const QString& homepage, const QString& status )
	: PackageItem( parent, name, id, description, status ), m_homepage( homepage ), m_category( NULL ), hasDetailedInfo( false )
{
}

/**
 * Accessor for category.
 * @return the package category.
 */
QString PortageListView::PortageItem::category()
{
	return m_category;
}

/**
 * Accessor for homepage.
 * @return the package homepage.
 */
QString PortageListView::PortageItem::homepage()
{
	return m_homepage;
}

/**
 * Initialize the package with all its versions and info. Executed when PortageItem get focus first time.
 */
void PortageListView::PortageItem::initVersions()
{
	if ( hasDetailedInfo )
		return;
	
// 	clock_t start = clock();
	
	m_category = PortageSingleton::Instance()->category( id() );
	QString acceptedKeywords = PortageFilesSingleton::Instance()->getKeywordsAtom( id() ).first();

	const QStringList versionsList = PortageSingleton::Instance()->packageVersionsInfo( id() );
	foreach ( versionsList ) {
		QString versionString = *it++;
		QString meta = *it++;
		QString licenses = *it++;
		QString useFlags = *it++;
		QString slot = *it++;
		QString keywords = *it++;
		QString size = *it;
		
		PackageVersion* version = new PackageVersion( this, versionString );
		version->setLicenses( QStringList::split( " ", licenses ) );
		version->setUseflags( QStringList::split( " ", useFlags ) );
		version->setSlot( slot );
		version->setKeywords( QStringList::split( " ", keywords ) );
		version->setAcceptedKeywords( QStringList::split( " ", acceptedKeywords ) );
		version->setSize( size );
		
		if ( meta == FILTERINSTALLED_STRING )
			version->setInstalled( true );
		
		m_versions.append( version );
		m_versionMap.insert( versionString, version );
	}
	
	// Initialize the 'atom' member variable
	atom = new DependAtom( this );
	
	// Check if any of this package versions are hardmasked
	const QStringList atomMaskedList = PortageFilesSingleton::Instance()->getHardMaskedAtom( id() );
	foreach ( atomMaskedList ) {

		// Test the atom string on validness, and fill the internal variables with the extracted atom parts,
		// and get the matching versions
		if ( atom->parse( *it ) ) {
			QValueList<PackageVersion*> versions = atom->matchingVersions();
			QValueList<PackageVersion*>::iterator versionIterator;
			for( versionIterator = versions.begin(); versionIterator != versions.end(); versionIterator++ ) {
				( *versionIterator )->setHardMasked( true );
			}
		}
	}
	
	// Initialize the 'atom' member variable
	atom = new DependAtom( this );
	
	// Check if any of this package versions are unmasked
	const QStringList atomUnmaskedList = PortageFilesSingleton::Instance()->getUnmaskedAtom( id() );
	foreach ( atomUnmaskedList ) {
		
		// Test the atom string on validness, and fill the internal variables with the extracted atom parts,
		// and get the matching versions
		if ( atom->parse( *it ) ) {
			QValueList<PackageVersion*> versions = atom->matchingVersions();
			QValueList<PackageVersion*>::iterator versionIterator;
			for( versionIterator = versions.begin(); versionIterator != versions.end(); versionIterator++ ) {
				( *versionIterator )->setHardMasked( false );
			}
		}
	}
// 	clock_t finish = clock();
// 	const double duration = (double) ( finish - start ) / CLOCKS_PER_SEC;
// 	kdDebug() << "PortageListView::PortageItem::initVersions SQL-query (" << duration << "s): " << endl;
	hasDetailedInfo = true;
}

/**
 * Return list of versions.
 * @return QValueList<PackageVersion*>
 */
QValueList<PackageVersion*> PortageListView::PortageItem::versionList()
{
	return m_versions;
}

/**
 * Return list of versions.
 * @return QMap<QString,PackageVersion*>
 */
QMap<QString,PackageVersion*> PortageListView::PortageItem::versionMap()
{
	return m_versionMap;
}

/**
 * Return a list of PackageVersion objects sorted by their version numbers,
 * with the oldest version at the beginning and the latest version at the end
 * of the list.
 * @return sortedVersions
 */
QValueList<PackageVersion*> PortageListView::PortageItem::sortedVersionList()
{
	QValueList<PackageVersion*> sortedVersions;
	QValueList<PackageVersion*>::iterator sortedVersionIterator;
	
	for( QValueList<PackageVersion*>::iterator versionIterator = m_versions.begin(); versionIterator != m_versions.end(); versionIterator++ ) {
		if ( versionIterator == m_versions.begin() ) {
			sortedVersions.append( *versionIterator );
			continue; // if there is only one version, it can't be compared
		}
		
		// reverse iteration through the sorted version list
		sortedVersionIterator = sortedVersions.end();
		while ( true ) {
			if ( sortedVersionIterator == sortedVersions.begin() ) {
				sortedVersions.prepend( *versionIterator );
				break;
			}
			
			sortedVersionIterator--;
			if ( (*versionIterator)->isNewerThan( (*sortedVersionIterator)->version() ) ) {
				sortedVersionIterator++; // insert after the compared one, not before
				sortedVersions.insert( sortedVersionIterator, *versionIterator );
				break;
			}
		}
	}
	return sortedVersions;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @class PortageListView
 * @short All packages listview.
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

/**
 * Name of current package with focus.
 * @return name
 */
PortageListView::PortageItem* PortageListView::currentPortagePackage()
{
	return dynamic_cast<PortageItem*>( currentPackage() );
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
void PortageListView::addSubCategoryPackages( const QStringList& packageList )
{
// 	clock_t start = clock();
	
// 	kdDebug() << "PortageListView::addSubCategoryPackages packageList=" << packageList << endl;
	
	// Store package focus
	QString currentId = this->currentId();

	// Disable sorting for faster inserting. Packages are already sorted alfabetically.
	setSorting( -1 );
	resetListView();
	setHeader( NULL );
	
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
		QString update = *it++;
		QString homepage = *it;
		
		PortageItem* item = new PortageItem( this, id, name, description, homepage, meta );
		item->setText( 2, update );
		item->setText( 3, description );
		
		indexPackage( id, item );
	}
	setSorting( 0 );
	setHeader( QString::number( packageCount ) );
	setPackageFocus( currentId );
	
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
