/***************************************************************************
*   Copyright (C) 2004 by Karye                                           *
*   karye@users.sourceforge.net                                                      *
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

#include <klocale.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kactivelabel.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kurlrequester.h>

/**
 * @class IntroDlg
 * @short Kuroo introduction wizard.
 */
IntroDlg::IntroDlg( QWidget* parent, const char* name, bool modal, WFlags fl )
	: Intro( parent, name, modal, fl )
{
	setCaption( "Kuroo-" + KurooConfig::hardVersion() );
	QWizard::backButton()->hide();
	QWizard::cancelButton()->hide();
	QWizard::finishButton()->setText( i18n("Ok") );
	QWizard::showPage( page1 );
	QWizard::setHelpEnabled( page1, false );
	setFinishEnabled( page1, true );
	
	introText->setText(   	"<h2>Kuroo-" + KurooConfig::version().section( "_", 0, 0 ) + "</h2><p>" +
					  i18n( "Kuroo - A KDE Portage frontend that allows you to do most common "
							"software maintenance tasks on gentoo systems</p>"
							"New features:<br>"
							"Unified package view: fast package browsing and searching with filters.<br>"
							"Two Column Selector, Package Inspector, Progress bars, Use-Flag Editor,"
							"Emerge and Configuration History, Emerge Queue.<br>"
							"<br>"
					      	"We would be happy to know how many of you are using Kuroo: "
					      	"<a href=http://kuroo.org/letusknow.html>Let us know!</a><br>"
					      	"<br>"
							"<b><font color=red>Warning!<br>"
							"Please take backup of all files in /etc/portage, /etc/make.conf and<br>"
							"/var/lib/portage/world.</font></b>" ) );
}

IntroDlg::~IntroDlg()
{
}

void IntroDlg::back()
{
	QWizard::back();
}

void IntroDlg::next()
{
// 	if ( QWizard::indexOf(this->currentPage()) == 0 )
		KMessageBox::enableAllMessages();
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

