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

#ifndef INSTALLED_H
#define INSTALLED_H

#include <qobject.h>

/**
 * @class Installed
 * @short Object for installed packages.
 */
class Installed : public QObject
{
Q_OBJECT
public:
	Installed( QObject *m_parent = 0 );
    ~Installed();
	
public slots:
	void			init( QObject *parent = 0 );
	void			slotChanged();
	void			slotReset();
	void			uninstallPackageList( const QStringList& packageIdList );
	void			addPackage( const QString& package );
	void			removePackage( const QString& package );
	
signals:
	void			signalInstalledChanged();
	void			signalInstalledReset();
	
private:
	QObject			*m_parent;
	
};

#endif
