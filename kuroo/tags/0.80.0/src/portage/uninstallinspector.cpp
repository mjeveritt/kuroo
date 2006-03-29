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
#include "uninstallbase.h"

#include <klistview.h>

/**
 * @class UninstallInspector
 * @short Dialog for selected package and version to uninstall.
 */
UninstallInspector::UninstallInspector( QWidget *parent )
	: KDialogBase( KDialogBase::Swallow, 0, parent, i18n( "Uninstall Packages" ), false, i18n( "Uninstall Packages" ), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, false )
{
	m_dialog = new UninstallBase( this );
	setMainWidget( m_dialog );
	m_dialog->setMinimumSize( 300, 280 );
}

UninstallInspector::~UninstallInspector()
{
}

/**
 * Open dialog and list package and versions installed.
 * @param packageList
 */
void UninstallInspector::view( const QStringList& packageList )
{
	m_dialog->uninstallView->clear();
	
	for ( QStringList::ConstIterator itPackage = packageList.begin(), itPackageEnd = packageList.end(); itPackage != itPackageEnd; ++itPackage ) {
		QString id = *itPackage++;
		QString package = *itPackage;
		
		QCheckListItem* itemPackage = new QCheckListItem( m_dialog->uninstallView, package, QCheckListItem::CheckBoxController );
		itemPackage->setOpen( true );
		itemPackage->setOn( true );
		
		// List all versions if more that one installed version is found
		const QStringList versionsList = KurooDBSingleton::Instance()->packageVersionsInstalled( id );
		if ( versionsList.size() > 1 )
			foreach ( versionsList ) {
				QCheckListItem* itemVersion = new QCheckListItem( itemPackage, *it, QCheckListItem::CheckBox );
				itemVersion->setOn( true );
			}
	}
	
	show();
}

/**
 * Collect user checked packages or versions and launch unmerge.
 */
void UninstallInspector::slotOk()
{
	QStringList packageList;
	
	QListViewItemIterator it( m_dialog->uninstallView );
	while ( it.current() ) {
		
		if ( dynamic_cast<QCheckListItem*>( it.current() )->state() == QCheckListItem::On )
			if ( it.current()->parent() ) {
				if ( dynamic_cast<QCheckListItem*>( it.current()->parent() )->state() != QCheckListItem::On )
					packageList += "=" + it.current()->parent()->text(0) + "-" + it.current()->text(0);
			}
			else
				packageList += it.current()->text(0);
		
		++it;
	}
	
	EmergeSingleton::Instance()->unmerge( packageList );
	hide();
}

#include "uninstallinspector.moc"
