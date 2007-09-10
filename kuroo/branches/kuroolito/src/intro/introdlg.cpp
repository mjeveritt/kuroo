/***************************************************************************
*   Copyright (C) 2004 by Karye                                           *
*   info@kuroo.org                                                        *
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

#include "singleton.h"
#include "global.h"
#include "introdlg.h"
#include "settings.h"

#include <qwizard.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qtooltip.h>

#include <klocale.h>
#include <kdebug.h>
#include <kactivelabel.h>
#include <kmessagebox.h>
#include <kio/job.h>

typedef Singleton<Global> GlobalSingleton;

/**
 * @class IntroDlg
 * @short Kuroolito introduction wizard.
 */
IntroDlg::IntroDlg( QWidget* parent, const char* name, bool modal, WFlags fl )
	: Intro( parent, name, modal, fl )
{
	setCaption( "Kuroolitolito-" + KuroolitoConfig::hardVersion() );
	QWizard::backButton()->hide();
	QWizard::finishButton()->setText( i18n("Ok") );
	QWizard::showPage( page1 );
	QWizard::setHelpEnabled( page1, false );
	setFinishEnabled( page1, true );
	
	introText->setText(   	"<h2>Kuroolito-" + KuroolitoConfig::hardVersion().section( "_", 0, 0 ) + "</h2><p>" +
					  i18n( "<p>Kuroolito - A Portage browser</p>"
							"<p>In order to activate the sqlite module you must uncomment #portdbapi.auxdbmodule = cache.sqlite.database in /etc/portage/modules.</p>"
							"<p>Remember to update the cache by running emerge --regen after each sync.</p>"
					      ) );
	
	adjustSize();
}

IntroDlg::~IntroDlg()
{}

void IntroDlg::back()
{
	QWizard::back();
}

void IntroDlg::next()
{
}

void IntroDlg::reject()
{
	QWizard::reject();
}

void IntroDlg::accept()
{
	KMessageBox::enableAllMessages();
	QWizard::accept();
}

#include "introdlg.moc"

