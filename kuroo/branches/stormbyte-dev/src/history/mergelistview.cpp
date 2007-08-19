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

#include <klistview.h>

/**
 * @class MergeItem
 * @short ListViewItem with merge files.
 */
MergeListView::MergeItem::MergeItem( QListView* parent, const char* date )
	: KListViewItem( parent, date )
{}

MergeListView::MergeItem::MergeItem( QListView* parent, const char* source, const char* destination )
	: KListViewItem( parent, QString::null ), m_source( source ), m_destination( destination )
{
	setText( 0 , m_source );
}

MergeListView::MergeItem::MergeItem( MergeItem* parent, const char* source, const char* destination )
	: KListViewItem( parent, QString::null ), m_source( source ), m_destination( destination )
{
	setText( 0 , m_source.section( QRegExp( "\\d{8}_\\d{4}/" ), 1, 1 ).replace( ":" , "/" ) );
}

QString MergeListView::MergeItem::source() const
{
	return m_source;
}

QString MergeListView::MergeItem::destination() const
{
	return m_destination;
}

/**
 * @class MergeListView
 * @short Specialized listview for emerge history.
 */
MergeListView::MergeListView( QWidget *parent, const char *name )
	: KListView( parent, name ), m_loc( KGlobal::locale() )
{
	addColumn( i18n("Configuration file") );
	
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
{}

/**
 * Append the new unmerged configuration files ontop.
 */
void MergeListView::loadConfFiles( const QStringList& confFilesList )
{
	clear();
	m_itemMap.clear();
	
	foreach ( confFilesList ) {
		QString source = *it;
		
// 		kdDebug() << "source=" << source << LINE_INFO;
		
		QString destination = source;
		destination.remove( QRegExp("\\._cfg\\d\\d\\d\\d_") );
		
		if ( source.contains( GlobalSingleton::Instance()->kurooDir() ) ) {
			
			QString date = source.section( "/", -2, -2 );
			if ( !m_itemMap.contains( date ) ) {
				MergeItem *item = new MergeItem( this, date );
				m_itemMap[ date ] = item;
				item->setOpen( true );
			}
			
			new MergeItem( m_itemMap[ date ], source, source + ".orig" );
		}
		else
			new MergeItem( this, source, destination );
	}
}

#include "mergelistview.moc"
