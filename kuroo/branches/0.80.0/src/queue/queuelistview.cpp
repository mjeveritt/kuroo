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

// Tweak for time take unpacking and installing each package.
const int diffTime( 10 );

/**
 * Specialized listview for packages in the installation queue.
 */
QueueListView::QueueListView( QWidget* parent, const char* name )
	: PackageListView( parent, name )
{
	// Setup geometry
	addColumn( i18n( "Package" ) );
	addColumn( i18n( "Time" ) );
	addColumn( i18n( "Description" ) );
	setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );

	setProperty( "selectionMode", "Extended" );
	setRootIsDecorated( true );
	setFullWidth( true );
	setColumnWidthMode( 0, QListView::Manual );
	setResizeMode( QListView::LastColumn );
	setColumnWidth( 0, 200 );
	setColumnWidth( 1, 80 );
	setTooltipColumn( 2 );
	
	// Settings in kuroorc may conflict and enable sorting. Make sure it is deleted first.
	setSorting( -1, false );
	
	disconnect( SignalistSingleton::Instance(), SIGNAL( signalSetQueued(const QString&, bool) ), 
	            this, SLOT( setQueued(const QString&, bool) ) );
	disconnect( SignalistSingleton::Instance(), SIGNAL( signalClearQueued() ), 
	            this, SLOT( slotClearQueued() ) );
}

QueueListView::~QueueListView()
{
}

/**
 * Move the package up in the list.
 */
void QueueListView::slotPackageUp()
{
	QListViewItem* packageItem = currentItem();
	if ( packageItem->itemAbove() )
		packageItem->itemAbove()->moveItem( packageItem );
}

/**
 * Move the package down in the list.
 */
void QueueListView::slotPackageDown()
{
	QListViewItem* packageItem = currentItem();
	if ( packageItem->itemBelow() )
		packageItem->moveItem( packageItem->itemBelow() );
}

/**
 * Populate queue with packages from db
 */
void QueueListView::insertPackageList()
{
	reset();
	totalDuration = QTime( 0, 0, 0 );
	sumSize = 0;
		
	// Get list of update packages with info
	const QStringList packageList = QueueSingleton::Instance()->allPackages();
	foreach ( packageList ) {
		QString idDB = *it++;
		QString category = *it++;
		category = category + "-" + *it++;
		QString name = *it++;
		QString description = *it++;
		QString installed = *it;
		
		Meta packageMeta;
		packageMeta.insert( i18n( "2Time" ), timeFormat( HistorySingleton::Instance()->packageTime( category + "/" + name ) ) );
		packageMeta.insert( i18n( "3Description" ), description );
		
		PackageItem* packageItem = new PackageItem( this, category + "/" + name, packageMeta, PACKAGE );
		if ( installed != "0" )
			packageItem->setStatus( INSTALLED );

// 		addSize(size);
// 		packages.insert( idDB, packageItem );
		insertPackage( idDB, packageItem );
		
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
	if ( !time.isEmpty() && time != "na" ) {
		QTime emergeTime( 0, 0, 0 );
		emergeTime = emergeTime.addSecs( time.toInt() );
		totalDuration = totalDuration.addSecs( time.toInt() + diffTime );
		return emergeTime.toString( Qt::TextDate );
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
	return totalDuration.toString( Qt::TextDate );
}

/**
 * Get total emerge duration in seconds.
 * @return int 
 */
int QueueListView::sumTime()
{
	return abs( totalDuration.secsTo( QTime( 0, 0, 0 ) ) );
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

