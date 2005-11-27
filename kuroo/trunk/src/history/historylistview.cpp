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

#include <qheader.h>
#include <qlabel.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qregexp.h>

#include <klistview.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>

/**
 * Specialized listview for emerge history.
 */
HistoryListView::HistoryListView( QWidget *parent, const char *name )
	: KListView( parent, name )
{
	// Load icons for category, package ...
	KIconLoader *ldr = KGlobal::iconLoader();
	pxPackageHeader = ldr->loadIcon("kuroo_history", KIcon::Toolbar);
	pxCategory = ldr->loadIcon("kuroo_category", KIcon::Small);
	pxNew = ldr->loadIcon("kuroo_new", KIcon::Small);
	pxUnmerged = ldr->loadIcon("kuroo_unmerged", KIcon::Small);
	
	addColumn(i18n("Date"));
	addColumn(i18n("Time"));
	setMinimumSize(QSize(50, 0));
	setProperty("selectionMode", "Extended");
	
	setRootIsDecorated(true);
	setFullWidth(true);

	setColumnWidthMode(0, QListView::Manual);
	setColumnWidthMode(1, QListView::Manual);
	
	setColumnWidth(0, 300);
	setColumnWidth(1, 80);
	
	setSorting(-1);
}

HistoryListView::~HistoryListView()
{
}

/**
 * @return current entry.
 */
QString HistoryListView::current()
{
	QListViewItem *item = currentItem();
	
	if ( item && item->parent() )
		return item->text(0);
	else
		return i18n("na");
}

/**
 * @return list of selected packages.
 */
QStringList HistoryListView::selected()
{
	QStringList packageList;
	QListViewItemIterator it(this);
	
	for ( ; it.current(); ++it ) {
		if ( it.current()->parent() && it.current()->isSelected() )
			packageList += it.current()->text(0);
	}
		
	return packageList;
}

/** 
 * Populate listview with log entries
 */
void HistoryListView::loadFromDB()
{
	clear();
	itemMap.clear();
	
	const QStringList historyList = HistorySingleton::Instance()->allHistory();
	foreach ( historyList ) {
		QString date = *it++;
		QString package = *it++;
		QString time = *it;
		
		if ( !time.isEmpty() || KurooConfig::viewUnmerges() ) {
			if ( !package.isEmpty() ) {
				
				ItemMap::iterator itMap =  itemMap.find(date) ;
				if ( itMap == itemMap.end() ) {
					KListViewItem *itemDate = new KListViewItem(this, date);
					itemDate->setOpen(true);
					KListViewItem *itemPackage = new KListViewItem(itemDate, package);
					itemMap.insert(date, itemDate);
					
					if ( time.isEmpty() )
						itemPackage->setPixmap(0, pxUnmerged);
					else {
						itemPackage->setPixmap(0, pxNew);
						itemPackage->setText(1, time);
					}
				}
				else {
					KListViewItem *itemPackage = new KListViewItem(itMap.data(), package);
					
					if (time.isEmpty())
						itemPackage->setPixmap(0, pxUnmerged);
					else {
						itemPackage->setPixmap(0, pxNew);
						itemPackage->setText(1, time);
					}
				}
			}
		}
	}
	
	// Count emerge/unmerge events
	QListViewItem * myChild = firstChild();
	if ( myChild ) {
		while ( myChild ) {
			QString events = myChild->text(0) + " (" + QString::number(myChild->childCount()) + ")";
			myChild->setText(0, events);
			myChild = myChild->nextSibling();
		}
	}
	
	emit signalHistoryLoaded();
}

#include "historylistview.moc"
