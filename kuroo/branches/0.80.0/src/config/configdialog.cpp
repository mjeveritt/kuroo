/**************************************************************************
*   Copyright (C) 2004 by                                                 *
*   karye@users.sourceforge.net                                           *
*   Stefan Bogner <bochi@online.ms>                                       *
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
#include "configdialog.h"
#include "options1.h"
#include "options2.h"
#include "options7.h"

#include <qtextstream.h>
#include <qdir.h>
#include <qfile.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>

#include <kconfigdialog.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <ktextedit.h>
#include <kuser.h>
#include <kstringhandler.h>

/**
 * @class ConfigDialog
 * @short Kuroo preferences.
 */
ConfigDialog::ConfigDialog( QWidget *parent, const char* name, KConfigSkeleton *config )
	: KConfigDialog( parent, name, config )
{
	setWFlags( WDestructiveClose );
	
	Options1* opt1 = new Options1( this, i18n("General") );
	Options2* opt2 = new Options2( this, i18n("make.conf") );
	Options7* opt7 = new Options7( this, i18n("Etc-update warnings") );
	
	addPage( opt1, i18n("General"), "kuroo", i18n("General preferences") );
	addPage( opt2, i18n("make.conf"), "kuroo_makeconf", i18n("Edit your make.conf file") );
	addPage( opt7, i18n("Etc-update warnings"), "messagebox_warning", i18n("Edit your etc-update warning file list") );
	
	connect( this, SIGNAL( settingsChanged() ), this, SLOT( saveAll() ) );
	
	readMakeConf();
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::slotDefault()
{
	readMakeConf();
	show();
}

/**
 * Parse /etc/make.conf.
 */
void ConfigDialog::readMakeConf()
{
	QFile makeconf( "/etc/make.conf" );
	QRegExp rx( "(\")((\\s|\\S)*)(\")" );
	
	if ( makeconf.open( IO_ReadOnly ) ) {
		QTextStream stream( &makeconf );
		KStringHandler kstr;
		QStringList lines;
		
		while ( !stream.atEnd() ) {
			QString line = stream.readLine();
			
			// Skip comment lines
			if ( line.isEmpty() || line.contains( QRegExp("^\\s*#") ) )
				continue;
			
			// Catch extended lines
			while ( line.contains( QRegExp("\\\\s*$") ) )
				line += stream.readLine().simplifyWhiteSpace();
			
			lines += line.replace('\\', ' ').simplifyWhiteSpace();
		}
		
		foreach ( lines ) {
			
			if ( (*it).contains( QRegExp("^\\s*ACCEPT_KEYWORDS") ) && rx.search( *it ) > -1 )
				KurooConfig::setAcceptKeywords( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*AUTOCLEAN") ) && rx.search( *it ) > -1 )
				KurooConfig::setAutoClean( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*BUILD_PREFIX") ) && rx.search( *it ) > -1 )
				KurooConfig::setBuildPrefix( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*CBUILD") ) && rx.search( *it ) > -1 )
				KurooConfig::setCBuild( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*CCACHE_SIZE") ) && rx.search( *it ) > -1 )
				KurooConfig::setCCacheSize( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*CFLAGS") ) && rx.search( *it ) > -1 )
				KurooConfig::setCFlags( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*CXXFLAGS") ) && rx.search( *it ) > -1 )
				KurooConfig::setCXXFlags( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*CHOST") ) && rx.search( *it ) > -1 )
				KurooConfig::setChost( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*CLEAN_DELAY") ) && rx.search( *it ) > -1 )
				KurooConfig::setCleanDelay( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*CONFIG_PROTECT") ) && rx.search( *it ) > -1 )
				KurooConfig::setConfigProtect( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*CONFIG_PROTECT_MASK") ) && rx.search( *it ) > -1 )
				KurooConfig::setConfigProtectMask( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*DEBUGBUILD") ) && rx.search( *it ) > -1 )
				KurooConfig::setDebugBuild( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*DISTDIR") ) && rx.search( *it ) > -1 )
				KurooConfig::setDirDist( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*FEATURES") ) && rx.search( *it ) > -1 )
				KurooConfig::setFeatures( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*FETCHCOMMAND") ) && rx.search( *it ) > -1 )
				KurooConfig::setFetchCommand( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*GENTOO_MIRRORS") ) && rx.search( *it ) > -1 )
				KurooConfig::setGentooMirrors( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*HTTP_PROXY\\s+FTP_PROXY") ) && rx.search( *it ) > -1 )
				KurooConfig::setProxy( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*MAKEOPTS") ) && rx.search( *it ) > -1 )
				KurooConfig::setMakeOpts( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*NOCOLOR") ) && rx.search( *it ) > -1 )
				KurooConfig::setNoColor( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*PKG_TMPDIR") ) && rx.search( *it ) > -1 )
				KurooConfig::setDirPkgTmp( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*PKGDIR") ) && rx.search( *it ) > -1 )
				KurooConfig::setDirPkg( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*PORT_LOGDIR") ) && rx.search( *it ) > -1 )
				KurooConfig::setDirPortLog( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*PORTAGE_BINHOST") ) && rx.search( *it ) > -1 )
				KurooConfig::setPortageBinHost( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*PORTAGE_NICENESS") ) && rx.search( *it ) > -1 )
				KurooConfig::setPortageNiceness( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*PORTAGE_TMPDIR") ) && rx.search( *it ) > -1 )
				KurooConfig::setDirPortageTmp( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*PORTDIR") ) && rx.search( *it ) > -1 )
				KurooConfig::setDirPortage( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*PORTDIR_OVERLAY") ) && rx.search( *it ) > -1 ) {
				KurooConfig::setDirPortageOverlayAll( rx.cap(2) );
				KurooConfig::setDirPortageOverlay( rx.cap(2) );
			}
			
			if ( (*it).contains( QRegExp("^\\s*RESUMECOMMAND") ) && rx.search( *it ) > -1 )
				KurooConfig::setResumeCommand( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*ROOT") ) && rx.search( *it ) > -1 )
				KurooConfig::setRoot( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*RSYNC_EXCLUDEFROM") ) && rx.search( *it ) > -1 )
				KurooConfig::setRsyncExcludeFrom( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*RSYNC_PROXY") ) && rx.search( *it ) > -1 )
				KurooConfig::setRsyncProxy( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*RSYNC_RETRIES") ) && rx.search( *it ) > -1 )
				KurooConfig::setRsyncRetries( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*RSYNC_RATELIMIT") ) && rx.search( *it ) > -1 )
				KurooConfig::setRsyncRateLimit( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*RSYNC_TIMEOUT") ) && rx.search( *it ) > -1 )
				KurooConfig::setRsyncTimeOut(rx.cap(2)  );
			
			if ( (*it).contains( QRegExp("^\\s*RPMDIR") ) && rx.search( *it ) > -1 )
				KurooConfig::setDirRpm( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*SYNC") ) && rx.search( *it ) > -1 )
				KurooConfig::setSync( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*USE") ) && rx.search( *it ) > -1 )
				KurooConfig::setUse( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*USE_ORDER") ) && rx.search( *it ) > -1 )
				KurooConfig::setUseOrder( rx.cap(2) );
			
			if ( (*it).contains( QRegExp("^\\s*NOCOLOR") ) && rx.search( *it ) > -1 )
				KurooConfig::setNoColor( rx.cap(2) );
			
		}
		makeconf.close();
	}
	else
		kdDebug() << i18n("Error reading: /etc/make.conf") << endl;
}

/**
 * Save settings when user press "Apply".
 */
void ConfigDialog::saveAll()
{
	switch( activePageIndex() ) {
		
		case 0:
			break;
		
		case 1: {
			if ( !saveMakeConf() ) {
				readMakeConf();
				show();
				KMessageBox::error( this, i18n("Failed to save makeconf. Please run as root."), i18n("Saving"));
			}
		}
		
	}
}

/**
 * Save back /etc/make.conf.
 * @return success
 */
bool ConfigDialog::saveMakeConf() 
{
	QString line;
	QFile file( "/etc/make.conf" );
	
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		
		while ( !stream.atEnd() ) {
			QString eString;
			do
				eString += stream.readLine();
			while ( eString.endsWith("\\") );

			// Remove old lines to avoid duplicates
			if ( !eString.contains( QRegExp("^CHOST|^CFLAGS|^CXXFLAGS|^MAKEOPTS|^USE|^GENTOO_MIRRORS|^PORTDIR_OVERLAY|^FEATURES|^PORTDIR|^PORTAGE_TMPDIR|^DISTDIR|^ACCEPT_KEYWORDS|^AUTOCLEAN|^BUILD_PREFIX|^CBUILD|^CCACHE_SIZE|^CLEAN_DELAY|^CONFIG_PROTECT|^CONFIG_PROTECT_MASK|^DEBUGBUILD|^FETCHCOMMAND|^HTTP_PROXY FTP_PROXY|^PKG_TMPDIR|^PKGDIR|^PORT_LOGDIR|^PORTAGE_BINHOST|^PORTAGE_NICENESS|^RESUMECOMMAND|^ROOT|^RSYNC_EXCLUDEFROM|^RSYNC_PROXY|^RSYNC_RETRIES|^RSYNC_RATELIMIT|^RSYNC_TIMEOUT|^RPMDIR|^SYNC|^USE_ORDER|^NOCOLOR")) )
			line += eString + "\n";
		}
		file.close();
	}
	else
		kdDebug() << i18n("Error reading: /etc/make.conf") << endl;
	
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		stream << line;
		
		if( KurooConfig::acceptKeywords() != "" ) stream << "ACCEPT_KEYWORDS=\"" << KurooConfig::acceptKeywords() << "\"\n";
		if( KurooConfig::chost() != "" ) stream << "CHOST=\"" << KurooConfig::chost() << "\"\n";
		if( KurooConfig::cFlags() != "" ) stream << "CFLAGS=\"" << KurooConfig::cFlags() << "\"\n";
		if( KurooConfig::cXXFlags() != "" ) stream << "CXXFLAGS=\"" << KurooConfig::cXXFlags() << "\"\n";
		if( KurooConfig::makeOpts() != "" ) stream << "MAKEOPTS=\"" << KurooConfig::makeOpts() << "\"\n";
		if( KurooConfig::use() != "" ) stream << "USE=\"" << KurooConfig::use() << "\"\n";
		if( KurooConfig::configProtect() != "" ) stream << "CONFIG_PROTECT=\"" << KurooConfig::configProtect() << "\"\n";
		if( KurooConfig::configProtectMask() != "" ) stream << "CONFIG_PROTECT_MASK=\"" << KurooConfig::configProtectMask() << "\"\n";
		if( KurooConfig::features() != "" ) stream << "FEATURES=\"" << KurooConfig::features() << "\"\n";
		if( KurooConfig::useOrder() != "" ) stream << "USE_ORDER=\"" << KurooConfig::useOrder() << "\"\n";
		if( KurooConfig::noColor() != "" ) stream << "NOCOLOR=\"" << KurooConfig::noColor() << "\"\n";
		
		if( KurooConfig::root() != "" ) stream << "ROOT=\"" << KurooConfig::root() << "\"\n";
		if( KurooConfig::dirPortage() != "" ) stream << "PORTDIR=\"" << KurooConfig::dirPortage() << "\"\n";
		if( KurooConfig::dirPortageOverlay() != "" ) stream << "PORTDIR_OVERLAY=\"" << KurooConfig::dirPortageOverlay() << "\"\n";
		if( KurooConfig::dirDist() != "" ) stream << "DISTDIR=\"" << KurooConfig::dirDist() << "\"\n";
		if( KurooConfig::dirRpm() != "" ) stream << "RPMDIR=\"" << KurooConfig::dirRpm() << "\"\n";
		if( KurooConfig::dirPkgTmp() != "" ) stream << "PKG_TMPDIR=\"" << KurooConfig::dirPkgTmp() << "\"\n";
		if( KurooConfig::dirPkg() != "" ) stream << "PKGDIR=\"" << KurooConfig::dirPkg() << "\"\n";
		if( KurooConfig::dirPortLog() != "" ) stream << "PORT_LOGDIR =\"" << KurooConfig::dirPortLog() << "\"\n";
		if( KurooConfig::portageBinHost() != "" ) stream << "PORTAGE_BINHOST=\"" << KurooConfig::portageBinHost() << "\"\n";
		if( KurooConfig::portageNiceness() != "" ) stream << "PORTAGE_NICENESS=\"" << KurooConfig::portageNiceness() << "\"\n";
		if( KurooConfig::dirPortageTmp() != "" ) stream << "PORTAGE_TMPDIR=\"" << KurooConfig::dirPortageTmp() << "\"\n";
		
		if( KurooConfig::autoClean() != "" ) stream << "AUTOCLEAN=\"" << KurooConfig::autoClean() << "\"\n";
		if( KurooConfig::buildPrefix() != "" ) stream << "BUILD_PREFIX=\"" << KurooConfig::buildPrefix() << "\"\n";
		if( KurooConfig::cBuild() != "" ) stream << "CBUILD=\"" << KurooConfig::cBuild() << "\"\n";
		if( KurooConfig::cCacheSize() != "" ) stream << "CCACHE_SIZE=\"" << KurooConfig::cCacheSize() << "\"\n";
		if( KurooConfig::cleanDelay() != "" ) stream << "CLEAN_DELAY=\"" << KurooConfig::cleanDelay() << "\"\n";
		
		if( KurooConfig::debugBuild() != "" ) stream << "DEBUGBUILD=\"" << KurooConfig::debugBuild() << "\"\n";
		if( KurooConfig::fetchCommand() != "" ) stream << "FETCHCOMMAND=\"" << KurooConfig::fetchCommand() << "\"\n";
		
		if( KurooConfig::resumeCommand() != "" ) stream << "RESUMECOMMAND=\"" << KurooConfig::resumeCommand() << "\"\n";
		if( KurooConfig::rsyncExcludeFrom() != "" ) stream << "RSYNC_EXCLUDEFROM=\"" << KurooConfig::rsyncExcludeFrom() << "\"\n";
		
		if( KurooConfig::proxy() != "" ) stream << "HTTP_PROXY FTP_PROXY=\"" << KurooConfig::proxy() << "\"\n";
		if( KurooConfig::gentooMirrors() != "" ) stream << "GENTOO_MIRRORS=\"" << KurooConfig::gentooMirrors() << "\"\n";
		if( KurooConfig::rsyncProxy() != "" ) stream << "RSYNC_PROXY=\"" << KurooConfig::rsyncProxy() << "\"\n";
		if( KurooConfig::rsyncRetries() != "" ) stream << "RSYNC_RETRIES=\"" << KurooConfig::rsyncRetries() << "\"\n";
		if( KurooConfig::rsyncRateLimit() != "" ) stream << "RSYNC_RATELIMIT=\"" << KurooConfig::rsyncRateLimit() << "\"\n";
		if( KurooConfig::rsyncTimeOut() != "" ) stream << "RSYNC_TIMEOUT=\"" << KurooConfig::rsyncTimeOut() << "\"\n";
		if( KurooConfig::sync() != "" ) stream << "SYNC=\"" << KurooConfig::sync() << "\"\n";
		
		file.close();
		return true;
	}
	else {
		kdDebug() << i18n("Error writing: /etc/make.conf") << endl;
		return false;
	}
}

#include "configdialog.moc"
