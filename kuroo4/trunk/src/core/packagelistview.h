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

#include <QTreeWidget>
#include <QMap>
#include <QList>

class PackageItem;

/**
 * @class PackageListView
 * @short Base class for packages listviews.
 */
class PackageListView : public QTreeWidget
{
Q_OBJECT
public:
	PackageListView( QWidget *parent = 0, const char *name = 0 );
	~PackageListView();

public:
	virtual	void					resetListView();
	virtual	PackageItem* 			packageItemById( const QString& id ) const;
	virtual const QString			currentId() const;
	int								currentItemStatus() const;

	virtual PackageItem* 			currentPackage() const;
	virtual const QStringList		selectedIds() const;
	virtual const QStringList		selectedPackages() const;
	virtual const QStringList		allId() const;
	virtual const QStringList		allPackages() const;
	/**
	 * Total number of packages in listview.
	 * @return QString
	 */
	inline virtual const QString	count() const { return QString::number( m_packageIndex.count() ); }
	void							nextPackage( const bool& isPrevious );

protected slots:
// 	void							rollOver( QTreeWidgetItem* item );
	void							setPackageFocus( const QString& id );
	virtual void					indexPackage( const QString& id, PackageItem *item );

protected:
	QMap< QString, PackageItem*>	m_packageIndex;

signals:
//	void							signalCurrentChanged();

private:
	QTreeWidgetItem* 				lastItem;
};

#endif
