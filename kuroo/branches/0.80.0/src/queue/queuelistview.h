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

#ifndef QUEUELISTVIEW_H
#define QUEUELISTVIEW_H

#include "packagelistview.h"

#include <qpixmap.h>

class QTime;
class QPixmap;
class Package;
class PackageItem;

/**
 * @class QueueListView
 * @short Specialized listview for packages in the installation queue.
 */
class QueueListView : public PackageListView
{
Q_OBJECT

public:
	QueueListView( QWidget* parent = 0, const char* name = 0 );
	~QueueListView();

public slots:

	void								slotPackageUp();
	void								slotPackageDown();

	/**
	* Populate queue with packages from db
	*/
	void 								insertPackageList();

	/**
	* Get total emerge duration in format hh:mm:ss and int.
	*/
	QString		 						totalTime();
	int									sumTime();

	/**
	* Get sum of packages sizes.
	* @return sumSize 
	*/
	QString		 						totalSize();
	
private slots:
	
	/**
	* Clear this listView and packages.
	*/
// 	void								reset();
	
	/**
	* Format package size nicely 
	* @fixme: Check out KIO_EXPORT QString KIO::convertSize
	* @param size 
	* @return total		as "xxx kB"
	*/
	QString		 						kBSize( const int& size );
	
	/**
	* Add this package size to total.
	* @param size
	*/
	void 								addSize( const QString& size );
	
	/**
	* Convert emerge duration from seconds to format hh:mm:ss.
	* @param time 			
	* @return emergeTime  
	*/
	QString 							timeFormat( const QString& time );
	
signals:
	void								signalQueueLoaded();
	void								signalPackageEmerged();
	
private:
	KLocale 							*loc;
	int 								sumSize;
	QPixmap 							pxPackageHeader, pxCategory, pxPackage, pxInstalled, pxStable, pxTesting;	
};

#endif
