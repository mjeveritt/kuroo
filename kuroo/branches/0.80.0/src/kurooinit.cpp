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

#include <kcmdlineargs.h>
#include <kstringhandler.h>
#include <kuser.h>
#include <kmessagebox.h>

/**
 * @class kurooinit
 * @short KurooInit checks that kuroo environment is correctly setup.
 * 
 * And launch intro wizard whenever a new version of kuroo is installed.
 * Set ownership for directories and files to portage:portage.
 * Check that user is in portage group.
 */
KurooInit::KurooInit( QObject *parent, const char *name )
	: QObject( parent, name ), wizardDialog( 0 )
{
	// Run intro if new version is installed or no DirHome directory is detected.
	QDir d( KUROODIR );
	if ( KurooConfig::version() != KurooConfig::hardVersion() || !d.exists() || KurooConfig::wizard() ) {
		getEnvironment();
		firstTimeWizard();
	}
	else
		if ( !KUser().isSuperUser() )
			checkUser();
	
	// Get portage version
	QDir dPortageApp( KurooConfig::dirDbPkg() + "/sys-apps" );
	dPortageApp.setNameFilter( "portage-*" );
	dPortageApp.setSorting( QDir::Time );
	QString portage = dPortageApp.entryList().first();
	if ( portage.isEmpty() ) {
		KMessageBox::error( 0, i18n("Can not identify portage version!\n"
		                            "Kuroo can not correctly parse package information.\n"
		                            "You can select portage version in settings."), i18n("Portage version") );
	}
	else {
		if ( portage.section( "portage-", 1, 1).startsWith( "2.1" ) )
			KurooConfig::setPortageVersion21( true );
		else
			KurooConfig::setPortageVersion21( false );
		
		kdDebug() << i18n("Identifying portage version. Found: %1").arg( portage ) << endl;
	}
	
	// Get portage groupid to set directories and files owned by portage
	struct group* portageGid = getgrnam( QFile::encodeName("portage") );
	struct passwd* portageUid = getpwnam( QFile::encodeName("portage") );
	
	// Setup kuroo environment
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	if ( args->getOption("option") == "init" || KurooConfig::init() ) {
		KurooConfig::setSaveLog(false);
		KurooConfig::setScanUpdateDate( i18n("No scan") );
		KurooConfig::setScanSizeDate( i18n("No scan") );
		
		if ( !KurooConfig::wizard() )
			getEnvironment();
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
	kdDebug() << "databaseFile=" << databaseFile << endl;
	
	QString database = KUROODIR + KurooConfig::databas();
	if ( KurooDBSingleton::Instance()->kurooDbVersion().isEmpty() ) {
		KurooDBSingleton::Instance()->setKurooDbVersion( KurooConfig::version().section( "_db", 1, 1 ) );
		
		// Give permissions to portage:portage to access the db also
		chmod( databaseFile, 0660 );
		chown( databaseFile, portageGid->gr_gid, portageUid->pw_uid );
	}
	else // Check db structure version
		if ( KurooConfig::version().section( "_db", 1, 1 ) != KurooDBSingleton::Instance()->kurooDbVersion() ) {
			
			// Old db structure, must delete old db and backup history 
			KurooDBSingleton::Instance()->backupDb();
			KurooDBSingleton::Instance()->destroy();
			remove( database );
			kdDebug() << i18n("Database structure is changed. Deleting old version of database %1").arg( database ) << endl;
			
			// and recreate with new structure
			KurooDBSingleton::Instance()->init( this );
			KurooDBSingleton::Instance()->setKurooDbVersion( KurooConfig::version().section( "_db", 1, 1 ) );
			
			// Give permissions to portage:portage to access the db also
			chmod( databaseFile, 0660 );
			chown( databaseFile, portageGid->gr_gid, portageUid->pw_uid );
		}
	
	// Initialize singletons objects
	ImagesSingleton::Instance()->init( this );
	SignalistSingleton::Instance()->init( this );
	EmergeSingleton::Instance()->init( this );
	EtcUpdateSingleton::Instance()->init( this );
	HistorySingleton::Instance()->init( this );
	InstalledSingleton::Instance()->init( this );
	PortageSingleton::Instance()->init( this );
	UpdatesSingleton::Instance()->init( this );
	QueueSingleton::Instance()->init( this );
	ResultsSingleton::Instance()->init( this );
	PortageFilesSingleton::Instance()->init( this );
	FileWatcherSingleton::Instance()->init( this );
}

KurooInit::~KurooInit()
{
	KurooConfig::setInit(false);
}

/**
 * Parse /etc/make.conf for location of Portage directories.
 * arch is found in /etc/make.profile/make.defaults.
 * @return bool			@fixme
 */
bool KurooInit::getEnvironment()
{
	QString line;
	bool success(false);
	
	QFile makeconf("/etc/make.conf");
	if ( makeconf.open(IO_ReadOnly) ) {
		QTextStream stream(&makeconf);
		KStringHandler kstr;
		
		while (!stream.atEnd()) {
			line = stream.readLine();
			
			if ( line.contains(QRegExp("^DISTDIR=")) )
				KurooConfig::setDirDist( kstr.word( line.section("DISTDIR=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("^PORTDIR=")) )
				KurooConfig::setDirPortage( kstr.word( line.section("PORTDIR=", 1, 1).remove("\"") , "0" ) );
			else
				KurooConfig::setDirPortage("/usr/portage");
			
			if ( line.contains(QRegExp("^PORTAGE_TMPDIR=")) )
				KurooConfig::setDirPortageTmp( kstr.word( line.section("PORTAGE_TMPDIR=", 1, 1).remove("\"") , "0" ) );
			
			// Parse out first overlay directory, because can only handle one overlay
			if ( line.contains(QRegExp("^PORTDIR_OVERLAY=")) ) {
				KurooConfig::setDirPortageOverlayAll( kstr.word( line.section("PORTDIR_OVERLAY=", 1, 1).remove("\"") , "0:" ) );
				KurooConfig::setDirPortageOverlay( kstr.word( line.section("PORTDIR_OVERLAY=", 1, 1).remove("\"") , "0" ) );
			}
			
			success = true;
		}
		makeconf.close();
	}
	else {
		kdDebug() << i18n("Error reading: /etc/make.conf") << endl;
		success = false;
	}
	
	QDir d("/etc/make.profile");
	QFile f(d.canonicalPath() + "/../make.defaults");
	
	if ( f.open(IO_ReadOnly) ) {
		QTextStream stream(&f);
		while ( !stream.atEnd() ) {
			line = stream.readLine();
			if ( line.contains("ARCH=\"") > 0 ) {
				line = line.section("ARCH=\"", 1, 1);
				KurooConfig::setArch(line.section("\"", 0, 0));
			}
		}
		f.close();
	}
	else
		kdDebug() << i18n("Error reading: /etc/make.profile") << endl;
	
	// Add default etc warning files
	KurooConfig::setEtcFiles("/etc/make.conf\n/etc/securetty\n/etc/rc.conf\n/etc/fstab\n/etc/hosts\n/etc/conf.d/hostname\n/etc/conf.d/domainname\n/etc/conf.d/net\n/etc/X11/XF86Config\n/etc/X11/xorg.conf\n/etc/modules.conf\n/boot/grub/grub.conf\n/boot/lilo/lilo.conf\n~/.xinitrc");
	
	KurooConfig::writeConfig();
	return success;
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
		KurooConfig::setWizard(false);
	
	KurooConfig::setInit(true);
}

/**
 * Control that user is in portage group.
 */
void KurooInit::checkUser()
{
	QStringList userGroups = KUser().groupNames();
	foreach( userGroups ) {
		if ( *it == "portage" )
			return;
	}
	KMessageBox::error( 0, i18n("You don't have enough permissions to run kuroo.\nPlease add yourself into portage group!"), i18n("User permissions") );
	exit(0);
}

#include "kurooinit.moc"
