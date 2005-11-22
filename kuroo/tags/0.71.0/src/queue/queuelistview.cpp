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
#include "queuelistview.h"
#include "tooltip.h"
#include "packageitem.h"
#include "packagelistview.h"

#include <qheader.h>
#include <qlabel.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qregexp.h>

#include <klistview.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kcursor.h>

static QTime totalDuration;

// Tweak for time unpacking and installing each package.
const int diffTime(10);

/**
 * Specialized listview for packages in the installation queue.
 */
QueueListView::QueueListView( QWidget* parent, const char* name )
	: PackageListView( parent, name )
{
	// Setup geometry
	addColumn(i18n("Package"));
	addColumn(i18n("Time"));
	addColumn(i18n("Size"));
	addColumn(i18n("Description"));
	setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth()));

	setProperty("selectionMode", "Extended");
	setShowSortIndicator(true);
	setRootIsDecorated(true);
	setFullWidth(true);

	setColumnWidthMode(0, QListView::Manual);
	setResizeMode(QListView::LastColumn);
	setColumnAlignment(2, Qt::AlignRight);
	
	setColumnWidth(0, 200);
	setColumnWidth(1, 80);
	setColumnWidth(2, 80);
	
	setTooltipColumn(3);
	
	disconnect( SignalistSingleton::Instance(), SIGNAL( signalSetQueued(const QString&, bool) ), 
	            this, SLOT( setQueued(const QString&, bool) ) );
	disconnect( SignalistSingleton::Instance(), SIGNAL( signalClearQueued() ), 
	            this, SLOT( slotClearQueued() ) );
}

QueueListView::~QueueListView()
{
}

/**
 * Clear this listView and packages.
 */
void QueueListView::reset()
{
	clear();
	packages.clear();
	categoryItems.clear();
}

/**
 * Get all packages in the queue
 * @return packageList
 */
QStringList QueueListView::allPackages()
{
	QStringList packageList;
	for ( QDictIterator<PackageItem> it(packages); it.current(); ++it )
		packageList += it.current()->parent()->text(0) + "/" +  it.current()->text(0);
	
	return packageList;
}

/**
 * Get current package in queue (clicked on).
 * @return package
 */
QString QueueListView::currentPackage()
{
	QListViewItem *item = currentItem();
	if ( item && item->parent() ) {
		return (item->parent()->text(0) + "/" +  item->text(0));
	}
	else
		return i18n("na");
}

/**
 * Return selected packages in queue
 * @return packageList		
 */
QStringList QueueListView::selectedPackages()
{
	QStringList packageList;
	for ( QDictIterator<PackageItem> it(packages); it.current(); ++it ) {
		if ( it.current()->parent() && it.current()->isSelected() )
			packageList += it.current()->parent()->text(0) + "/" + it.current()->text(0);
	}
	return packageList;
}

/**
 * Populate queue with packages from db
 */
void QueueListView::loadFromDB()
{
	PackageItem *categoryItem, *packageItem;
	
	reset();
	totalDuration = QTime(0, 0, 0);
	sumSize = 0;
		
	// Get list of update packages with info
	const QStringList packageList = QueueSingleton::Instance()->allPackages();
	foreach ( packageList ) {
		QString idDB = *it++;
		QString category = *it++;
		QString name = *it++;
		QString version = *it++;
		QString package = name + "-" + version;
		QString description = *it++;
		QString size = *it++;
		QString keywords = *it++;
		QString installed = *it;
		
		QString time = HistorySingleton::Instance()->packageTime( category + "/" + name );
		Meta packageMeta;
		
		if ( !categoryItems.contains(category) ) {
			categoryItem = new PackageItem( this, category, packageMeta, CATEGORY );
			categoryItem->setExpandable(true);
			categoryItem->setOpen(true);
			categoryItems[category].item = categoryItem;
		}
		
		if ( !categoryItems[category].packageItems.contains(package) ) {
			packageMeta.insert(i18n("3Description"), description);
			packageMeta.insert(i18n("4Size"), size);
			packageMeta.insert(i18n("5Time"), timeFormat(time));
			packageItem = new PackageItem( categoryItems[category].item, package, packageMeta, PACKAGE );
			categoryItems[category].packageItems[package] = packageItem;
			
			if ( installed != "0" )
				packageItem->setStatus(INSTALLED);
			
			if ( !keywords.contains( QRegExp("(^" + KurooConfig::arch() + "\\b)|(\\s" + KurooConfig::arch() + "\\b)") ))
				packageItem->setStatus(MASKED);
			
		}
		
		addSize(size);
		packages.insert( idDB, packageItem );
		
		// Inform all other listviews that this package is in queue
		QueueSingleton::Instance()->insertInCache( idDB );
	}
	
	emit( signalQueueLoaded() );
}

/**
 * Convert emerge duration from seconds to format hh:mm:ss.
 * @param time
 * @return emergeTime
 */
QString QueueListView::timeFormat( const QString& time )
{
	if ( !time.isEmpty() && time != i18n("na") ) {
		QTime emergeTime(0, 0, 0);
		emergeTime = emergeTime.addSecs(time.toInt());
		totalDuration = totalDuration.addSecs( time.toInt() + diffTime );
		return emergeTime.toString(Qt::TextDate);
	}
	else
		return i18n("na");
}

/**
 * Get total emerge duration in format hh:mm:ss.
 * @return totalDuration 
 */
QString QueueListView::totalTime()
{
	return totalDuration.toString(Qt::TextDate);
}

/**
 * Get total emerge duration in seconds.
 * @return int 
 */
int QueueListView::sumTime()
{
	return abs(totalDuration.secsTo(QTime(0, 0, 0)));
}

/**
 * Add this package size to total.
 * @param size
 */
void QueueListView::addSize( const QString& size )
{
	QString packageSize;
	packageSize = size.section(" ", 0, 0);
	packageSize = packageSize.remove(',');
	sumSize += packageSize.toInt() * 1024;
}

/**
 * Get sum of packages sizes.
 * @return sumSize 
 */
QString QueueListView::totalSize()
{
	return kBSize(sumSize);
}

/**
 * Format package size nicely 
 * @fixme: Check out KIO_EXPORT QString KIO::convertSize
 * @param size 
 * @return total		as "xxx kB"
 */
QString QueueListView::kBSize( const int& size )
{
	QString total("");
	if ( size == 0 )
		total = "0 kB ";
	else
		if ( size < 1024 ) {
			total = "1 kB ";
		}
		else {
			QString eString = QString::number(size / 1024);
			
			while ( !eString.isEmpty() ) {
				QString part = eString.right(3);
				eString = eString.left(eString.length() - part.length());
				
				if (!total.isEmpty())
					total = part + "," + total;
				else
					total = part;
			}
			total += " kB ";
		}
	
	return total;
}

#include "queuelistview.moc"

