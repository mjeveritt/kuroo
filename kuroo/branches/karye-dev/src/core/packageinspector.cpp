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
#include "dependencyview.h"

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

/**
 * @class PackageInspector
 * @short The package Inspector dialog for viewing and editing all advanced package settings.
 * 
 * Builds a tabbed-dialog to view all relevant info for current package.
 * This dialog is used both in Packages view and Queue view.
 */
PackageInspector::PackageInspector( QWidget *parent )
: KDialogBase( KDialogBase::Swallow, 0, parent, i18n( "Package details" ), false, i18n( "Package details" ), 
               KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel, KDialogBase::Apply, false ), 
	m_category( QString::null ), m_package( QString::null ), m_portagePackage( 0 ),
	m_versionSettingsChanged( false ), m_useSettingsChanged( false ),
	m_isVirginState( true ), m_stabilityBefore( 0 ), m_versionBefore( QString::null ), m_hardMaskComment( QString::null )
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
	
	// Listen to Queue and World checkboxes
	connect( dialog->cbQueue, SIGNAL( clicked() ), this, SLOT( slotQueue() ) );
	connect( dialog->cbWorld, SIGNAL( clicked() ), this, SLOT( slotWorld() ) );
}

PackageInspector::~PackageInspector()
{}

/**
 * Return the caller.
 */
bool PackageInspector::isParentView( int view )
{
	return m_view == view;
}

/**
 * Update the Inspector gui with new versions and data.
 */
void PackageInspector::updateVersionData()
{
	// Clear dropdown menus
	dialog->versionsView->clear();
	dialog->cbVersionsEbuild->clear();
	dialog->cbVersionsDependencies->clear();
	dialog->cbVersionsInstalled->clear();
	dialog->cbVersionsUse->clear();
	dialog->cbVersionsSpecific->clear();

	// Iterate sorted version list
	QString installedVersion;
	QStringList versionList, versionInstalledList;
	foreach ( m_portagePackage->versionDataList() ) {
		
		// Parse out version and data
		QString version = *it++;
		QString stability = *it++;
		QString size = *it++;
		
		// Collect installed version in "Installed files" tab
		bool isInstalled;
		if ( *it == "1" ) {
			isInstalled = true;
			installedVersion = version;
			versionInstalledList.prepend( version );
		}
		else
			isInstalled = false;
		
		// Collect in inverse order to fill dropdown menus correctly
		versionList.prepend( version );
		
		// Insert version in versionview
		dialog->versionsView->insertItem( version, stability, size, isInstalled );
	}
	
	// Insert all versions in dropdown-menus
	dialog->cbVersionsEbuild->insertStringList( versionList );
	dialog->cbVersionsDependencies->insertStringList( versionList );
	dialog->cbVersionsUse->insertStringList( versionList );
	dialog->cbVersionsSpecific->insertStringList( versionList );
	
	// Set active version in Inspector dropdown menus
	if ( !versionInstalledList.isEmpty() ) {
		
		// Enable installed tab
		dialog->inspectorTabs->page(5)->setDisabled( false );
		
		dialog->cbVersionsInstalled->insertStringList( versionInstalledList );
		dialog->cbVersionsEbuild->setCurrentText( installedVersion );
		dialog->cbVersionsDependencies->setCurrentText( installedVersion );
		dialog->cbVersionsUse->setCurrentText( installedVersion );
		dialog->cbVersionsInstalled->setCurrentText( installedVersion );
	}
	else {
		
		// Disable installed tab
		dialog->inspectorTabs->page(5)->setDisabled( true );
		
		if ( !m_portagePackage->emergeVersion().isEmpty() ) {
			dialog->cbVersionsEbuild->setCurrentText( m_portagePackage->emergeVersion() );
			dialog->cbVersionsDependencies->setCurrentText( m_portagePackage->emergeVersion() );
			dialog->cbVersionsUse->setCurrentText( m_portagePackage->emergeVersion() );
		}
	}
	
	if ( !m_portagePackage->emergeVersion().isEmpty() ) {
		dialog->cbVersionsSpecific->setCurrentText( m_portagePackage->emergeVersion() );
		dialog->versionsView->usedForInstallation( m_portagePackage->emergeVersion() );
	}
	
	// Toggle checkboxes if package in World and Queue
	if ( m_portagePackage->isInWorld() )
		dialog->cbWorld->setChecked( true );
	else
		dialog->cbWorld->setChecked( false );
	
	if ( m_portagePackage->isQueued() )
		dialog->cbQueue->setChecked( true );
	else
		dialog->cbQueue->setChecked( false );
}

/**
 * Add/remove package in World profile using checkbox.
 */
void PackageInspector::slotWorld()
{
	if ( dialog->cbWorld->isChecked() )
		PortageSingleton::Instance()->appendWorld( QStringList( m_category + "/" + m_package ) );
	else
		PortageSingleton::Instance()->removeFromWorld( QStringList( m_category + "/" + m_package ) );
}

/**
 * Add/remove package in Queue using checkbox.
 */
void PackageInspector::slotQueue()
{
	if ( dialog->cbQueue->isChecked() )
		QueueSingleton::Instance()->addPackageIdList( QStringList( m_portagePackage->id() ) );
	else
		QueueSingleton::Instance()->removePackageIdList( QStringList( m_portagePackage->id() ) );
	
	// If user removes last package in Queue, disable the Inspector
	if ( m_view == VIEW_QUEUE && QueueSingleton::Instance()->size() == 1 ) {
		dialog->inspectorTabs->setDisabled( true );
		dialog->cbQueue->setDisabled( true );
		dialog->cbWorld->setDisabled( true );
	}
}

/**
 * Activate Inspector with current package.
 * @param portagePackage
 */
void PackageInspector::edit( PackageItem* portagePackage, int view )
{
	m_view = view;
	m_portagePackage = portagePackage;
	m_package = m_portagePackage->name();
	m_category = m_portagePackage->category();
	updateVersionData();
	
	// Actions that superuser privileges
	if ( !KUser().isSuperUser() ) {
		enableButtonApply( false );
		dialog->groupSelectStability->setDisabled( true );
		dialog->useView->setDisabled( true );
		dialog->cbWorld->setDisabled( true );
	}
	else {
		dialog->inspectorTabs->setDisabled( false );
		dialog->cbQueue->setDisabled( false );
		dialog->cbWorld->setDisabled( false );
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
		m_isVirginState = true;
	}
	else
		m_isVirginState = false;
	
	// Construct header text
	dialog->headerFrame->setPaletteBackgroundColor( colorGroup().highlight() );
	dialog->package->setText( "<b><font color=#" + GlobalSingleton::Instance()->fgHexColor() 
	                          + "><font size=+1>" + m_package + "</font>&nbsp;" +
	                          "(" + m_category.section( "-", 0, 0 ) + "/" + m_category.section( "-", 1, 1 ) + ")</b></font>" );
	dialog->description->setText( m_portagePackage->description() );
	
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
	if ( m_versionSettingsChanged || m_useSettingsChanged )
		switch( KMessageBox::warningYesNo( this,
			i18n( "<qt>Settings are changed!<br>"
					"Do you want to save them?</qt>"), i18n("Saving settings"), i18n("Yes"), i18n("No"), 0 ) ) {
						
			case KMessageBox::Yes:
				slotApply();
				break;
					
			case KMessageBox::No:
				rollbackSettings();
		}
	
	m_versionSettingsChanged = false;
	m_useSettingsChanged = false;
	emit signalNextPackage( true );
}

/**
 * Make next package in package view current - for easier browsing.
 * Ask to save settings if user has changed settings.
 * If no saving the changes rollback to the latest radiobutton setting.
 */
void PackageInspector::slotNextPackage()
{
	if ( m_versionSettingsChanged || m_useSettingsChanged )
		switch( KMessageBox::warningYesNo( this,
			i18n( "<qt>Settings are changed!<br>"
					"Do you want to save them?</qt>"), i18n("Saving settings"), i18n("Yes"), i18n("No"), 0 ) ) {
						
			case KMessageBox::Yes:
				slotApply();
				break;
				
			case KMessageBox::No:
				rollbackSettings();
		}
	
	m_versionSettingsChanged = false;
	m_useSettingsChanged = false;
	emit signalNextPackage( false );
}

/**
 * Save the stability setting for this package.
 * @fixme: save only changed tables.
 */
void PackageInspector::slotApply()
{
	DEBUG_LINE_INFO;
	
	if ( m_versionSettingsChanged ) {
			PortageFilesSingleton::Instance()->savePackageKeywords();
			PortageFilesSingleton::Instance()->savePackageUserMask();
			PortageFilesSingleton::Instance()->savePackageUserUnMask();
		
		// Check if this version is in updates. If not add it! (Only for packages in world).
		if ( PortageSingleton::Instance()->isInWorld( m_category + "/" + m_package ) )
			PortageSingleton::Instance()->checkUpdates( m_id, dialog->versionsView->updateVersion(), dialog->versionsView->hasUpdate() );
	}
	
	if ( m_useSettingsChanged ) {
		
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
			
			//set use flags to nothing to check if a string is necessary in package.use
			KurooDBSingleton::Instance()->setPackageUse( m_id, "" );
			PortageFilesSingleton::Instance()->savePackageUse();

			//recalculate use flags
			m_pretendUseLines.clear();
			QTextCodec *codec = QTextCodec::codecForName("utf8");
			KProcIO* eProc = new KProcIO( codec );
			*eProc << "emerge" << "--columns" << "--nospinner" << "--nocolor" << "-pv" << m_category + "/" + m_package;
			m_useList = useList;

			connect( eProc, SIGNAL( processExited( KProcess* ) ), this, SLOT( slotParseTempUse( KProcess* ) ) );
			connect( eProc, SIGNAL( readReady( KProcIO* ) ), this, SLOT( slotCollectPretendOutput( KProcIO* ) ) );
			eProc->start( KProcess::NotifyOnExit, true );
			SignalistSingleton::Instance()->setKurooBusy( true );
			
			if ( !eProc->isRunning() )
				LogSingleton::Instance()->writeLog( i18n("\nError: Could not calculate use flag for package %1/%2.")
				                                    .arg( m_category ).arg( m_package ), ERROR );
			
		}
	}
	
	enableButtonApply( false );
	m_versionSettingsChanged = false;
	m_useSettingsChanged = false;
	m_isVirginState = true;
}

/**
 * Cancel and rollback to old settings.
 */
void PackageInspector::slotCancel()
{
	DEBUG_LINE_INFO;
	
	rollbackSettings();
	m_versionSettingsChanged = false;
	accept();
}

void PackageInspector::slotOk()
{
	DEBUG_LINE_INFO;
	
	slotApply();
	accept();
}

/**
 * Rollback settings to state before changed by user.
 */
void PackageInspector::rollbackSettings()
{
	if ( m_versionSettingsChanged ) {
		slotSetStability( m_stabilityBefore  );
		
		if ( m_stabilityBefore == 3 )
			slotSetSpecificVersion( m_versionBefore );
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Package masking editing
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
		
		m_hardMaskComment =
			"<font size=\"+2\">" + m_package + "</font> " + 
			"(" + m_category.section( "-", 0, 0 ) + "/" + m_category.section( "-", 1, 1 ) + ")<br><br>" +
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
	KMessageBox::messageBox( this, KMessageBox::Information, m_hardMaskComment, 
	                         i18n("%1/%2 hardmask info!").arg( m_category ).arg( m_package ), i18n("Yes"), i18n("No"), 0 );
}

/**
 * Stability choice for versions - enable the right radiobutton.
 * Priority is: specific version >> unmask package >> untest package >> stable package.
 */
void PackageInspector::showSettings()
{
	// Get user mask specific version
	QString userMaskVersion = KurooDBSingleton::Instance()->packageUserMaskAtom( m_id ).first();
	
	// Enable stability radiobutton
	if ( !userMaskVersion.isEmpty() ) {
		dialog->rbVersionsSpecific->setChecked( true );
		dialog->cbVersionsSpecific->setDisabled( false );
		dialog->cbVersionsSpecific->setCurrentText( m_portagePackage->emergeVersion() );
	}
	else {
		if ( KurooDBSingleton::Instance()->isPackageUnMasked( m_id ) )
			dialog->rbMasked->setChecked( true );
		else
			if ( KurooDBSingleton::Instance()->isPackageUnTesting( m_id ) )
				dialog->rbTesting->setChecked( true );
			else
				dialog->rbStable->setChecked( true );
	}
	
	// Stability settings before user has changed it
	if ( m_isVirginState ) {
		m_stabilityBefore = dialog->groupSelectStability->selectedId();
		m_versionBefore = userMaskVersion;
	}
	
	showHardMaskInfo();

	// Reset the apply button for new package
	if ( !m_versionSettingsChanged )
		enableButtonApply( false );
}

/**
 * Apply stability settings from radiobuttons.
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
			emit signalPackageChanged();
			break;
		
		// User wants only specific version
		case 3 :
			dialog->cbVersionsSpecific->setDisabled( false );
		
	}

	enableButtonApply( true );
	m_versionSettingsChanged = true;
}

/**
 * User has selected a specific version to unmask and wants no higher version.
 * @param version
 */
void PackageInspector::slotSetSpecificVersion( const QString& version )
{
	enableButtonApply( true );
	m_versionSettingsChanged = true;
	
	KurooDBSingleton::Instance()->setPackageUnTesting( m_id );
	KurooDBSingleton::Instance()->setPackageUserMasked( m_id );
	KurooDBSingleton::Instance()->setPackageUnMasked( m_id, version );
	
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
	
	// Break if no checkbox
	if ( use.contains("(") )
		return;
	
	switch ( dynamic_cast<QCheckListItem*>(useItem)->state() ) {
	
		case ( QCheckListItem::Off ) :
			if ( useItem->text(0).startsWith( "+" ) )
				useItem->setText( 0, use.replace( QRegExp("^\\+"), "-" ) );
			if ( !useItem->text(0).startsWith( "-" ) )
				useItem->setText( 0, use.insert( 0, "-" ) );
			break;
		
		case ( QCheckListItem::On ) :
			if ( useItem->text(0).startsWith( "-" ) )
			useItem->setText( 0, use.replace( QRegExp("^\\-"), "+" ) );
			if ( !useItem->text(0).startsWith( "+" ) )
				useItem->setText( 0, use.insert( 0, "+" ) );
		
	}
	
	enableButtonApply( true );
	m_useSettingsChanged = true;
}


//////////////////////////////////////////////////////////////////////////////////////////////
// Viewing package files
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
}

/**
 * Load internal map with use flag description. @todo: /usr/portage/profiles/use.local.desc
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
				m_useMap.insert( use, useDescription );
			}
		}
		f.close();
	}
	else
		kdError(0) << "Loading use flag description. Reading: " << useFile << LINE_INFO;
}

/**
 * View use flags for selected version.
 * @param version
 */
void PackageInspector::slotLoadUseFlags( const QString& version )
{
// 	dialog->useView->setDisabled( true );
	dialog->useView->setColumnWidth( 0, 20 );
	
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
			QMap<QString, QString>::iterator itMap = m_useMap.find( *it );
			if ( itMap != m_useMap.end() )
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
	dialog->changelogBrowser->clear();
	if ( dialog->inspectorTabs->currentPageIndex() == 2 ) {
		QString fileName = KurooDBSingleton::Instance()->packagePath( m_id ) + "/" + m_category + "/" + m_package + "/ChangeLog";
		QFile file( fileName );
		
		if ( file.open( IO_ReadOnly ) ) {
			QTextStream stream( &file );
			QString textLines;
			QRegExp rx("#(\\d*)\\b");
			while ( !stream.atEnd() ) {
				QString line = stream.readLine();
				
				// Make bugs links to http://bugs.gentoo.org
				if ( rx.search( line ) > -1 )
					line.replace( rx.cap(0) , "<a href=\"http://bugs.gentoo.org/show_bug.cgi?id=" + rx.cap(1) + "\">" + rx.cap(0) + "</a>" );
				
				textLines += line + "<br>";
			}
			file.close();
			dialog->changelogBrowser->setText( textLines );
		}
		else {
			kdError(0) << "Loading changelog. Reading: " << fileName << LINE_INFO;
			dialog->changelogBrowser->setText( i18n("%1No ChangeLog found.%2").arg("<font color=darkRed><b>").arg("</b></font>") );
		}
	}
}

/**
 * Get ebuild for selected version.
 * @param version
 */
void PackageInspector::slotLoadEbuild( const QString& version )
{
	dialog->ebuildBrowser->clear();
	if ( dialog->inspectorTabs->currentPageIndex() == 3 ) {
		QString fileName = KurooDBSingleton::Instance()->packagePath( m_id ) + 
			"/" + m_category + "/" + m_package + "/" + m_package + "-" + version + ".ebuild";
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
			kdError(0) << "Loading ebuild. Reading: " << fileName << LINE_INFO;
			dialog->ebuildBrowser->setText( i18n("%1No ebuild found.%2").arg("<font color=darkRed><b>").arg("</b></font>") );
		}
	}
}

/**
 * Get dependencies for selected version.
 * @param version
 */
void PackageInspector::slotLoadDependencies( const QString& version )
{
	dialog->dependencyView->clear();
	if ( dialog->inspectorTabs->currentPageIndex() == 4 ) {
		QString fileName = KurooConfig::dirEdbDep() + KurooDBSingleton::Instance()->packagePath( m_id ) + 
			"/" + m_category + "/" + m_package + "-" + version;
		QFile file( fileName );
		
		if ( file.open( IO_ReadOnly ) ) {
			QTextStream stream( &file );
			QString textLines;
			int lineCount( 0 );
			
			if ( KurooConfig::portageVersion21() )
				while ( !stream.atEnd() ) {
					QString line = stream.readLine();
					if ( line.contains( "DEPEND=" ) && !line.endsWith( "DEPEND=" ) )
						textLines += line + " ";
				}
			else
				while ( !stream.atEnd() ) {
					QString line = stream.readLine();
					if ( lineCount++ > 13 )
						break;
					else {
						if ( !line.isEmpty() ) {
							switch (lineCount) {
							case 1:  textLines += "DEPEND= " + line + " ";
								break;
							case 2:  textLines += "RDEPEND= " + line + " ";
								break;
							case 13: textLines += "PDEPEND= " + line + " ";
								break;
							}
						}
					}
				}
			file.close();

			// Make sure all words are space-separated
			textLines.replace( "DEPEND=", "DEPEND= " );
			textLines.simplifyWhiteSpace();
			
			// Remove bootstrap and all it's duplicate
			textLines.remove( "!bootstrap? ( sys-devel/patch )" );
		
			dialog->dependencyView->insertDependAtoms( QStringList::split( " ", textLines ) );
		}
		else {
			kdError(0) << "Loading dependencies. Reading: " << fileName << LINE_INFO;
// 			dialog->dependencyBrowser->setText( i18n("%1No dependencies found.%2")
// 			                                    .arg("<font color=darkRed><b>").arg("</b></font>") );
		}
	}
}

/**
 * Get list of installed files for selected version.
 * @param version
 */
void PackageInspector::slotLoadInstalledFiles( const QString& version )
{
	dialog->installedFilesBrowser->clear();
	if ( !version.isEmpty() && dialog->inspectorTabs->currentPageIndex() == 5 ) {
		QString filename = KurooConfig::dirDbPkg() + "/" + m_category + "/" + m_package + "-" + version + "/CONTENTS";
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
		else {
			kdError(0) << "Loading installed files list. Reading: " << filename << LINE_INFO;
			dialog->installedFilesBrowser->setText( i18n("%1No installed files found.%2").arg("<font color=darkRed><b>").arg("</b></font>") );
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Use calculation
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Run emerge pretend to get use active use flags.
 */
void PackageInspector::slotCalculateUse()
{
	m_pretendUseLines.clear();
	QTextCodec *codec = QTextCodec::codecForName("utf8");
	KProcIO* eProc = new KProcIO( codec );
	*eProc << "emerge" << "--columns" << "--nospinner" << "--nocolor" << "-pv" << m_category + "/" + m_package;
	
	connect( eProc, SIGNAL( processExited( KProcess* ) ), this, SLOT( slotParsePackageUse( KProcess* ) ) );
	connect( eProc, SIGNAL( readReady( KProcIO* ) ), this, SLOT( slotCollectPretendOutput( KProcIO* ) ) );
	eProc->start( KProcess::NotifyOnExit, true );
	SignalistSingleton::Instance()->setKurooBusy( true );
	
	if ( !eProc->isRunning() ) {
		LogSingleton::Instance()->writeLog( i18n("\nError: Could not calculate use flag for package %1/%2.")
		                                    .arg( m_category ).arg( m_package ), ERROR );
		slotParsePackageUse( eProc );
	}
	else
		dialog->setDisabled( true );
}

/**
 * Collect emerge pretend output in m_pretendUseLines.
 * @param eProc
 */
void PackageInspector::slotCollectPretendOutput( KProcIO* eProc )
{
	QString line;
	while ( eProc->readln( line, true ) >= 0 )
		m_pretendUseLines += line;
}

/**
 * Parse emerge pretend output for all use flags, and return a List.
 * @param eProc
 */
void PackageInspector::slotParseTempUse( KProcess* eProc )
{
	SignalistSingleton::Instance()->setKurooBusy( false );
	delete eProc;
	eProc = 0;
	
	QRegExp rxPretend = GlobalSingleton::Instance()->rxEmerge();
	
// 	kdDebug() << "m_pretendUseLines=" << m_pretendUseLines << LINE_INFO;
	
	QStringList tmpUseList;
	foreach ( m_pretendUseLines ) {
		if ( (*it).contains( m_category + "/" + m_package ) && rxPretend.search( *it ) > -1 ) {
			QString use = rxPretend.cap(7).simplifyWhiteSpace();
			tmpUseList = QStringList::split( " ", use );
		}
	}
// 	kdDebug() << "tmpUseList=" << tmpUseList << LINE_INFO;
	
	dialog->useView->clear();
	if ( tmpUseList.isEmpty() ) {
		new QListViewItem( dialog->useView, i18n("Use flags could not be calculated. Please check log for more information") );
		foreach ( m_pretendUseLines )
			LogSingleton::Instance()->writeLog( *it, ERROR );
		return;
	}
	
	//recalculated use, now needs to check if a line in package.use is needed
	//do it better: check if a word is needed in package.use
	QStringList useList = useList.split( QString(", "), m_useList.join(", ").remove( QRegExp("/b\\+|\\*|\\%") ) );
	foreach ( tmpUseList ){
		QString aux = (*it);
		
		//removes all * since it's not a characted admitted in use flags
		aux = aux.remove( QRegExp("/b\\+|\\*|\\%") );
		foreach ( m_useList ) {
			QString aux2 = (*it);
			aux2 = aux2.remove( QRegExp("/b\\+|\\*|\\%") );
			if (aux == aux2 )
				useList = useList.grep( QRegExp (QString("^(?!").append( aux ).append(")") ) );
		}
	}
	//end of better
// 	kdDebug() << "useList=" << useList << LINE_INFO;
	
	QString checkUse = useList.join(", ");
	if ( !checkUse.remove(", ").remove(" ").isEmpty() ) {
		KurooDBSingleton::Instance()->setPackageUse( m_id, useList.join(" ") );
		PortageFilesSingleton::Instance()->savePackageUse();
	}
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
	
	QRegExp rxPretend = GlobalSingleton::Instance()->rxEmerge();
	
// 	kdDebug() << "m_pretendUseLines=" << m_pretendUseLines << LINE_INFO;
	
	QStringList pretendUseList;
	foreach ( m_pretendUseLines ) {
		if ( (*it).contains( m_category + "/" + m_package ) && rxPretend.search( *it ) > -1 ) {
			QString use = rxPretend.cap(7).simplifyWhiteSpace();
			pretendUseList = QStringList::split( " ", use );
		}
	}
	
// 	kdDebug() << "pretendUseList=" << pretendUseList << LINE_INFO;
	
	dialog->useView->clear();
	if ( pretendUseList.isEmpty() ) {
		new QListViewItem( dialog->useView, i18n("Use flags could not be calculated. Please check log for more information") );
		foreach ( m_pretendUseLines )
			LogSingleton::Instance()->writeLog( *it, ERROR );
		return;
	}
	else
		dialog->useView->setColumnWidth( 0, 20 );
	
	// Get user set package use flags
	foreach ( pretendUseList ) {
		QString lines;
		
		QString useFlag = *it;
		useFlag = useFlag.remove( QRegExp("^\\+|^-|^\\(-|^\\(\\+|\\*$|\\*\\)$|\\)$") );
		QMap<QString, QString>::iterator itMap = m_useMap.find( useFlag );
		if ( itMap != m_useMap.end() )
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
