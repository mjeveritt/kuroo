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
	QFile makeconf( KurooConfig::fileMakeConf() );
	QRegExp rx( "^\\s*(\\w*)(\\s*=\\s*)(\"?([^\"#]*)\"?)#*" );
	
	if ( makeconf.open( IO_ReadOnly ) ) {
		QTextStream stream( &makeconf );
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
			
			if ( (*it).contains( QRegExp("^\\bACCEPT_KEYWORDS\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setAcceptKeywords( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse ACCEPT_KEYWORDS.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bAUTOCLEAN\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setAutoClean( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse AUTOCLEAN.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bBUILD_PREFIX\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setBuildPrefix( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse BUILD_PREFIX.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bCBUILD\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setCBuild( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse CBUILD.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bCCACHE_SIZE\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setCCacheSize( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse CCACHE_SIZE.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bCFLAGS\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setCFlags( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse CFLAGS.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bCXXFLAGS\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setCXXFlags( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse CXXFLAGS.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bCHOST\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setChost( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse CHOST.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bCLEAN_DELAY\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setCleanDelay( rx.cap(4) );
				else	
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse CLEAN_DELAY.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bCONFIG_PROTECT\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setConfigProtect( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse CONFIG_PROTECT.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bCONFIG_PROTECT_MASK\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setConfigProtectMask( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse BUILD_PREFIX.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bDEBUGBUILD\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setDebugBuild( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse DEBUGBUILD.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bDISTDIR\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setDirDist( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse DISTDIR.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bFEATURES\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setFeatures( rx.cap(4) );
			else
				kdDebug() << i18n("Error parsing /etc/make.conf: can not parse FEATURES.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bFETCHCOMMAND\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setFetchCommand( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse FETCHCOMMAND.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bGENTOO_MIRRORS\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setGentooMirrors( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse GENTOO_MIRRORS.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bHTTP_PROXY FTP_PROXY\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setProxy( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse FTP_PROXY FTP_PROXY.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bMAKEOPTS\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setMakeOpts( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse MAKEOPTS.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bNOCOLOR\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setNoColor( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse NOCOLOR.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bPKG_TMPDIR\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setDirPkgTmp( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse PKG_TMPDIR.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bPKGDIR\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setDirPkg( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse PKGDIR.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bPORT_LOGDIR\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setDirPortLog( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse PORT_LOGDIR.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bPORTAGE_BINHOST\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setPortageBinHost( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse PORTAGE_BINHOST.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bPORTAGE_NICENESS\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setPortageNiceness( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse PORTAGE_NICENESS.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bPORTAGE_TMPDIR\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setDirPortageTmp( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse PORTAGE_TMPDIR.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bPORTDIR\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setDirPortage( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse PORTDIR.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bPORTDIR_OVERLAY\\b") ) ) {
				if ( rx.search( *it ) > -1 ) {
					KurooConfig::setDirPortageOverlayAll( rx.cap(4) );
					KurooConfig::setDirPortageOverlay( rx.cap(4) );
				}
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse PORTDIR_OVERLAY.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bRESUMECOMMAND\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setResumeCommand( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse RESUMECOMMAND.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bROOT\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setRoot( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse ROOT.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bRSYNC_EXCLUDEFROM\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setRsyncExcludeFrom( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse RSYNC_EXCLUDEFROM.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bRSYNC_PROXY\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setRsyncProxy( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse RSYNC_PROXY.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bRSYNC_RETRIES\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setRsyncRetries( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse RSYNC_RETRIES.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bRSYNC_RATELIMIT\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setRsyncRateLimit( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse RSYNC_RATELIMIT.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bRSYNC_TIMEOUT\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setRsyncTimeOut(rx.cap(4)  );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse RSYNC_TIMEOUT.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bRPMDIR\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setDirRpm( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse RPMDIR.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bSYNC\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setSync( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse SYNC.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bUSE\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setUse( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse USE.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bUSE_ORDER\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setUseOrder( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse USE_ORDER.") << endl;
				continue;
			}
			
			if ( (*it).contains( QRegExp("^\\bNOCOLOR\\b") ) ) {
				if ( rx.search( *it ) > -1 )
					KurooConfig::setNoColor( rx.cap(4) );
				else
					kdDebug() << i18n("Error parsing /etc/make.conf: can not parse NOCOLOR.") << endl;
			}
		}
		makeconf.close();
	}
	else
		kdDebug() << i18n("Error reading: %1").arg( KurooConfig::fileMakeConf() ) << endl;
}

/**
 * Save settings when user press "Apply".
 */
void ConfigDialog::saveAll()
{
	switch( activePageIndex() ) {
		
		case 0:
			break;
		
		case 1:
			if ( !saveMakeConf() ) {
				readMakeConf();
				show();
				KMessageBox::error( this, i18n("Failed to save %1. Please run as root.").arg( KurooConfig::fileMakeConf() ), i18n("Saving"));
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
	QStringList lines;
	QFile file( KurooConfig::fileMakeConf() );
	QMap<QString, QString> keywords;
	QRegExp rxLine( "^\\s*(\\w*)(\\s*=\\s*)(\"?([^\"#]*)\"?)#*" );
	
	// Collect all keywords
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		
		while ( !stream.atEnd() ) {
			QString line = stream.readLine();
			
			// Catch extended lines
			while ( line.contains( QRegExp("\\\\s*$") ) )
				line += stream.readLine().simplifyWhiteSpace();
			
			if ( line.contains( QRegExp( "^\\s*(CHOST|CFLAGS|CXXFLAGS|MAKEOPTS|USE|GENTOO_MIRRORS|PORTDIR_OVERLAY|FEATURES|PORTDIR|PORTAGE_TMPDIR|DISTDIR|ACCEPT_KEYWORDS|AUTOCLEAN|BUILD_PREFIX|CBUILD|CCACHE_SIZE|CLEAN_DELAY|CONFIG_PROTECT|CONFIG_PROTECT_MASK|DEBUGBUILD|FETCHCOMMAND|HTTP_PROXY|PKG_TMPDIR|PKGDIR|PORT_LOGDIR|PORTAGE_BINHOST|PORTAGE_NICENESS|RESUMECOMMAND|ROOT|RSYNC_EXCLUDEFROM|RSYNC_PROXY|RSYNC_RETRIES|RSYNC_RATELIMIT|RSYNC_TIMEOUT|RPMDIR|SYNC|USE_ORDER|NOCOLOR)" ) ) ) {
				if ( rxLine.search( line ) > -1 )
					keywords[ rxLine.cap(1) ] = rxLine.cap(4);
				else
					kdDebug() << i18n("Parsing %1: can not match keyword %2.").arg( KurooConfig::fileMakeConf() ).arg( rxLine.cap(1) ) << endl;
				
				lines += rxLine.cap(1) + "=" + rxLine.cap(3);
			}
			else
				lines += line;
		}
		file.close();
	}
	else
		kdDebug() << i18n("Error reading: %1").arg( KurooConfig::fileMakeConf() ) << endl;
	
	// Update keywords from settings
	keywords[ "ACCEPT_KEYWORDS" ] = KurooConfig::acceptKeywords();
	
	keywords[ "CHOST" ] = KurooConfig::chost();
	
	keywords[ "CFLAGS" ] = KurooConfig::cFlags();
	
	keywords[ "CXXFLAGS" ] = KurooConfig::cXXFlags();
	
	keywords[ "MAKEOPTS" ] = KurooConfig::makeOpts();

	keywords[ "USE" ] = KurooConfig::use();

	keywords[ "CONFIG_PROTECT" ] = KurooConfig::configProtect();

	keywords[ "CONFIG_PROTECT_MASK" ] = KurooConfig::configProtectMask();

	keywords[ "FEATURES" ] = KurooConfig::features();

	keywords[ "USE_ORDER" ] = KurooConfig::useOrder();

	keywords[ "NOCOLOR" ] = KurooConfig::noColor();

	keywords[ "ROOT" ] = KurooConfig::root();

	keywords[ "PORTDIR" ] = KurooConfig::dirPortage();

	keywords[ "PORTDIR_OVERLAY" ] = KurooConfig::dirPortageOverlay();

	keywords[ "DISTDIR" ] = KurooConfig::dirDist();

	keywords[ "RPMDIR" ] = KurooConfig::dirRpm();

	keywords[ "PKG_TMPDIR" ] = KurooConfig::dirPkgTmp();

	keywords[ "PKGDIR" ] = KurooConfig::dirPkg();

	keywords[ "PORT_LOGDIR" ] = KurooConfig::dirPortLog();

	keywords[ "PORTAGE_BINHOST" ] = KurooConfig::portageBinHost();

	keywords[ "PORTAGE_NICENESS" ] = KurooConfig::portageNiceness();

	keywords[ "PORTAGE_TMPDIR" ] = KurooConfig::dirPortageTmp();

	keywords[ "AUTOCLEAN" ] = KurooConfig::autoClean();

	keywords[ "BUILD_PREFIX" ] = KurooConfig::buildPrefix();

	keywords[ "CBUILD" ] = KurooConfig::cBuild();

	keywords[ "CCACHE_SIZE" ] = KurooConfig::cCacheSize();

	keywords[ "CLEAN_DELAY" ] = KurooConfig::cleanDelay();

	keywords[ "DEBUGBUILD" ] = KurooConfig::debugBuild();

	keywords[ "FETCHCOMMAND" ] = KurooConfig::fetchCommand();

	keywords[ "RESUMECOMMAND" ] = KurooConfig::resumeCommand();

	keywords[ "RSYNC_EXCLUDEFROM" ] = KurooConfig::rsyncExcludeFrom();

	keywords[ "HTTP_PROXY" ] = KurooConfig::proxy();

	keywords[ "GENTOO_MIRRORS" ] = KurooConfig::gentooMirrors();

	keywords[ "RSYNC_PROXY" ] = KurooConfig::rsyncProxy();

	keywords[ "RSYNC_RETRIES" ] = KurooConfig::rsyncRetries();

	keywords[ "RSYNC_RATELIMIT" ] = KurooConfig::rsyncRateLimit();

	keywords[ "RSYNC_TIMEOUT" ] = KurooConfig::rsyncTimeOut();

	keywords[ "SYNC" ] = KurooConfig::sync();
	
	// Write back everything
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		
		foreach ( lines ) {
			
			if ( (*it).contains( QRegExp( "^\\s*(CHOST|CFLAGS|CXXFLAGS|MAKEOPTS|USE|GENTOO_MIRRORS|PORTDIR_OVERLAY|FEATURES|PORTDIR|PORTAGE_TMPDIR|DISTDIR|ACCEPT_KEYWORDS|AUTOCLEAN|BUILD_PREFIX|CBUILD|CCACHE_SIZE|CLEAN_DELAY|CONFIG_PROTECT|CONFIG_PROTECT_MASK|DEBUGBUILD|FETCHCOMMAND|HTTP_PROXY|PKG_TMPDIR|PKGDIR|PORT_LOGDIR|PORTAGE_BINHOST|PORTAGE_NICENESS|RESUMECOMMAND|ROOT|RSYNC_EXCLUDEFROM|RSYNC_PROXY|RSYNC_RETRIES|RSYNC_RATELIMIT|RSYNC_TIMEOUT|RPMDIR|SYNC|USE_ORDER|NOCOLOR)" ) ) ) {
				if ( rxLine.search( *it ) > -1 ) {
					QString keyword = rxLine.cap(1);
					if ( !keywords[ keyword ].isEmpty() )
						stream << keyword << "=\"" << keywords[ keyword ] << "\"" << endl;
					keywords.erase( keyword );
				}
			}
			else
				stream << *it << endl;
		}

		// Add the rest (new) entries into make.conf
		for ( QMap<QString, QString>::Iterator it = keywords.begin(), end = keywords.end(); it != end; ++it )
			if ( !it.data().isEmpty() )
				stream << it.key() << "=\"" << it.data() << "\"" << endl;
		
		file.close();
		return true;
	}
	else {
		kdDebug() << i18n("Error writing: %1").arg( KurooConfig::fileMakeConf() ) << endl;
		return false;
	}
}

#include "configdialog.moc"
