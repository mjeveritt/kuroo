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

/**
 * @class VersionViewItem
 * @short Subclass for formating text.
 */
class VersionView::VersionItem : public KListViewItem
{
public:
	VersionItem::VersionItem( QListView* parent, const char* version, bool isInstalled, bool isStable );
	
	bool	isInstalled();
	
protected:
	void 	paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment );
	
private:
	bool	m_isInstalled, m_isStable;
};

VersionView::VersionItem::VersionItem( QListView* parent, const char* version, bool isInstalled, bool isStable )
	: KListViewItem( parent, version ), m_isInstalled( isInstalled ), m_isStable( isStable )
{
}

bool VersionView::VersionItem::isInstalled()
{
	return m_isInstalled;
}

/**
 * Paint the installed version in dark green.
 */
void VersionView::VersionItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment )
{
	QColorGroup m_cg( cg );
	QFont font( p->font() );
	
	if ( m_isInstalled )
		font.setBold( true );
	
	if ( !m_isStable ) {
		font.setItalic( true );
		m_cg.setColor( QColorGroup::Text, Qt::darkRed );
	}
		
	p->setFont( font );
	KListViewItem::paintCell( p, m_cg, column, width, alignment );
}

/**
 * @class VersionView
 * @short Version listview.
 */
VersionView::VersionView( QWidget *parent, const char *name )
	: KListView( parent, name ), m_installedIndex( 0 ), m_emergeIndex( 0 ), m_emergeVersion( QString::null )
{
	addColumn( i18n( " " ) );
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
	VersionItem* item = new VersionItem( this, " ", isInstalled, stability == i18n("Stable") );
	item->setText( 1, version );
	item->setText( 2, stability );
	item->setText( 3, size );
}

/**
 * Mark the installation version with icon.
 * @param version
 */
void VersionView::usedForInstallation( const QString& version )
{
	QListViewItem* myChild = firstChild();
	while ( myChild ) {
		if ( dynamic_cast<VersionItem*>( myChild )->isInstalled() )
			m_installedIndex = itemIndex( myChild );
		
		if ( myChild->text(1) == version ) {
			myChild->setPixmap( 0, ImagesSingleton::Instance()->icon( VERSION_INSTALLED ) );
			m_emergeIndex = itemIndex( myChild );
		}
		myChild = myChild->nextSibling();
	}
	
	m_emergeVersion = version;
}

int VersionView::hasUpdate()
{
	return m_installedIndex - m_emergeIndex;
}

QString VersionView::updateVersion()
{
	return m_emergeVersion;
}

#include "versionview.moc"
