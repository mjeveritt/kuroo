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

#include <klistview.h>
#include <kprogress.h>

// Tweak for time taken unpacking and installing single package.
const int diffTime( 10 );

/**
 * @class QueueListView::QueueItem
 * @short Package item with progressbar
 */
QueueListView::QueueItem::QueueItem( QListView* parent, const QString& category, const QString& name, const QString& id, const QString& description, const QString& status, int duration )
	: PackageItem( parent, name, id, category, description, status ),
	m_duration( duration ),	m_isChecked( false ), m_isComplete( false ), m_progress( 0 ),
	bar( 0 )
{
	setQueued();
	setText( 0, category + "/" + name );
	bar = new KProgress( duration, parent->viewport() );
	bar->hide();
}

QueueListView::QueueItem::QueueItem( QueueItem* parent, const QString& category, const QString& name, const QString &id, const QString& description, const QString& status, int duration )
	: PackageItem( parent, name, id, category, description, status ),
	m_duration( duration ), m_isChecked( false ), m_isComplete( false ), m_progress( 0 ),
	bar( 0 )
{
	setQueued();
	setText( 0, category + "/" + name );
	bar = new KProgress( duration, parent->listView()->viewport() );
	bar->hide();
}

QueueListView::QueueItem::~QueueItem()
{
	delete bar;
	bar = 0;
}

/**
 * Reimplement for only two state.
 * @param status
 */
void QueueListView::QueueItem::setStatus( int status )
{
	switch ( status ) {
		
		case INSTALLED :
			if ( KurooConfig::installedColumn() ) {
				setPixmap( 0, ImagesSingleton::Instance()->icon( PACKAGE ) );
				setPixmap( 1, ImagesSingleton::Instance()->icon( VERSION_INSTALLED ) );
			}
			else
				setPixmap( 0, ImagesSingleton::Instance()->icon( INSTALLED ) );
			break;
		
		case PACKAGE :
			setPixmap( 0, ImagesSingleton::Instance()->icon( PACKAGE ) );
		
	}
}

/**
 * Show progressbar and launch it.
 */
void QueueListView::QueueItem::setStart()
{
	m_isComplete = false;
	m_isChecked = true;
	repaint();
}

/**
 * Set progressbar as 100%.
 */
void QueueListView::QueueItem::setComplete()
{
	m_progress = m_duration;
	m_isComplete = true;
	bar->setTextEnabled( true );
	bar->setTotalSteps( 100 );
	bar->setProgress( 100 );
	QueueItem::setStatus( INSTALLED );
	bar->hide();
}

bool QueueListView::QueueItem::isComplete()
{
	return m_isComplete;
}

int QueueListView::QueueItem::remainingDuration()
{
	int diff = m_duration - m_progress;
	if ( diff > 0 )
		return diff;
	else
		return 0;
}

/**
 * Advance progress by 1 sec.
 */
void QueueListView::QueueItem::oneStep()
{
	if ( m_progress < m_duration )
		bar->setProgress( m_progress++ );
	else
		if ( m_progress++ == m_duration ) {
			bar->setTotalSteps( 0 );
			bar->setTextEnabled( false );
		}
		else
			bar->advance( 3 );
}

/**
 * Show progressbar after emerge pretend.
 * @param isChecked
 */
void QueueListView::QueueItem::setChecked( bool isChecked )
{
	m_isChecked = isChecked;
}

void QueueListView::QueueItem::hideBar()
{
	bar->hide();
}

/**
 * Reimplemented to paint the progressbar inside column 3 in the listview.
 */
void QueueListView::QueueItem::paintCell( QPainter* painter, const QColorGroup& colorgroup, int column, int width, int alignment )
{
	if ( column == 6 && m_isChecked ) {
		QRect rect = listView()->itemRect( this );
		QHeader *head = listView()->header();
		rect.setLeft( head->sectionPos( 6 ) - head->offset() );
		rect.setWidth( head->sectionSize( 6 ) );
		bar->setGeometry( rect );
		bar->show();
	}
	
	PackageItem::paintCell( painter, colorgroup, column, width, alignment );
}

/**
 * @class QueueListView
 * @short Specialized listview for packages in the installation queue.
 */
QueueListView::QueueListView( QWidget* parent, const char* name )
	: PackageListView( parent, name ), loc( KGlobal::locale() ), m_id( QString::null )
{
	// Setup geometry
	addColumn( i18n( "Package" ), 320 );
	addColumn( "" );
	addColumn( "", 25 );
	header()->setLabel( 2, ImagesSingleton::Instance()->icon( WORLD_COLUMN ), "" );
	setColumnAlignment( 2, Qt::AlignHCenter );
	addColumn( i18n( "Version" ), 100 );
	addColumn( i18n( "Duration" ), 100 );
	addColumn( i18n( "Size" ), 100 );
	addColumn( i18n( "Progress" ), 70 );
	
	setProperty( "selectionMode", "Extended" );
	setRootIsDecorated( true );
	setFullWidth( true );

	setColumnWidthMode( 0, QListView::Manual );
	setColumnWidthMode( 1, QListView::Manual );
	setColumnWidthMode( 2, QListView::Manual );
	setColumnWidthMode( 3, QListView::Manual );
	setColumnWidthMode( 4, QListView::Manual );
	setColumnWidthMode( 5, QListView::Manual );
	setColumnWidthMode( 6, QListView::Manual );
	
	setColumnAlignment( 5, Qt::AlignRight );
	
	// Settings in kuroorc may conflict and enable sorting. Make sure it is deleted first.
	setSorting( -1, false );
	
	if ( KurooConfig::installedColumn() ) {
		header()->setLabel( 1, ImagesSingleton::Instance()->icon( INSTALLED_COLUMN ), "" );
		setColumnAlignment( 1, Qt::AlignHCenter );
		setColumnWidth( 1, 25 );
	}
	else
		hideColumn( 1 );
	
	header()->setResizeEnabled( false, 1 );
	header()->setResizeEnabled( false, 2 );
	
	disconnect( SignalistSingleton::Instance(), SIGNAL( signalSetQueued(const QString&, bool) ), this, SLOT( slotSetQueued(const QString&, bool) ) );
	disconnect( SignalistSingleton::Instance(), SIGNAL( signalClearQueued() ), this, SLOT( slotClearQueued() ) );
	
	connect( this, SIGNAL( collapsed( QListViewItem* ) ), this, SLOT( slotHideBars( QListViewItem* ) ) );
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
 * All packages in listview by name - no children
 * @return packageList
 */
QStringList QueueListView::allPackagesNoChildren()
{
	QStringList packageList;
	QListViewItem* myChild = firstChild();
	while ( myChild ) {
		packageList += myChild->text(0).section( rxPortageVersion, 0, 0 );
		myChild = myChild->nextSibling();
	}
	return packageList;
}

/**
 * Populate queue with packages from db
 */
void QueueListView::insertPackageList( bool hasCheckedQueue )
{
	QueueItem* item;
	sumSize = 0;
	
	resetListView();
	
	// Get list of update packages with info
	const QStringList packageList = KurooDBSingleton::Instance()->allQueuePackages();
	foreach ( packageList ) {
		QString id = *it++;
		QString category = *it++;
		QString name = *it++;
		QString description = *it++;
		QString status = *it++;
		QString idDepend = *it++;
		QString size = *it++;
		QString version = *it;

		// Get package emerge duration from statistics
		int duration = HistorySingleton::Instance()->packageTime( category + "/" + name ).toInt() + diffTime;
		
		// If version get size also
		if ( !version.isEmpty() )
			size = KurooDBSingleton::Instance()->versionSize( id, version );

		if ( idDepend.isEmpty() || idDepend == "0" ) {
			item = new QueueItem( this, category, name, id, description, status, duration );
			item->setOpen( true );
		}
		else {
			QueueItem* itemDepend = dynamic_cast<QueueItem*>( this->itemId( idDepend ) );
			if ( itemDepend )
				item = new QueueItem( itemDepend, category, name, id, description, status, duration );
		}
		
		// Add package info
		if ( version.isEmpty() )
			item->setText( 3, i18n("na") );
		else
			item->setText( 3, version );
		
		if ( duration == diffTime )
			item->setText( 4, i18n("na") );
		else
			item->setText( 4, formatTime( duration ) );
		
		if ( size.isEmpty() )
			item->setText( 5, i18n("na") );
		else {
			item->setText( 5, size );
			addSize( size );
		}
		
		item->setChecked( hasCheckedQueue );
		
		indexPackage( id, item );
		
		// Inform all other listviews that this package is in queue
		QueueSingleton::Instance()->insertInCache( id );
	}
	setPackageFocus( QString::null );
	
	emit( signalQueueLoaded() );
}

/**
 * Resize package column only when resizing widget.
 */
void QueueListView::viewportResizeEvent( QResizeEvent *e )
{
	setColumnWidth( 6, 70 );
	int totalWidth = columnWidth(2) + columnWidth(3) + columnWidth(4) + columnWidth(5) + 70;
	
	if ( KurooConfig::installedColumn() )
		setColumnWidth( 0, this->visibleWidth() - totalWidth - 25 );
	else
		setColumnWidth( 0, this->visibleWidth() - totalWidth );
}

/**
 * Clear use column after user changed use flag settings.
 */
void QueueListView::clearQueuePackageUse()
{
	QListViewItem* myChild = firstChild();
	while ( myChild ) {
		myChild->setText( 4, QString::null );
		myChild = myChild->nextSibling();
	}
}


/////////////////////////////////////////////////////////////////////////////////////
// Emerge duration and progress
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Convert emerge duration from seconds to format hh:mm:ss.
 * @param time
 * @return emergeTime
 */
QString QueueListView::formatTime( int time )
{
	QTime emergeTime(0, 0, 0);
	emergeTime = emergeTime.addSecs( time );
	return loc->formatTime( emergeTime, true, true );
}

/**
 * Get total emerge duration in format hh:mm:ss.
 * @return totalDuration 
 */
QTime QueueListView::totalTime()
{
	QTime totalDuration = QTime(0, 0, 0);
	QListViewItemIterator it( this );
	while ( it.current() ) {
		if ( !dynamic_cast<QueueItem*>( it.current() )->isComplete() )
			totalDuration = totalDuration.addSecs( dynamic_cast<QueueItem*>( it.current() )->remainingDuration() );
		++it;
	}
	return totalDuration;
}

/**
 * Get total emerge duration in seconds.
 * @return int 
 */
int QueueListView::sumTime()
{
	return abs( totalTime().secsTo( QTime(0, 0, 0) ) );
}

QString QueueListView::totalTimeFormatted()
{
	return formatTime( sumTime() );
}


/////////////////////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////////////////////

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

/**
 * Initialize the package's progressbar.
 * @param package id
 */
void QueueListView::slotPackageStart( const QString& id )
{
	if ( id.isEmpty() || !packageIndex[id] ) {
		m_id = QString::null;
		return;
	}
	else
		dynamic_cast<QueueItem*>( packageIndex[id] )->setStart();
	
	m_id = id;
	
	// Update kuroo statusbar with remaining emerge duration
	KurooStatusBar::instance()->setTotalSteps( sumTime() );
}

/**
 * Stop progressbar!
 * @param package id
 */
void QueueListView::slotPackageComplete( const QString& id, bool removeInstalled )
{
	if ( id.isEmpty() || !packageIndex[id] ) {
		m_id = QString::null;
		return;
	}
	else {
		dynamic_cast<QueueItem*>( packageIndex[id] )->setComplete();
		
		if ( removeInstalled )
			packageIndex[id]->setVisible( false );
	}
	
	m_id = QString::null;
}

/**
 * Forward 1 sec signal to the progressbar for the package which is emerging.
 */
void QueueListView::slotPackageProgress()
{
	if ( m_id.isEmpty() || !packageIndex[m_id] )
		return;
	else
		dynamic_cast<QueueItem*>( packageIndex[m_id] )->oneStep();
}

/**
 * Hide dependency packages progressbar when collapsing the tree.
 * @param the end-user package
 */
void QueueListView::slotHideBars( QListViewItem* item )
{
	QListViewItem* myChild = item->firstChild();
	while ( myChild ) {
		dynamic_cast<QueueItem*>( myChild )->hideBar();
		myChild = myChild->nextSibling();
	}
}

#include "queuelistview.moc"

