/***************************************************************************
 *   Copyright (C) 2005 by Karye   *
 *   karye@users.sourceforge.net   *
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

#ifndef _KUROOVIEW_H_
#define _KUROOVIEW_H_

#include "kurooviewbase.h"

#include <qwidget.h>

#include <kurooiface.h>

class PortageTab;
class QueueTab;
class HistoryTab;
class LogsTab;
class MergeTab;
class PackageInspector;

/**
 * @class KuroolitoView
 * @short Create the gui content with icon-menu and pages.
 */
class KuroolitoView : public KuroolitoViewBase, public kurooIface
{
Q_OBJECT
public:
	KuroolitoView( QWidget *parent, const char *name );
	~KuroolitoView();
	
	PortageTab*			viewPortage;
	PackageInspector	*packageInspector;
	
	void 				slotEmergePretend( QString package );
	
public:
	class 				IconListItem;
	
public slots:
	void 				slotInit();

private slots:
	void 				slotCheckPortage();
	void 				slotPortageUpdated();
	void				slotResetMenu( QListBoxItem* menuItem );
	void 				slotShowView();
	
private:
	IconListItem		*iconPackages, *iconQueue, *iconHistory, *iconLog, *iconMerge/*, *iconWhatsThis*/;
	
	// True if history needs to be recreated from scratch
	bool				m_isHistoryRestored;
};

class KuroolitoView::IconListItem : public QListBoxItem
{
public:
	IconListItem( QListBox *listbox, const QPixmap &pixmap, const QString &text );
	virtual int 		height( const QListBox *lb ) const;
	virtual int 		width( const QListBox *lb ) const;
	int 				expandMinimumWidth( int width );
	void 				setChanged( bool modified );
	bool 				isChanged();
	
protected:
	const QPixmap 		&defaultPixmap();
	void 				paint( QPainter *painter );
	
private:
	bool 				m_modified;
	QPixmap 			mPixmap;
	int 				mMinimumWidth;
};

#endif // _KUROOVIEW_H_
