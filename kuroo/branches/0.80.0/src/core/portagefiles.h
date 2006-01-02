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

#ifndef PORTAGEFILES_H
#define PORTAGEFILES_H

#include <qobject.h>

/**
 * @class PortageFiles
 * @short Object for handling masked packages.
 */
class PortageFiles : public QObject
{
Q_OBJECT	
public:
	PortageFiles( QObject *parent = 0 );
    ~PortageFiles();
	
public slots:
	void			init( QObject *myParent = 0 );
	void			refresh();
	
	/**
 	 * Add packages to the results table in the db
 	 * @param packageList
 	 */
	void 			loadPackageMask();
	void			loadPackageMaskUser();
	void 			loadPackageUnmask();
	void			loadPackageKeywords();
	QStringList		getHardMaskedAtom( const QString& id );

private slots:
	
signals:
	void			signalPortageFilesChanged();
	
private:
	QObject*		parent;
};

#endif
