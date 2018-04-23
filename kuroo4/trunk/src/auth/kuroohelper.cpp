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

#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QString>
#include <QUrl>
#include <QVariantMap>
#include <KIO/SimpleJob>
#include <KProcess>

#include "kuroohelper.h"

using namespace KAuth;

ActionReply KurooHelper::createkuroodir( const QVariantMap& args )
{
	Q_UNUSED( args );
	ActionReply reply;
	QString kurooDir( "/var/cache/kuroo" );
	QDir d( kurooDir );
	if ( !d.exists()) {
		if ( !d.mkdir( kurooDir ) ) {
			reply = ActionReply::HelperErrorReply();
			reply.setErrorDescription( "Couldn't create kuroo dir" );
			return reply;
		} else {
			if (-1 == chmod( kurooDir.toAscii(), 0770 ) ) {
				reply = ActionReply::HelperErrorReply();
				reply.setError( errno ? errno : -1 );
				reply.setErrorDescription( "Couldn't change mode of kuroo dir" );
				return reply;
			}
			if ( !KIO::chown( QUrl::fromLocalFile( kurooDir ), "portage", "portage" )->exec() ) {
				reply = ActionReply::HelperErrorReply();
				reply.setErrorDescription( "Couldn't change group on kuroo dir" );
				return reply;
			}
		}
	}
	reply = ActionReply::SuccessReply();
	return reply;
}

ActionReply KurooHelper::chownkuroodir( const QVariantMap& args )
{
	Q_UNUSED( args );
	ActionReply reply;
	//Force it to the right ownership in case it was created as root:root previously
	if ( !KIO::chown( QUrl::fromLocalFile( "/var/cache/kuroo" ), "portage", "portage"  )->exec()) {
		reply = ActionReply::HelperErrorReply();
		reply.setErrorDescription( "Couldn't change group on kuroo dir" );
		return reply;
	}
	reply = ActionReply::SuccessReply();
	return reply;
}

ActionReply KurooHelper::emerge( const QVariantMap& args)
{
	Q_UNUSED( args );
	return ActionReply::InvalidActionReply();
}

ActionReply KurooHelper::sync( const QVariantMap& args )
{
	Q_UNUSED( args );
	if ( NULL != syncProc ) {
		syncProc->close();
		syncProc->clearProgram();
	} else {
		syncProc = new KProcess();
		//use merged mode because emerge seems to output everything on stdout when there's any error (like a slot conflict)
		//including all the useful information
		syncProc->setOutputChannelMode( KProcess::MergedChannels );
	}

	*syncProc << "emerge" << "--sync" << "--quiet" << "--color=n" << "--nospinner";

	connect(syncProc, &KProcess::readyReadStandardOutput, this, &KurooHelper::slotEmergeOutput);
	syncProc->start();

	qDebug() << "Started sync";
	while ( QProcess::Running == syncProc->state() ) {
		if ( HelperSupport::isStopped() ) {
			qDebug() << "Sync cancelled by user";
			syncProc->kill();
			return ActionReply::UserCancelledReply();
		}
		sleep( 10 );
		qDebug() << "Sync tick";
	}
	qDebug() << "Sync process is no longer running, status is now " << syncProc->state();
	return ActionReply::SuccessReply();
}

void KurooHelper::slotEmergeOutput()
{
	QByteArray data = syncProc->readAllStandardOutput();
	QVariantMap retVal;
	retVal["output"] = data;
	KAuth::HelperSupport::progressStep( retVal );
}


KAUTH_HELPER_MAIN("org.gentoo.portage.kuroo", KurooHelper)
