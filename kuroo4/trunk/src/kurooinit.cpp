/***************************************************************************
 *   Copyright (C) 2005 by Karye   *
 *   karye@users.sourceforge.net   *
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
#include "kurooinit.h"
#include "introdlg.h"

#include <stdlib.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>

#include <qdir.h>
#include <qtextcodec.h>

#include <kcmdlineargs.h>
#include <kstringhandler.h>
#include <kuser.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <KProcess>

/**
 * @class KurooInit
 * @short KurooInit checks that kuroo environment is correctly setup.
 *
 * And launch intro wizard whenever a new version of kuroo is installed.
 * Set ownership for directories and files to portage:portage.
 * Check that user is in portage group.
 */
KurooInit::KurooInit( QObject *parent )
	: QObject( parent ), wizardDialog( 0 )
{
	kDebug() << "Initializing Kuroo Environment" << LINE_INFO;
	getEnvironment();

	// Run intro if new version is installed or no DirHome directory is detected.
	QDir d( kurooDir );
	if ( KurooConfig::version() != KurooConfig::hardVersion() || !d.exists() || KurooConfig::wizard() ) {
		firstTimeWizard();
	} else {
		if ( !KUser().isSuperUser() ) {
			checkUser();
		}
	}

	// Get portage groupid to set directories and files owned by portage
	struct group* portageGid = getgrnam( QFile::encodeName("portage") );
	struct passwd* portageUid = getpwnam( QFile::encodeName("portage") );

	// Setup kuroo environment
	if ( KurooConfig::init() ) {
		KurooConfig::setSaveLog( false );

		// Create DirHome dir and set permissions so common user can run Kuroo
		if ( !d.exists() ) {
			if ( !d.mkdir( kurooDir ) ) {
				KMessageBox::error( 0, i18n("<qt>Could not create kuroo home directory.<br>"
											"You must start Kuroo with kdesu first time for a secure initialization.<br>"
											"Please try again!</qt>"), i18n("Initialization") );
				exit(0);
			} else {
				chmod( kurooDir.toAscii(), 0770 );
				chown( kurooDir.toAscii(), portageGid->gr_gid, portageUid->pw_uid );
			}

			d.setCurrent( kurooDir );
		}
	}

	// Check that backup directory exists and set correct permissions
	QString backupDir = kurooDir + "backup";
	if ( !d.cd( backupDir ) ) {
		if ( !d.mkdir( backupDir ) ) {
			KMessageBox::error( 0, i18n("<qt>Could not create kuroo backup directory.<br>"
										"You must start Kuroo with kdesu first time for a secure initialization.<br>"
										"Please try again!</qt>"), i18n("Initialization") );
			exit(0);
		}
		else {
			chmod( backupDir.toAscii(), 0770 );
			chown( backupDir.toAscii(), portageGid->gr_gid, portageUid->pw_uid );
		}
	}

	KurooConfig::setVersion( KurooConfig::hardVersion() );
	//KurooConfig::writeConfig(); //HACK: port KConfig

	// Initialize the log
	QString logFile = LogSingleton::Instance()->init( this );
	if ( !logFile.isEmpty() ) {
		chmod( logFile.toAscii(), 0660 );
		chown( logFile.toAscii(), portageGid->gr_gid, portageUid->pw_uid );
	}

	// Initialize the database
	QString databaseFile = KurooDBSingleton::Instance()->init( this );
	QString database = kurooDir + KurooConfig::databas();
	QString dbVersion = KurooDBSingleton::Instance()->getKurooDbMeta( "kurooVersion" );

	// Check for conflicting db design or new install
	if ( KurooConfig::version().section( "_db", 1, 1 ) != dbVersion ) {

		// Backup history if there's old db version
		if ( !dbVersion.isEmpty() ) {
			KurooDBSingleton::Instance()->backupDb();
			remove( database.toAscii() );
			kWarning(0) << QString("Database structure is changed. Deleting old version of database %1").arg( database ) << LINE_INFO;

			// and recreate with new structure
			KurooDBSingleton::Instance()->init( this );
		}

		KurooDBSingleton::Instance()->setKurooDbMeta( "kurooVersion", KurooConfig::version().section( "_db", 1, 1 ) );
	}

	// Give permissions to portage:portage to access the db also
	chmod( databaseFile.toAscii(), 0660 );
	chown( databaseFile.toAscii(), portageGid->gr_gid, portageUid->pw_uid );

	// Initialize singletons objects
	ImagesSingleton::Instance()->init( this );
	SignalistSingleton::Instance()->init( this );
	EmergeSingleton::Instance()->init( this );
	EtcUpdateSingleton::Instance()->init( this );
	HistorySingleton::Instance()->init( this );
	PortageSingleton::Instance()->init( this );
	QueueSingleton::Instance()->init( this );
	PortageFilesSingleton::Instance()->init( this );
	FileWatcherSingleton::Instance()->init( this );
}

KurooInit::~KurooInit()
{
	KurooConfig::setInit( false );
}

/**
 * Run "emerge --info" to collect system info like "ACCEPT_KEYWORDS" and "CONFIG_PROTECT".
 */
void KurooInit::getEnvironment()
{
	DEBUG_LINE_INFO;
	//QTextCodec *codec = QTextCodec::codecForName("utf8");
	eProc = new KProcess();
	*eProc << "emerge" << "--info";
	eProc->setOutputChannelMode( KProcess::OnlyStdoutChannel );
	connect( eProc, SIGNAL( finished(int) ), this, SLOT( slotEmergeInfo() ) );
	connect( eProc, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotCollectOutput() ) );
	eProc->start();
}

void KurooInit::slotCollectOutput()
{
	QByteArray line;
	while( !( line = eProc->readLine() ).isEmpty() )
		m_emergeInfoLines += QString( line );
}

void KurooInit::slotEmergeInfo()
{
	kDebug() << "Parsing emerge --info" << LINE_INFO;
	foreach( QString line, m_emergeInfoLines ) {
		if ( line.startsWith( "Portage 2.0" ) ) {
			KurooConfig::setPortageVersion21( false );
		} else {
			KurooConfig::setPortageVersion21( true );
		}

		if ( line.startsWith( "ACCEPT_KEYWORDS=" ) ) {
			QString arch = line.section( "\"", 1, 1 );

			// When testing we have two keywords, only pick one
			if( arch.contains( "~" ) ) {
				arch = arch.section( "~", 1, 1 );
			}

			KurooConfig::setArch( arch );
		}

		if ( line.startsWith( "CONFIG_PROTECT=" ) )
			KurooConfig::setConfigProtectList( line.section( "\"", 1, 1 ) );
	}

	kDebug(0) << "KurooConfig::arch()=" << KurooConfig::arch() << LINE_INFO;

	//KurooConfig::writeConfig();
	DEBUG_LINE_INFO;
}

/**
 * Run wizard to inform user of latest changes and collect user settings like kuroo DirHome directory,
 * and overlay location.
 * If user aborts the wizard it will be relaunched again next time.
 */
void KurooInit::firstTimeWizard()
{
	IntroDlg wizardDialog;
	kDebug() << "Running Wizard" << LINE_INFO;
	if( wizardDialog.exec() == QDialog::Accepted ) {
		KurooConfig::setWizard( false );
	} else {
		exit(0); //is this the correct way to exit ?
	}
	KurooConfig::setInit( true );
}

/**
 * Control if user is in portage group.
 */
void KurooInit::checkUser()
{
	QStringList userGroups = KUser().groupNames();
	foreach( QString user, userGroups ) {
		if ( user == "portage" )
			return;
	}

	KMessageBox::error( 0, i18n("You don't have enough permissions to run kuroo.\nPlease add yourself into portage group!"),
						   i18n("User permissions") );
	exit(0);
}

#include "kurooinit.moc"
