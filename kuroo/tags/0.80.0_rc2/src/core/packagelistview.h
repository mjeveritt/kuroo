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

#ifndef PACKAGELISTVIEW_H
#define PACKAGELISTVIEW_H

#include <klistview.h>

#include <qdict.h>

class PackageItem;

/**
 * @class PackageListView
 * @short Base class for packages listviews.
 */
class PackageListView : public KListView
{
Q_OBJECT
public:
    PackageListView( QWidget *parent = 0, const char *name = 0 );
    ~PackageListView();

public:
	virtual	void			resetListView();
	virtual	PackageItem* 	itemId( const QString& id );
	
	virtual QString			currentId();
	QString					currentItemStatus();
	virtual PackageItem* 	currentPackage();
	
	virtual QStringList		selectedId();
	virtual QStringList		selectedPackages();
	
	virtual QStringList		allId();
	virtual QStringList		allPackages();
	
	virtual QString			count();
	
public slots:
	void					slotNextPackage( bool isPrevious );
	
protected slots:
	void					setPackageFocus( const QString& id );
	virtual void 			indexPackage( const QString& id, PackageItem *item );
	
protected:
	QDict<PackageItem>		packageIndex;
	
signals:
	void					signalPackageChanged();
	
private:
	
};

#endif
