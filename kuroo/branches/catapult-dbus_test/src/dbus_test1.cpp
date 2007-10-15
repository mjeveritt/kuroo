/***************************************************************************
 *   Copyright (C) 2007 by Karim Ryde   *
 *   karim@bredband.net   *
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


#include "dbus_test1.h"

#include <qlabel.h>

// Qt DBUS includes
#include <dbus/qdbusdatalist.h>
#include <dbus/qdbuserror.h>
#include <dbus/qdbusconnection.h>
#include <dbus/qdbusmessage.h>
#include <dbus/qdbusproxy.h>

#include <kmainwindow.h>
#include <klocale.h>

dbus_test1::dbus_test1()
    : KMainWindow( 0, "dbus_test1" )
{
    // set the shell's ui resource file
    setXMLFile("dbus_test1ui.rc");

	QString hostname("kde.org");
	QDBusConnection bus = QDBusConnection::sessionBus();
	QDBusInterface *interface = new QDBusInterface("org.foo.bar", "/network", "org.foo.bar.network", bus, this);
	interface->call("ping");
	interface->call("ping", hostname);
	
	QList<QVariant> args;
	args.append("kde.org");
	interface->callWithArgumentList("ping", args);
	QDBusReply<int> reply = interface->call("ping",hostname);
	
	if (reply.isValid()) {
		KMessageBox::information(winId(), i18n("Ping to %1 took %2s").arg(hostname).arg(reply.value()), i18n("Pinging %1").arg(hostname));
	}
	args.clear();
	interface->callWithCallback("listInterfaces", args, this, SLOT(interfaceList(QDBusMessage));
	connect(interface, SIGNAL(interfaceUp(QString)), this, SLOT(interfaceUp(QString)));
	
    new QLabel( "Hello World", this, "hello label" );
}

dbus_test1::~dbus_test1()
{
}

#include "dbus_test1.moc"
