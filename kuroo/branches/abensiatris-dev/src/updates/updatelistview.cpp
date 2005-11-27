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
#include "updatelistview.h"
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
#include <kcursor.h>
#include <kiconloader.h>

/**
 * Specialized listview for alfabetical list of update packages.
 */
UpdateListView::UpdateListView( QWidget *parent, const char *name )
	: PackageListView( parent, name )
{
	// Load icon
	KIconLoader *ldr = KGlobal::iconLoader();
	pxQueuedColumn = ldr->loadIcon( "kuroo_queued_column", KIcon::Small );
	
	// Setup geometry
	addColumn(i18n("Package"));
	addColumn( " " );
	header()->setLabel( 1, pxQueuedColumn, " " );
	addColumn(i18n("Installed"));
	addColumn(i18n("Action"));
	addColumn(i18n("Size"));
	addColumn(i18n("Nr"));
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
	setColumnWidthMode(4, QListView::Manual);
	setColumnWidthMode(5, QListView::Manual);
	
	setColumnWidth(0, 300);
	setColumnWidth(1, 20);
	setColumnWidth(2, 80);
	setColumnWidth(3, 80);
	setColumnWidth(4, 80);
	setColumnWidth(5, 20);
	
	setSorting(5, true);
	setColumnAlignment(4, Qt::AlignRight);
	setColumnAlignment(5, Qt::AlignHCenter);
	setResizeMode(QListView::LastColumn);
	
	header()->moveSection(5, 0);
	setTooltipColumn(6);
}

UpdateListView::~UpdateListView()
{
}

/**
 * Populate listview from db.
 */
void UpdateListView::loadFromDB()
{
	reset();
	int order(1);
	
	// Get list of update packages with info
	const QStringList packageList = UpdatesSingleton::Instance()->allPackages();
	foreach ( packageList ) {
		QString idDB = *it++;
		QString category = *it++;
		QString name = *it++;
		QString version = *it++;
		QString package = category + "/" + name + "-" + version;
		QString description = *it++;
		QString size = *it++;
		QString keywords = *it++;
		QString installedVersion = *it++;
		QString updateFlags = *it++;
		QString useFlags = *it;
		
		Meta packageMeta;
		packageMeta.insert(i18n("3Action"), updateFlags);
		packageMeta.insert(i18n("3Nr"), QString::number(order++));
		packageMeta.insert(i18n("5Description"), description);
		packageMeta.insert(i18n("6Installed"), installedVersion);
		packageMeta.insert(i18n("7Size"), size);
		packageMeta.insert(i18n("8USE"), useFlags);
		
		// Insert package in listView 
		PackageItem *packageItem = new PackageItem( this, package, packageMeta, PACKAGE );
		
		if ( !keywords.contains( QRegExp("(^" + KurooConfig::arch() + "\\b)|(\\s" + KurooConfig::arch() + "\\b)") ))
			packageItem->setStatus(MASKED);
		
		// Found in world file
		if ( PortageSingleton::Instance()->isWorld( category + "/" + name ) )
			packageItem->setStatus(INSTALLED_WORLD);
		
		insertPackage( idDB, packageItem );
	}
	
	emit( signalUpdatesLoaded() );
}

#include "updatelistview.moc"
