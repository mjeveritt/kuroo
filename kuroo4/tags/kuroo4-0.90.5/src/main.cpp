/*
 *   Copyright (C) 2005 by Karye
 *   karye@users.sourceforge.net
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "settings.h"
#include "kuroo.h"

#include <stdlib.h>

#include <kapplication.h>
#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>

static const char version[] = "0.90.5";

int main( int argc, char **argv )
{
	KAboutData about("kuroo", 0, ki18n("Kuroo"), version, ki18n("Frontend to Gentoo Portage"),
			KAboutData::License_GPL, ki18n("(C) 2006 karye") ); //, 0, 0, "info@kuroo.org" new email ?
	about.addAuthor(ki18n("Karye"), ki18n("Original author and maintainer"), "info@kuroo.org");
	about.addAuthor(ki18n("David C. Manuelda"), ki18n("Main developer and maintainer"), "StormByte@gmail.com");
	about.addCredit(ki18n("Gombault Damien"), ki18n("French translation"), "desintegr@gmail.com");
	about.addCredit(ki18n("Jan Schnackenberg"), ki18n("German translation"), "jan@schnackenberg.org");
	about.addCredit(ki18n("Alexander Reiterer"), ki18n("German translation"), "alexander.reiterer@tuwien.ac.at");
	about.addCredit(ki18n("Martin Baranski"), ki18n("German translation"), "eagle@eagle-cage.de");
	about.addCredit(ki18n("Matteo Azzali"), ki18n("Italian translation"), "kaioth@tiscalinet.it");
	about.addCredit(ki18n("Alexander N. Sørnes"), ki18n("Norwegian translation"), "alex@thehandofagony.com");
	about.addCredit(ki18n("Konrad Mantorski"), ki18n("Polish translation"), "konrad@mantorski.com");
	about.addCredit(ki18n("Wolfgang Bartelme"), ki18n("Kuroo icons"), "design@bartelme.at");
	about.addCredit(ki18n("Jakob Petsovits"), ki18n("Portage version code"), "jpetso@gmx.at");
	about.addCredit(ki18n("Björn Balazs"), ki18n("OpenUsability"), "B@lazs.de");
	about.addCredit(ki18n("Florian Graessle"), ki18n("OpenUsability"), "holehan@gmx.de");
	about.setHomepage("http://kuroo.org");
	KCmdLineArgs::init( argc, argv, &about );

	KUniqueApplication app;

	kDebug(0) << "Kuroo version=" << version;

	KurooConfig::setHardVersion( version );
	//KurooConfig::writeConfig();

	// see if we are starting with session management
	if ( app.isSessionRestored() ) {
		RESTORE( Kuroo );
	} else {
		Kuroo *mainWindow = new Kuroo();
		mainWindow->setObjectName( "kuroo" );
		mainWindow->show();
	}

	return app.exec();
}
