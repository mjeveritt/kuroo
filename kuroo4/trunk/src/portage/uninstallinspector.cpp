/***************************************************************************
*   Copyright (C) 2004 by karye                                           *
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

#include "common.h"
#include "uninstallinspector.h"
#include "ui_uninstallbase.h"

#include <QTreeWidget>

/**
 * @class UninstallInspector
 * @short Dialog for selected package and version to uninstall.
 */
UninstallInspector::UninstallInspector( QWidget *parent )
    : KDialog( parent )
{
    //i18n( "Uninstall Packages" ), false, i18n( "Uninstall Packages" ), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, false
    m_uninstallbase.setupUi( m_dialog );
	setMainWidget( m_dialog );
}

UninstallInspector::~UninstallInspector()
{}

/**
 * Open dialog and list package and versions installed.
 * @param packageList
 */
void UninstallInspector::view( const QStringList& packageList )
{
    m_uninstallbase.uninstallView->clear();
	
    const QStringList systemFilesList = KurooConfig::systemFiles().split(" ");
	bool isPartOfSystem( false );
	
	for ( QStringList::ConstIterator itPackage = packageList.begin(), itPackageEnd = packageList.end(); itPackage != itPackageEnd; ++itPackage ) {
		QString id = *itPackage++;
		QString package = *itPackage;
		
        QTreeWidgetItem* itemPackage = new QTreeWidgetItem( m_uninstallbase.uninstallView );
        itemPackage->setText( 0, package );
        itemPackage->setFlags( Qt::ItemIsUserCheckable );
        itemPackage->setExpanded( true );
        itemPackage->setCheckState( 0, Qt::Checked );
		
		// Warn if package is included in gentoo base system profile
        foreach ( QString file, systemFilesList )
            if ( file == package ) {
                itemPackage->setIcon( 0, ImagesSingleton::Instance()->icon( WARNING ) );
				isPartOfSystem = true;
			}
		
		// List all versions if more that one installed version is found
		const QStringList versionsList = KurooDBSingleton::Instance()->packageVersionsInstalled( id );
		if ( versionsList.size() > 1 )
            foreach( QString version, versionsList ) {
                QTreeWidgetItem* itemVersion = new QTreeWidgetItem( itemPackage );
                itemPackage->setText( 0, version );
                itemPackage->setFlags( Qt::ItemIsUserCheckable );
                //itemVersion->setEnabled( true );
                itemPackage->setCheckState( 0, Qt::Checked );
			}
	}
	
	if ( isPartOfSystem ) {
        m_uninstallbase.uninstallWarning->setText( i18n("<font color=red><b>You are uninstalling packages part of your system profile!<br>"
		                                          "This may be damaging to your system!</b></font>") );
        m_uninstallbase.uninstallWarning->show();
	}
    else {
        m_uninstallbase.uninstallWarning->hide();
    }
	
	show();
}

/**
 * Collect user checked packages or versions and launch unmerge.
 */
void UninstallInspector::slotOk()
{
	QStringList packageList;
	
    QTreeWidgetItemIterator it( m_uninstallbase.uninstallView );
    while ( *it ) {
        if ( (*it)->checkState(0) == Qt::Checked ) {
            if ( (*it)->parent() ) {
                if ( (*it)->parent()->checkState(0) == Qt::Unchecked )
                    packageList += "=" + (*it)->parent()->text(0) + "-" + (*it)->text(0);
            } else {
                packageList += (*it)->text(0);
            }
        }
		++it;
	}
	
	EmergeSingleton::Instance()->unmerge( packageList );
	hide();
}

#include "uninstallinspector.moc"
