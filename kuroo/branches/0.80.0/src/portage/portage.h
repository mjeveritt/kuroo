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

class QRegExp;

extern QRegExp pv;

/**
 * @class Portage
 * @short Object for the Portage tree.
 */
class Portage : public QObject
{
Q_OBJECT
public:
	Portage( QObject *parent = 0 );
    ~Portage();
	
public slots:
	void						init( QObject *myParent = 0 );
	void						slotChanged();
	
	QString						category( const QString& id );
	QString						package( const QString& id );
	
	/**
	 * Get this packages database id.
	 * @return idDB
	 */
	QString						idDb( const QString& package );
	
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
	* Get list of all categories, subcategories or packages.
	* @return QStringList
	*/
	QStringList					categories( int filter, const QString& text, int combo );
	QStringList					subCategories( const QString& category, int filter, const QString& text, int combo );
	QStringList					packagesInSubCategory( const QString& category, const QString& subCategoryId, int filter, const QString& text, int combo );
	
	/**
	* Get list of versions available of this package.
	* @param name
	*/
	QStringList					packageVersions( const QString& id );
	QStringList					packageVersionsInfo( const QString& id );
	
	/**
	* Find packages by name or description.
	* @param text		string
	* @param isName	find in name or description
	*/
	QStringList					searchPackages( const QString& text, const bool& isName );
	
	/**
	* Count packages.
	* @return total
	*/
	QString						count();
	
	/**
	* Return info for package as description, homepage ...
	* @return info
	*/
	Info						packageInfo( const QString& packageId );
	
	/**
	* Find cached size for package.
	* @param packages
	* @return size or NULL if na
	*/
	QString						cacheFind( const QString& package );
	
	/**
	* Get cache from threaded scan.
	*/
	void						setCache( const QMap< QString, QString > &cacheMapIn );
	
	/**
	* Load cacheMap with items from DB.
	*/
	void						loadCache();
	
	/**
	* Free cache memory.
	*/
	void						clearCache();
	
	/**
	* Load unmasked packages list = packages in package.keyword.
	*/
	void						loadUnmaskedList();
	
	/**
	* Check if package is unmasked. @fixme not checking if just testing or hardmasked.
	* @param package
	* @return true/false
	*/
	bool						isUnmasked( const QString& package );
	
	/**
	* Unmask list of packages by adding them to package.keyword.
	* @param category
	* @param packageList
	*/
	void						unmaskPackageList( const QStringList& packageIdList );
	
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
	void						clearUnmaskPackageList( const QStringList& packageIdList );
	
	/**
	 * Methods for parsing info files.
	 */
	QString						packageSummary( const QString& packageId );
	QString						versionSummary( const QString& packageId );

signals:
	void						signalPortageChanged();
	
private:
	QObject						*parent;
	QMap< QString, QString > 	cacheMap, unmaskedMap;

};

#endif
