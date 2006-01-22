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
#include "mergelistview.h"

#include <qheader.h>
#include <qlabel.h>
#include <qimage.h>
#include <qpixmap.h>

#include <klistview.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>

/**
 * @class MergeListView
 * @short Specialized listview for emerge history.
 */
MergeListView::MergeListView( QWidget *parent, const char *name )
	: KListView( parent, name ), loc( KGlobal::locale() )
{
	addColumn( i18n("New") );
	addColumn( i18n("Original") );
	
	setProperty( "selectionMode", "Extended" );
	setFrameShape( QFrame::NoFrame );
	setRootIsDecorated( true );
	setFullWidth( true );

	setColumnWidth( 0, 300 );
	setColumnWidth( 1, 300 );
	setColumnWidthMode( 0, QListView::Manual );
	setResizeMode( QListView::LastColumn );
	
	setSorting( -1 );
}

MergeListView::~MergeListView()
{
}

/**
 * Populate listview with log entries
 */
void MergeListView::loadFromDB()
{
	clear();
	itemMap.clear();
	
	const QStringList historyList = HistorySingleton::Instance()->allMergeHistory();
	foreach ( historyList ) {
		QString timeStamp = *it++;
		QString source = *it++;
		QString destination = *it;
		
		QDateTime dt;
		dt.setTime_t( timeStamp.toUInt() );
		QString date = loc->formatDate( dt.date() );
		
		if ( !itemMap.contains( date ) ) {
			KListViewItem *item = new KListViewItem( this, date );
			itemMap[ date ] = item;
			item->setOpen( true );
		}

		new KListViewItem( itemMap[ date ], source, destination );
	}
	
	emit signalHistoryLoaded();
}

#include "mergelistview.moc"
