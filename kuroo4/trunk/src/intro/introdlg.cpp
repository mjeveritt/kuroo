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

#include <QWizard>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qtooltip.h>

#include <klocale.h>
#include <kdebug.h>
#include <kactivelabel.h>
#include <kmessagebox.h>
#include <kio/job.h>

#include "singleton.h"
#include "global.h"
#include "introdlg.h"
#include "settings.h"

/**
 * @class IntroDlg
 * @short Kuroo introduction wizard.
 */
IntroDlg::IntroDlg( QWidget* parent )
{
	setupUi( this );
	setWindowTitle( "Kuroo-" + KurooConfig::hardVersion() );
	setOption( NoBackButtonOnStartPage, true );
	setButtonText( FinishButton, i18n("Ok") );
	setOption( HaveHelpButton, false );

	introText->setText("<h2>Kuroo-" + KurooConfig::hardVersion().section( "_", 0, 0 ) + "</h2><p>" +
					   i18n("Kuroo - A KDE Portage frontend that allows you to do most common software maintenance tasks on gentoo systems</p>"));

	backupFiles->setText( i18n( "<qt><table width=100%><tr><td>Make copies into %1 of following files:</td></tr>", kurooDir + "backup/" ) +
						  "<tr><td>" + KurooConfig::filePackageKeywords() + "</td></tr>" +
						  "<tr><td>" + KurooConfig::filePackageUserUnMask() + "</td></tr>" +
						  "<tr><td>" + KurooConfig::filePackageUserMask() + "</td></tr>" +
						  "<tr><td>" + KurooConfig::filePackageUserUse() + "</td></tr>" +
						  "<tr><td>" + KurooConfig::fileWorld() + "</td></tr>" +
						  "<tr><td>" + KurooConfig::fileMakeConf() + "</td></tr></table></qt>" );
	adjustSize();
}
IntroDlg::~IntroDlg() {}

void IntroDlg::accept()
{
	// Backup all portage files changeable by kuroo
	if ( cbBackup->isChecked() ) {
		QString dt = "_" + QDateTime::currentDateTime().toString( "yyyyMMdd_hhmm" );
		QString filePackageKeywords( KurooConfig::filePackageKeywords() );
		KIO::file_copy( filePackageKeywords, kurooDir + "backup/" + filePackageKeywords.section( "/", -1 ) + dt,
				-1, KIO::Overwrite | KIO::HideProgressInfo);
		QString filePackageUserUnMask( KurooConfig::filePackageUserUnMask() );
		KIO::file_copy( filePackageUserUnMask, kurooDir + "backup/" + filePackageUserUnMask.section( "/", -1 ) + dt,
				-1, KIO::Overwrite | KIO::HideProgressInfo );
		QString filePackageUserMask( KurooConfig::filePackageUserMask() );
		KIO::file_copy( filePackageUserMask, kurooDir + "backup/" + filePackageUserMask.section( "/", -1 ) + dt,
				-1, KIO::Overwrite | KIO::HideProgressInfo );
		QString filePackageUserUse( KurooConfig::filePackageUserUse() );
		KIO::file_copy( filePackageUserUse, kurooDir + "backup/" + filePackageUserUse.section( "/", -1 ) + dt,
				-1, KIO::Overwrite | KIO::HideProgressInfo );
		QString fileWorld( KurooConfig::fileWorld() );
		KIO::file_copy( fileWorld, kurooDir + "backup/" + fileWorld.section( "/", -1 ) + dt,
				-1, KIO::Overwrite | KIO::HideProgressInfo );
		QString fileMakeConf( KurooConfig::fileMakeConf() );
		KIO::file_copy( fileMakeConf, kurooDir + "backup/" + fileMakeConf.section( "/", -1 ) + dt,
				-1, KIO::Overwrite | KIO::HideProgressInfo );
	}

	KMessageBox::enableAllMessages();
	QWizard::accept();
}

#include "introdlg.moc"

