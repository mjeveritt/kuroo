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
	QWizard::setHelpEnabled(page3, false);
	QWizard::setHelpEnabled(page4, false);
	QWizard::setHelpEnabled(page5, false);
	QWizard::setHelpEnabled(page6, false);
	QWizard::setHelpEnabled(page7, false);
	QWizard::setHelpEnabled(page8, false);
	setFinishEnabled(page8, true);
	
	QString Changelog("/usr/share/doc/HTML/en/kuroo/ChangeLog");
	QFile file(Changelog);
	
	if ( file.open(IO_ReadOnly) ) {
		QTextStream stream(&file);
		LogBrowser->setText(stream.read());
		file.close();
	}
	else
		kdDebug() << i18n("Error reading: ") << Changelog << endl;
	
	path_dirPortage->setMode(KFile::Directory | KFile::LocalOnly);
	path_dirPortage->setURL(KurooConfig::dirPortage());
	
	path_portageOverlay->setMode(KFile::Directory | KFile::LocalOnly);
	path_portageOverlay->setURL(KurooConfig::dirPortageOverlay());
	
	kcfg_Arch->setCurrentText(KurooConfig::arch());
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
	QStringList error;
	QDir d;
	
	if ( QWizard::indexOf(this->currentPage()) == 3 ) {
		
		d.setPath(path_dirPortage->url());
		if ( d.exists() )
			KurooConfig::setDirPortage(path_dirPortage->url());
		else {
			error = i18n("Path to Portage directory not valid...");
			error += i18n(" Please correct!");
		}

		// Check all portage directories
		const QStringList categoryUrl = QStringList::split(" ", path_portageOverlay->url());
		QStringList categoryUrlValid;
		foreach ( categoryUrl ) {
			d.setPath(*it);
			if ( d.exists() )
				categoryUrlValid += *it;
			else {
				error += i18n("Path to Portage overlay directory %1 not valid...").arg(*it);
				error += i18n(" Please correct!");
			}
			KurooConfig::setDirPortageOverlay(categoryUrlValid.join(" "));
		}
		
		KurooConfig::setArch(kcfg_Arch->currentText());
		
		if ( error.isEmpty() ) {
			KurooConfig::writeConfig();
			QWizard::next();
		}
		else {
			switch (KMessageBox::warningYesNoList(this, i18n("Error checking paths..."), error, i18n("Kuroo"), KGuiItem::KGuiItem(i18n("Continue")), KGuiItem::KGuiItem(i18n("Cancel")))) {
				case KMessageBox::Yes : {
					QWizard::next();
					break;
				}
			}
		}

	}
	else
		QWizard::next();

	if ( QWizard::indexOf(this->currentPage()) == 7 ) {
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

