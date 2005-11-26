/***************************************************************************
*   Copyright (C) 2004 by karye                                           *
*   karye@users.sourceforge.net                                           *
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

#include <qdir.h>
#include <qregexp.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <dcopclient.h>
#include <kuniqueapplication.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <dcopref.h>

#include "gentoowatcher.h"
#include "watchersettings.h"

// Regexp from Portage
QRegExp pv("(-(?:\\d+\\.)*\\d+[a-z]?)");

static const char description[] = I18N_NOOP("A KDE KPart Application");
static const char version[] = "0.4";

static KCmdLineOptions options[] =
{
	{ "option <argument>", I18N_NOOP( "start or init" ), 0 }, KCmdLineLastOption
};

int main(int argc, char **argv)
{
	KAboutData about("kuroo_watcher", I18N_NOOP("Gentoo Watcher"), version, description,
			KAboutData::License_GPL, "(C) %{YEAR} karye", 0, 0, "karye@users.sourceforge.net");
	about.addAuthor( "karye", 0, "karye@users.sourceforge.net" );
	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions( options );
	KUniqueApplication app;
	QString myOption;
	
	if ( !KUniqueApplication::start() ) {
		kdDebug() << "kuroo_watcher is already running!" << endl;
		exit(0);
	}
	
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	myOption = args->getOption("option");
	
	if ( (myOption != "start") && (myOption != "init") ) {
	if ( !watcherSettings::autoStart() )
		exit(0);
	}
	
	QString arch, line;
	
	//arch is found in /etc/make.profile/make.defaults
	QDir d1( "/etc/make.profile" );
	QFile f( d1.canonicalPath() + "/../make.defaults" );
	
	if ( f.open( IO_ReadOnly ) ) {
	QTextStream stream( &f );
	while ( !stream.atEnd() ) {
		line = stream.readLine();
		if ( line.contains("ARCH=\"")>0 ) {
			line = line.section("ARCH=\"", 1, 1);
			arch = line.section("\"", 0, 0);
		}
	}
	f.close();
	}
	
	// Write initial settings
	if ( watcherSettings::setupDone() or (myOption == "init") ) {
	switch( KMessageBox::questionYesNo( 0, 
						i18n("Start Gentoo Watcher automatically at login?"), "Gentoo Watcher autostart" ) ) {
		case KMessageBox::No: {
			watcherSettings::setAutoStart( false );
			break;
		}
		case KMessageBox::Yes: {
			watcherSettings::setAutoStart( true );
			break;
		}
	}
	
	// arch is found in /etc/make.profile/make.defaults
	f.setName( "/etc/make.profile/make.defaults" );
	if ( f.open( IO_ReadOnly ) ) {
		QTextStream stream( &f );
		
		while ( !stream.atEnd() ) {
			line = stream.readLine();
			if ( line.contains("ARCH=\"")>0 ) {
				line = line.section("ARCH=\"", 1, 1);
				arch = line.section("\"", 0, 0);
			}
		}
		f.close();
	}
	
	watcherSettings::setSetupDone( false );
	watcherSettings::setRssInterval( 1 );
	watcherSettings::setDirHome( "/var/tmp/kuroo" );
	watcherSettings::setRowsGlsa( 20 );
	watcherSettings::setUrlGlsa( "http://www.gentoo.org/rdf/en/glsa-index.rdf" );
	watcherSettings::setUrlTesting( "http://packages.gentoo.org/archs/" + arch + "/testing/gentoo_simple.rss" );
	watcherSettings::setUrlStable( "http://packages.gentoo.org/archs/" + arch + "/stable/gentoo_simple.rss" );
	watcherSettings::writeConfig();
	}
	
	GentooWatcher *mainWin = 0;
	mainWin = new GentooWatcher();
	app.setMainWidget( mainWin );
	mainWin->show();
	
	// mainWin has WDestructiveClose flag by default, so it will delete itself.
	return app.exec();
}

