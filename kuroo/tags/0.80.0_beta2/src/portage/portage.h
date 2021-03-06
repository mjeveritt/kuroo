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
	bool						slotRefresh();
	bool						slotScan();
	bool						slotSync();
	void						pretendPackageList( const QStringList& packageIdList );
	QString						cacheFind( const QString& package );
	void						setCache( const QMap< QString, QString > &mapCacheIn );
	void						loadCache();
	void						clearCache();
	void						untestingPackageList( const QStringList& packageIdList );
	bool						unmaskPackage( const QString& package, const QString& maskFile );
	bool						saveWorld( const QMap<QString, QString>& map );
	void						appendWorld( const QString& package );
	void						removeFromWorld( const QString& package );
	
signals:
	void						signalPortageChanged();
	void						signalWorldChanged();
	
private:
	QObject*					m_parent;
	QMap<QString, QString> 		mapCache;
	QMap<QString, QString>		mapWorld;
};

#endif
