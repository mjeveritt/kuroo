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
	PackageItem( QListView *parent, const char* name, const QString& id, const QString& category, const QString& description, const QString& status );
	PackageItem( QListViewItem *parent, const char* name, const QString& id, const QString& category, const QString& description, const QString& status );
	~PackageItem();
	
	QString							status();
	QString 						id();
	QString							name();
	QString							description();
	
	virtual void					setPackageIndex( int index );
	virtual void					setStatus( int status );
	
	virtual bool					isInstalled();
	virtual bool					isQueued();
	virtual bool					isInWorld();
	
	void							setQueued();
	
	bool							isFirstPackage();
	bool							isLastPackage();
	
	QString 						category();
	void 							initVersions();
	QValueList<PackageVersion*> 	versionList();
	QMap<QString,PackageVersion*> 	versionMap();
	QValueList<PackageVersion*> 	sortedVersionList();
	void							resetDetailedInfo();

protected:
	void							paintCell( QPainter* painter, const QColorGroup& colorgroup, int column, int width, int alignment );
	
private:
	int								m_index;
	
	QString							m_id, m_name, m_status, m_packageTip, m_description, m_category;
	QListView						*m_parent;
	bool							m_isQueued, m_inWorld, hasDetailedInfo;
	
	QValueList<PackageVersion*>		m_versions;
	QMap<QString,PackageVersion*>	m_versionMap;
	DependAtom* 					atom;

};

#endif
