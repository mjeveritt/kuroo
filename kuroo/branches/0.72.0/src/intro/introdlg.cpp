/***************************************************************************
*   Copyright (C) 2004 by shiva                                           *
*   shiva@Katmandu                                                        *
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

#include "introdlg.h"
#include "settings.h"

#include <qfile.h> 
#include <qtextbrowser.h>
#include <qwizard.h>
#include <qdir.h>
#include <qcombobox.h>
#include <qlabel.h>

#include <klocale.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kactivelabel.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kurlrequester.h>

#define foreach( x ) \
for( QStringList::ConstIterator it = x.begin(), end = x.end(); it != end; ++it )

/**
 * Kuroo introduction wizard.
 */
IntroDlg::IntroDlg( QWidget* parent, const char* name, bool modal, WFlags fl )
	: Intro( parent, name, modal, fl )
{
	setCaption( "Kuroo-" + KurooConfig::hardVersion() );
	
	QWizard::showPage(page1);
	QWizard::setHelpEnabled(page1, false);
	QWizard::setHelpEnabled(page2, false);
	QWizard::setHelpEnabled(page3, false);
	QWizard::setHelpEnabled(page4, false);
	setFinishEnabled(page4, true);
}

IntroDlg::~IntroDlg()
{
}

void IntroDlg::help()
{
	QWizard::help();
}

void IntroDlg::back()
{
	QWizard::back();
}

/**
 * Check for essential Portage directories.
 * Ask user to confirm their location.
 */
void IntroDlg::next()
{
	QWizard::next();

	if ( QWizard::indexOf(this->currentPage()) == 3 ) {
		
		if ( KurooConfig::version() == "0.70.1" )
			importantLabel->setText(i18n("<font color=\"red\"><b><p>Upgrading from version 0.70.1!<br>"
		                             "1. Run \"regenworld\" from console as root to update the list of installed packages. "
		                             "As of version 0.71.0 only emerged packages and not their dependencies are added to world file.<br>"
		                             "2. Kuroo cache is moved to \"/var/kuroo\".</b></p></font>"));
		
		KMessageBox::enableAllMessages();
		
		// Add default etc warning files
		KurooConfig::setEtcFiles("/etc/make.conf\n/etc/securetty\n/etc/rc.conf\n/etc/fstab\n/etc/hosts\n/etc/conf.d/hostname\n/etc/conf.d/domainname\n/etc/conf.d/net\n/etc/X11/XF86Config\n/etc/X11/xorg.conf\n/etc/modules.conf\n/boot/grub/grub.conf\n/boot/lilo/lilo.conf\n~/.xinitrc");
	}
}

void IntroDlg::reject()
{
	QWizard::reject();
}

void IntroDlg::accept()
{
	QWizard::accept();
}

#include "introdlg.moc"

