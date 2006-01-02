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

#ifndef PACKAGEMASK_H
#define PACKAGEMASK_H

#include <qobject.h>

/**
 * @class PackageMask
 * @short Object for handling masked packages.
 */
class PackageMask : public QObject
{
Q_OBJECT	
public:
	PackageMask( QObject *parent = 0 );
    ~PackageMask();
	
public slots:
	void			init( QObject *myParent = 0 );
	void			refresh();
	
	/**
 	 * Add packages to the results table in the db
 	 * @param packageList
 	 */
	void 			loadPackageMask();
	void 			loadPackageUnmask();
	QStringList		getHardMaskedAtom( const QString& id );

private slots:
	
signals:
	void			signalPackageMaskChanged();
	
private:
	bool			isDirtyPackageMask;
	QObject*		parent;
};

#endif
