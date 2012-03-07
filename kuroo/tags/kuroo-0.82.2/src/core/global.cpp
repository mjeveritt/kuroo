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

#include "common.h"
#include "global.h"

#include <qwidget.h>
#include <qpalette.h>
#include <qdatetime.h>

#include <kglobal.h>

const QString Global::kuroo_dir=QString("/var/cache/kuroo/");

/**
 * @class Global
 * @short Some useful global methods.
 */
Global::Global( QObject *parent )
	: QObject( parent )
{}

Global::~Global()
{}

void Global::init( QObject *parent )
{
	m_parent = parent;
}

/** Gets Kuroo's working directory
 *
 */
const QString Global::kurooDir() const { return kuroo_dir; }

/**
 * Regexp to parse emerge output.
 */
const QRegExp Global::rxEmerge() const
{
	if ( KurooConfig::portageVersion21() )
		return QRegExp( "^\\[ebuild([\\s\\w~]*)\\]\\s+"
		                "((\\S+)/(\\S+))"
		                "(?:\\s*\\[([^\\]]*)\\])?"
		                "(?:\\s*\\[([^\\]]*)\\])?"
		                "(?:\\s*USE=\"([^\"]*)\")?"
		                "(?:\\s*LINGUAS=\"(?:[^\"]*)\")?"
		                "(?:\\s(\\d*,?\\d*)\\skB)?" );
	else
		return QRegExp( "^\\[ebuild([\\s\\w]*)\\]\\s+"
		                "((\\S+)/(\\S+))"
		                "(?:\\s*\\[([^\\]]*)\\])?"
		                "(?:\\s*\\[([^\\]]*)\\])?"
		                "((?:\\s*[\\(\\-\\+]+\\w+[\\)%]?)*)"
		                "(?:\\s(\\d*,?\\d*)\\skB)?" );
}

/**
 * Parse out category, package name and version parts from package.
 * @param sring to parse out cpv from
 */
const QStringList Global::parsePackage( const QString& packageString ) const
{
	QRegExp rx( "(?:[a-z]|[A-Z]|[0-9]|-)*"
	            "("
	            "(-(?:\\d+\\.)*\\d+[a-z]?)"
	            "(?:_(?=alpha|beta|pre|rc|p)\\d*)?"
	            "(?:-r\\d*)?"
	            ")" );
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
		return QStringList();
}

/**
 * Kuroo widget parameters.
 */
void Global::setColorTheme()
{
	QColor c = KGlobalSettings::highlightColor();
	m_bgColor = QString::number( c.red(), 16 ) + QString::number( c.green(), 16 ) + QString::number( c.blue(), 16 );
	
	c = KGlobalSettings::highlightedTextColor();
	m_fgColor = QString::number( c.red(), 16 ) + QString::number( c.green(), 16 ) + QString::number( c.blue(), 16 );
}

/**
 * Return duration in seconds formated as "d hh.mm.ss".
 */
const QString Global::formatTime( const long& duration ) const
{
	KLocale *loc = KGlobal::locale();
	QString totalDays;
	unsigned durationDays, totalSeconds;
	
	durationDays = duration / 86400;
	totalSeconds = duration % 86400;
	
	if ( durationDays > 0 )
		totalDays = i18n( "%1d " ).arg( QString::number( durationDays ) );
	
	QTime emergeTime( 0, 0, 0 );
	emergeTime = emergeTime.addSecs( totalSeconds );
	return totalDays + loc->formatTime( emergeTime, true, true );
}

#include "global.moc"
