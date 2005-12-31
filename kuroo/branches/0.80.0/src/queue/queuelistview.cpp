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

class QueueListView::QueueItem : public PackageItem
{
public:
	QueueItem::QueueItem( QListView* parent, const char* name, const QString &id, const QString& description, const QString& status, int duration );
	~QueueItem();
	
	void		setTotalSteps( int totalSteps );
	void		setProgress( int progress );
	void		advance();
	
protected:
	void 		paintCell( QPainter* painter, const QColorGroup& colorgroup, int column, int width, int alignment );
	
private:
	KProgress* 	bar;
	int			progress;
};

QueueListView::QueueItem::QueueItem( QListView* parent, const char* name, const QString &id, const QString& description, const QString& status, int duration )
	: PackageItem( parent, name, id, description, status ), bar( 0 ), progress( 0 )
{
	bar = new KProgress( duration, parent->viewport() );
}

QueueListView::QueueItem::~QueueItem()
{
	delete bar;
	bar = 0;
}

void QueueListView::QueueItem::setTotalSteps( int totalSteps )
{
	bar->setTotalSteps( totalSteps );
	repaint();
}

void QueueListView::QueueItem::setProgress( int progress )
{
	bar->setProgress( progress );
	repaint();
}

void QueueListView::QueueItem::advance()
{
	bar->setProgress( progress++ );
}

void QueueListView::QueueItem::paintCell( QPainter* painter, const QColorGroup& colorgroup, int column, int width, int alignment )
{
	QRect rect = listView()->itemRect( this );
	QHeader *head = listView()->header();
	rect.setLeft( head->sectionPos( 3 ) - head->offset() );
	rect.setWidth( head->sectionSize( 3 ) );
	bar->setGeometry( rect );
	bar->show();
	PackageItem::paintCell( painter, colorgroup, column, width, alignment );
}

/**
 * Specialized listview for packages in the installation queue.
 */
QueueListView::QueueListView( QWidget* parent, const char* name )
	: PackageListView( parent, name ), loc( KGlobal::locale() )
{
	// Setup geometry
	addColumn( i18n( "Package" ) );
	addColumn( i18n( "Duration" ) );
	addColumn( i18n( "Description" ) );
	addColumn( i18n( "Progress" ) );
	
	setProperty( "selectionMode", "Extended" );
	setRootIsDecorated( true );
	setFullWidth( true );
	setColumnWidthMode( 0, QListView::Manual );
	setResizeMode( QListView::LastColumn );
	setColumnWidth( 0, 200 );
	setColumnWidth( 1, 80 );
	setColumnWidth( 3, 80 );
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
	resetListView();
	totalDuration = QTime( 0, 0, 0 );
	sumSize = 0;
		
	// Get list of update packages with info
	const QStringList packageList = QueueSingleton::Instance()->allPackages();
	foreach ( packageList ) {
		QString id = *it++;
		QString category = *it++;
		category = category + "-" + *it++;
		QString name = *it++;
		QString description = *it++;
		QString meta = *it;
		
		QString duration = HistorySingleton::Instance()->packageTime( category + "/" + name );
		
		QueueItem* item = new QueueItem( this, id, category + "/" + name, description, meta, duration.toInt() );
		item->setText( 1, timeFormat( duration ) );
		item->setText( 2, description );
		indexPackage( id, item );
		
		// Inform all other listviews that this package is in queue
		QueueSingleton::Instance()->insertInCache( id );
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
	if ( !time.isEmpty() && time != NULL ) {
		QTime emergeTime( 0, 0, 0 );
		emergeTime = emergeTime.addSecs( time.toInt() );
		totalDuration = totalDuration.addSecs( time.toInt() + diffTime );
		return loc->formatTime( emergeTime, true, true );
	}
	else
		return NULL;
}

/**
 * Get total emerge duration in format hh:mm:ss.
 * @return totalDuration 
 */
QString QueueListView::totalTime()
{
	return loc->formatTime(totalDuration, true, true);
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
	QString packageSize(size);
	packageSize = packageSize.remove(QRegExp("\\D"));
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
	QString total;
	
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

/**
 * 
 */
void QueueListView::slotPackageProgress( const QString& id )
{
	if ( !id.isEmpty() && packageIndex[id] ) {
		dynamic_cast<QueueItem*>( packageIndex[id] )->advance();
	}
}

#include "queuelistview.moc"

