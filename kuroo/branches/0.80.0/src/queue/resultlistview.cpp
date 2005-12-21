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
#include "resultlistview.h"
#include "packageitem.h"
#include "tooltip.h"
#include "packagelistview.h"

#include <qheader.h>
#include <qlabel.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qregexp.h>

#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kcursor.h>

/**
 * Specialized listview for results packages.
 */
ResultListView::ResultListView( QWidget *parent, const char *name )
	: PackageListView( parent, name )
{
	// Load icon
	KIconLoader *ldr = KGlobal::iconLoader();
	pxQueuedColumn = ldr->loadIcon( "kuroo_queued_column", KIcon::Small );
	
	// Setup geometry
	addColumn( i18n( "Package" ) );
	addColumn( " " );
	header()->setLabel( 1, pxQueuedColumn, " " );
	addColumn( i18n( "Nr" ) );
	addColumn( i18n( "Action" ) );
	addColumn( i18n( "Size" ) );
	addColumn( i18n( "USE" ) );
	addColumn( i18n( "Description" ) );
	setSizePolicy( QSizePolicy((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth()) );
	setMinimumSize( QSize( 800, 200 ) );
	setProperty( "selectionMode", "Extended" );
	setShowSortIndicator(true);
	setFullWidth(true);
	setSorting( -1 );
	
	setColumnWidthMode( 0, QListView::Manual );
	setColumnWidthMode( 1, QListView::Manual );
	setColumnWidthMode( 2, QListView::Manual );
	setColumnWidthMode( 3, QListView::Manual );
	setColumnWidthMode( 4, QListView::Manual );
	setColumnWidthMode( 5, QListView::Manual );
	
	setColumnWidth( 0, 200 );
	setColumnWidth( 1, 20 );
	setColumnWidth( 2, 20 );
	setColumnWidth( 3, 50 );
	setColumnWidth( 4, 80 );
	setColumnWidth( 5, 80 );
		
	setSorting( 2, true );
	setColumnAlignment( 2, Qt::AlignHCenter );
	setResizeMode( QListView::LastColumn );
	setColumnAlignment( 4, Qt::AlignRight );
	
	setTooltipColumn( 6 );
	header()->moveSection( 2, 0 );
}

ResultListView::~ResultListView()
{
}

/** 
 * Populate listview with content of table resultPackages 
 */
bool ResultListView::loadFromDB()
{
	PackageItem *packageItem;
	reset();
	categories.clear();
	
	//Get list of update packages with info
	int order( 1 );
	const QStringList packageList = ResultsSingleton::Instance()->allPackages();
	foreach ( packageList ) {
		QString idDB = *it++;
		QString package = *it++;
		QString description = *it++;
		QString size = *it++ + " kB";
		QString use = *it++;
		QString flags = *it++;
		QString installed = *it;
		
		Meta packageMeta;
		packageMeta.insert(i18n("3Nr"), QString::number(order++));
		packageMeta.insert(i18n("4Action"), flags);
		packageMeta.insert(i18n("5Description"), description);
		packageMeta.insert(i18n("6Size"), size);
		packageMeta.insert(i18n("7USE"), use);
		
		packageItem = new PackageItem( this, package, packageMeta, PACKAGE, idDB );
		
		if ( installed != "0" )
			packageItem->setStatus(INSTALLED);
		
// 		if ( !keywords.contains( QRegExp("(^" + KurooConfig::arch() + "\\b)|(\\s" + KurooConfig::arch() + "\\b)") ))
// 			packageItem->setStatus(MASKED);
		
		insertPackage( idDB, packageItem );
	}
	
	if ( packageList.isEmpty() )
		return false;
	else
		return true;
}

#include "resultlistview.moc"
