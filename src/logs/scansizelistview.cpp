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

#include "settings.h"
#include "scansizelistview.h"
#include "common.h"

#include <qheader.h>
#include <qlabel.h>
#include <qpixmap.h>

#include <klistview.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>

/**
 * Specialized listview for viewing portage directorie sizes.
 */
ScanSizeListView::ScanSizeListView( QWidget* parent, const char* name )
	: KListView( parent, name )
{
	// Load icons for category, package ...
	KIconLoader* ldr = KGlobal::iconLoader();
	pxPackageHeader = ldr->loadIcon("kuroo_category", KIcon::Toolbar);
	pxCategory = ldr->loadIcon("kuroo_category", KIcon::Small);
	
	addColumn(i18n("Directory"));
	addColumn(i18n("Size"));
	setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth()));
	setShowSortIndicator(true);
	setRootIsDecorated(true);
	setFullWidth(true);
	setResizeMode(QListView::LastColumn);
}

ScanSizeListView::~ScanSizeListView()
{
}

/**
 * Get portage directory size scan
 * @fixme: how to abort!
 */
void ScanSizeListView::scanPortageSize()
{
	KListViewItem *element, *item;
	
	clear();
	setRootIsDecorated(true);
	
	QDateTime dt = QDateTime::currentDateTime();
	KurooConfig::setScanSizeDate( i18n("Refreshed ") + dt.toString("yyyy MM dd") );
	
	element = new KListViewItem(this, KurooConfig::scanSizeDate());
	element->setOpen(true);
	
	SignalistSingleton::Instance()->scanDiskUsage( true );
	
	// Get size of /tmp/kuroo 
	QString directory = DiskUsageSingleton::Instance()->scanSize( KurooConfig::dirHome() );
	KurooConfig::setDiskUsagehome(directory);
	item = new KListViewItem(element, KurooConfig::dirHome(), directory);
	item->setPixmap(0, pxCategory);
	
	// Get size of /var/tmp/portage
	directory = DiskUsageSingleton::Instance()->scanSize( KurooConfig::dirPortageTmp() + "/portage" );
	KurooConfig::setDiskUsageportage(directory);
	item = new KListViewItem(element, (KurooConfig::dirPortageTmp() + "/portage"), directory);
	item->setPixmap(0, pxCategory);
	
	// Get size of /var/cache/edb/dep
	directory = DiskUsageSingleton::Instance()->scanSize( KurooConfig::dirEdbDep() );
	KurooConfig::setDiskUsagedep(directory);
	item = new KListViewItem(element, KurooConfig::dirEdbDep(), directory);
	item->setPixmap(0, pxCategory);
	
	// Get size of /var/db/pkg
	directory = DiskUsageSingleton::Instance()->scanSize( KurooConfig::dirDbPkg() );
	KurooConfig::setDiskUsagepkg(directory);
	item = new KListViewItem(element, KurooConfig::dirDbPkg(), directory);
	item->setPixmap(0, pxCategory);
	
	// Get size of all portage overlay directories
	const QStringList categoryUrl = QStringList::split(" ", KurooConfig::dirPortageOverlay());
	long overlaySize(0);
	foreach ( categoryUrl ) {
		overlaySize += DiskUsageSingleton::Instance()->scanSizeLong( *it );
	}
	directory = QString::number(overlaySize) + " MB";
	KurooConfig::setDiskUsagelocalportage(directory);
	item = new KListViewItem(element, "Portage Overlay", directory);
	item->setPixmap(0, pxCategory);
	
	// Get size of /usr/portage
	directory = DiskUsageSingleton::Instance()->scanSize( KurooConfig::dirPortage() );
	KurooConfig::setDiskUsageusrportage(directory);
	element = new KListViewItem(element, KurooConfig::dirPortage(), directory);
	element->setPixmap(0, pxCategory);
	element->setOpen(true);
	
	// Get size of /usr/portage/distfiles
	directory = DiskUsageSingleton::Instance()->scanSize( KurooConfig::dirDist() );
	KurooConfig::setDiskUsagedistfiles(directory);
	item = new KListViewItem(element, KurooConfig::dirDist(), directory);
	item->setPixmap(0, pxCategory);
	
	// Get size of /usr/portage/packages
	directory = DiskUsageSingleton::Instance()->scanSize( KurooConfig::dirPkg() );
	KurooConfig::setDiskUsagePkgDir(directory);
	item = new KListViewItem(element, KurooConfig::dirPkg(), directory);
	item->setPixmap(0, pxCategory);
	
	KurooConfig::writeConfig();
	SignalistSingleton::Instance()->scanDiskUsage( false );
}

/**
 * Restore last Portage directories size
 */
void ScanSizeListView::loadPortageSize()
{
	clear();
	setRootIsDecorated(true);
	KListViewItem *element = new KListViewItem(this, KurooConfig::scanSizeDate());
	element->setOpen(true);
	
	// Get size of /tmp/kuroo
	KListViewItem *item = new KListViewItem(element, KurooConfig::dirHome(), KurooConfig::diskUsagehome());
	item->setPixmap(0, pxCategory);
	
	// Get size of /var/tmp/portage
	item = new KListViewItem(element, (KurooConfig::dirPortageTmp() + "/portage"), KurooConfig::diskUsageportage());
	item->setPixmap(0, pxCategory);
	
	// Get size of /var/cache/edb/dep
	item = new KListViewItem(element, KurooConfig::dirEdbDep(), KurooConfig::diskUsagedep());
	item->setPixmap(0, pxCategory);
	
	// Get size of /var/db/pkg
	item = new KListViewItem(element, KurooConfig::dirDbPkg(), KurooConfig::diskUsagepkg());
	item->setPixmap(0, pxCategory);
	
	// Get size of all portage overlay directories
	item = new KListViewItem(element, "Portage Overlay", KurooConfig::diskUsagelocalportage());
	item->setPixmap(0, pxCategory);
	
	// Get size of /usr/portage
	element = new KListViewItem(element, KurooConfig::dirPortage(), KurooConfig::diskUsageusrportage());
	element->setOpen(true);
	element->setPixmap(0, pxCategory);
	
	// Get size of /usr/portage/distfiles
	item = new KListViewItem(element, KurooConfig::dirDist(), KurooConfig::diskUsagedistfiles());
	item->setPixmap(0, pxCategory);
	
	// Get size of /usr/portage/packages
	item = new KListViewItem(element, KurooConfig::dirPkg(), KurooConfig::diskUsagePkgDir());
	item->setPixmap(0, pxCategory);
}

#include "scansizelistview.moc"
