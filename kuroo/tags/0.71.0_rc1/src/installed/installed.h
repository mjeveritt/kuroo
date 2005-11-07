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

#ifndef INSTALLED_H
#define INSTALLED_H

#include <qobject.h>

class QRegExp;
extern QRegExp pv;

typedef struct Info {
	QString packageSlots;
	QString homepage;
	QString licenses;
	QString description;
	QString keywords;
	QString useFlags;
	QString size;
};

/**
 * @class Installed
 * @short Object for installed packages.
 */
class Installed : public QObject
{
Q_OBJECT
public:
	Installed( QObject *parent = 0 );
    ~Installed();
	
public slots:
	void			init( QObject *myParent = 0 );
	
	/**
	 * Forward signal after a new scan.
	 */
	void			slotChanged();
	
	/**
	 * Clear packages.
	 */
	void			slotReset();
	
	/**
	 * Start scan of installed packages.
	 * @return bool
	 */
	bool			slotRefresh();
	
	/**
	 * Launch emerge pretend of packages.
	 * @param category
	 * @param packageList
	 */
	void			pretendPackage( const QString& category, const QStringList& packageList );
	
	/**
	 * Launch unmerge of packages
	 * @param category
	 * @param packageList
	 */
	void			uninstallPackage( const QString& category, const QStringList& packageList );
	
	/**
	 * @fixme: Check for failure.
	 * Add package as installed in db.
	 * @param package
	 */
	void			addPackage( const QString& package );
	
	/**
	 * @fixme: Check for failure.
	 * Remove packages from db.
	 * @param packageIdList
 	*/
	void			removePackage( const QString& package );
	
	/**
	 * Get list of all categories for installed packages.
	 * @return QStringList
	 */
	QStringList		categories();
	
	/**
	 * Get list of packages in this category from database.
	 * @param category
	 * @return QStringList
	 */
	QStringList		packagesInCategory( const QString& category );
	
	/**
	 * Find packages by name or description.
	 * @param text		string
	 * @param isName	find in name or description
	 */
	void			findPackage( const QString& text, const bool& isName );
	
	/**
	 * Return info for package as description, homepage ...
	 * @return info
	 */
	Info			packageInfo( const QString& packageId );
	
	/**
	 * Methods for parsing info files.
	 */
	QString			installedFiles( const QString& packageId );
	QString			installedSummary( const QString& packageId );
	
	/**
	 * Count total installed packages.
	 * @return total
	 */
	QString			count();
	
	/**
	 * Convert emerge duration from seconds to format hh:mm:ss.
	 * @param time 			
	 * @return emergeTime  
	 */
	QString			timeFormat( const QString& time );
	
signals:
	void			signalInstalledChanged();
	void			signalInstalledReset();
	
private:
	QObject			*parent;
	QString			package;
	
};

#endif
