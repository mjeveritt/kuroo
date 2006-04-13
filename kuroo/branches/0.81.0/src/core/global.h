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

#ifndef GLOBAL_H
#define GLOBAL_H

#include <qobject.h>

class QRegExp;
class QWidget;

/**
 * @class Global
 * @short Some useful global methods.
 */
class Global : public QObject
{
Q_OBJECT
public:
    Global( QObject *parent = 0 );
    ~Global();
	
	void 					init( QObject *parent = 0 );
	const QString 			kurooDir();
	const QStringList 		parsePackage( const QString& packageString );
	void 					setKurooView( QWidget* view );
	const long&				kurooViewId();
	const QString&			bgHexColor();
	const QString&			fgHexColor();
	const QString 			formatTime( long duration );
	
private:
	QObject*				m_parent;
	
	// Kuroo widget id so MessageBox's can be made modal
	long					m_wId;
	
	QString					m_bgColor;
	
	QString					m_fgColor;
};

#endif
