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

#ifndef PACKAGEITEM_H
#define PACKAGEITEM_H

#include <klistview.h>

#include <qobject.h>

class PackageVersion;
class DependAtom;

/**
 * @class PackageItem
 * @short KListViewItem subclass
 */
class PackageItem : public KListViewItem
{
public:
	PackageItem( QListView *parent, const char* name, const QString& id, const QString& category, const QString& description, const int status );
	PackageItem( QListViewItem *parent, const char* name, const QString& id, const QString& category, const QString& description, const int status );
	~PackageItem();
	/**
	 * Package status describing if this package is installed or not.
	 * @return status
	 */
	inline const int					status() const { return m_status; }
	/**
	 * Package db id.
	 * @return id
	 */
	inline const QString& 					id() const { return m_id; }
	/**
	 * Package name as kuroo in app-portage/kuroo.
	 * @return name
	 */
	inline const QString&					name() const { return m_name; }
	/**
	 * Accessor for category.
	 * @return the package category.
	 */
	inline const QString& 					category() const { return m_category; }
	/**
	 * Package description.
	 * @return description
	 */
	inline const QString&					description() const { return m_description; }
	
	virtual void						setPackageIndex( const int& index );	
	virtual bool						isInstalled() const;
	virtual bool						isInPortage() const;
	/**
	 * Is this package is in the emerge queue?
	 * @return true/false
	 */
	inline virtual bool					isQueued() const { return m_isQueued; }
	/**
	 * Is this package in world?
	 * @return true/false
	 */
	inline virtual bool					isInWorld() const { return m_inWorld; }
	
// 	void							setRollOver( bool isMouseOver );
	void							setInstalled();
	void							setDescription( const QString& description );
	void							setQueued( const bool& isQueued );
	bool							isFirstPackage() const;
	/**
	 * Is this the last package?
	 * @return true if last
	 */
	inline bool						isLastPackage() const  { return ( m_index == 1 ); }
	
	void 							initVersions();
	/**
	 * Return list of versions.
	 * @return QValueList<PackageVersion*>
	 */
	inline QValueList<PackageVersion*> 			versionList()const  { return m_versions; }
	/**
	 * Return map of versions - faster find.
	 * @return QMap<QString, PackageVersion*>
	 */
	inline QMap<QString,PackageVersion*> 			versionMap() const { return m_versionMap; }
	QValueList<PackageVersion*> 				sortedVersionList();
	void							resetDetailedInfo();

	void							parsePackageVersions();
	/**
	 * Return versions list together with stability info etc...
	 */
	inline const QStringList&				versionDataList() const { return m_versionsDataList; }
	/**
	 * Return version used by emerge.
	 */
	inline const QString&					emergeVersion() const { return m_emergeVersion; }
	/** Queries homepage of application
	 * @return that homepage
	 */
	inline const QString&					homepage() const { return m_homepage; }
	/**
	 * Returns list of installed versions in html-format.
	 */
	inline const QString&					linesInstalled() const { return m_linesInstalled; }
	/**
	 * Returns list of available versions in html-format.
	 */
	inline const QString&					linesAvailable() const { return m_linesAvailable; }
	/**
	 * Returns list emergeable versions in html-format.
	 */
	inline const QString&					linesEmerge() const { return m_linesEmerge; }
	inline const bool					isInArch() const { return m_isInArch; }
	
protected:
	void			paintCell( QPainter* painter, const QColorGroup& colorgroup, int column, int width, int alignment );
	
private:
	QListView						*m_parent;
	
	// Is the mouse pointer over this package
	bool							m_isMouseOver;
	
	// Keep track of package's index in parent listview
	int							m_index;
	
	// Package's db id
	QString							m_id;
	
	// Package name-string
	QString							m_name;
	
	// Is package INSTALLED or OLD ( INSTALLED but not in Portage anymore )
	int							m_status;
	
	// Package description
	QString							m_description;
	
	// Keep track of package's category
	QString							m_category;
	
	// True if package is in installation queue
	bool							m_isQueued;
	
	// True if package is in world file
	bool							m_inWorld;
	
	// True if package and its versions has been initialized with all data
	bool							m_isInitialized;
	
	// Valuelist with all versions and their data
	QValueList<PackageVersion*>				m_versions;
	
	// Alternatively map with all versions and their data
	QMap<QString, PackageVersion*>				m_versionMap;
	
	// Atom object needed for versions stability
	DependAtom* 						atom;

	// Formatted string
	QString							m_linesInstalled, m_linesAvailable, m_linesEmerge;
	
	// Version used by emerge
	QString							m_emergeVersion;
	
	// Latest versions homepage supposed to be most current
	QString							m_homepage;
	
	// Versions list together with stability info etc...
	QStringList						m_versionsDataList;
	
	// Is this package available in this arch?
	bool							m_isInArch;
};

#endif
