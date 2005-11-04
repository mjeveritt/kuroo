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
#include <qdir.h>
#include <qtimer.h>

#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kcursor.h>
#include <kiconloader.h>

/**
 * Specialized listview for viewing all portage packages in selected category.
 */
PortageListView::PortageListView( QWidget* parent, const char* name )
	: PackageListView( parent, name )
{
	// Load icon
	KIconLoader *ldr = KGlobal::iconLoader();
	pxQueuedColumn = ldr->loadIcon( "kuroo_queued_column", KIcon::Small );
	
	// Setup geometry
	addColumn(i18n("Package"));
	addColumn( " " );
	header()->setLabel( 1, pxQueuedColumn, " " );
	addColumn(i18n("Size"));
	addColumn(i18n("Description"));
	setSizePolicy(QSizePolicy((QSizePolicy::SizeType)5, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth()));

	setProperty("selectionMode", "Extended");
	setShowSortIndicator(true);

	setItemMargin(1);
	setRootIsDecorated(true);
	setFullWidth(true);

	setColumnWidthMode(0, QListView::Manual);
	setColumnWidthMode(1, QListView::Manual);
	setColumnWidthMode(2, QListView::Manual);
	setColumnAlignment(2, Qt::AlignRight);
	setResizeMode(QListView::LastColumn);
	
	setColumnWidth(0, 200);
	setColumnWidth(1, 20);
	setColumnWidth(2, 80);
	setColumnWidth(3, 80);
	
	setTooltipColumn(3);
}

PortageListView::~PortageListView()
{
}

/**
 * Current package id.
 * If Package is selected return ebuild id.
 * @param id
 */
QString PortageListView::currentId()
{
	if ( !packages.isEmpty() ) {
		for ( QDictIterator<PackageItem> it(packages); it.current(); ++it ) {
			if ( it.current()->text(0) == this->currentItem()->text(0) ) {
				return it.currentKey();
			}
			else {
				if ( it.current()->text(0).section(pv, 0, 0) == this->currentItem()->text(0) )
					return it.currentKey();
			}
		}
	}
	else
		return i18n("na");
}

/**
 * Mark package as selected in view
 * @param package	
 */
void PortageListView::setCurrentPackage( const QString& package )
{
	clearSelection();
	QListViewItemIterator it(this);
	for (; it.current(); ++it)
		if ( package == it.current()->text(0) ) {
			ensureItemVisible(it.current());
			setCurrentItem(it.current());
			it.current()->setSelected(true);
			it.current()->setOpen(true);
			break;
		}
}

/**
 * Get selected packages.
 * @param packageList
 */
QStringList PortageListView::selectedPackages()
{
	QStringList packageList;
	QListViewItemIterator it(this);
	for ( ; it.current(); ++it )
		if ( it.current()->isSelected() ) {
			packageList += it.current()->text(0);
		}
		
	return packageList;
}

/**
 * Populate listview with packages from selected category.
 * Jakob Petsovits technique for fast item loading.
 * @param category package clicked on in categories listview.
 */
void PortageListView::addCategoryPackages( const QString& category )
{
	PackageItem *packageItem, *versionItem;
	static bool packageInstalled(false);
	
	reset();
	packageItems.clear();
	
	const QStringList packageList = PortageSingleton::Instance()->packagesInCategory(category);
	foreach ( packageList ) {
		QString idDB = *it++;
		QString name = *it++;
		QString description = *it++;
		QString keywords = *it++;
		QString size = *it++;
		QString latest = *it++;
		QString version = *it++;
		QString installed = *it;
		QString package = name + "-" + version;
		
		Meta packageMeta;
		packageMeta.insert(i18n("3Description"), description);
		packageMeta.insert(i18n("4Size"), size);
		
		// A version of a package may be installed but not actual in Portage anymore.
		if ( installed == "2" )
			packageInstalled = true;
		else {
			if ( !packageItems.contains(name) ) {
				packageItem = new PackageItem( this, name, packageMeta, PACKAGE );
				packageItem->setExpandable(true);
				packageItems[name].item = packageItem;
				
				// Mark package as installed even when the version is not available in Portage.
				if ( packageInstalled ) {
					packageItem->setStatus(INSTALLED);
					packageInstalled = false;
				}
			}
			
			if ( !packageItems[name].versionItems.contains(version) ) {
				packageMeta.insert(i18n("3Description"), "");
				versionItem = new PackageItem( packageItems[name].item, package, packageMeta, EBUILD );
				packageItems[name].versionItems[version] = versionItem;

				// Only list packages from official Portage tree and Portage Overlay (not old packages)
				if ( installed != "0" ) {
					versionItem->setStatus(EBUILD_INSTALLED);
					if ( !keywords.contains( QRegExp("(^" + KurooConfig::arch() + "\\b)|(\\s" + KurooConfig::arch() + "\\b)") ))
						versionItem->setStatus(MASKED);
					
					// We have an installed version
					packageItem->setStatus(INSTALLED);
				}
				else {
					if ( !keywords.contains( QRegExp("(^" + KurooConfig::arch() + "\\b)|(\\s" + KurooConfig::arch() + "\\b)") ))
						versionItem->setStatus(MASKED);
				}
				
				insertPackage( idDB, versionItem );
			}
			
			if ( PortageSingleton::Instance()->isUnmasked( category + "/" + name ) )
				packageItem->setStatus(UNMASKED);
		}
	}
}

#include "portagelistview.moc"
