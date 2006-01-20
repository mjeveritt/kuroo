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
 * Kuroo preferences widget.
 */
ConfigDialog::ConfigDialog( QWidget *parent, const char* name, KConfigSkeleton *config )
	: KConfigDialog( parent, name, config )
{
	setWFlags( WDestructiveClose );
	
	Options1* opt1 = new Options1( this, i18n("General") );
	Options2* opt2 = new Options2( this, i18n("make.conf") );
	Options7* opt7 = new Options7( this, i18n("etc warnings") );
	
	addPage( opt1, i18n("General"), "kuroo", i18n("General preferences") );
	addPage( opt2, i18n("make.conf"), "kuroo_makeconf", i18n("Edit your make.conf file") );
	addPage( opt7, i18n("etc warnings"), "messagebox_warning", i18n("Edit your etc-update warning file list") );
	
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
	
	if ( makeconf.open( IO_ReadOnly ) ) {
		QTextStream stream( &makeconf );
		KStringHandler kstr;
		
		while ( !stream.atEnd() ) {
			QString line = stream.readLine();
			line = line.simplifyWhiteSpace();
			
			if ( line.contains(QRegExp("^ACCEPT_KEYWORDS=")) )
				KurooConfig::setAcceptKeywords( kstr.word( line.section("ACCEPT_KEYWORDS=", 1, 1).remove("\"") , "0:" ) );
			
			if ( line.contains(QRegExp("^AUTOCLEAN=")) )
				KurooConfig::setAutoClean( kstr.word( line.section("AUTOCLEAN=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("^BUILD_PREFIX=\"")) )
				KurooConfig::setBuildPrefix( kstr.word( line.section("BUILD_PREFIX=", 1, 1).remove("\"") , "0:" ) );
			
			if ( line.contains(QRegExp("^CBUILD=\"")) )
				KurooConfig::setCBuild( kstr.word( line.section("CBUILD=", 1, 1).remove("\"") , "0:" ) );
			
			if ( line.contains(QRegExp("^CCACHE_SIZE=\"")) )
				KurooConfig::setCCacheSize( kstr.word( line.section("CCACHE_SIZE=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("^CFLAGS=\"")) ) 
				KurooConfig::setCFlags( kstr.word( line.section("CFLAGS=", 1, 1).remove("\"") , "0:" ) );
			
			if ( line.contains(QRegExp("^CXXFLAGS=\"")) )
				KurooConfig::setCXXFlags( kstr.word( line.section("CXXFLAGS=", 1, 1).remove("\"") , "0:" ) );
			
			if ( line.contains(QRegExp("^CHOST=\"")) )
				KurooConfig::setChost( kstr.word( line.section("CHOST=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("^CLEAN_DELAY=\"")) )
				KurooConfig::setCleanDelay( kstr.word( line.section("CLEAN_DELAY=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("^CONFIG_PROTECT=\"")) )
				KurooConfig::setConfigProtect( kstr.word( line.section("CONFIG_PROTECT=", 1, 1).remove("\"") , "0:" ) );
			
			if ( line.contains(QRegExp("^CONFIG_PROTECT_MASK=\"")) )
				KurooConfig::setConfigProtectMask( kstr.word( line.section("CONFIG_PROTECT_MASK=", 1, 1).remove("\"") , "0:" ) );
			
			if ( line.contains(QRegExp("^DEBUGBUILD=\"")) )
				KurooConfig::setDebugBuild( kstr.word( line.section("DEBUGBUILD=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("^DISTDIR=\"")) )
				KurooConfig::setDirDist( kstr.word( line.section("DISTDIR=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("^FEATURES=\"")) )
				KurooConfig::setFeatures( kstr.word( line.section("FEATURES=", 1, 1).remove("\"") , "0:" ) );
			
			if ( line.contains(QRegExp("^FETCHCOMMAND=\"")) )
				KurooConfig::setFetchCommand( kstr.word( line.section("FETCHCOMMAND=", 1, 1).remove("\"") , "0:" ) );
			
			if ( line.contains(QRegExp("^GENTOO_MIRRORS=\"")) )
				KurooConfig::setGentooMirrors( kstr.word( line.section("GENTOO_MIRRORS=", 1, 1).remove("\"") , "0:" ) );
			
			if ( line.contains(QRegExp("^HTTP_PROXY FTP_PROXY=\"")) )
				KurooConfig::setProxy( kstr.word( line.section("HTTP_PROXY=", 1, 1).remove("\"") , "0:" ) );
			
			if ( line.contains(QRegExp("^MAKEOPTS=\"")) )
				KurooConfig::setMakeOpts( kstr.word( line.section("MAKEOPTS=", 1, 1).remove("\"") , "0:" ) );
			
			if ( line.contains(QRegExp("^NOCOLOR=\"")) )
				KurooConfig::setNoColor( kstr.word( line.section("NOCOLOR=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("^PKG_TMPDIR=\"")) )
				KurooConfig::setDirPkgTmp( kstr.word( line.section("PKG_TMPDIR=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("^PKGDIR=\"")) )
				KurooConfig::setDirPkg( kstr.word( line.section("PKGDIR=", 1, 1).remove("\"") , "0:" ) );
			
			if ( line.contains(QRegExp("^PORT_LOGDIR =\"")) )
				KurooConfig::setDirPortLog( kstr.word( line.section("PORT_LOGDIR=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("^PORTAGE_BINHOST=\"")) )
				KurooConfig::setPortageBinHost( kstr.word( line.section("PORTAGE_BINHOST=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("^PORTAGE_NICENESS=\"")) )
				KurooConfig::setPortageNiceness( kstr.word( line.section("PORTAGE_NICENESS=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("^PORTAGE_TMPDIR=\"")) )
				KurooConfig::setDirPortageTmp( kstr.word( line.section("PORTAGE_TMPDIR=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("^PORTDIR=\"")) )
				KurooConfig::setDirPortage( kstr.word( line.section("PORTDIR=", 1, 1) .remove("\""), "0" ) );
			
			if ( line.contains(QRegExp("^PORTDIR_OVERLAY=\"")) ) {
				KurooConfig::setDirPortageOverlayAll( kstr.word( line.section("PORTDIR_OVERLAY=", 1, 1).remove("\"") , "0:" ) );
				KurooConfig::setDirPortageOverlay( kstr.word( line.section("PORTDIR_OVERLAY=", 1, 1).remove("\"") , "0" ) );
			}
			
			if ( line.contains(QRegExp("^RESUMECOMMAND=\"")) )
				KurooConfig::setResumeCommand( kstr.word( line.section("RESUMECOMMAND=", 1, 1).remove("\"") , "0:" ) );
			
			if ( line.contains(QRegExp("^ROOT=\"")) )
				KurooConfig::setRoot( kstr.word( line.section("ROOT=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("^RSYNC_EXCLUDEFROM=\"")) )
				KurooConfig::setRsyncExcludeFrom( kstr.word( line.section("RSYNC_EXCLUDEFROM=", 1, 1).remove("\"") , "0:" ) );
			
			if ( line.contains(QRegExp("^RSYNC_PROXY=\"")) )
				KurooConfig::setRsyncProxy( kstr.word( line.section("RSYNC_PROXY=", 1, 1).remove("\"") , "0:" ) );
			
			if ( line.contains(QRegExp("^RSYNC_RETRIES=\"")) )
				KurooConfig::setRsyncRetries( kstr.word( line.section("RSYNC_RETRIES=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("^RSYNC_RATELIMIT=\"")) )
				KurooConfig::setRsyncRateLimit( kstr.word( line.section("RSYNC_RATELIMIT=", 1, 1) , "0" ) );
			
			if ( line.contains(QRegExp("^RSYNC_TIMEOUT=\"")) )
				KurooConfig::setRsyncTimeOut( kstr.word( line.section("RSYNC_TIMEOUT=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("^RPMDIR=\"")) )
				KurooConfig::setDirRpm( kstr.word( line.section("RPMDIR=", 1, 1).remove("\"") , "0" ) );
			
			if ( line.contains(QRegExp("^SYNC=\"")) )
				KurooConfig::setSync( kstr.word( line.section("SYNC=", 1, 1).remove("\"") , "0:" ) );
			
			if ( line.contains(QRegExp("^USE=\"")) ) {
				if ( !line.endsWith("\"") )
					do {
						line += stream.readLine();
					} while ( !line.endsWith("\"") );
			
				line.replace('\\', ' ');
				line = line.simplifyWhiteSpace();
				KurooConfig::setUse( kstr.word( line.section("USE=", 1, 1).remove("\"") , "0:" ) );
			}
			if ( line.contains(QRegExp("^USE_ORDER=\"")) )
				KurooConfig::setUseOrder( kstr.word( line.section("USE_ORDER=", 1, 1).remove("\"") , "0:" ) );
			
			if ( line.contains(QRegExp("^NOCOLOR=\"")) )
				KurooConfig::setNoColor( kstr.word( line.section("NOCOLOR=", 1, 1).remove("\"") , "0" ) );
			
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
			do {
				eString += stream.readLine();
			}
			while ( eString.endsWith("\\") );

			// Remove old lines to avoid duplicates
			if ( !eString.contains(QRegExp("^CHOST|^CFLAGS|^CXXFLAGS|^MAKEOPTS|^USE|^GENTOO_MIRRORS|^PORTDIR_OVERLAY|^FEATURES|^PORTDIR|^PORTAGE_TMPDIR|^DISTDIR|^ACCEPT_KEYWORDS|^AUTOCLEAN|^BUILD_PREFIX|^CBUILD|^CCACHE_SIZE|^CLEAN_DELAY|^CONFIG_PROTECT|^CONFIG_PROTECT_MASK|^DEBUGBUILD|^FETCHCOMMAND|^HTTP_PROXY FTP_PROXY|^PKG_TMPDIR|^PKGDIR|^PORT_LOGDIR|^PORTAGE_BINHOST|^PORTAGE_NICENESS|^RESUMECOMMAND|^ROOT|^RSYNC_EXCLUDEFROM|^RSYNC_PROXY|^RSYNC_RETRIES|^RSYNC_RATELIMIT|^RSYNC_TIMEOUT|^RPMDIR|^SYNC|^USE_ORDER|^NOCOLOR")) )
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
