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

#include <klistview.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kcursor.h>
#include <kprogress.h>

static QTime totalDuration;

// Tweak for time taken unpacking and installing single package.
const int diffTime( 10 );

/**
 * @class QueueListView::QueueItem
 * @short Package item with progressbar
 */
QueueListView::QueueItem::QueueItem( QListView* parent, const char* name, const QString &id, const QString& description, const QString& status, int duration )
	: PackageItem( parent, name, id, description, status ), bar( 0 ), progress( 0 ), m_duration( duration ), m_isChecked( false )
{
	bar = new KProgress( duration + diffTime, parent->viewport() );
	bar->hide();
}

QueueListView::QueueItem::QueueItem( PackageItem* parent, const char* name, const QString &id, const QString& description, const QString& status, int duration )
	: PackageItem( parent, name, id, description, status ), bar( 0 ), progress( 0 ), m_duration( duration ), m_isChecked( false )
{
	bar = new KProgress( duration + diffTime, parent->listView()->viewport() );
	bar->hide();
}

QueueListView::QueueItem::~QueueItem()
{
	delete bar;
	bar = 0;
}

/**
 * Disable the queue checkmark.
 */
void QueueListView::QueueItem::setStatus( int status )
{
	if ( status < QUEUED )
		PackageItem::setStatus( status );
}

/**
 * Set progressbar as 100%.
 */
void QueueListView::QueueItem::setStart()
{
	m_isChecked = true;
	repaint();
}

/**
 * Set progressbar as 100%.
 */
void QueueListView::QueueItem::setComplete()
{
	bar->setTotalSteps( 100 );
	bar->setProgress( 100 );
	QueueItem::setStatus( INSTALLED );
}

/**
 * Advance progress by 1 sec.
 */
void QueueListView::QueueItem::oneStep()
{
	bar->setProgress( progress++ );
}

/**
 * Reimplemented to paint the progressbar inside column 3 in the listview.
 */
void QueueListView::QueueItem::paintCell( QPainter* painter, const QColorGroup& colorgroup, int column, int width, int alignment )
{
	if ( this->isVisible() ) {
		if ( column == 5 && m_isChecked ) {
			QRect rect = listView()->itemRect( this );
			QHeader *head = listView()->header();
			rect.setLeft( head->sectionPos( 5 ) - head->offset() );
			rect.setWidth( head->sectionSize( 5 ) );
			bar->setGeometry( rect );
			bar->show();
		}
		PackageItem::paintCell( painter, colorgroup, column, width, alignment );
	}
}

/**
 * Show progressbar after emerge pretend.
 * @param isChecked
 */
void QueueListView::QueueItem::setChecked( bool isChecked )
{
	m_isChecked = isChecked;
}

/**
 * @class QueueListView
 * @short Specialized listview for packages in the installation queue.
 */
QueueListView::QueueListView( QWidget* parent, const char* name )
	: PackageListView( parent, name ), loc( KGlobal::locale() )
{
	// Setup geometry
	addColumn( i18n( "Package" ) );
	addColumn( i18n( "Version" ) );
	addColumn( i18n( "Duration" ) );
	addColumn( i18n( "Size" ) );
	addColumn( i18n( "Use Flags" ) );
	addColumn( i18n( "Progress" ) );
	
	setProperty( "selectionMode", "Extended" );
	setRootIsDecorated( true );
	setFullWidth( true );
	setColumnAlignment( 3, Qt::AlignRight );
	setColumnWidthMode( 0, QListView::Manual );
	setColumnWidthMode( 1, QListView::Manual );
	setColumnWidthMode( 2, QListView::Manual );
	setColumnWidthMode( 3, QListView::Manual );
	setColumnWidthMode( 4, QListView::Manual );
	setResizeMode( QListView::LastColumn );
	setColumnWidth( 0, 200 );
	setColumnWidth( 1, 80 );
	setColumnWidth( 2, 80 );
	setColumnWidth( 3, 80 );
	setColumnWidth( 5, 100 );
	setTooltipColumn( 2 );
	
	// Settings in kuroorc may conflict and enable sorting. Make sure it is deleted first.
	setSorting( -1, false );
	
	disconnect( SignalistSingleton::Instance(), SIGNAL( signalSetQueued(const QString&, bool) ), this, SLOT( slotSetQueued(const QString&, bool) ) );
	disconnect( SignalistSingleton::Instance(), SIGNAL( signalClearQueued() ), this, SLOT( slotClearQueued() ) );
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
	QueueItem* item;
	totalDuration = QTime(0, 0, 0);
	sumSize = 0;
	
	resetListView();
	
	// Get list of update packages with info
	const QStringList packageList = KurooDBSingleton::Instance()->allQueuePackages();
	foreach ( packageList ) {
		QString id = *it++;
		QString category = *it++;
		QString name = *it++;
		QString description = *it++;
		QString meta = *it++;
		QString idDepend = *it++;
		QString useFlags = *it++;
		QString size = *it++;
		QString version = *it;

		// Get package emerge duration from statistics
		QString duration = HistorySingleton::Instance()->packageTime( category + "/" + name );
		
		// If version get size also
// 		QString size;
// 		if ( !version.isEmpty() )
// 			size = KurooDBSingleton::Instance()->versionSize( id, version );

		if ( idDepend.isEmpty() || idDepend == "0" ) {
			item = new QueueItem( this, category + "/" + name, id, useFlags, meta, duration.toInt() );
			item->setOpen( true );
			item->setChecked( false );
		}
		else {
			PackageItem* itemDepend = this->itemId( idDepend );
			if ( itemDepend )
				item = new QueueItem( itemDepend, category + "/" + name, id, useFlags, meta, duration.toInt() );
		}
		
		// Add package info
		if ( version.isEmpty() )
			item->setText( 1, i18n("na") );
		else
			item->setText( 1, version );
		
		if ( duration.isEmpty() )
			item->setText( 2, i18n("na") );
		else
			item->setText( 2, timeFormat( duration ) );
		
		if ( size.isEmpty() )
			item->setText( 3, i18n("na") );
		else {
			item->setText( 3, size );
			addSize( size );
		}
		
		item->setText( 4, description );
		
		indexPackage( id, item );
		
		// Inform all other listviews that this package is in queue
		QueueSingleton::Instance()->insertInCache( id );
	}
	
	emit( signalQueueLoaded() );
}

void QueueListView::setPackagesChecked()
{
	kdDebug() << "QueueListView::setPackagesChecked" << endl;
	QListViewItem* myChild = firstChild();
	while ( myChild ) {
		dynamic_cast<QueueItem*>( myChild )->setChecked( true );
		myChild = myChild->nextSibling();
	}
}

/**
 * Convert emerge duration from seconds to format hh:mm:ss.
 * @param time
 * @return emergeTime
 */
QString QueueListView::timeFormat( const QString& time )
{
	if ( !time.isEmpty() && time != NULL ) {
		QTime emergeTime( 0, 0, 0 );
		emergeTime = emergeTime.addSecs( time.toInt() );
		totalDuration = totalDuration.addSecs( time.toInt() + diffTime );
		return loc->formatTime( emergeTime, true, true );
	}
	else
		return QString::null;
}

/**
 * Get total emerge duration in format hh:mm:ss.
 * @return totalDuration 
 */
QString QueueListView::totalTime()
{
	return loc->formatTime( totalDuration, true, true );
}

/**
 * Get total emerge duration in seconds.
 * @return int 
 */
int QueueListView::sumTime()
{
	return abs( totalDuration.secsTo( QTime(0, 0, 0) ) );
}

/**
 * Add this package size to total.
 * @param size
 */
void QueueListView::addSize( const QString& size )
{
	QString packageSize( size );
	packageSize = packageSize.remove( QRegExp("\\D") );
	sumSize += packageSize.toInt() * 1024;
}

/**
 * Get sum of packages sizes.
 * @return sumSize 
 */
QString QueueListView::totalSize()
{
	return formatSize( QString::number( sumSize ) );
}

/**
 * Format package size nicely 
 * @fixme: Check out KIO_EXPORT QString KIO::convertSize
 * @param size 
 * @return total		as "xxx kB"
 */
QString QueueListView::formatSize( const QString& sizeString )
{
	QString total;
	int size = sizeString.toInt();
	
	if ( size == 0 )
		total = "0 kB ";
	else {
		if ( size < 1024 )
			total = "1 kB ";
		else
			total = loc->formatNumber( (double)(size / 1024), 0 ) + " kB ";
	}
	
	return total;
}

void QueueListView::slotPackageStart( const QString& id )
{
	if ( id.isEmpty() || !packageIndex[id] )
		return;
	else
		dynamic_cast<QueueItem*>( packageIndex[id] )->setStart();
}

void QueueListView::slotPackageComplete( const QString& id )
{
	if ( id.isEmpty() || !packageIndex[id] )
		return;
	else
		dynamic_cast<QueueItem*>( packageIndex[id] )->setComplete();
}

/**
 * 
 */
void QueueListView::slotPackageProgress( const QString& id )
{
	if ( id.isEmpty() || !packageIndex[id] )
		return;
	else
		dynamic_cast<QueueItem*>( packageIndex[id] )->oneStep();
}

#include "queuelistview.moc"

