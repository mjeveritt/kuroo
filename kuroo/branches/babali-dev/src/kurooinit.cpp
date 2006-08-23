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

#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>

#include <qdir.h>
#include <qtextcodec.h>

#include <kcmdlineargs.h>
#include <kstringhandler.h>
#include <kuser.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kprocio.h>

/**
 * @class KurooInit
 * @short KurooInit checks that kuroo environment is correctly setup.
 * 
 * And launch intro wizard whenever a new version of kuroo is installed.
 * Set ownership for directories and files to portage:portage.
 * Check that user is in portage group.
 */
KurooInit::KurooInit( QObject *parent, const char *name )
	: QObject( parent, name ), wizardDialog( 0 )
{
	getEnvironment();
	
	// Run intro if new version is installed or no DirHome directory is detected.
	QDir d( GlobalSingleton::Instance()->kurooDir() );
	if ( KurooConfig::version() != KurooConfig::hardVersion() || !d.exists() || KurooConfig::wizard() )
		firstTimeWizard();
	else
		if ( !KUser().isSuperUser() )
			checkUser();
	
	// Get portage groupid to set directories and files owned by portage
	struct group* portageGid = getgrnam( QFile::encodeName("portage") );
	struct passwd* portageUid = getpwnam( QFile::encodeName("portage") );
	
	// Setup kuroo environment
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	if ( args->getOption("option") == "init" || KurooConfig::init() ) {
		KurooConfig::setSaveLog( false );
		
		// Create DirHome dir and set permissions so common user can run Kuroo
		if ( !d.exists() ) {
			if ( !d.mkdir(GlobalSingleton::Instance()->kurooDir()) ) {
				KMessageBox::error( 0, i18n("<qt>Could not create kuroo home directory.<br>"
				                            "You must start Kuroo with kdesu first time for a secure initialization.<br>"
				                            "Please try again!</qt>"), i18n("Initialization") );
				exit(0);
			} else {
				chmod( GlobalSingleton::Instance()->kurooDir(), 0770 );
				chown( GlobalSingleton::Instance()->kurooDir(), portageGid->gr_gid, portageUid->pw_uid );
			}
			
			d.setCurrent( GlobalSingleton::Instance()->kurooDir() );
		}
	}
	
	// Check that backup directory exists and set correct permissions
	QString backupDir = GlobalSingleton::Instance()->kurooDir() + "backup";
	if ( !d.cd( backupDir ) ) {
		if ( !d.mkdir( backupDir ) ) {
			KMessageBox::error( 0, i18n("<qt>Could not create kuroo backup directory.<br>"
			                            "You must start Kuroo with kdesu first time for a secure initialization.<br>"
			                            "Please try again!</qt>"), i18n("Initialization") );
			exit(0);
		}
		else {
			chmod( backupDir, 0770 );
			chown( backupDir, portageGid->gr_gid, portageUid->pw_uid );
		}
	}
	
	KurooConfig::setVersion( KurooConfig::hardVersion() );
	KurooConfig::writeConfig();
	
	// Initialize the log
	QString logFile = LogSingleton::Instance()->init( this );
	if ( !logFile.isEmpty() ) {
		chmod( logFile, 0660 );
		chown( logFile, portageGid->gr_gid, portageUid->pw_uid );
	}
	
	// Initialize the database
	QString databaseFile = KurooDBSingleton::Instance()->init( this );
	QString database = GlobalSingleton::Instance()->kurooDir() + KurooConfig::databas();
	QString dbVersion = KurooDBSingleton::Instance()->getKurooDbMeta( "kurooVersion" );
	
	// Check for conflicting db design or new install
	if ( KurooConfig::version().section( "_db", 1, 1 ) != dbVersion ) {
		
		// Backup history if there's old db version
		if ( !dbVersion.isEmpty() ) {
			KurooDBSingleton::Instance()->backupDb();
			remove( database );
			kdWarning(0) << QString("Database structure is changed. Deleting old version of database %1").arg( database ) << LINE_INFO;
			
			// and recreate with new structure
			KurooDBSingleton::Instance()->init( this );
		}
		
		KurooDBSingleton::Instance()->setKurooDbMeta( "kurooVersion", KurooConfig::version().section( "_db", 1, 1 ) );
	}
	
	// Give permissions to portage:portage to access the db also
	chmod( databaseFile, 0660 );
	chown( databaseFile, portageGid->gr_gid, portageUid->pw_uid );
	
	// Initialize singletons objects
	GlobalSingleton::Instance()->init( this );
	ImagesSingleton::Instance()->init( this );
	SignalistSingleton::Instance()->init( this );
	EmergeSingleton::Instance()->init( this );
	EtcUpdateSingleton::Instance()->init( this );
	HistorySingleton::Instance()->init( this );
	PortageSingleton::Instance()->init( this );
	QueueSingleton::Instance()->init( this );
	PortageFilesSingleton::Instance()->init( this );
	FileWatcherSingleton::Instance()->init( this );
	ShutdownSingleton::Instance()->init( this );
}

KurooInit::~KurooInit()
{
	KurooConfig::setInit( false );
}

/**
 * Parse /etc/make.conf for location of Portage directories.
 * arch is found in /etc/make.profile/make.defaults.
 */
void KurooInit::getEnvironment()
{
	DEBUG_LINE_INFO;
	QTextCodec *codec = QTextCodec::codecForName("utf8");
	KProcIO* eProc = new KProcIO( codec );
	*eProc << "emerge" << "--info";
	
	if ( !eProc->start( KProcess::NotifyOnExit, KProcess::All ) ) {
		kdError(0) << "Cannot run emerge --info, quitting!" << LINE_INFO;
		exit(0);
	}
	connect( eProc, SIGNAL( processExited( KProcess* ) ), this, SLOT( slotEmergeInfo( KProcess* ) ) );
	connect( eProc, SIGNAL( readReady( KProcIO* ) ), this, SLOT( slotCollectOutput( KProcIO* ) ) );
}

void KurooInit::slotCollectOutput( KProcIO* eProc )
{
	QString line;
	while ( eProc->readln( line, true ) >= 0 )
		m_emergeInfoLines += line;
}

void KurooInit::slotEmergeInfo( KProcess* )
{
	foreach ( m_emergeInfoLines ) {
		
		if ( (*it).startsWith( "Portage 2.0" ) )
			KurooConfig::setPortageVersion21( false );
		else
			KurooConfig::setPortageVersion21( true );
		
		if ( (*it).startsWith( "ACCEPT_KEYWORDS=" ) )
			KurooConfig::setArch( (*it).section( "\"", 1, 1 ) );

		if ( (*it).startsWith( "CONFIG_PROTECT=" ) )
			KurooConfig::setConfigProtectList( (*it).section( "\"", 1, 1 ) );
		
// 		if ( (*it).startsWith( "USE=" ) )
// 			KurooConfig::setUse( (*it).section( "\"", 1, 1 ) );
	}
	KurooConfig::writeConfig();
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
	
	if ( wizardDialog.exec() != QDialog::Accepted )
		exit(0);
	else
		KurooConfig::setWizard( false );
	
	KurooConfig::setInit( true );
}

/**
 * Control if user is in portage group.
 */
void KurooInit::checkUser()
{
	QStringList userGroups = KUser().groupNames();
	foreach( userGroups ) {
		if ( *it == "portage" )
			return;
	}
	
	KMessageBox::error( 0, i18n("You don't have enough permissions to run kuroo.\nPlease add yourself into portage group!"),
	                       i18n("User permissions") );
	exit(0);
}

#include "kurooinit.moc"
