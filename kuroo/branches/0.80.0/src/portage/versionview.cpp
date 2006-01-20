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
#include "versionview.h"

#include <qpainter.h>

#include <klistview.h>
#include <kiconloader.h>

class VersionView::VersionItem : public KListViewItem
{
public:
	VersionItem::VersionItem( QListView* parent, const char* version, bool isInstalled );
	
protected:
	void 	paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment );
	
private:
	bool	m_isInstalled;
};

VersionView::VersionItem::VersionItem( QListView* parent, const char* version, bool isInstalled )
	: KListViewItem( parent, version ), m_isInstalled( isInstalled )
{
}

void VersionView::VersionItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment )
{
	QColorGroup m_cg( cg );
	QFont font( p->font() );
	
	if ( m_isInstalled ) {
		font.setBold( true );
		p->setFont( font );
		m_cg.setColor( QColorGroup::Text, Qt::darkGreen );
	}
	else {
		font.setBold( false );
		p->setFont( font );
		m_cg.setColor( QColorGroup::Text, Qt::black );
	}
	KListViewItem::paintCell( p, m_cg, column, width, alignment );
}

VersionView::VersionView( QWidget *parent, const char *name )
	: KListView( parent, name )
{
// 	addColumn( i18n( " " ) );
	addColumn( i18n( "Version" ) );
	addColumn( i18n( "Stability" ) );
	addColumn( i18n( "Size" ) );
	setColumnWidth( 1, 100 );
	setColumnWidth( 2, 100 );
	setColumnWidth( 3, 70 );
	setColumnAlignment( 3, Qt::AlignRight );
	setResizeMode( QListView::LastColumn );
	setSorting( -1 );
}

VersionView::~VersionView()
{
}

void VersionView::insertItem( const char* version, const char* stability, const char* size, bool isInstalled )
{
	VersionItem* item = new VersionItem( this, " ", isInstalled );
	item->setText( 1, version );
	item->setText( 2, stability );
	item->setText( 3, size );
}

void VersionView::usedForInstallation( const QString& version )
{
	QListViewItem* myChild = firstChild();
	while ( myChild ) {
		if ( myChild->text(1) == version )
			myChild->setPixmap( 0, ImagesSingleton::Instance()->icon( VERSION_INSTALLED ) );
		myChild = myChild->nextSibling();
	}
}

#include "versionview.moc"
