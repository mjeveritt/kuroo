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

#ifndef INSTALLEDPACKAGESLISTVIEW_H
#define INSTALLEDPACKAGESLISTVIEW_H

#include "packagelistview.h"

class Package;
class QRegExp;

extern QRegExp pv;

/**
 * @class InstalledPackagesListView
 * @short Installed packages listview.
 */
class InstalledPackagesListView : public PackageListView
{
Q_OBJECT
public:
	InstalledPackagesListView( QWidget *parent = 0, const char *name = 0 );
	~InstalledPackagesListView();
	
public slots:
	
	/**
	 * Mark package as current.
	 * @param package
	 */
	void 			setCurrentPackage( const QString& package );
	
	/** 
	 * Get selected packages in list without versions.
	 * @return packageList
	 */
	QStringList 	selectedNoVersion();
	
	/**
	 * Populate listview with content of this category..
	 * @param package
	 */
	void 			addCategoryPackages( const QString& category );
	
private:
	QStringList		unmaskedList;
	QPixmap 		pxQueuedColumn;
};

#endif
