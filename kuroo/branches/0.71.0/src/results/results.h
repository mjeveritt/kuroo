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

#ifndef RESULTS_H
#define RESULTS_H

#include "package.h"

#include <qobject.h>

/**
 * @class Results
 * @short Object for the resulting list of packages from emerge actions.
 */
class Results : public QObject
{
Q_OBJECT	
public:
	Results( QObject *parent = 0 );
    ~Results();
	
public slots:
	void			init( QObject *myParent = 0 );
	void			refresh();
	void			reset();
	
	/**
	 * Add packages by id to the results table in the db
	 * @param packageIdList
	 */
	void 			addPackageIdList( const QStringList& packageIdList );
	
	/**
 	 * Add packages to the results table in the db
 	 * @param packageList
 	 */
	void 			addPackageList( const EmergePackageList &packageList );
	
	/**
	 * Launch emerge pretend of packages.
	 * @param packageList
	 */
	void			pretendPackageList( const QStringList& packageList );
	
	/**
 	 * Get list of all packages.
 	 * @return QStringList
	 */
	QStringList		allPackages();
	QString			count();
	
signals:
	void			signalResultsChanged();
	
private:
	QObject			*parent;
};

#endif
