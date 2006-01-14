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
#include "packageinspector.h"
#include "portagelistview.h"
#include "packageversion.h"
#include "versionview.h"

#include <qcombobox.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qheader.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qvbox.h>
#include <qhbox.h>

#include <ktabwidget.h>
#include <kactionselector.h>
#include <ktextbrowser.h>
#include <kmessagebox.h>
#include <kuser.h>
#include <klistview.h>
#include <kpassivepopup.h>

/**
 * Specialized dialog for editing Use Flags per package.
 */
PackageInspector::PackageInspector( QWidget *parent )
	: KDialogBase( KDialogBase::Swallow, 0, parent, i18n( "Package Inspector" ), false, i18n( "Package Inspector" ), KDialogBase::Apply | KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Apply, false ), category( NULL ), package( NULL ), packageId( NULL ), m_portagePackage( 0 ), m_hasSettingsChanged( false )
{
	dialog = new InspectorBase( this );
	dialog->setMinimumSize( 550, 450 );
	setMainWidget( dialog );
	adjustSize();
	
	loadUseFlagDescription();
	
	connect( dialog->pbPrevious, SIGNAL( clicked() ), this, SLOT( slotPreviousPackage() ) );
	connect( dialog->pbNext, SIGNAL( clicked() ), this, SLOT( slotNextPackage() ) );
	
	connect( dialog->cbVersionsEbuild, SIGNAL( activated( const QString& ) ), this, SLOT( slotGetEbuild( const QString& ) ) );
	connect( dialog->cbVersionsDependencies, SIGNAL( activated ( const QString& ) ), this, SLOT( slotGetDependencies( const QString& ) ) );
	connect( dialog->cbVersionsInstalled, SIGNAL( activated ( const QString& ) ), this, SLOT( slotGetInstalledFiles( const QString& ) ) );
	connect( dialog->cbVersionsUse, SIGNAL( activated ( const QString& ) ), this, SLOT( slotGetUseFlags( const QString& ) ) );
	
	// Load files if tabpage is open
	connect( dialog->inspectorTabs, SIGNAL( currentChanged( QWidget* ) ), this, SLOT( slotActivateTabs() ) );
	
	// Toggle between all 4 stability version
	connect( dialog->groupSelectStability, SIGNAL( released( int ) ), this, SLOT( slotSetStability( int ) ) );
	
	// Activate specific version menu
	connect( dialog->cbVersionsSpecific, SIGNAL( activated( const QString& ) ), this, SLOT( slotSetVersionSpecific( const QString& ) ) );
	
}

PackageInspector::~PackageInspector()
{
}

/**
 * Make previous package in package view current - for easier browsing.
 */
void PackageInspector::slotPreviousPackage()
{
	kdDebug() << "PackageInspector::slotPreviousPackage" << endl;
	
	if ( m_hasSettingsChanged )
		switch( KMessageBox::warningYesNo( this,
			i18n( "<qt>Settings are changed!<br>"
					"Do you want to save them?</qt>"), i18n("Saving settings"), i18n("Yes"), i18n("No"), 0 ) ) {
						
				case KMessageBox::Yes:
					slotApply();
			}
	
	if ( m_hasSettingsChanged )
		m_hasSettingsChanged = false;
	emit signalNextPackage( true );
}

/**
 * Make next package in package view current - for easier browsing.
 */
void PackageInspector::slotNextPackage()
{
	kdDebug() << "PackageInspector::slotNextPackage" << endl;
	
	if ( m_hasSettingsChanged )
		switch( KMessageBox::warningYesNo( this,
			i18n( "<qt>Settings are changed!<br>"
					"Do you want to save them?</qt>"), i18n("Saving settings"), i18n("Yes"), i18n("No"), 0 ) ) {
						
				case KMessageBox::Yes:
					slotApply();
			}
	
	if ( m_hasSettingsChanged )
		m_hasSettingsChanged = false;
	emit signalNextPackage( false );
}

void PackageInspector::showHardMaskInfo()
{
	kdDebug() << "PackageInspector::showHardMaskInfo" << endl;
	QString comment = KurooDBSingleton::Instance()->packageHardMaskComment( m_portagePackage->id() );
	if ( !comment.isEmpty() )
		KPassivePopup::message( comment, dialog->versionsView );
}

/**
 * Activate advanced groupBox.
 * @param isOn
 */
void PackageInspector::slotAdvancedToggle( bool isOn )
{
	dialog->groupArchitecture->setDisabled( !isOn );
}

/**
 * Activate Inspector with current package.
 * @param portagePackage
 */
void PackageInspector::edit( PortageListView::PortageItem* portagePackage )
{
	if ( !KUser().isSuperUser() ) {
		enableButtonApply( false );
		dialog->groupSelectStability->setDisabled( true );
		dialog->groupAdvanced->setDisabled( true );
		dialog->useView->setDisabled( true );
	}
	
	m_portagePackage = portagePackage;
	package = m_portagePackage->name();
	category = m_portagePackage->category();
	dialog->package->setText( "<font size=\"+2\">" + package + "</font> " +
	                          "(" + category.section( "-", 0, 0 ) + " / " +
	                          category.section( "-", 1, 1 ) + ")");
	
	slotInstallVersion();
	slotActivateTabs();
	show();
}

/**
 * Stability choice for versions - enable the right radiobutton.
 * Priority is: specific version >> unmask package >> untest package >> stable package.
 */
void PackageInspector::slotInstallVersion()
{
	kdDebug() << "\nPackageInspector::slotInstallVersion" << m_portagePackage->id() << endl;
	
	disconnect( dialog->ckbAvailable, SIGNAL( toggled( bool ) ), this, SLOT( slotAvailable( bool ) ) );
	dialog->cbVersionsSpecific->setDisabled( true );
	dialog->ckbAvailable->setChecked( false );
	
	// Get user mask specific version
	QString userMaskVersion = KurooDBSingleton::Instance()->packageUserMaskAtom( m_portagePackage->id() ).first();
	userMaskVersion = userMaskVersion.section( ( userMaskVersion.section( rxPortageVersion, 0, 0 ) + "-" ), 1, 1 );
	
	if ( !userMaskVersion.isEmpty() ) {
		dialog->rbVersionsSpecific->setChecked( true );
		dialog->cbVersionsSpecific->setDisabled( false );
		dialog->cbVersionsSpecific->setCurrentText( userMaskVersion );
	}
	else
		if ( KurooDBSingleton::Instance()->isPackageUnMasked( m_portagePackage->id() ) )
			dialog->rbMasked->setChecked( true );
		else
			if ( KurooDBSingleton::Instance()->isPackageUnTesting( m_portagePackage->id() ) )
				dialog->rbTesting->setChecked( true );
			else
				dialog->rbStable->setChecked( true );
	
	if ( KurooDBSingleton::Instance()->isPackageAvailable( m_portagePackage->id() ) )
		dialog->ckbAvailable->setChecked( true );
	
	showHardMaskInfo();
	
	enableButton( KDialogBase::Apply, false );
	connect( dialog->ckbAvailable, SIGNAL( toggled( bool ) ), this, SLOT( slotAvailable( bool ) ) );
}

/**
 * Load the files for this package and this version.
 */
void PackageInspector::slotActivateTabs()
{
	slotGetUseFlags( dialog->cbVersionsUse->currentText() );
	slotGetEbuild( dialog->cbVersionsEbuild->currentText() );
	slotGetDependencies( dialog->cbVersionsDependencies->currentText() );
	slotGetInstalledFiles( dialog->cbVersionsInstalled->currentText() );
	getChangeLog();
}

/**
 * Load internal map with use flag description. @fixme: not complete yet!
 */
void PackageInspector::loadUseFlagDescription()
{
	QString useFile( KurooConfig::dirPortage() + "/profiles/use.desc" );
	QFile f( useFile );
	
	if ( f.open( IO_ReadOnly ) ) {
		QTextStream stream( &f );
		
		while ( !stream.atEnd() ) {
			QString line = stream.readLine();
			if ( !line.startsWith( "#" ) && !line.isEmpty() ) {
				if ( !line.contains( QRegExp( "^alpha|^amd64|^arm|^hppa|^ia64|^mips|^ppc|^ppc64|^ppc-macos|^s390|^sh|^sparc|^x86" ) ) ) {
					QString use = line.section( " - ", 0, 0 );
					QString useDescription = line.section( use + " - ", 1, 1 );
					useMap.insert( use, useDescription );
				}
			}
		}
		f.close();
	}
	else
		kdDebug() << i18n( "Error reading: " ) << useFile << endl;
}

/**
 * View use flags for selected version.
 * @param version
 */
void PackageInspector::slotGetUseFlags( const QString& version )
{
	if (  dialog->inspectorTabs->currentPageIndex() == 1 ) {
		QStringList useList;
		
		QMap<QString,PackageVersion*>::iterator itMap = m_portagePackage->versionMap().find( version );
		if ( itMap != m_portagePackage->versionMap().end() )
			useList = itMap.data()->useflags();
	
		dialog->useView->clear();
		foreach ( useList ) {
			QString lines;
			
			QMap<QString, QString>::iterator itMap = useMap.find( *it );
			if ( itMap != useMap.end() )
				lines = itMap.data();
			
			// Split long description into multiple lines
			QStringList description;
			if ( lines.length() <= 90 )
				description = lines;
			else {
				while ( lines.length() > 90 ) {
					int pos = ( lines.left(90) ).findRev(' ');
					QString line = lines.left(pos+1);
					lines = lines.right( lines.length() - line.length() );
					description += line;
				}
				description += lines;
			}
			
			QCheckListItem* useItem = new QCheckListItem( dialog->useView, *it, QCheckListItem::CheckBox );
			useItem->setTristate( true );
			useItem->setMultiLinesEnabled( true );
			useItem->setText( 1, description.join("\n") );
			
			if ( KurooDBSingleton::Instance()->hasPackageUse( m_portagePackage->id(), *it ) )
				useItem->setOn( true );
				
		}
	}
}
/**
 * Save the new use flags created with ActionSelector
 */
void PackageInspector::slotApply()
{
	
	PortageFilesSingleton::Instance()->savePackageKeywords();
	PortageFilesSingleton::Instance()->savePackageUserMask();
	PortageFilesSingleton::Instance()->savePackageUserUnMask();
	PortageFilesSingleton::Instance()->savePackageUse();
	enableButton( KDialogBase::Apply, false );
	m_hasSettingsChanged = false;

}

/**
 * Get ebuild for selected version.
 * @param version
 */
void PackageInspector::slotGetEbuild( const QString& version )
{
	if (  dialog->inspectorTabs->currentPageIndex() == 2 ) {
		QString fileName = KurooConfig::dirPortage() + "/" + category + "/" + package + "/" + package + "-" + version + ".ebuild";
		QFile file( fileName );
		
		if ( !file.exists() ) {
			fileName = KurooConfig::dirPortageOverlay() + "/" + category + "/" + package + "/" + package + "-" + version + ".ebuild";
			file.setName( fileName );
		}
		
		if ( file.open( IO_ReadOnly ) ) {
			QTextStream stream( &file );
			QString textLines;
			while ( !stream.atEnd() )
				textLines += stream.readLine() + "<br>";
			file.close();
			dialog->ebuildBrowser->setText( textLines );
		}
		else {
			kdDebug() << i18n("Error reading: ") << fileName << endl;
			dialog->ebuildBrowser->setText( i18n("<font color=darkGrey><b>Ebuild not found.</b></font>") );
		}
	}
}

/**
 * Get dependencies for selected version.
 * @param version
 */
void PackageInspector::slotGetDependencies( const QString& version )
{
	if (  dialog->inspectorTabs->currentPageIndex() == 3 ) {
		QString fileName = KurooConfig::dirEdbDep() + "/usr/portage/" + category + "/" + package + "-" + version;
		QFile file( fileName );
		
		if ( !file.exists() ) {
			fileName = KurooConfig::dirEdbDep() + "/usr/local/portage/" + category + "/" + package + "-" + version;
			file.setName( fileName );
		}
		
		if ( file.open( IO_ReadOnly ) ) {
			QTextStream stream( &file );
			QString textLines;
			int lineCount( 0 );
			while ( !stream.atEnd() ) {
				QString line = stream.readLine();
				if ( line.isEmpty() )
					continue;
				
				if ( lineCount++ > 1 || line == "0" )
					break;
				else
					textLines += line + "<br>";
			}
			file.close();
			dialog->dependencyBrowser->setText( textLines );
		}
		else {
			kdDebug() << i18n("Error reading: ") << fileName << endl;
			dialog->dependencyBrowser->setText( i18n("<font color=darkGrey><b>Dependencies not found.</b></font>") );
		}
	}
}

/**
 * Get this package changelog.
 */
void PackageInspector::getChangeLog()
{
	if (  dialog->inspectorTabs->currentPageIndex() == 4 ) {
		QString fileName = KurooConfig::dirPortage() + "/" + category + "/" + package + "/ChangeLog";
		QFile file( fileName );
		
		if ( !file.exists() ) {
			fileName = KurooConfig::dirPortageOverlay() + "/" + category + "/" + package + "/ChangeLog";
			file.setName( fileName );
		}
		
		if ( file.open( IO_ReadOnly ) ) {
			QTextStream stream( &file );
			QString textLines;
			while ( !stream.atEnd() )
				textLines += stream.readLine() + "<br>";
			file.close();
			dialog->changelogBrowser->setText( textLines );
		}
		else {
			kdDebug() << i18n("Error reading: ") << fileName << endl;
			dialog->changelogBrowser->setText( i18n("<font color=darkGrey><b>ChangeLog not found.</b></font>") );
		}
	}
}

/**
 * Get list of installed files for selected version.
 * @param version
 */
void PackageInspector::slotGetInstalledFiles( const QString& version )
{
	if ( !version.isEmpty() && dialog->inspectorTabs->currentPageIndex() == 5 ) {
		QString filename = KurooConfig::dirDbPkg() + "/" + category + "/" + package + "-" + version + "/CONTENTS";
		QFile file( filename );
		QString textLines;
		if ( file.open( IO_ReadOnly ) ) {
			QTextStream stream( &file );
			while ( !stream.atEnd() ) {
				QString line = stream.readLine();
				if ( line.startsWith( "obj" ) )
					textLines += line.section( "obj ", 1, 1 ).section( " ", 0, 0 ) + "\n";
			}
			file.close();
			dialog->installedFilesBrowser->setText( textLines );
		}
		else
			kdDebug() << i18n( "Error reading: " ) << filename << endl;
	}
	else
		dialog->installedFilesBrowser->setText( i18n("<font color=darkGrey><b>Installed files list not found.</b></font>") );
}

/**
 * Store stability settings from radiobuttons.
 * @param the selected radiobutton
 */
void PackageInspector::slotSetStability( int rbStability )
{
// 	kdDebug() << "PackageInspector::slotSetStability id=" << m_portagePackage->id() << " rbStability=" << rbStability << endl;

	switch ( rbStability ) {
	
		// User wants only stable package
		case 0 :
			dialog->cbVersionsSpecific->setDisabled( true );
		
			// Clear package from package.keywords, package.unmask and package.mask
			KurooDBSingleton::Instance()->clearPackageUnTesting( m_portagePackage->id() );
			KurooDBSingleton::Instance()->clearPackageUnMasked( m_portagePackage->id() );
			KurooDBSingleton::Instance()->clearPackageUserMasked( m_portagePackage->id() );
			KurooDBSingleton::Instance()->clearPackageAvailable( m_portagePackage->id() );
		
			m_portagePackage->resetDetailedInfo();
			emit signalPackageChanged();
			break;
		
		// User wants only testing package
		case 1 :
			dialog->cbVersionsSpecific->setDisabled( true );
		
			// Clear package from package.unmask and package.mask
			KurooDBSingleton::Instance()->clearPackageUnMasked( m_portagePackage->id() );
			KurooDBSingleton::Instance()->clearPackageUserMasked( m_portagePackage->id() );
		
			KurooDBSingleton::Instance()->setPackageUnTesting( m_portagePackage->id() );
			m_portagePackage->resetDetailedInfo();
			emit signalPackageChanged();
			break;
		
		// User wants only hardmasked package
		case 2 :
			dialog->cbVersionsSpecific->setDisabled( true );
		
			// Clear package from package.keywords and package.mask
			KurooDBSingleton::Instance()->clearPackageUserMasked( m_portagePackage->id() );
		
			KurooDBSingleton::Instance()->setPackageUnTesting( m_portagePackage->id() );
			KurooDBSingleton::Instance()->setPackageUnMasked( m_portagePackage->id() );
			m_portagePackage->resetDetailedInfo();
			emit signalPackageChanged();
			break;
		
		// User wants only specific version and no further
		case 3 :
			dialog->cbVersionsSpecific->setDisabled( false );
		
	}
	
	enableButton( KDialogBase::Apply, true );
	m_hasSettingsChanged = true;
}

/**
 * User has selected a specific version to unmask and wants no higher version. @fixme: check if untesting and unmasked first?
 * @param version
 */
void PackageInspector::slotSetVersionSpecific( const QString& version )
{
// 	kdDebug() << "PackageInspector::slotSetVersionSpecific version=" << version << endl;
	
	KurooDBSingleton::Instance()->setPackageUnTesting( m_portagePackage->id() );
	KurooDBSingleton::Instance()->setPackageUnMasked( m_portagePackage->id() );
	KurooDBSingleton::Instance()->setPackageUserMasked( m_portagePackage->id(), version );
	
	m_portagePackage->resetDetailedInfo();
	emit signalPackageChanged();
}

/**
 * Make this package available on users architecture.
 * @param isAvailable
 */
void PackageInspector::slotAvailable( bool isAvailable )
{
	if ( isAvailable )
		KurooDBSingleton::Instance()->setPackageAvailable( m_portagePackage->id() );
	else
		KurooDBSingleton::Instance()->clearPackageAvailable( m_portagePackage->id() );
	
	m_portagePackage->resetDetailedInfo();
	enableButton( KDialogBase::Apply, true );
	m_hasSettingsChanged = true;
	emit signalPackageChanged();
}

#include "packageinspector.moc"
