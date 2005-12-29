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

#include <qpixmap.h>

class PackageVersion;

typedef QMap<QString, QString> Meta;

/**
 * @class PackageItem
 * @short KListViewItem subclass to implement sorting, tooltip, color...
 */
class PackageItem : public KListViewItem
{
public:
	PackageItem( QListView *parent, const QString& id, const char* name, const QString& description, const QString& homepage, const QString& status );
	~PackageItem();

	void							initVersions();
	QValueList<PackageVersion*> 	sortedVersionList();
	
	
	QStringList		getVersionsList();
	int				status();
	QString 		id();
	QString			name();
	QString			description();
	QString			homepage();
	
	/**
	 * @return meta inf on package
	 */
	Meta			getMeta();
	
	/**
 	 * Is the listViewItem category, package or ebuild.
 	 * Set icon and tooltip text.
 	 * @param status
 	 */
	void			setStatus( int status );
	
protected:
	
	/**
 	 * Insert package meta text into the right column.
 	 */
	void			init();
	
	/**
 	 * Convenience method.
 	 * @return true if package is in the queue.
 	 */
	virtual bool	isQueued();
	
	/**
 	 * Compare size and order numerically.
 	 * @param i
 	 * @param col
 	 * @param ascending
  	 * @return -1 or 0 or 1
  	 */
// 	virtual int		compare( QListViewItem*, int, bool ) const;
	
	/**
 	 * Mark package as "masked"/"unmasked"/"present in queue" with text formating.
	 * @param p
	 * @param cq
	 * @param column
	 * @param width
	 * @param alignment
	 */
// 	virtual void 	paintCell( QPainter *p, const QColorGroup &cg,int column, int width, int alignment );
	
// protected:
// 	virtual PackageVersion* createPackageVersion( const QString& version ) = 0;
	
private:
	QString									m_id, m_name, m_description, m_homepage, m_status, m_packageTip;
	QListView								*m_parent;
	Meta									m_meta;
	QPixmap 								pxPackageHeader, pxCategory, pxPackage, pxInstalled, pxStable, pxTesting, pxStableUnmasked;
	QPixmap									pxPackageUnmasked, pxInstalledUnmasked, pxEbuild, pxEbuildMasked, pxEbuildInstalled, pxQueued;
	bool									queued, versionsLoaded;
	
	typedef QMap<QString, PackageVersion*>	PackageVersionMap;
	PackageVersionMap						m_versions;
};

#endif
