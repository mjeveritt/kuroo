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
#include "updatepackageslistview.h"
#include "packageitem.h"
#include "tooltip.h"
#include "packagelistview.h"

#include <qheader.h>
#include <qlabel.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qregexp.h>

#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>

/**
 * Specialized listview update packages sorted by category.
 */
UpdatePackagesListView::UpdatePackagesListView( QWidget *parent, const char *name )
	: PackageListView( parent, name )
{
	// Load icon
	KIconLoader *ldr = KGlobal::iconLoader();
	pxQueuedColumn = ldr->loadIcon( "kuroo_queued_column", KIcon::Small );
	
	// Setup geometry
	addColumn(i18n("Package "));
	addColumn( " " );
	header()->setLabel( 1, pxQueuedColumn, " " );
	addColumn(i18n("Installed"));
	addColumn(i18n("Size"));
	addColumn(i18n("Description"));
	setSizePolicy(QSizePolicy((QSizePolicy::SizeType)5, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth()));
	setProperty("selectionMode", "Extended");
	setShowSortIndicator(true);
	
	setRootIsDecorated(true);
	setFullWidth(true);

	setColumnWidthMode(0, QListView::Manual);
	setColumnWidthMode(1, QListView::Manual);
	setColumnWidthMode(2, QListView::Manual);
	setColumnWidthMode(3, QListView::Manual);
	setColumnAlignment(3, Qt::AlignRight);
	setResizeMode(QListView::LastColumn);
	
	setColumnWidth(0, 200);
	setColumnWidth(1, 20);
	setColumnWidth(2, 80);
	setColumnWidth(3, 80);
	
	setTooltipColumn(6);
}

UpdatePackagesListView::~UpdatePackagesListView()
{
}

/**
 * Populate listview with content of packageList.
 * @param category
 */
void UpdatePackagesListView::addCategoryPackages( const QString& category )
{
	reset();
	
	// Get list of packages in this category from database
	const QStringList packageList = UpdatesSingleton::Instance()->packagesInCategory(category);
	kdDebug() << "packageList=" << packageList << endl;
	foreach ( packageList ) {
		QString idDB = *it++;
		QString name = *it++;
		QString version = *it++;
		QString package = name + "-" + version;
		QString description = *it++;
		QString size = *it++;
		QString keywords = *it++;
		QString installedVersion = *it++;
		QString updateFlags = *it;
		
		Meta packageMeta;
		packageMeta.insert(i18n("3Action"), updateFlags);
		packageMeta.insert(i18n("4Description"), description);
		packageMeta.insert(i18n("5Installed"), installedVersion);
		packageMeta.insert(i18n("5Size"), size);
		
		// Insert package in listView
		PackageItem *packageItem = new PackageItem( this, package, packageMeta, PACKAGE );
		
		if ( !keywords.contains( QRegExp("(^" + KurooConfig::arch() + "\\b)|(\\s" + KurooConfig::arch() + "\\b)") ))
			packageItem->setStatus(MASKED);
		
		insertPackage( idDB, packageItem );
	}
}

#include "updatepackageslistview.moc"
