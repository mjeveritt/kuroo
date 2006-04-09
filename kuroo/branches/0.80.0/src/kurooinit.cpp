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
#include <kinputdialog.h>

/**
 * @class kurooInit
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
	QDir d( GlobalSingleton::Instance()->kurooDir() );
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
	}
	
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
			d.setCurrent(GlobalSingleton::Instance()->kurooDir());
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
	
	// Old db structure, must delete it and backup history 
	if ( KurooConfig::version().section( "_db", 1, 1 ) != dbVersion ) {
		
		// Backup history if there's old db version
		if ( !dbVersion.isEmpty() )
			KurooDBSingleton::Instance()->backupDb();
		
		KurooDBSingleton::Instance()->destroy();
		remove( database );
		kdWarning(0) << i18n("Database structure is changed. Deleting old version of database %1").arg( database ) << LINE_INFO;
		
		// and recreate with new structure
		KurooDBSingleton::Instance()->init( this );
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
	ResultsSingleton::Instance()->init( this );
	PortageFilesSingleton::Instance()->init( this );
	FileWatcherSingleton::Instance()->init( this );
}

KurooInit::~KurooInit()
{
	KurooConfig::setInit( false );
}

/**
 * Parse /etc/make.conf for location of Portage directories.
 * arch is found in /etc/make.profile/make.defaults.
 * @return bool			@fixme
 */
void KurooInit::getEnvironment()
{
	QString line;
	bool success( false );
	KStringHandler kstr;
	
	QFile makeconf("/etc/make.conf");
	if ( makeconf.open(IO_ReadOnly) ) {
		QTextStream stream(&makeconf);
		
		while ( !stream.atEnd() ) {
			line = stream.readLine();
			
			if ( line.contains(QRegExp("DISTDIR=")) )
				KurooConfig::setDirDist( kstr.word( line.section("DISTDIR=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("PORTDIR=")) )
				KurooConfig::setDirPortage( kstr.word( line.section("PORTDIR=", 1, 1).remove("\"") , "0" ) );
			else
				KurooConfig::setDirPortage("/usr/portage");
			
			if ( line.contains(QRegExp("PORTAGE_TMPDIR=")) )
				KurooConfig::setDirPortageTmp( kstr.word( line.section("PORTAGE_TMPDIR=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("PORTDIR_OVERLAY=")) )
				KurooConfig::setDirPortageOverlay( kstr.word( line.section("PORTDIR_OVERLAY=", 1, 1).remove("\"") , "0" ) );
			
			success = true;
		}
		makeconf.close();
	}
	else
		kdError(0) << i18n("Reading: /etc/make.conf") << LINE_INFO;
	
	QDir d("/etc/make.profile");
	QFile f( d.canonicalPath() + "/../make.defaults" );
	QString arch;
	if ( f.open(IO_ReadOnly) ) {
		QTextStream stream(&f);
		while ( !stream.atEnd() ) {
			line = stream.readLine();
			if ( line.contains("ARCH=") > 0 ) {
				arch = kstr.word( line.section("ARCH=", 1, 1).remove("\"") , "0" );
				success = true;
				break;
			}
		}
		f.close();
	}
	else {
		kdError(0) << i18n("Reading: /etc/make.profile") << LINE_INFO;
		success = false;
	}
	
	if ( arch.isEmpty() ){
		QStringList archList;
		
		f.setName( "/usr/portage/profiles/arch.list" );
		if ( f.open(IO_ReadOnly) ) {
			QTextStream stream(&f);
			while ( !stream.atEnd() )
				archList << stream.readLine();
			f.close();
			
			arch = KInputDialog::getItemList( i18n("Initialization"), 
											i18n("Kuroo can not detect your architecture!\n"
											     "You must select appropriate architecture to proceed.\n"
												 "Please select:"), archList, QStringList::QStringList() ).first();
			if ( arch.isEmpty() ) {
				kdError(0) << i18n("No architecture selected, quitting!") << LINE_INFO;
				success = false;
			}
			else
				success = true;
		}
	}
	
	KurooConfig::setArch( arch );
	
	// Add default etc-files warnings
	KurooConfig::setEtcFiles("/etc/make.conf\n/etc/securetty\n/etc/rc.conf\n/etc/fstab\n/etc/hosts\n/etc/conf.d/hostname\n"
	                         "/etc/conf.d/domainname\n/etc/conf.d/net\n/etc/X11/XF86Config\n/etc/X11/xorg.conf\n/etc/modules.conf\n"
	                         "/boot/grub/grub.conf\n/boot/lilo/lilo.conf\n~/.xinitrc");
	
	// Add default Gentoo Base Profile
	KurooConfig::setSystemFiles("app-arch/bzip2\napp-arch/cpio\napp-arch/tar\napp-shells/bash\ndev-lang/perl\ndev-lang/python\nnet-misc/iputils\n"
	                            "net-misc/rsync\nnet-misc/wget\nsys-apps/coreutils\nsys-apps/debianutils\nsys-apps/diffutils\n"
	                            "sys-apps/file\nsys-apps/findutils\nsys-apps/gawk\nsys-apps/grep\nsys-apps/groff\nsys-apps/kbd\n"
	                            "sys-apps/net-tools\nsys-apps/portage\nsys-process/procps\nsys-process/psmisc\nsys-apps/sed\n"
	                            "sys-apps/shadow\nsys-apps/texinfo\nsys-apps/which\nsys-devel/autoconf\nsys-devel/autoconf-wrapper\n"
	                            "sys-devel/automake\nsys-devel/automake-wrapper\nsys-devel/binutils\nsys-devel/bison\nsys-devel/flex\n"
	                            "sys-devel/gcc\nsys-devel/gnuconfig\nsys-devel/libtool\nsys-devel/m4\nsys-devel/mak\nsys-devel/patch\n"
	                            "sys-fs/e2fsprogs\nsys-libs/cracklib\nsys-libs/ncurses\nsys-libs/readline\nsys-libs/zlib\n"
	                            "virtual/dev-manager\nvirtual/editor\nvirtual/gzip\nvirtual/libc\nvirtual/man\nvirtual/modutils\n"
	                            "virtual/os-headers\nvirtual/pager\nvirtual/ssh");
	
	if ( !success )
		exit(0);
	
	KurooConfig::writeConfig();
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
 * Control that user is in portage group.
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
