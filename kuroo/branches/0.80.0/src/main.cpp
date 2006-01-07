/***************************************************************************
 *   Copyright (C) 2005 by Karye                                           *
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

#include "settings.h"
#include "kuroo.h"

#include <qregexp.h>

#include <kapplication.h>
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>

static const char description[] = I18N_NOOP("Frontend to Gentoo Portage");
static const char version[] = "0.80.0_070106_db6";

static KCmdLineOptions options[] =
{
	{ "option <argument>", I18N_NOOP("Initialize Kuroo with init"), 0 }, KCmdLineLastOption
};

int main(int argc, char **argv)
{
	KAboutData about("kuroo8", I18N_NOOP("Kuroo8"), version, description,
	KAboutData::License_GPL, "(C) 2005 karye", 0, 0, "karye@users.sourceforge.net");
	about.addAuthor("Karye", I18N_NOOP("Original author and maintainer"), "karye@users.sourceforge.net");
	about.addCredit("Alexander N. Sørnes", I18N_NOOP("Norwegian translation"), "alex@thehandofagony.com");
	about.addCredit("Martin Baranski", I18N_NOOP("German translation"), "eagle@eagle-cage.de");
	about.addCredit("Wolfgang Bartelme", I18N_NOOP("Kuroo icons"), "design@bartelme.at");
	
	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(options);
	KApplication app;
	
    // register ourselves as a dcop client
	if ( app.dcopClient()->isApplicationRegistered("kuroo8") )  {
		kdDebug() << I18N_NOOP("Kuroo8 is already running!") << endl;
		exit(0);
	}
	app.dcopClient()->registerAs(app.name(), false);
	
	KurooConfig::setHardVersion(version);
	KurooConfig::writeConfig();
	
	app.dcopClient()->setAcceptCalls(true);
	
	// see if we are starting with session management
	if ( app.isRestored() ) {
		RESTORE( Kuroo );
	}
	else {
		Kuroo *widget = new Kuroo;
		widget->show();
	}
	
	return app.exec();
}
