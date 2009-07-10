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

#include <Q3Wizard>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qtooltip.h>

#include <klocale.h>
#include <kdebug.h>
#include <k3activelabel.h>
#include <kmessagebox.h>
#include <kio/job.h>

#include "singleton.h"
#include "global.h"
#include "introdlg.h"
#include "settings.h"

typedef Singleton<Global> GlobalSingleton;

/**
 * @class IntroDlg
 * @short Kuroo introduction wizard.
 */
IntroDlg::IntroDlg( QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
	//: Intro( parent, name, modal, fl )
{
	setupUi(this);
    setWindowTitle( "Kuroo-" + KurooConfig::hardVersion() );
	Q3Wizard::backButton()->hide();
	Q3Wizard::finishButton()->setText( i18n("Ok") );
	Q3Wizard::showPage( page1 );
	Q3Wizard::setHelpEnabled( page1, false );
	setFinishEnabled( page1, true );

	introText->setText(   	"<h2>Kuroo-" + KurooConfig::hardVersion().section( "_", 0, 0 ) + "</h2><p>" +
					  i18n( "Kuroo - A KDE Portage frontend that allows you to do most common "
							"software maintenance tasks on gentoo systems</p>"
							"<br>"
					        "<a href=http://trac.kuroo.org/kuroo/wiki/TellUs>We love to hear from you. "
					        "Tell us what you think of Kuroo!</a><br>"
					      ) );

	backupFiles->setText( i18n( "<qt><table width=100%><tr><td>Make copies into %1 of following files:</td></tr>" )
						.arg( GlobalSingleton::Instance()->kurooDir() + "backup/" ) +
						"<tr><td>" + KurooConfig::filePackageKeywords() + "</td></tr>" +
						"<tr><td>" + KurooConfig::filePackageUserUnMask() + "</td></tr>" +
						"<tr><td>" + KurooConfig::filePackageUserMask() + "</td></tr>" +
						"<tr><td>" + KurooConfig::filePackageUserUse() + "</td></tr>" +
						"<tr><td>" + KurooConfig::fileWorld() + "</td></tr>" +
						"<tr><td>" + KurooConfig::fileMakeConf() + "</td></tr></table></qt>"
						);

	adjustSize();
}

IntroDlg::~IntroDlg()
{}

void IntroDlg::back()
{
	Q3Wizard::back();
}

void IntroDlg::next()
{
// 	if ( QWizard::indexOf(this->currentPage()) == 0 )
// 		KMessageBox::enableAllMessages();
}

void IntroDlg::reject()
{
	Q3Wizard::reject();
}

void IntroDlg::accept()
{
	// Backup all portage files changeable by kuroo
    if ( cbBackup->isChecked() ) {
		QString dt = "_" + QDateTime::currentDateTime().toString( "yyyyMMdd_hhmm" );
		QString filePackageKeywords( KurooConfig::filePackageKeywords() );
		KIO::file_copy( filePackageKeywords, GlobalSingleton::Instance()->kurooDir() + "backup/" + filePackageKeywords.section( "/", -1 ) + dt,
				-1, KIO::Overwrite | KIO::HideProgressInfo);
		QString filePackageUserUnMask( KurooConfig::filePackageUserUnMask() );
		KIO::file_copy( filePackageUserUnMask, GlobalSingleton::Instance()->kurooDir() + "backup/" + filePackageUserUnMask.section( "/", -1 ) + dt,
				-1, KIO::Overwrite | KIO::HideProgressInfo );
		QString filePackageUserMask( KurooConfig::filePackageUserMask() );
		KIO::file_copy( filePackageUserMask, GlobalSingleton::Instance()->kurooDir() + "backup/" + filePackageUserMask.section( "/", -1 ) + dt,
				-1, KIO::Overwrite | KIO::HideProgressInfo );
		QString filePackageUserUse( KurooConfig::filePackageUserUse() );
		KIO::file_copy( filePackageUserUse, GlobalSingleton::Instance()->kurooDir() + "backup/" + filePackageUserUse.section( "/", -1 ) + dt,
				-1, KIO::Overwrite | KIO::HideProgressInfo );
		QString fileWorld( KurooConfig::fileWorld() );
		KIO::file_copy( fileWorld, GlobalSingleton::Instance()->kurooDir() + "backup/" + fileWorld.section( "/", -1 ) + dt,
				-1, KIO::Overwrite | KIO::HideProgressInfo );
		QString fileMakeConf( KurooConfig::fileMakeConf() );
		KIO::file_copy( fileMakeConf, GlobalSingleton::Instance()->kurooDir() + "backup/" + fileMakeConf.section( "/", -1 ) + dt,
				-1, KIO::Overwrite | KIO::HideProgressInfo );
	}

	KMessageBox::enableAllMessages();
	Q3Wizard::accept();
}

#include "introdlg.moc"

