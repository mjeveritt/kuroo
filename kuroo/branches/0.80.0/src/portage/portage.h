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

#ifndef PORTAGE_H
#define PORTAGE_H

#include <qobject.h>

/**
 * @class Portage
 * @short Object for the Portage tree.
 */
class Portage : public QObject
{
Q_OBJECT
public:
	Portage( QObject *m_parent = 0 );
    ~Portage();
	
	void						loadWorld();
	bool						isInWorld( const QString& package );
	
public slots:
	void						init( QObject *parent = 0 );
	void						slotChanged();
	
	/**
	 * Start scan of portage packages.
	 * @return bool
	 */
	bool						slotRefresh();
	
	/**
	 * Continue scan of portage packages.
	 * @return bool
	 */
	bool						slotScan();
	
	/**
	 * Start emerge sync.
	 * @return bool
	 */
	bool						slotSync();
	
	/**
	 * Launch emerge pretend of packages.
	 * @param category
	 * @param packageList
	 */
	void						pretendPackageList( const QStringList& packageIdList );
	
	/**
	* Find cached size for package.
	* @param packages
	* @return size or NULL if na
	*/
	QString						cacheFind( const QString& package );
	
	/**
	* Get cache from threaded scan.
	*/
	void						setCache( const QMap< QString, QString > &mapCacheIn );
	
	/**
	* Load mapCache with items from DB.
	*/
	void						loadCache();
	
	/**
	* Free cache memory.
	*/
	void						clearCache();
	
	/**
	* Unmask list of packages by adding them to package.keyword.
	* @param category
	* @param packageList
	*/
	void						untestingPackageList( const QStringList& packageIdList );
	
	/**
	* Unmask package by adding to "maskFile".
	* @param package
	* @param maskFile
	* @return success
	*/
	bool						unmaskPackage( const QString& package, const QString& maskFile );
	
	/**
	* Clear the unmasking of packages by removing them to package.keyword.
	* @param category
	* @param packageList
	*/
	void						saveWorld();
	void						appendWorld( const QString& package );
	void						removeFromWorld( const QString& package );
	
signals:
	void						signalPortageChanged();
	void						signalWorldChanged();
	
private:
	QObject*					m_parent;
	QMap<QString, QString> 		mapCache, mapPackageKeywords;
	QMap<QString, QString>		mapWorld;
};

#endif
