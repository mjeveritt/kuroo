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

#include "global.h"

#include <qregexp.h>
#include <qwidget.h>
#include <qpalette.h>

/**
 * @class Global
 * @short To keep global variables
 */
Global::Global( QObject *parent )
	: QObject( parent )
{
}

Global::~Global()
{
}

void Global::init( QObject *parent )
{
	m_parent = parent;
}

/**
 * Kuroo home directory
 */
QString Global::kurooDir()
{
	return "/var/cache/kuroo/";
}

/**
 * Regexp from Portage.
 */
QRegExp Global::rxPortageVersion()
{
	return QRegExp("(?:[a-z]|[A-Z]|[0-9]|-)*((-(?:\\d+\\.)*\\d+[a-z]?)(?:_(?=alpha|beta|pre|rc|p)\\d*)?(?:-r\\d*)?)");
}

/**
 * Kuroo widget.
 */
void Global::setKurooView( QWidget* view )
{
	m_wId = view->winId();
	
	QColor c = view->colorGroup().highlight();
	m_bgColor = QString::number( c.red(), 16 ) + QString::number( c.green(), 16 ) + QString::number( c.blue(), 16 );
	
	c = view->colorGroup().highlightedText();
	m_fgColor = QString::number( c.red(), 16 ) + QString::number( c.green(), 16 ) + QString::number( c.blue(), 16 );
}

/**
 * Kuroo widget id so MessageBox's can be made modal.
 */
long Global::kurooViewId()
{
	return	m_wId;
}

QString Global::bgHexColor()
{
	return m_bgColor;
}

QString Global::fgHexColor()
{
	return m_fgColor;
}

#include "global.moc"
