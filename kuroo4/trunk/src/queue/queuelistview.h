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
#include "QProgressBar"

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
	
	const QStringList		allPackagesNoChildren();
	const QStringList		allEndUserPackages();
	void 					insertPackageList( bool hasCheckedQueue );
	long			 		totalDuration();
	const QString		 	totalSize();
	
public slots:
	void					slotPackageUp();
	void					slotPackageDown();
	void					slotPackageComplete( const QString& id );
	void					slotPackageStart( const QString& id );
	void					slotPackageProgress();
	
private:
	void					viewportResizeEvent( QResizeEvent* );
	const QString		 	formatSize( const QString& sizeString );
	void 					addSize( const QString& size );
	
private slots:
    void					slotHideBars( QTreeWidgetItem* item );
	
signals:
	void					signalPackageEmerged();
	
private:

	// Total sum of all package emerge duration 
	int 					m_sumSize;
	
	// Current emerging package
	QString					m_id;
};

/**
 * @class QueueListView::QueueItem
 * @short Package item with progressbar
 */
class QueueListView::QueueItem : public PackageItem
{
public:
    QueueItem( QTreeWidget* parent, const QString& category, const QString& name, const QString &id, const int status, int duration );
    QueueItem( QueueItem* parent, const QString& category, const QString& name, const QString &id, const int status, int duration );
	~QueueItem();
	
	QString			package();
	void			setComplete();
	bool			isComplete();
	int				remainingDuration();
	void			setStart();
	void			oneStep();
	void			setPretended( bool isChecked );
	void			hideBar();
	
protected:
    void 			paintCell( QPainter* painter, const QPalette& colorgroup, int column, int width, int alignment );
	
private:
	
	// Individual package progressbar
    QProgressBar* 		m_bar;
	
	// Current emerge progress in seconds
	int				m_progress;
	
	// Total emerge duration
	int				m_duration;
	
	// Is this package ok by "emerge --pretend"
	bool			m_isChecked;
	
	// Is this package progress = 100% eg completed
	bool			m_isComplete;
	
	QString			m_package;
};

#endif
