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

#include <stdlib.h>
#include <QResizeEvent>
#include <kprogressdialog.h>
#include "common.h"
#include "queuelistview.h"
#include "tooltip.h"
#include "packageitem.h"
#include "packagelistview.h"

// Tweak for time taken unpacking and installing single package.
const int diffTime = 10;

/**
 * @class QueueListView::QueueItem
 * @short Package item with progressbar
 */
QueueListView::QueueItem::QueueItem( QTreeWidget* parent, const QString& category, const QString& name, const QString& id, const int status, int duration )
	: PackageItem( parent, name, id, category, QString::null, status ), m_bar( 0 ),
	m_progress( 0 ), m_duration( duration ), m_isChecked( false ), m_isComplete( false ), m_package( category + "/" + name )
{
	setQueued( true );
	setText( 0, name + " (" + category.section( "-", 0, 0 ) + "/" +  category.section( "-", 1, 1 ) + ")" );
    m_bar = new QProgressBar( parent->viewport() );
    m_bar->setMaximum( duration );
	m_bar->hide();
}

QueueListView::QueueItem::QueueItem( QueueItem* parent, const QString& category, const QString& name, const QString &id, const int status, int duration )
    : PackageItem( parent, name, id, category, QString::null, status ), m_bar( 0 ),
    m_progress( 0 ), m_duration( duration ), m_isChecked( false ), m_isComplete( false ), m_package( category + "/" + name )
{
    setQueued( true );
    setText( 0, name + " (" + category.section( "-", 0, 0 ) + "/" +  category.section( "-", 1, 1 ) + ")" );
    m_bar = new QProgressBar( parent->treeWidget()->viewport() );
    m_bar->setMaximum( duration );
    m_bar->hide();
}

QueueListView::QueueItem::~QueueItem()
{
	delete m_bar;
	m_bar = 0;
}

QString QueueListView::QueueItem::package()
{
	return m_package;
}

/**
 * Show progressbar and launch it.
 */
void QueueListView::QueueItem::setStart()
{
	m_isComplete = false;
	m_isChecked = true;
}

/**
 * Set progressbar as 100%.
 */
void QueueListView::QueueItem::setComplete()
{
	m_progress = m_duration;
	m_isComplete = true;
    m_bar->setTextVisible( true );
    m_bar->setMaximum( 100 );
    m_bar->setValue( 100 );
	setInstalled();
}

/**
 * Is this package progress 100%?
 */
bool QueueListView::QueueItem::isComplete()
{
	return m_isComplete;
}

/**
 * Calculate remaining emerge duration.
 * @return time or '0' if time has exceeded
 */
int QueueListView::QueueItem::remainingDuration()
{
	int diff = m_duration - m_progress;
	if ( diff > 0 )
		return diff;
	else
		return 0;
}

/**
 * Advance package progress by 1 sec.
 */
void QueueListView::QueueItem::oneStep()
{
	if ( m_progress < m_duration )
        m_bar->setValue( m_progress++ );
	else
		if ( m_progress++ == m_duration ) {
            m_bar->setMaximum( 0 );
            m_bar->setTextVisible( false );
		}
		else
            m_bar->setValue( m_bar->value() + 3 );
}

/**
 * Show progressbar after emerge pretend.
 * @param isChecked
 */
void QueueListView::QueueItem::setPretended( bool isChecked )
{
	m_isChecked = isChecked;
}

void QueueListView::QueueItem::hideBar()
{
	m_bar->hide();
}

/**
 * Reimplemented to paint the progressbar inside column 3 in the listview.
 */
/*void QueueListView::QueueItem::paintCell( QPainter* painter, const QPalette& colorgroup, int column, int width, int alignment )
{
	if ( column == 6 && m_isChecked ) {
		QRect rect = listView()->itemRect( this );
		Q3Header *head = listView()->header();
		rect.setLeft( head->sectionPos( 6 ) - head->offset() );
		rect.setWidth( head->sectionSize( 6 ) );
		m_bar->setGeometry( rect );
		m_bar->show();
	}

	PackageItem::paintCell( painter, colorgroup, column, width, alignment );
}*/

/**
 * @class QueueListView
 * @short Specialized listview for packages in the installation queue.
 */
QueueListView::QueueListView( QWidget* parent, const char* name )
	: PackageListView( parent, name ), m_id( QString::null )
{
	// Setup geometry
    QTreeWidgetItem header;
    header.setText( 0, i18n( "Package" ) );
    header.setText( 1, "" );
    header.setIcon( 2, KIcon("kuroo_world_column") );
    //setColumnAlignment( 2, Qt::AlignHCenter );
    header.setText( 3, i18n( "Version") );
    header.setText( 4, i18n( "Duration") );
    header.setText( 5, i18n( "Size") );
    header.setText( 6, i18n( "Progress") );
	
	setProperty( "selectionMode", "Extended" );
	setRootIsDecorated( true );
    //setFullWidth( true );

    /*setColumnWidthMode( 0, QTreeWidget::Manual );
    setColumnWidthMode( 1, QTreeWidget::Manual );
    setColumnWidthMode( 2, QTreeWidget::Manual );
    setColumnWidthMode( 3, QTreeWidget::Manual );
    setColumnWidthMode( 4, QTreeWidget::Manual );
    setColumnWidthMode( 5, QTreeWidget::Manual );
    setColumnWidthMode( 6, QTreeWidget::Manual );*/
	
    //setColumnAlignment( 5, Qt::AlignRight );
	
	// Settings in kuroorc may conflict and enable sorting. Make sure it is deleted first.
    //setSorting( -1, false );
	
	if ( KurooConfig::installedColumn() ) {
        header.setIcon( 1, KIcon("kuroo_installed_column") );
        //setColumnAlignment( 1, Qt::AlignHCenter );
        //setColumnWidth( 1, 25 );
	}
	else
		hideColumn( 1 );
	
    /*header.setResizeEnabled( false, 1 );
    header.setResizeEnabled( false, 2 );*/

    setHeaderItem( &header );
	
    connect( this, SIGNAL( itemCollapsed(QTreeWidgetItem*) ), this, SLOT( slotHideBars( QTreeWidgetItem* ) ) );
}

QueueListView::~QueueListView()
{}

/**
 * Move the package up in the list.
 */
void QueueListView::slotPackageUp()
{
    //FIXME: move item
    /*QTreeWidgetItem* packageItem = currentItem();
    if( itemAbove( packageItem ) )
        itemAbove( packageItem );
        move( packageItem );*/
}

/**
 * Move the package down in the list.
 */
void QueueListView::slotPackageDown()
{
    /*QTreeWidgetItem* packageItem = currentItem();
    if ( itemBelow( packageItem ) )
        packageItem->moveItem( itemBelow( packageItem ) );*/
}

/** 
 * All packages in listview by name - no children
 * @return packageList
 */
const QStringList QueueListView::allPackagesNoChildren()
{
	QStringList packageList;
    QTreeWidgetItemIterator it( this );
    while ( *it ) {
        packageList += dynamic_cast<QueueItem*>(*it)->package();
        it++;
	}
	return packageList;
}

/** 
 * All packages in listview by name - no children
 * @return packageList
 */
const QStringList QueueListView::allEndUserPackages()
{
	QStringList packageList;
    QTreeWidgetItemIterator it( this );
    while ( *it ) {
        packageList += dynamic_cast<QueueItem*>(*it)->package();
        packageList += (*it)->text(3);
        it++;
	}
	return packageList;
}

/**
 * Populate queue with packages from db.
 * @param bool hasCheckedQueue: whether packageList is the result of emerge pretend or just added by user manually.
 */
void QueueListView::insertPackageList( bool hasCheckedQueue )
{
	QueueItem* item;
	m_sumSize = 0;
	
	resetListView();
	
	// Get list of update packages with info
	const QStringList packageList = KurooDBSingleton::Instance()->allQueuePackages();
	int packageCount = packageList.size() / 7;
    QListIterator<QString> it( packageList );
    while( it.hasNext() ) {
        QString id = it.next();
        QString category = it.next();
        QString name = it.next();
        QString status = it.next();
        QString idDepend = it.next();
        QString size = it.next();
        QString version = it.next();
		
		// Get package emerge duration from statistics
		int duration = HistorySingleton::Instance()->packageTime( category + "/" + name ).toInt() + diffTime;
		
		// If version get size
		if ( size.isEmpty() || size == "0" )
			size = KurooDBSingleton::Instance()->versionSize( id, version );
		else
			size = formatSize( size );

		if ( idDepend.isEmpty() || idDepend == "0" ) {
			item = new QueueItem( this, category, name, id, status.toInt(), duration );
            item->setExpanded( true );
		}
		else {
			QueueItem* itemDepend = dynamic_cast<QueueItem*>( this->packageItemById( idDepend ) );
			if ( itemDepend ) {
				item = new QueueItem( itemDepend, category, name, id, status.toInt(), duration );
			}
			else {
				item = new QueueItem( this, category, name, id, status.toInt(), duration );
                item->setExpanded( true );
			}
		}
		
		// Add version to be emerged
		if ( version.isEmpty() )
			item->setText( 3, i18n("na") );
		else
			item->setText( 3, version );
		
		// Add emerge duration
		if ( duration == diffTime )
			item->setText( 4, i18n("na") );
		else
            item->setText( 4, formatTime( duration ) );
		
		// Add download size = tarball size
		if ( size.isEmpty() )
			item->setText( 5, i18n("na") );
		else {
			item->setText( 5, size );
			addSize( size );
		}
		
		item->setPretended( hasCheckedQueue );
		
		indexPackage( id, item );
		
		// Inform all other listviews that this package is in queue
		QueueSingleton::Instance()->insertInCache( id );
	}
	setPackageFocus( QString::null );
	
	// Cannot have current changed for only one package so emit manually
    /*if ( packageCount == 1 )
        emit currentChanged( 0 );*/
}

/**
 * Resize package column only when resizing widget.
 */
void QueueListView::viewportResizeEvent( QResizeEvent* )
{
	setColumnWidth( 6, 70 );
	int totalWidth = columnWidth(2) + columnWidth(3) + columnWidth(4) + columnWidth(5) + 70;
	
    /*if ( KurooConfig::installedColumn() )
		setColumnWidth( 0, this->visibleWidth() - totalWidth - 25 );
	else
        setColumnWidth( 0, this->visibleWidth() - totalWidth );*/
}


/////////////////////////////////////////////////////////////////////////////////////
// Emerge duration and progress
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Get total emerge duration in format hh:mm:ss.
 * @return totalDuration 
 */
long QueueListView::totalDuration()
{
	long totalSeconds( 0 );
    QTreeWidgetItemIterator it( this );
    while ( *it ) {
        if ( !dynamic_cast<QueueItem*>( *it )->isComplete() )
            totalSeconds += dynamic_cast<QueueItem*>( *it )->remainingDuration();
		++it;
	}
	return totalSeconds;
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
	m_sumSize += packageSize.toInt() * 1024;
}

/**
 * Get sum of packages sizes.
 * @return sumSize 
 */
const QString QueueListView::totalSize()
{
	return formatSize( QString::number( m_sumSize ) );
}

/**
 * Format package size nicely 
 * @param size 
 * @return total		as "xxx kB"
 */
const QString QueueListView::formatSize( const QString& sizeString )
{
	KLocale *loc = KGlobal::locale();
	QString total;
	QString tmp ( sizeString );
	int size = tmp.remove(',').toInt();
	
	if ( size == 0 )
		total = "0 kB ";
	else
		total = loc->formatNumber( (double)size, 0 ) + " kB ";
	
	return total;
}

/**
 * Initialize the package's progressbar.
 * @param package id
 */
void QueueListView::slotPackageStart( const QString& id )
{
	if ( id.isEmpty() || !m_packageIndex[id] ) {
		m_id = QString::null;
		return;
	}
	else
		dynamic_cast<QueueItem*>( m_packageIndex[id] )->setStart();
	
	m_id = id;
	
	// Update kuroo statusbar with remaining emerge duration
	KurooStatusBar::instance()->updateTotalSteps( totalDuration() );
}

/**
 * Stop progressbar!
 * @param package id
 */
void QueueListView::slotPackageComplete( const QString& id )
{
	if ( id.isEmpty() || !m_packageIndex[id] ) {
		m_id = QString::null;
		return;
	}
	else
		dynamic_cast<QueueItem*>( m_packageIndex[id] )->setComplete();
	
	m_id = QString::null;
}

/**
 * Forward 1 sec signal to the progressbar for the package which is emerging.
 */
void QueueListView::slotPackageProgress()
{
	if ( m_id.isEmpty() || !m_packageIndex[m_id] )
		return;
	else
		dynamic_cast<QueueItem*>( m_packageIndex[m_id] )->oneStep();
}

/**
 * Hide dependency packages progressbar when collapsing the tree.
 * @param the end-user package
 */
void QueueListView::slotHideBars( QTreeWidgetItem* item )
{
    QTreeWidgetItemIterator it( item );
    while ( *it ) {
        dynamic_cast<QueueItem*>( *it )->hideBar();
        it++;
	}
}

#include "queuelistview.moc"

