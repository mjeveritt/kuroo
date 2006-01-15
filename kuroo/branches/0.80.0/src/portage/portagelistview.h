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

#ifndef PORTAGELISTVIEW_H
#define PORTAGELISTVIEW_H

#include <qpixmap.h>

#include "packagelistview.h"
#include "packageitem.h"

class PackageVersion;
class DependAtom;

/**
 * @class PortageListView
 * @short Installed packages listview.
 */
class PortageListView : public PackageListView
{
Q_OBJECT
public:
	PortageListView( QWidget *parent = 0, const char *name = 0 );
	~PortageListView();
	
	class 							PortageItem;
	
	PortageListView::PortageItem* 	currentPortagePackage();
	
public slots:
	
	void							setHeader( const QString& text );
	
	/**
	 * Populate listview with content of this category.
	 * @param package
	 */
	int 							addSubCategoryPackages( const QStringList& packageList );

	void							slotNextPackage( bool isPrevious );
	
private slots:
	
// 	void			slotLastPackage();
// 	void			slotNewItem( int x, int y );
	
private:
	QStringList						unmaskedList;
	QPixmap 						pxQueuedColumn;
};

/**
 * @class PortageListView::PortageItem
 * @short Package item with all versions.
 */
class PortageListView::PortageItem : public PackageItem
{
public:
	PortageItem::PortageItem( QListView* parent, const char* name, const QString &id, const QString& description, const QString& homepage, const QString& status );
	
	QString 						category();
	QString 						homepage();
	void 							initVersions();
	QValueList<PackageVersion*> 	versionList();
	QMap<QString,PackageVersion*> 	versionMap();
	QValueList<PackageVersion*> 	sortedVersionList();
	void							resetDetailedInfo();
	
protected:
	bool							hasDetailedInfo;
	QString							m_homepage, m_category;
	QValueList<PackageVersion*>		m_versions;
	QMap<QString,PackageVersion*>	m_versionMap;
	DependAtom* 					atom;
};

#endif
