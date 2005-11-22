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

#include "kurooinit.h"
#include "introdlg.h"
#include "common.h"

#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>

#include <qdir.h>
#include <qregexp.h>

#include <kcmdlineargs.h>
#include <kstringhandler.h>
#include <kuser.h>
#include <kmessagebox.h>

/**
 * KurooInit checks that kuroo environment is correctly setup.
 * And launch intro wizard whenever a new version of kuroo is installed.
 * Set ownership for directories and files to portage:portage.
 * Check that user is in portage group.
 */
KurooInit::KurooInit( QObject *parent, const char *name )
	: QObject( parent, name ), wizardDialog(0)
{
	QDir d(KUROODIR);
	
	// Get portage uid and groupid to set directories and files owned by portage
	struct group* gid = getgrnam(QFile::encodeName("portage"));
	struct passwd* uid = getpwnam(QFile::encodeName("portage"));
	int portageUid = uid->pw_uid;
	int portageGid = gid->gr_gid;
	
	// Run intro if new version is installed or no DirHome directory is detected
	if ( KurooConfig::version() != KurooConfig::hardVersion() || !d.exists() || KurooConfig::wizard() ) {
		getEnvironment();
		firstTimeWizard();
	}
	
	// Control that user is in portage group
	if ( !KUser().isSuperUser() )
		checkUser();
	
	// Setup kuroo environment
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	if ( args->getOption("option") == "init" || KurooConfig::init() ) {
		KurooConfig::setSaveLog(false);
		KurooConfig::setScanUpdateDate(i18n("No scan"));
		KurooConfig::setScanSizeDate(i18n("No scan"));
		
		if ( !KurooConfig::wizard() )
			getEnvironment();
		
		// Create DirHome dir and set permissions so common user can run Kuroo
		if ( !d.exists() ) {
			if ( !d.mkdir(KUROODIR) ) {
				KMessageBox::error( 0, i18n("<qt>Could not create kuroo home directory.<br>"
				                            "You must start Kuroo with kdesu first time for a secure initialization.<br>"
				                            "Please try again!</qt>"), i18n("Initialization") );
				exit(0);
			} else {
				chmod(KUROODIR, 0770);
				chown(KUROODIR, portageUid, portageGid);
			}
			d.setCurrent(KUROODIR);
		}
	}
	
	// Check that backup directory exists and set correct permissions
	QString backupDir = KUROODIR + "backup";
	if ( !d.cd(backupDir) ) {
		if ( !d.mkdir(backupDir) ) {
			KMessageBox::error( 0, i18n("<qt>Could not create kuroo backup directory.<br>"
			                            "You must start Kuroo with kdesu first time for a secure initialization.<br>"
			                            "Please try again!</qt>"), i18n("Initialization") );
			exit(0);
		}
		else {
			chmod(backupDir, 0770);
			chown(backupDir, portageUid, portageGid);
		}
	}
	
	// If new release delete old db files
	QString databaseFile = KUROODIR + KurooConfig::databas();
	if ( KurooConfig::version() != KurooConfig::hardVersion() ) {
		remove(databaseFile);
		kdDebug() << i18n("Deleting old version of database %1").arg(databaseFile) << endl;
	}
	KurooConfig::setVersion( KurooConfig::hardVersion() );
	KurooConfig::writeConfig();
	
	// Check if existing database is owned by portage, if not remove it
	struct stat st;
	stat(QFile::encodeName(databaseFile), &st);
	if ( st.st_gid != portageGid && !KUser().isSuperUser() ) {
		KMessageBox::error( 0, i18n("<qt>Could not setup kuroo portage database.<br>"
		                            "You must start Kuroo with kdesu first time for a secure initialization.<br>"
		                            "Please try again!</qt>"), i18n("Initialization") );
		remove(databaseFile);
		exit(0);
	}
	
	
	//////////////////////////////////////////////////////////////////////////////////
	// Initialize singletons objects
	/////////////////////////////////////////////////////////////////////////////////
	
	databaseFile = KurooDBSingleton::Instance()->init(this);
	if ( KUser().isSuperUser() ) {
		chmod(databaseFile, 0660);
		chown(databaseFile, portageUid, portageGid);
	}
	
	QString logFile = LogSingleton::Instance()->init(this);
	if ( KUser().isSuperUser() ) {
		chmod(logFile, 0660);
		chown(logFile, portageUid, portageGid);
	}
	
	EtcUpdateSingleton::Instance()->init(this);
	SignalistSingleton::Instance()->init(this);
	EmergeSingleton::Instance()->init(this);
	HistorySingleton::Instance()->init(this);
	InstalledSingleton::Instance()->init(this);
	PortageSingleton::Instance()->init(this);
	UpdatesSingleton::Instance()->init(this);
	QueueSingleton::Instance()->init(this);
	ResultsSingleton::Instance()->init(this);
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
			
			// Parse out first overlay directory, because kuroo can only handle one overlay
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
	KMessageBox::error( 0, i18n("You don't have enough permissions to run kuroo."
	                            "Please add yourself into portage group!"), i18n("User permissions") );
	exit(0);
}

#include "kurooinit.moc"
