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
#include "packageitem.h"

class PackageItem;
class KProgress;

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

	class					QueueItem;
	
public slots:
	void					slotPackageUp();
	void					slotPackageDown();
	QStringList				allPackagesNoChildren();
	QStringList				allEndUserPackages();
	void 					insertPackageList( bool hasCheckedQueue );
	void					clearQueuePackageUse();
	QTime			 		totalTime();
	int						sumTime();
	QString					totalTimeFormatted();
	QString		 			totalSize();
	void					slotPackageComplete( const QString& id, bool removeInstalled );
	void					slotPackageStart( const QString& id );
	void					slotPackageProgress();
	
private slots:
	void					viewportResizeEvent(QResizeEvent *e);
	void					slotHideBars( QListViewItem* item );
	QString		 			formatSize( const QString& sizeString );
	void 					addSize( const QString& size );
	QString 				formatTime( int time );
	
signals:
	void					signalQueueLoaded();
	void					signalPackageEmerged();
	
private:
	KLocale 				*loc;
	int 					sumSize;
	QString					m_id;
};

/**
 * @class QueueListView::QueueItem
 * @short Package item with progressbar
 */
class QueueListView::QueueItem : public PackageItem
{
public:
	QueueItem::QueueItem( QListView* parent, const QString& category, const QString& name, const QString &id, const QString& description, const QString& status, int duration );
	QueueItem::QueueItem( QueueItem* parent, const QString& category, const QString& name, const QString &id, const QString& description, const QString& status, int duration );
	~QueueItem();
	
	void			setComplete();
	bool			isComplete();
	int				remainingDuration();
	void			setStart();
	void			oneStep();
	void			setChecked( bool isChecked );
	void			hideBar();
	
protected:
	void 			paintCell( QPainter* painter, const QColorGroup& colorgroup, int column, int width, int alignment );
	
private:
	KProgress* 		bar;
	int				m_progress, m_duration;
	bool			m_isChecked, m_isComplete;
};

#endif
