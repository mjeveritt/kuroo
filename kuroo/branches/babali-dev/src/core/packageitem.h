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
	
	const int						status() const;
	const QString& 					id() const;
	const QString&					name() const;
	const QString& 					category() const;
	const QString&					description() const;
	
	virtual void					setPackageIndex( int index );	
	virtual bool					isInstalled();
	virtual bool					isInPortage();
	virtual bool					isQueued();
	virtual bool					isInWorld();
	
// 	void							setRollOver( bool isMouseOver );
	void							setInstalled();
	void							setDescription( const QString& description );
	void							setQueued( bool isQueued );
	bool							isFirstPackage();
	bool							isLastPackage();
	
	void 							initVersions();
	QValueList<PackageVersion*> 	versionList();
	QMap<QString,PackageVersion*> 	versionMap();
	QValueList<PackageVersion*> 	sortedVersionList();
	void							resetDetailedInfo();

	void							parsePackageVersions();
	const QStringList&				versionDataList() const;
	const QString&					emergeVersion() const;
	const QString&					homepage() const;
	const QString&					linesInstalled() const;
	const QString&					linesAvailable() const;
	const QString&					linesEmerge() const;
	const bool						isInArch() const;
	
protected:
	void							paintCell( QPainter* painter, const QColorGroup& colorgroup, int column, int width, int alignment );
	
private:
	QListView						*m_parent;
	
	// Is the mouse pointer over this package
	bool							m_isMouseOver;
	
	// Keep track of package's index in parent listview
	int								m_index;
	
	// Package's db id
	QString							m_id;
	
	// Package name-string
	QString							m_name;
	
	// Is package INSTALLED or OLD ( INSTALLED but not in Portage anymore )
	int								m_status;
	
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
	QValueList<PackageVersion*>		m_versions;
	
	// Alternatively map with all versions and their data
	QMap<QString, PackageVersion*>	m_versionMap;
	
	// Atom object needed for versions stability
	DependAtom* 					atom;

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
