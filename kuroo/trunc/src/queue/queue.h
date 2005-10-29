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

#ifndef QUEUE_H
#define QUEUE_H

#include <qobject.h>
#include <qvaluevector.h>

/**
 * @class Queue
 * @short Object for packages to be emerged = installation queue.
 */
class Queue : public QObject
{
Q_OBJECT
	
public:
	Queue( QObject *parent = 0 );
	~Queue();
	
public slots:
	void					init( QObject *myParent = 0 );
	
	/**
	* When the package is inserted in the register it in the cache too.
	* @param id
	*/
	void					clearCache();
	
	/**
	* When the package is inserted in the register it in the cache too.
	* @param id
	*/
	void					insertInCache( const QString& idDB );
	
	/**
	* When the package is inserted in the register it in the cache too.
	* @param id
	*/
	void					deleteFromCache( const QString& idDB );
	
	/**
	* Check if package is the queue.
	* @param id
	* @return true/false
	*/
	bool					isQueued( const QString& idDB );
	
	/**
	* Clear the queue.
	* @param packageIdList
	*/
	void					reset();
	
	/**
	* Forward signal to refresh queue.
	*/
	void					refresh();
	
	/**
	* Remove packages from queue.
	* @param packageIdList
	*/
	void					removePackageIdList( const QStringList& packageIdList );
	
	/**
	* Add packages to the installation queue table in the db.
	* @param packageIdList
	*/
    void 					addPackageIdList( const QStringList& packageIdList );
	
	/**
	* Launch emerge pretend of packages.
	* @param packageList
	*/
	void					pretendPackageList( const QStringList& packageList );
	
	/**
	* Launch emerge package list
	* @param packageList
	*/
	void					installPackageList( const QStringList& packageList );
	
	/**
	* @fixme: Check for failure.
	* Add package as installed in db.
	* @param package
	*/
	void					addPackage( const QString& package );
	
	/**
	* Get list of all Queue packages.
	* @return QStringList
	*/
	QStringList				allPackages();
	
	void					installQueue( const QStringList& packageIdList );
	QString					count();
	
signals:
	void					signalQueueChanged();
	
private:
	QObject					*parent;
	QString					package;
	QStringList				packageIdList;
	QValueVector<int>	packagesCache;
};

#endif
