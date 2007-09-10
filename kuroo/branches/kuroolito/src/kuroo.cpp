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
#include "threadweaver.h"
#include "systemtray.h"
#include "kurooinit.h"
#include "kuroo.h"
#include "statusbar.h"
#include "introdlg.h"
#include "portagetab.h"
// #include "packageinspector.h"

#include <unistd.h>

#include <qdragobject.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qtimer.h>
#include <qcheckbox.h>

#include <kdeversion.h>
#include <kstatusbar.h>
#include <kaccel.h>
#include <kfiledialog.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kmessagebox.h>
#include <ktabwidget.h>
#include <kuser.h>
#include <kio/job.h>
#include <klibloader.h>

/**
 * @class Kuroolito
 * @short Main kde window with menus, system tray icon and statusbar.
 */
Kuroolito::Kuroolito()
	: KParts::MainWindow( 0L, "Kuroolito" )
{
	// set the shell's ui resource file
    setXMLFile("kuroolito/kuroolito_shellui.rc");
	
	setMinimumSize( 600, 600 );
	
    // then, setup our actions
    setupActions();

    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell
    KLibFactory *factory = KLibLoader::self()->factory( "libkuroolitopart" );
    if ( factory ) {
		
        // now that the Part is loaded, we cast it to a Part to get
        // our hands on it
        m_part = static_cast<KParts::ReadWritePart *>(factory->create(this, "kuroolito_part", "KParts::ReadWritePart" ));

        if (m_part) {
			
            // tell the KParts::MainWindow that this is indeed the main widget
            setCentralWidget( m_part->widget() );

            // and integrate the part's GUI with the shell's
            createGUI( m_part );
        }
    }
    else {
		
        // if we couldn't find our Part, we exit since the Shell by
        // itself can't do anything useful
        KMessageBox::error( this, i18n("Could not find our part.") );
        kapp->quit();
		
        // we return here, cause kapp->quit() only means "exit the
        // next time we enter the event loop...
        return;
    }
}

/**
 * If necessary wait for job to finish before terminating.
 */
Kuroolito::~Kuroolito()
{
}

/**
 * Build mainwindow menus and toolbar.
 */
void Kuroolito::setupActions()
{
	KStdAction::quit( this, SLOT( slotQuit() ), actionCollection() );
}

/**
 * Backup emerge and merge history entries to text file.
 * Wait for the backup of the log is completed before terminating.
 */
void Kuroolito::slotQuit()
{
	exit(0);
}

#include "kuroo.moc"
