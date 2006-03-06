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
#include <qtextcodec.h>

#include <ktabwidget.h>
#include <kactionselector.h>
#include <ktextbrowser.h>
#include <kmessagebox.h>
#include <kuser.h>
#include <klistview.h>
#include <kurllabel.h>
#include <kprocio.h>

// Portage files
enum portageFiles {
		PACKAGE_STABLE = 1,
		PACKAGE_KEYWORDS = 2,
		PACKAGE_UNMASK = 4,
		PACKAGE_MASK = 8
};

/**
 * @class PackageInspector
 * @short The package Inspector dialog for all advanced settings.
 */
PackageInspector::PackageInspector( QWidget *parent )
	: KDialogBase( KDialogBase::Swallow, 0, parent, i18n( "Package details" ), false, i18n( "Package details" ), KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel, KDialogBase::Apply, false ), category( QString::null ), package( QString::null ), m_portagePackage( 0 ),
	hasVersionSettingsChanged( false ), hasUseSettingsChanged( false ),
	isVirginState( true ), stabilityBefore ( 0 ), versionBefore( QString::null ), isAvailableBefore( false ), packageStability( PACKAGE_STABLE ),
	hardMaskComment( QString::null )
{
	dialog = new InspectorBase( this );
	setMainWidget( dialog );
	adjustSize();
	
	// Get use flag description @fixme: load local description
	loadUseFlagDescription();
	
	// Shortcuts for browsing packages
	connect( dialog->pbPrevious, SIGNAL( clicked() ), this, SLOT( slotPreviousPackage() ) );
	connect( dialog->pbNext, SIGNAL( clicked() ), this, SLOT( slotNextPackage() ) );
	
	// Refresh files when changing version
	connect( dialog->cbVersionsEbuild, SIGNAL( activated( const QString& ) ), this, SLOT( slotLoadEbuild( const QString& ) ) );
	connect( dialog->cbVersionsDependencies, SIGNAL( activated ( const QString& ) ), this, SLOT( slotLoadDependencies( const QString& ) ) );
	connect( dialog->cbVersionsInstalled, SIGNAL( activated ( const QString& ) ), this, SLOT( slotLoadInstalledFiles( const QString& ) ) );
	connect( dialog->cbVersionsUse, SIGNAL( activated ( const QString& ) ), this, SLOT( slotLoadUseFlags( const QString& ) ) );
	
	// Load files only if tabpage is open
	connect( dialog->inspectorTabs, SIGNAL( currentChanged( QWidget* ) ), this, SLOT( slotRefreshTabs() ) );
	
	// Toggle between all 4 stability version
	connect( dialog->groupSelectStability, SIGNAL( released( int ) ), this, SLOT( slotSetStability( int ) ) );
	
	// Activate specific version menu
	connect( dialog->cbVersionsSpecific, SIGNAL( activated( const QString& ) ), this, SLOT( slotSetSpecificVersion( const QString& ) ) );
	
	connect( dialog->infoHardMasked, SIGNAL( leftClickedURL( const QString& ) ), SLOT( slotHardMaskInfo() ) );
	
	connect( dialog->useView, SIGNAL( clicked( QListViewItem* ) ), this, SLOT( slotSetUseFlags( QListViewItem* ) ) );
	
	connect( dialog->pbUse, SIGNAL( clicked() ), this, SLOT( slotCalculateUse() ) );
}

PackageInspector::~PackageInspector()
{
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Buttons slots
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Make previous package in package view current - for easier browsing.
 * Ask to save settings if user has changed settings.
 * If no saving the changes rollback to the latest radiobutton setting.
 */
void PackageInspector::slotPreviousPackage()
{
	if ( hasVersionSettingsChanged || hasUseSettingsChanged )
		switch( KMessageBox::warningYesNo( this,
			i18n( "<qt>Settings are changed!<br>"
					"Do you want to save them?</qt>"), i18n("Saving settings"), i18n("Yes"), i18n("No"), 0 ) ) {
						
			case KMessageBox::Yes:
				slotApply();
				break;
					
			case KMessageBox::No:
				rollbackSettings();
		}
	
	hasVersionSettingsChanged = false;
	hasUseSettingsChanged = false;
	emit signalNextPackage( true );
}

/**
 * Make next package in package view current - for easier browsing.
 * Ask to save settings if user has changed settings.
 * If no saving the changes rollback to the latest radiobutton setting.
 */
void PackageInspector::slotNextPackage()
{
	if ( hasVersionSettingsChanged || hasUseSettingsChanged )
		switch( KMessageBox::warningYesNo( this,
			i18n( "<qt>Settings are changed!<br>"
					"Do you want to save them?</qt>"), i18n("Saving settings"), i18n("Yes"), i18n("No"), 0 ) ) {
						
			case KMessageBox::Yes:
				slotApply();
				break;
				
			case KMessageBox::No:
				rollbackSettings();
		}
	
	hasVersionSettingsChanged = false;
	hasUseSettingsChanged = false;
	emit signalNextPackage( false );
}

/**
 * Save the stability setting for this package.
 * @fixme: save only changed tables.
 */
void PackageInspector::slotApply()
{
	if ( hasVersionSettingsChanged ) {
		if ( packageStability & PACKAGE_KEYWORDS )
			PortageFilesSingleton::Instance()->savePackageKeywords();
		if ( packageStability & PACKAGE_MASK )
			PortageFilesSingleton::Instance()->savePackageUserMask();
		if ( packageStability & PACKAGE_UNMASK )
			PortageFilesSingleton::Instance()->savePackageUserUnMask();
		
		// Check if this version is in updates. If not add it! (Only for packages in world).
		if ( PortageSingleton::Instance()->isInWorld( category + "/" + package ) )
			UpdatesSingleton::Instance()->checkUpdates( m_id, dialog->versionsView->updateVersion(), dialog->versionsView->hasUpdate() );
		
	}
	
	if ( hasUseSettingsChanged ) {
		
		// Get use flags
		QStringList useList, pretendUseList;
		QListViewItem* myChild = dialog->useView->firstChild();
		while ( myChild ) {
			QString useFlag = myChild->text(0);
			pretendUseList += useFlag;
			if ( !useFlag.contains( "^\\(|\\)$" ) )
				useList += useFlag.remove( QRegExp( "^\\+|\\*$" ) );
			myChild = myChild->nextSibling();
		}
		
		// Store in db and save to file
		if ( !useList.isEmpty() ) {
			KurooDBSingleton::Instance()->setPackageUse( m_id, useList.join(" ") );
			PortageFilesSingleton::Instance()->savePackageUse();
		}
		
		emit signalUseChanged();
	}
	
	enableButtonApply( false );
	hasVersionSettingsChanged = false;
	hasUseSettingsChanged = false;
	isVirginState = true;
}

/**
 * Cancel and rollback to old settings.
 */
void PackageInspector::slotCancel()
{
	rollbackSettings();
	hasVersionSettingsChanged = false;
	accept();
}

void PackageInspector::slotOk()
{
	slotApply();
	accept();
}

/**
 * Rollback settings to state before changed by user.
 */
void PackageInspector::rollbackSettings()
{
	if ( hasVersionSettingsChanged ) {
		slotSetStability( stabilityBefore  );
		
		if ( stabilityBefore == 3 )
			slotSetSpecificVersion( versionBefore );
		
		if ( isAvailableBefore )
			slotSetAvailable( true );
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Create hardmask info link.
 */
void PackageInspector::showHardMaskInfo()
{
	const QStringList hardMaskInfo = KurooDBSingleton::Instance()->packageHardMaskInfo( m_id );
	
	if ( !hardMaskInfo.isEmpty() ) {
		QFont font;
		font.setBold( true );
		dialog->infoHardMasked->setFont( font );
		dialog->infoHardMasked->setHighlightedColor( Qt::red );
		dialog->infoHardMasked->setText( i18n("Click for hardmask info!") );
		
		hardMaskComment =
			"<font size=\"+2\">" + package + "</font> " + 
			"(" + category.section( "-", 0, 0 ) + "/" + category.section( "-", 1, 1 ) + ")<br><br>" +
			hardMaskInfo.last() + "<br><br>" +
			"Hardmask rule: <i>\"" + hardMaskInfo.first() + "\"</i>";
	}
	else
		dialog->infoHardMasked->setText( QString::null );
}

/**
 * Show gentoo devs reason for hardmasking this package/versions.
 */
void PackageInspector::slotHardMaskInfo()
{
	KMessageBox::messageBox( 0, KMessageBox::Information, hardMaskComment, 
	                         i18n("%1/%2 hardmask info!").arg( category ).arg( package ), i18n("Yes"), i18n("No"), 0 );
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
void PackageInspector::edit( PackageItem* portagePackage )
{
	m_portagePackage = portagePackage;
	package = m_portagePackage->name();
	category = m_portagePackage->category();
	
	if ( !KUser().isSuperUser() ) {
		enableButtonApply( false );
		dialog->groupSelectStability->setDisabled( true );
		dialog->useView->setDisabled( true );
		dialog->groupArchitecture->setDisabled( true );
	}
	
	// Disabled editing when package is in Queue and kuroo is emerging
	if ( m_portagePackage->isQueued() && EmergeSingleton::Instance()->isRunning() ) {
		dialog->inspectorTabs->page(0)->setDisabled( true );
		dialog->inspectorTabs->page(1)->setDisabled( true );
	}
	else {
		dialog->inspectorTabs->page(0)->setDisabled( false );
		dialog->inspectorTabs->page(1)->setDisabled( false );
	}
	
	// Is it first time we load this package
	if ( m_id != m_portagePackage->id() ) {
		m_id = m_portagePackage->id();
		isVirginState = true;
	}
	else
		isVirginState = false;

	// Construct header text
	dialog->headerFrame->setPaletteBackgroundColor( QColor(136,187,34) );
	dialog->package->setText( "<b><font color=white><font size=+1>" + package + "</font> " +
	                          "(" + category.section( "-", 0, 0 ) + "/" + category.section( "-", 1, 1 ) + ")</b></font>" );
	dialog->package2->setText( m_portagePackage->description() );
	
	showSettings();
	slotRefreshTabs();
	
	// Enable/disable shortcuts buttons if first or last package
	if ( m_portagePackage->isFirstPackage() )
		dialog->pbPrevious->setDisabled( true );
	else
		dialog->pbPrevious->setDisabled( false );
		
	if ( m_portagePackage->isLastPackage() )
		dialog->pbNext->setDisabled( true );
	else
		dialog->pbNext->setDisabled( false );
	
	show();
}

/**
 * Stability choice for versions - enable the right radiobutton.
 * Priority is: specific version >> unmask package >> untest package >> stable package.
 */
void PackageInspector::showSettings()
{
	disconnect( dialog->ckbAvailable, SIGNAL( toggled( bool ) ), this, SLOT( slotSetAvailable( bool ) ) );
	
	// Get user mask specific version
	QString userMaskVersion = KurooDBSingleton::Instance()->packageUserMaskAtom( m_id );
	userMaskVersion = userMaskVersion.section( ( userMaskVersion.section( rxPortageVersion, 0, 0 ) + "-" ), 1, 1 );
	
	// Enable stability radiobutton
	if ( !userMaskVersion.isEmpty() ) {
		dialog->rbVersionsSpecific->setChecked( true );
		dialog->cbVersionsSpecific->setDisabled( false );
		dialog->cbVersionsSpecific->setCurrentText( userMaskVersion );
	}
	else {
		dialog->ckbAvailable->setChecked( false );
		
		if ( KurooDBSingleton::Instance()->isPackageUnMasked( m_id ) )
			dialog->rbMasked->setChecked( true );
		else
			if ( KurooDBSingleton::Instance()->isPackageUnTesting( m_id ) )
				dialog->rbTesting->setChecked( true );
			else
				dialog->rbStable->setChecked( true );
	}
	
	// Enable available radiobutton
	if ( KurooDBSingleton::Instance()->isPackageAvailable( m_id ) )
		dialog->ckbAvailable->setChecked( true );
	else
		dialog->ckbAvailable->setChecked( false );
	
	// Stability settings before user has changed it
	if ( isVirginState ) {
		stabilityBefore = dialog->groupSelectStability->selectedId();
		versionBefore = userMaskVersion;
		isAvailableBefore = dialog->ckbAvailable->isChecked();
		dialog->groupArchitecture->setChecked( false );
	}
	
	showHardMaskInfo();

	// Reset the apply button for new package
	if ( !hasVersionSettingsChanged )
		enableButtonApply( false );
	
	connect( dialog->ckbAvailable, SIGNAL( toggled( bool ) ), this, SLOT( slotSetAvailable( bool ) ) );
}

/**
 * Apply stability settings from radiobuttons. @fixme: use enum
 * @param the selected radiobutton
 */
void PackageInspector::slotSetStability( int rbStability )
{
	switch ( rbStability ) {
		
		// User wants only stable package
		case 0 :
			dialog->cbVersionsSpecific->setDisabled( true );
		
			// Clear package from package.keywords, package.unmask and package.mask
			KurooDBSingleton::Instance()->clearPackageUnTesting( m_id );
			KurooDBSingleton::Instance()->clearPackageUnMasked( m_id );
			KurooDBSingleton::Instance()->clearPackageUserMasked( m_id );
			KurooDBSingleton::Instance()->clearPackageAvailable( m_id );
		
			m_portagePackage->resetDetailedInfo();
			packageStability = PACKAGE_STABLE;
			emit signalPackageChanged();
			break;
		
		// User wants only testing package
		case 1 :
			dialog->cbVersionsSpecific->setDisabled( true );
		
			// Clear package from package.unmask and package.mask
			KurooDBSingleton::Instance()->clearPackageUnMasked( m_id );
			KurooDBSingleton::Instance()->clearPackageUserMasked( m_id );
		
			KurooDBSingleton::Instance()->setPackageUnTesting( m_id );
			m_portagePackage->resetDetailedInfo();
			packageStability = PACKAGE_KEYWORDS;
			emit signalPackageChanged();
			break;
		
		// User wants only hardmasked package
		case 2 :
			dialog->cbVersionsSpecific->setDisabled( true );
		
			// Clear package from package.keywords and package.mask
			KurooDBSingleton::Instance()->clearPackageUserMasked( m_id );
		
			KurooDBSingleton::Instance()->setPackageUnTesting( m_id );
			KurooDBSingleton::Instance()->setPackageUnMasked( m_id );
			m_portagePackage->resetDetailedInfo();
			packageStability = PACKAGE_KEYWORDS | PACKAGE_UNMASK;
			emit signalPackageChanged();
			break;
		
		// User wants only specific version
		case 3 :
			dialog->cbVersionsSpecific->setDisabled( false );
		
	}

	enableButtonApply( true );
	hasVersionSettingsChanged = true;
}

/**
 * User has selected a specific version to unmask and wants no higher version.
 * @param version
 */
void PackageInspector::slotSetSpecificVersion( const QString& version )
{
	enableButtonApply( true );
	hasVersionSettingsChanged = true;
	
	KurooDBSingleton::Instance()->setPackageUnTesting( m_id );
	KurooDBSingleton::Instance()->setPackageUnMasked( m_id, version );
	KurooDBSingleton::Instance()->setPackageUserMasked( m_id, version );
	
	m_portagePackage->resetDetailedInfo();
	packageStability = PACKAGE_KEYWORDS | PACKAGE_UNMASK | PACKAGE_MASK;
	emit signalPackageChanged();
}

/**
 * Make this package available on users arch.
 * @param isAvailable
 */
void PackageInspector::slotSetAvailable( bool isAvailable )
{
	if ( isAvailable )
		KurooDBSingleton::Instance()->setPackageAvailable( m_id );
	else
		KurooDBSingleton::Instance()->clearPackageAvailable( m_id );
	
	enableButtonApply( true );
	hasVersionSettingsChanged = true;
	m_portagePackage->resetDetailedInfo();
	emit signalPackageChanged();
}

/**
 * Toggle use flag state to add or remove.
 * @param: useItem
 */
void PackageInspector::slotSetUseFlags( QListViewItem* useItem )
{
	if ( !useItem )
		return;
	
	QString use = useItem->text( 0 );
	switch ( dynamic_cast<QCheckListItem*>(useItem)->state() ) {
	
		case ( QCheckListItem::Off ) :
			useItem->setText( 0, use.replace( QRegExp("^\\+"), "-") );
			break;
		
		case ( QCheckListItem::On ) :
			useItem->setText( 0, use.replace( QRegExp("^\\-"), "+") );

	}
	
	enableButtonApply( true );
	hasUseSettingsChanged = true;
}


//////////////////////////////////////////////////////////////////////////////////////////////
// Load files
//////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Load the files for this package and this version.
 */
void PackageInspector::slotRefreshTabs()
{
	slotLoadUseFlags( dialog->cbVersionsUse->currentText() );
	slotLoadEbuild( dialog->cbVersionsEbuild->currentText() );
	slotLoadDependencies( dialog->cbVersionsDependencies->currentText() );
	slotLoadInstalledFiles( dialog->cbVersionsInstalled->currentText() );
	loadChangeLog();
// 	adjustSize();
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
			if ( !line.startsWith( "#" ) && !line.isEmpty() && !line.contains( QRegExp(
				"^alpha|^amd64|^arm|^hppa|^ia64|^mips|^ppc|^ppc64|^ppc-macos|^s390|^sh|^sparc|^x86" ) ) ) {
				QString use = line.section( " - ", 0, 0 );
				QString useDescription = line.section( use + " - ", 1, 1 );
				useMap.insert( use, useDescription );
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
void PackageInspector::slotLoadUseFlags( const QString& version )
{
	dialog->useView->setDisabled( true );
	
	if ( dialog->inspectorTabs->currentPageIndex() == 1 ) {
		QStringList useList;
		
		QMap<QString, PackageVersion*>::iterator itMap = m_portagePackage->versionMap().find( version );
		if ( itMap != m_portagePackage->versionMap().end() )
			useList = itMap.data()->useflags();
		
		// Get user set package use flags
		QStringList packageUseList = QStringList::split( " ", KurooDBSingleton::Instance()->packageUse( m_id ) );
		
		dialog->useView->clear();
		QStringList tmpUseList;
		foreach ( useList ) {
			
			// Seems some use flags are duplicated, filter them out
			if ( tmpUseList.find(*it) != tmpUseList.end() )
				continue;
			else
				tmpUseList += *it;
			
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
					QString line = lines.left( pos + 1 );
					lines = lines.right( lines.length() - line.length() );
					description += line;
				}
				description += lines;
			}
			
			// Add use flag in use view
			QCheckListItem* useItem = new QCheckListItem( dialog->useView, *it, QCheckListItem::CheckBox );
			useItem->setMultiLinesEnabled( true );
			useItem->setText( 1, description.join("\n") );
		}
	}
}

/**
 * Get this package changelog.
 */
void PackageInspector::loadChangeLog()
{
	if (  dialog->inspectorTabs->currentPageIndex() == 2 ) {
		QString fileName = KurooDBSingleton::Instance()->packagePath( m_id, dialog->cbVersionsEbuild->currentText() ) + "/" + category + "/" + package + "/ChangeLog";
		QFile file( fileName );
		
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
			dialog->changelogBrowser->setText( i18n("<font color=darkGrey><b>No ChangeLog found.</b></font>") );
		}
	}
}

/**
 * Get ebuild for selected version.
 * @param version
 */
void PackageInspector::slotLoadEbuild( const QString& version )
{
	if (  dialog->inspectorTabs->currentPageIndex() == 3 ) {
		QString fileName = KurooDBSingleton::Instance()->packagePath( m_id, version ) + "/" + category + "/" + package + "/" + package + "-" + version + ".ebuild";
		QFile file( fileName );
		
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
			dialog->ebuildBrowser->setText( i18n("<font color=darkGrey><b>No ebuild found.</b></font>") );
		}
	}
}

/**
 * Get dependencies for selected version.
 * @param version
 */
void PackageInspector::slotLoadDependencies( const QString& version )
{
	if (  dialog->inspectorTabs->currentPageIndex() == 4 ) {
		QString fileName = KurooConfig::dirEdbDep() + KurooDBSingleton::Instance()->packagePath( m_id, version ) + "/" + category + "/" + package + "-" + version;
		QFile file( fileName );
		
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
			dialog->dependencyBrowser->setText( i18n("<font color=darkGrey><b>No dependencies found.</b></font>") );
		}
	}
}

/**
 * Get list of installed files for selected version.
 * @param version
 */
void PackageInspector::slotLoadInstalledFiles( const QString& version )
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
		dialog->installedFilesBrowser->setText( i18n("<font color=darkGrey><b>No installed files found.</b></font>") );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Use calculation
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Run emerge pretend to get use active use flags.
 */
void PackageInspector::slotCalculateUse()
{
	pretendUseLines.clear();
	QTextCodec *codec = QTextCodec::codecForName("utf8");
	KProcIO* eProc = new KProcIO( codec );
	*eProc << "emerge" << "--nospinner" << "--nocolor" << "-pv" << category + "/" + package;
	
	connect( eProc, SIGNAL( processExited( KProcess* ) ), this, SLOT( slotParsePackageUse( KProcess* ) ) );
	connect( eProc, SIGNAL( readReady( KProcIO* ) ), this, SLOT( slotCollectPretendOutput( KProcIO* ) ) );
	eProc->start( KProcess::NotifyOnExit, true );
	SignalistSingleton::Instance()->setKurooBusy( true );
	
	if ( !eProc->isRunning() ) {
		LogSingleton::Instance()->writeLog( i18n("\nError: Could not calculate use flag for package %1/%2.").arg( category ).arg( package ), ERROR );
		slotParsePackageUse( eProc );
	}
	else
		dialog->setDisabled( true );
}

/**
 * Collect emerge pretend output in pretendUseLines.
 * @param eProc
 */
void PackageInspector::slotCollectPretendOutput( KProcIO* eProc )
{
	QString line;
	while ( eProc->readln( line, true ) >= 0 )
		pretendUseLines += line;
}

/**
 * Parse emerge pretend output for all use flags.
 * @param eProc
 */
void PackageInspector::slotParsePackageUse( KProcess* eProc )
{
	dialog->setDisabled( false );
	
	SignalistSingleton::Instance()->setKurooBusy( false );
	delete eProc;
	eProc = 0;
	
	QRegExp rxPretend( "^\\[ebuild([\\s|\\w]*)\\]\\s+"
	                   "((\\S+)/(\\S+))\\s*(?:\\[(\\S*)\\])*\\s*"
	                   "(?:USE=\")?([\\%\\-\\+\\w\\s\\(\\)\\*]*)\"?"
	                   "\\s+([\\d,]*)\\s+kB" );
	QStringList pretendUseList;
	foreach ( pretendUseLines ) {
		if ( !(*it).isEmpty() && rxPretend.search( *it ) > -1 ) {
			QString use = rxPretend.cap(6).simplifyWhiteSpace();
			pretendUseList = QStringList::split( " ", use );
		}
	}
	
	kdDebug() << "PackageInspector::slotParsePackageUse pretendUseList=" << pretendUseList << endl;
	
	dialog->useView->clear();
	if ( pretendUseList.isEmpty() ) {
		new QListViewItem( dialog->useView, i18n("No use flags available.") );
		return;
	}

	// Get user set package use flags
	foreach ( pretendUseList ) {
		QString lines;
		
		QString useFlag = *it;
		useFlag = useFlag.remove( QRegExp("^\\+|^-|^\\(-|^\\(\\+|\\*$|\\*\\)$|\\)$") );
		QMap<QString, QString>::iterator itMap = useMap.find( useFlag );
		if ( itMap != useMap.end() )
			lines = itMap.data();
		
		// Split long description into multiple lines
		QStringList description;
		if ( lines.length() <= 90 )
			description = lines;
		else {
			while ( lines.length() > 90 ) {
				int pos = ( lines.left(90) ).findRev(' ');
				QString line = lines.left( pos + 1 );
				lines = lines.right( lines.length() - line.length() );
				description += line;
			}
			description += lines;
		}
		
		// Set CheckBox state
		if ( (*it).startsWith( "+" ) || ( KurooConfig::portageVersion21() && !(*it).startsWith( "-" ) ) ) {
			QCheckListItem* useItem = new QCheckListItem( dialog->useView, *it, QCheckListItem::CheckBox );
			useItem->setMultiLinesEnabled( true );
			useItem->setText( 1, description.join("\n") );
			useItem->setOn( true );
		}
		else
			if ( (*it).startsWith( "-" ) ) {
				QCheckListItem* useItem = new QCheckListItem( dialog->useView, *it, QCheckListItem::CheckBox );
				useItem->setMultiLinesEnabled( true );
				useItem->setText( 1, description.join("\n") );
				useItem->setOn( false );
			}
			else {
				QListViewItem* useItem = new QListViewItem( dialog->useView, *it );
				useItem->setMultiLinesEnabled( true );
				useItem->setText( 1, description.join("\n") );
			}
	}
	
	if ( KUser().isSuperUser() || EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy())
		dialog->useView->setDisabled( false );
}

#include "packageinspector.moc"
