/***************************************************************************
*   Copyright (C) 2004 by Karye                                           *
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
					  i18n( "Kuroolito - A KDE Portage browser"
							"<br>"
					        "<a href=http://trac.kuroo.org/kuroo/wiki/TellUs>We love to hear from you. "
					        "Tell us what you think of Kuroolito!</a><br>"
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
// 	if ( QWizard::indexOf(this->currentPage()) == 0 )
// 		KMessageBox::enableAllMessages();
}

void IntroDlg::reject()
{
	QWizard::reject();
}

void IntroDlg::accept()
{
	// Backup all portage files changeable by kuroo
// 	if ( cbBackup->isOn() ) {
// 		QString dt = "_" + QDateTime::currentDateTime().toString( "yyyyMMdd_hhmm" );
// 		QString filePackageKeywords( KuroolitoConfig::filePackageKeywords() );
// 		KIO::file_copy( filePackageKeywords, lobalSingleton::Instance()->kurooDir() + "backup/" + filePackageKeywords.section( "/", -1 ) + dt,
// 		              -1, true, false, false );
// 		QString filePackageUserUnMask( KuroolitoConfig::filePackageUserUnMask() );
// 		KIO::file_copy( filePackageUserUnMask, lobalSingleton::Instance()->kurooDir() + "backup/" + filePackageUserUnMask.section( "/", -1 ) + dt,
// 		                -1, true, false, false );
// 		QString filePackageUserMask( KuroolitoConfig::filePackageUserMask() );
// 		KIO::file_copy( filePackageUserMask, lobalSingleton::Instance()->kurooDir() + "backup/" + filePackageUserMask.section( "/", -1 ) + dt,
// 		                -1, true, false, false );
// 		QString filePackageUserUse( KuroolitoConfig::filePackageUserUse() );
// 		KIO::file_copy( filePackageUserUse, lobalSingleton::Instance()->kurooDir() + "backup/" + filePackageUserUse.section( "/", -1 ) + dt,
// 		                -1, true, false, false );
// 		QString fileWorld( KuroolitoConfig::fileWorld() );
// 		KIO::file_copy( fileWorld, lobalSingleton::Instance()->kurooDir() + "backup/" + fileWorld.section( "/", -1 ) + dt,
// 		                -1, true, false, false );
// 		QString fileMakeConf( KuroolitoConfig::fileMakeConf() );
// 		KIO::file_copy( fileMakeConf, lobalSingleton::Instance()->kurooDir() + "backup/" + fileMakeConf.section( "/", -1 ) + dt,
// 		                -1, true, false, false );
// 	}
	
	KMessageBox::enableAllMessages();
	QWizard::accept();
}

#include "introdlg.moc"

