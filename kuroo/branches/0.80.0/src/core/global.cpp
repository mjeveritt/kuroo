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

#include <kdebug.h>

/**
 * @class Global
 * @short Some useful global methods.
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
const QString Global::kurooDir()
{
	return "/var/cache/kuroo/";
}

/**
 * Parse out category, package name and version parts from package.
 */
const QStringList Global::parsePackage( const QString& packageString )
{
	QRegExp rx( "(?:[a-z]|[A-Z]|[0-9]|-)*((-(?:\\d+\\.)*\\d+[a-z]?)(?:_(?=alpha|beta|pre|rc|p)\\d*)?(?:-r\\d*)?)" );
	QStringList list;
	QString nameVersion;
	QString package( packageString );
	package.remove( ' ' );
		
	// Parse out the category first
	if ( package.contains( '/' ) ) {
		list << package.section( "/", 0, 0 );
		nameVersion = package.section( "/", 1, 1 );
	}
	else {
		list << QString::null;
		nameVersion = package;
	}

	// Now package name and version
	if ( rx.search( nameVersion ) != -1 ) {
		QString name = nameVersion.section( rx.cap( 1 ), 0, 0 );
		list << name;
		list << nameVersion.section( name + "-", 1 );
		return list;
	}
	else
		return QStringList::QStringList();
}

/**
 * Kuroo widget parameters.
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
const long Global::kurooViewId()
{
	return	m_wId;
}

const QString Global::bgHexColor()
{
	return m_bgColor;
}

const QString Global::fgHexColor()
{
	return m_fgColor;
}

#include "global.moc"
