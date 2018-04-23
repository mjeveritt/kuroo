/*
 * Kuroo - A KDE frontend to Gentoo Portage
 * Copyright 2018  Amber Schenck galiven@users.sourceforge.net
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KUROOHELPER_H
#define KUROOHELPER_H

#include <QObject>
#include <KAuth>
#include <KProcess>

using namespace KAuth;

/**
 * Helper class for KAuth actions
 */
class KurooHelper : public QObject
{
    Q_OBJECT

public slots:
	ActionReply createkuroodir( const QVariantMap& args );
	ActionReply chownkuroodir( const QVariantMap& args );
	ActionReply emerge( const QVariantMap& args );
	ActionReply sync( const QVariantMap& args );

private:
	KProcess* syncProc;

private slots:
    void 						slotEmergeOutput();
};

#endif // KUROOHELPER_H
