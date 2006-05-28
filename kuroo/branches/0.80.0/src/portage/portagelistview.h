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

#ifndef PORTAGELISTVIEW_H
#define PORTAGELISTVIEW_H

#include "packagelistview.h"
#include "packageitem.h"

class PackageVersion;
class DependAtom;
class KTextBrowser;

/**
 * @class PortageListView
 * @short Installed packages listview.
 */
class PortageListView : public PackageListView
{
Q_OBJECT
public:
	PortageListView( QWidget *parent = 0, const char *name = 0 );
	~PortageListView();
	
	class 							PortageItem;
	
	void							showNoHitsWarning( bool noHits );
	PortageListView::PortageItem* 	currentPortagePackage();
	void							setHeader( const QString& text );
	int 							addSubCategoryPackages( const QStringList& packageList );
	
protected:
	KTextBrowser 					*noHitsWarning;
};

/**
 * @class PortageListView::PortageItem
 * @short Package item with all versions.
 */
class PortageListView::PortageItem : public PackageItem
{
public:
	PortageItem( QListView* parent, const char* name, const QString &id, const QString& category, const QString& description, const int status );
	
private:
	void							paintCell( QPainter* painter, const QColorGroup& colorgroup, int column, int width, int alignment );
	
protected:
	QListView						*m_parent;
};

#endif
