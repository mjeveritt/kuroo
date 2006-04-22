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
#include "packageemergetime.h"
#include "historylistview.h"

#include <klistview.h>

/**
 * @class HistoryItem
 * @short ListViewItem for package emerge/unmerges date
 */
HistoryListView::HistoryItem::HistoryItem( QListView* parent, const char* date )
	: KListViewItem( parent, date )
{
}

HistoryListView::HistoryItem::HistoryItem( HistoryItem* parent, const char* package )
	: KListViewItem( parent, package ), m_einfo( QString::null )
{
}

void HistoryListView::HistoryItem::setEinfo( const QString& einfo )
{
	m_einfo = einfo;
	setText( 2 , m_einfo.section( "<br>", 0, 0 ) );
}

QString HistoryListView::HistoryItem::einfo()
{
	return m_einfo;
}

/**
 * @class HistoryListView
 * @short Specialized listview for emerge history.
 */
HistoryListView::HistoryListView( QWidget *parent, const char *name )
	: KListView( parent, name ), m_loc( KGlobal::locale() )
{
	addColumn( i18n("Date") );
	addColumn( i18n("Duration") );
	addColumn( i18n("Emerge info") );
	
	setProperty( "selectionMode", "Extended" );
	setFrameShape( QFrame::NoFrame );
	setRootIsDecorated( true );
	setFullWidth( true );

	setColumnWidthMode( 0, QListView::Manual );
	setColumnWidthMode( 1, QListView::Manual );
	setColumnWidthMode( 2, QListView::Manual );
	
	setColumnWidth( 0, 300 );
	setColumnWidth( 1, 80 );
	setResizeMode( QListView::LastColumn );
	
	setSorting( -1 );
}

HistoryListView::~HistoryListView()
{}

/**
 * @return current entry.
 */
QString HistoryListView::current()
{
	QListViewItem *item = currentItem();
	
	if ( item && item->parent() )
		return item->text(0);
	else
		return QString::null;
}

/**
 * @return list of selected packages.
 */
QStringList HistoryListView::selected()
{
	QStringList packageList;
	QListViewItemIterator it(this);
	
	for ( ; it.current(); ++it )
		if ( it.current()->parent() && it.current()->isSelected() )
			packageList += it.current()->text(0);
		
	return packageList;
}

/** 
 * Populate listview with log entries
 */
void HistoryListView::loadFromDB( int days )
{
	clear();
	m_itemMap.clear();
	
	QDateTime dtLimit = QDateTime::currentDateTime();
	dtLimit = dtLimit.addDays( -days );
	
	const QStringList historyList = KurooDBSingleton::Instance()->allHistory();
	foreach ( historyList ) {
		QString timeStamp = *it++;
		QString package = *it++;
		QString duration = *it++;
		QString einfo = *it;
		einfo.replace( "&gt;", ">" ).replace( "&lt;", "<" );
		
		// Convert emerge date to local date format
		QDateTime dt;
		dt.setTime_t( timeStamp.toUInt() );
		QString emergeDate = m_loc->formatDate( dt.date() );
		
		if ( dt >= dtLimit ) {
		
			// Convert emerge duration (in seconds) to local time format
			QTime t( 0, 0, 0 );
			t = t.addSecs( duration.toUInt() );
			QString emergeDuration = m_loc->formatTime( t, true, true );
			
			if ( !duration.isEmpty() || KurooConfig::viewUnmerges() && !package.isEmpty() ) {
				
				if ( !m_itemMap.contains( emergeDate ) ) {
					HistoryItem *item = new HistoryItem( this, emergeDate );
					item->setOpen( true );
					m_itemMap[ emergeDate ] = item;
				}
				
				HistoryItem *item = new HistoryItem( m_itemMap[ emergeDate ], package );
				if ( duration.isEmpty() )
					item->setPixmap( 0, ImagesSingleton::Instance()->icon( UNMERGED ) );
				else {
					item->setPixmap( 0, ImagesSingleton::Instance()->icon( NEW ) );
					item->setText( 1, emergeDuration );
					item->setEinfo( einfo );
				}
			}
		}
	}
	
	// Count emerge/unmerge events
	QListViewItem * myChild = firstChild();
	if ( myChild ) {
		while ( myChild ) {
			QString events = myChild->text(0) + " (" + QString::number( myChild->childCount() ) + ")";
			myChild->setText( 0, events );
			myChild = myChild->nextSibling();
		}
	}
	
	emit signalHistoryLoaded();
}

#include "historylistview.moc"
