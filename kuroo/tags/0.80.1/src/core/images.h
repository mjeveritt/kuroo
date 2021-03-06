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

#ifndef IMAGES_H
#define IMAGES_H

#include <qobject.h>
#include <qpixmap.h>

/**
 * @class Images
 * @short Delivers icons in Kuroo.
 */
class Images : public QObject
{
Q_OBJECT
public:
	Images( QObject *m_parent = 0 );
    ~Images();

	void 			init( QObject *parent = 0 );
	const QPixmap&	icon( int image );
	
private:
	QObject*		m_parent;
	QPixmap			pxCategory, pxNew, pxUnmerged, pxPackage, pxInstalled, pxQueued, pxWorld, pxWarning;
	QPixmap			pxEmpty, pxKuroo, pxEmerging, pxQueuedColumn, pxWorldColumn, pxVersionInstalled, pxInstalledColumn;
	QPixmap			pxViewPackages, pxViewQueue, pxViewHistory, pxViewMerge, pxViewLog;
};

#endif
