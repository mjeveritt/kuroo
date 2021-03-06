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

#include <QRegularExpression>
#include <QStringList>

//TODO: dispatch in relevant module
Q_GLOBAL_STATIC_WITH_ARGS(QString, kurooDir, ("/var/cache/kuroo/")); //->KConfig
Q_GLOBAL_STATIC_WITH_ARGS(QRegularExpression, m_rxEmerge, ("^\\[ebuild([\\s\\w~#*]*)\\]\\s+"	//Also allow * for merging hardmasked packages
												"((\\S+)/(\\S+))"
												"(?:\\s*\\[([^\\]]*)\\])?"
												"(?:\\s*\\[([^\\]]*)\\])?"
												"(?:\\s*USE=\"([^\"]*)\")?"
												"(?:\\s*LINGUAS=\"(?:[^\"]*)\")?"
												"(?:\\s*\\w+=\"(?:[^\"]*)\")*"		//Capture and discard all extra use-expands
										"(?:\\s*(\\d*(,\\d*)*)\\s(?:Ki|k)B)?" ));
inline const QRegularExpression rxEmerge() { return *m_rxEmerge; } //->Emerge


const QStringList 		parsePackage( const QString& packageString );
const QString 			formatTime( const long& ); //->Queue

#endif
