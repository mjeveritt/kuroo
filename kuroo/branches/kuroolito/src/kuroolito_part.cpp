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

#include "kuroolito_part.h"
#include "common.h"
#include "threadweaver.h"
#include "kurooinit.h"
#include "kuroolito.h"
#include "introdlg.h"
#include "portagetab.h"

KuroolitoPart::KuroolitoPart( QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const QStringList& )
    : KParts::ReadWritePart( parent, name ),
	kurooInit( new KuroolitoInit( this, "KuroolitoInit" ) ),
	prefDialog( 0 )
{
	// we need an instance
    setInstance( KuroolitoPartFactory::instance() );
	
	GlobalSingleton::Instance()->setColorTheme();
	
	// Add all pages
	viewPortage = new PortageTab( parentWidget );
	setWidget( viewPortage );
	
	if ( !KuroolitoDBSingleton::Instance()->isPortageEmpty() )
		viewPortage->slotReload();
	else
		PortageSingleton::Instance()->slotRefresh();
	
	KStdAction::quit( this, SLOT( slotQuit() ), actionCollection(), "file_quit" )->setText( i18n( "Quit" ) );
	KStdAction::preferences( this, SLOT( slotPreferences() ), actionCollection(), "configure_kuroolito" )->setText( i18n( "Configure Kuroolito..." ) );

	actionRefreshPortage = new KAction( i18n("&Refresh Packages"), 0, KShortcut( CTRL + Key_P ),
	                                    PortageSingleton::Instance() , SLOT( slotRefresh() ), actionCollection(), "refresh_portage" );
	
	actionRefreshUpdates = new KAction( i18n("&Refresh Updates"), 0, KShortcut( CTRL + Key_U ),
	                                    PortageSingleton::Instance() , SLOT( slotRefreshUpdates() ), actionCollection(), "refresh_updates" );
	
	setXMLFile("kuroolito/kuroolito_partui.rc");
}

KuroolitoPart::~KuroolitoPart()
{
}

/**
 * Open kuroo preferences window.
 */
void KuroolitoPart::slotPreferences()
{
	prefDialog = KConfigDialog::exists( i18n( "settings" ) );
	if ( !prefDialog )
		prefDialog = new ConfigDialog( viewPortage, i18n( "settings" ), KuroolitoConfig::self() );
	prefDialog->show();
	prefDialog->raise();
	prefDialog->setActiveWindow();
}

bool KuroolitoPart::openFile()
{
}

bool KuroolitoPart::saveFile()
{
}

bool KuroolitoPart::slotQuit()
{
	return true;
}

void KuroolitoPart::fileSaveAs()
{
}

// It's usually safe to leave the factory code alone.. with the
// notable exception of the KAboutData data
#include <kaboutdata.h>
#include <klocale.h>

KInstance*  KuroolitoPartFactory::s_instance = 0L;
KAboutData* KuroolitoPartFactory::s_about = 0L;

KuroolitoPartFactory::KuroolitoPartFactory()
    : KParts::Factory()
{
}

KuroolitoPartFactory::~KuroolitoPartFactory()
{
    delete s_instance;
    delete s_about;

    s_instance = 0L;
}

KParts::Part* KuroolitoPartFactory::createPartObject( QWidget *parentWidget, const char *widgetName,
                                                        QObject *parent, const char *name,
                                                        const char *classname, const QStringList &args )
{
    // Create an instance of our Part
    KuroolitoPart* obj = new KuroolitoPart( parentWidget, widgetName, parent, name, args );

    // See if we are to be read-write or not
    if (QCString(classname) == "KParts::ReadOnlyPart")
        obj->setReadWrite(false);

    return obj;
}

KInstance* KuroolitoPartFactory::instance()
{
    if( !s_instance ) {
        s_about = new KAboutData("kuroolitopart", I18N_NOOP("KuroolitoPart"), "0.1");
        s_about->addAuthor("karye", 0, "info@kuroo.org");
        s_instance = new KInstance(s_about);
    }
    return s_instance;
}

extern "C"
{
    void* init_libkuroolitopart() {
		KGlobal::locale()->insertCatalogue("kuroolito");
        return new KuroolitoPartFactory;
    }
};

#include "kuroolito_part.moc"

