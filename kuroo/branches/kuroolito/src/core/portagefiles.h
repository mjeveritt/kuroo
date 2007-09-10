/***************************************************************************
*   Copyright (C) 2007 by Karye                                           *
*   info@kuroo.org                                                        *
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
 * @short Loading and saving portage files.
 */
class PortageFiles : public QObject
{
Q_OBJECT	
public:
	PortageFiles( QObject *m_parent = 0 );
    ~PortageFiles();

	void			init( QObject *parent = 0 );
	void			refresh( int mask );
	
	void 			loadPackageFiles();
	void			loadPackageKeywords();
	void 			loadPackageUnmask();
	void			loadPackageUserMask();
	void			loadPackageUse();
	
	void			savePackageKeywords();
	void 			savePackageUserUnMask();
	void			savePackageUserMask();
	void			savePackageUse();
	
signals:
	void			signalPortageFilesChanged();
	
private:
	QObject*		m_parent;
};

#endif
