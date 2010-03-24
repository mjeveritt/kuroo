/***************************************************************************
*	Copyright (C) 2004 by karye												*
*	karye@users.sourceforge.net												*
*																			*
*	This program is free software; you can redistribute it and/or modify	*
*	it under the terms of the GNU General Public License as published by	*
*	the Free Software Foundation; either version 2 of the License, or		*
*	(at your option) any later version.										*
*																			*
*	This program is distributed in the hope that it will be useful,			*
*	but WITHOUT ANY WARRANTY; without even the implied warranty of			*
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the			*
*	GNU General Public License for more details.							*
*																			*
*	You should have received a copy of the GNU General Public License		*
*	along with this program; if not, write to the							*
*	Free Software Foundation, Inc.,											*
*	59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.				*
***************************************************************************/

#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qtextcodec.h>
#include <QTreeWidget>

#include <ktabwidget.h>
#include <kactionselector.h>
#include <ktextbrowser.h>
#include <kmessagebox.h>
#include <kuser.h>
#include <kurllabel.h>

#include "common.h"
#include "packageinspector.h"
#include "portagelistview.h"
#include "packageversion.h"
#include "versionview.h"
#include "dependencyview.h"

/**
 * @class PackageInspector
 * @short The package Inspector dialog for viewing and editing all advanced package settings.
 *
 * Builds a tabbed-dialog to view all relevant info for current package.
 * This dialog is used both in Packages view and Queue view.
 */
PackageInspector::PackageInspector( QWidget *parent ) : KDialog( parent ),
		m_versionSettingsChanged( false ), m_useSettingsChanged( false ),
		m_isVirginState( true ), m_category( QString::null ),
		m_package( QString::null ), m_hardMaskComment( QString::null ),
		m_portagePackage( 0 ), m_stabilityBefore( 0 ), m_versionBefore( QString::null )
{
    /*KDialog::Swallow, 0,, i18n( "Package details" ), false, i18n( "Package details" ),
               KDialog::Ok | KDialog::Apply | KDialog::Cancel, KDialog::Apply, false ),*/
    setupUi( mainWidget() );
    adjustSize();

    // Get use flag description @fixme: load local description
    loadUseFlagDescription();

    // Shortcuts for browsing packages
    connect( pbPrevious, SIGNAL( clicked() ), this, SLOT( slotPreviousPackage() ) );
    connect( pbNext, SIGNAL( clicked() ), this, SLOT( slotNextPackage() ) );

    // Refresh files when changing version
    connect( cbVersionsEbuild, SIGNAL( activated( const QString& ) ), this, SLOT( slotLoadEbuild( const QString& ) ) );
    connect( cbVersionsDependencies, SIGNAL( activated ( const QString& ) ), this, SLOT( slotLoadDependencies( const QString& ) ) );
    connect( cbVersionsInstalled, SIGNAL( activated ( const QString& ) ), this, SLOT( slotLoadInstalledFiles( const QString& ) ) );
    connect( cbVersionsUse, SIGNAL( activated ( const QString& ) ), this, SLOT( slotLoadUseFlags( const QString& ) ) );

    // Load files only if tabpage is open
    connect( inspectorTabs, SIGNAL( currentChanged( QWidget* ) ), this, SLOT( slotRefreshTabs() ) );

    // Toggle between all 4 stability version
    connect( groupSelectStability, SIGNAL( released( int ) ), this, SLOT( slotSetStability( int ) ) );

    // Activate specific version menu
    connect( cbVersionsSpecific, SIGNAL( activated( const QString& ) ), this, SLOT( slotSetSpecificVersion( const QString& ) ) );

    connect( infoHardMasked, SIGNAL( leftClickedUrl( const QString& ) ), SLOT( slotHardMaskInfo() ) );

    connect( useView, SIGNAL( itemClicked(QTreeWidgetItem*,int) ), this, SLOT( slotSetUseFlags( QTreeWidgetItem*,int ) ) );

    connect( pbUse, SIGNAL( clicked() ), this, SLOT( slotCalculateUse() ) );

    // Listen to Queue and World checkboxes
    connect( cbQueue, SIGNAL( clicked() ), this, SLOT( slotQueue() ) );
    connect( cbWorld, SIGNAL( clicked() ), this, SLOT( slotWorld() ) );
}

PackageInspector::~PackageInspector()
{}


/**
 * Update the Inspector gui with new versions and data.
 */
void PackageInspector::updateVersionData()
{
    // Clear dropdown menus
    versionsView->clear();
    cbVersionsEbuild->clear();
    cbVersionsDependencies->clear();
    cbVersionsInstalled->clear();
    cbVersionsUse->clear();
    cbVersionsSpecific->clear();

    // Iterate sorted version list
    QString installedVersion;
    QStringList versionList, versionInstalledList;
    QStringListIterator it( m_portagePackage->versionDataList() );
    while( it.hasNext() ) {

        // Parse out version and data
        QString version = it.next();
        QString stability = it.next();
        QString size = it.next();

        // Collect installed version in "Installed files" tab
        bool isInstalled;
        if ( it.peekNext() == "1" ) {
            isInstalled = true;
            installedVersion = version;
            versionInstalledList.prepend( version );
        }
        else
            isInstalled = false;

        // Collect in inverse order to fill dropdown menus correctly
        versionList.prepend( version );

        // Insert version in versionview
        versionsView->insertItem( version, stability, size, isInstalled );
    }

    // Insert all versions in dropdown-menus
    cbVersionsEbuild->addItems( versionList );
    cbVersionsDependencies->addItems( versionList );
    cbVersionsUse->addItems( versionList );
    cbVersionsSpecific->addItems( versionList );

    // Set active version in Inspector dropdown menus
    if ( !versionInstalledList.isEmpty() ) {
        // Enable installed tab
        inspectorTabs->setTabEnabled( 5, true );
        cbVersionsInstalled->addItems( versionInstalledList );
        int installed = cbVersionsInstalled->findText(installedVersion);
        cbVersionsInstalled->setCurrentIndex( installed );

        int current = cbVersionsEbuild->findText(installedVersion);
        cbVersionsEbuild->setCurrentIndex( current );
        cbVersionsDependencies->setCurrentIndex( current );
        cbVersionsUse->setCurrentIndex( current );
    } else {
        // Disable installed tab
        inspectorTabs->setTabEnabled( 5, false );

        if( !m_portagePackage->emergeVersion().isEmpty() ) {
            int current =  cbVersionsEbuild->findText( m_portagePackage->emergeVersion() );
            cbVersionsEbuild->setCurrentIndex( current );
            cbVersionsDependencies->setCurrentIndex( current );
            cbVersionsUse->setCurrentIndex( current );
        }
    }

    if ( !m_portagePackage->emergeVersion().isEmpty() ) {
        int current = cbVersionsSpecific->findText( m_portagePackage->emergeVersion() );
        cbVersionsSpecific->setCurrentIndex( current );
        versionsView->usedForInstallation( m_portagePackage->emergeVersion() );
    }

    // Toggle checkboxes if package in World and Queue
    if ( m_portagePackage->isInWorld() )
        cbWorld->setChecked( true );
    else
        cbWorld->setChecked( false );

    if ( m_portagePackage->isQueued() )
        cbQueue->setChecked( true );
    else
        cbQueue->setChecked( false );
}

/**
 * Add/remove package in World profile using checkbox.
 */
void PackageInspector::slotWorld()
{
    if ( cbWorld->isChecked() )
        PortageSingleton::Instance()->appendWorld( QStringList( m_category + "/" + m_package ) );
    else
        PortageSingleton::Instance()->removeFromWorld( QStringList( m_category + "/" + m_package ) );
}

/**
 * Add/remove package in Queue using checkbox.
 */
void PackageInspector::slotQueue()
{
    if ( cbQueue->isChecked() )
        QueueSingleton::Instance()->addPackageIdList( QStringList( m_portagePackage->id() ) );
    else
        QueueSingleton::Instance()->removePackageIdList( QStringList( m_portagePackage->id() ) );

    // If user removes last package in Queue, disable the Inspector
    if ( m_view == VIEW_QUEUE && QueueSingleton::Instance()->size() == 1 ) {
        inspectorTabs->setDisabled( true );
        cbQueue->setDisabled( true );
        cbWorld->setDisabled( true );
    }
}

/**
 * Activate Inspector with current package.
 * @param portagePackage
 */
void PackageInspector::edit( PackageItem* portagePackage, const int& view )
{
    m_view = view;
    m_portagePackage = portagePackage;
    m_package = m_portagePackage->name();
    m_category = m_portagePackage->category();
    updateVersionData();

    // Actions that superuser privileges
    if ( !KUser().isSuperUser() ) {
        enableButtonApply( false );
        groupSelectStability->setDisabled( true );
        useView->setDisabled( true );
        cbWorld->setDisabled( true );
    }
    else {
        inspectorTabs->setDisabled( false );
        cbQueue->setDisabled( false );
        cbWorld->setDisabled( false );
    }

    // Disabled editing when package is in Queue and kuroo is emerging
    if ( m_portagePackage->isQueued() && EmergeSingleton::Instance()->isRunning() ) {
        inspectorTabs->setTabEnabled( 0, false );
        inspectorTabs->setTabEnabled( 1, false );
    } else {
        inspectorTabs->setTabEnabled( 0, true );
        inspectorTabs->setTabEnabled( 1, true );
    }

    // Is it first time we load this package
    if ( m_id != m_portagePackage->id() ) {
        m_id = m_portagePackage->id();
        m_isVirginState = true;
    }
    else
        m_isVirginState = false;

    // Construct header text
    //headerFrame->setPaletteBackgroundColor( colorGroup().highlight() );
    package->setText( "<b>" //<font color=#" + GlobalSingleton::Instance()->fgHexColor()+ ">"
                      "<font size=+1>" + m_package + "</font>&nbsp;" +
                      "(" + m_category.section( "-", 0, 0 ) + "/" + m_category.section( "-", 1, 1 ) + ")</b>"/*"</font>"*/);
    description->setText( m_portagePackage->description() );

    showSettings();
    slotRefreshTabs();

    // Enable/disable shortcuts buttons if first or last package
    if ( m_portagePackage->isFirstPackage() )
        pbPrevious->setDisabled( true );
    else
        pbPrevious->setDisabled( false );

    if ( m_portagePackage->isLastPackage() )
        pbNext->setDisabled( true );
    else
        pbNext->setDisabled( false );

    show();
}

/**
 * Ask to save settings if user has changed settings.
 * If no saving the changes rollback to the latest radiobutton setting.
 */
void PackageInspector::askApplySettings()
{
    if ( m_versionSettingsChanged || m_useSettingsChanged ) {
        switch( KMessageBox::warningYesNo( this, i18n( "<qt>Settings are changed!<br>Do you want to save them?</qt>"),
                                           i18n("Saving settings")) ) {
        case KMessageBox::Yes:
            slotApply();
            break;

        case KMessageBox::No:
            rollbackSettings();
        }
    }
    m_versionSettingsChanged = false;
    m_useSettingsChanged = false;
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
    askApplySettings();
    emit signalNextPackage( true );
}

/**
 * Make next package in package view current - for easier browsing.
 * Ask to save settings if user has changed settings.
 * If no saving the changes rollback to the latest radiobutton setting.
 */
void PackageInspector::slotNextPackage()
{
    askApplySettings();
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
            PortageSingleton::Instance()->checkUpdates( m_id, versionsView->updateVersion(), versionsView->hasUpdate() );
    }

    if ( m_useSettingsChanged ) {

        // Get use flags
        QStringList useList, pretendUseList;
        QTreeWidgetItemIterator it( useView );
        while ( *it ) {
            QString useFlag = (*it)->text(0);
            pretendUseList += useFlag;
            if ( !useFlag.contains( "^\\(|\\)$" ) )
                useList += useFlag.remove( QRegExp( "^\\+|\\*$" ) );
            ++it;
        }

        // Store in db and save to file
        if ( !useList.isEmpty() ) {

            //set use flags to nothing to check if a string is necessary in package.use
            KurooDBSingleton::Instance()->setPackageUse( m_id, "" );
            PortageFilesSingleton::Instance()->savePackageUse();

            //recalculate use flags
            m_pretendUseLines.clear();
            eProc = new KProcess();
            *eProc << "emerge" << "--columns" << "--nospinner" << "--color=n" << "-pv" << m_category + "/" + m_package;
            m_useList = useList;

            connect( eProc, SIGNAL( finished() ), this, SLOT( slotParseTempUse() ) );
            connect( eProc, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotCollectPretendOutput() ) );
            eProc->start( /*K3Process::NotifyOnExit, true*/ );
            SignalistSingleton::Instance()->setKurooBusy( true );

            if ( !eProc->state() != QProcess::Running )
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
        infoHardMasked->setFont( font );
        infoHardMasked->setHighlightedColor( Qt::red );
        infoHardMasked->setText( i18n("Click for hardmask info!") );

        m_hardMaskComment =
                "<font size=\"+2\">" + m_package + "</font> " +
                "(" + m_category.section( "-", 0, 0 ) + "/" + m_category.section( "-", 1, 1 ) + ")<br><br>" +
                hardMaskInfo.last() + "<br><br>" +
                "Hardmask rule: <i>\"" + hardMaskInfo.first() + "\"</i>";
    }
    else
        infoHardMasked->setText( QString::null );
}

/**
 * Show gentoo devs reason for hardmasking this package/versions.
 */
void PackageInspector::slotHardMaskInfo()
{
    KMessageBox::messageBox( this, KMessageBox::Information, m_hardMaskComment,
                             i18n("%1/%2 hardmask info!").arg( m_category ).arg( m_package ) );
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
        rbVersionsSpecific->setChecked( true );
        cbVersionsSpecific->setDisabled( false );
        int current = cbVersionsSpecific->findText( m_portagePackage->emergeVersion() );
        cbVersionsSpecific->setCurrentIndex( current );
    } else {
        if ( KurooDBSingleton::Instance()->isPackageUnMasked( m_id ) )
            rbMasked->setChecked( true );
        else
            if ( KurooDBSingleton::Instance()->isPackageUnTesting( m_id ) )
                rbTesting->setChecked( true );
        else
            rbStable->setChecked( true );
    }

    // Stability settings before user has changed it
    if ( m_isVirginState ) {
        m_stabilityBefore = groupSelectStability->selected();
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
        cbVersionsSpecific->setDisabled( true );

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
        cbVersionsSpecific->setDisabled( true );

        // Clear package from package.unmask and package.mask
        KurooDBSingleton::Instance()->clearPackageUnMasked( m_id );
        KurooDBSingleton::Instance()->clearPackageUserMasked( m_id );

        KurooDBSingleton::Instance()->setPackageUnTesting( m_id );
        m_portagePackage->resetDetailedInfo();
        emit signalPackageChanged();
        break;

        // User wants only hardmasked package
    case 2 :
        cbVersionsSpecific->setDisabled( true );

        // Clear package from package.keywords and package.mask
        KurooDBSingleton::Instance()->clearPackageUserMasked( m_id );

        KurooDBSingleton::Instance()->setPackageUnTesting( m_id );
        KurooDBSingleton::Instance()->setPackageUnMasked( m_id );
        m_portagePackage->resetDetailedInfo();
        emit signalPackageChanged();
        break;

        // User wants only specific version
    case 3 :
        cbVersionsSpecific->setDisabled( false );

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
void PackageInspector::slotSetUseFlags( QTreeWidgetItem* useItem, int column )
{
    if ( !useItem ) //is it possible ?
        return;

    QString use = useItem->text( 0 );

    // Break if no checkbox
    if ( use.contains("(") )
        return;

    switch ( useItem->checkState(0) ) {
    case ( Qt::Unchecked ) :
        if ( useItem->text(0).startsWith( "+" ) )
            useItem->setText( 0, use.replace( QRegExp("^\\+"), "-" ) );
        if ( !useItem->text(0).startsWith( "-" ) )
            useItem->setText( 0, use.insert( 0, "-" ) );
        break;
    case ( Qt::Checked ) :
        if ( useItem->text(0).startsWith( "-" ) )
            useItem->setText( 0, use.replace( QRegExp("^\\-"), "+" ) );
        if ( !useItem->text(0).startsWith( "+" ) )
            useItem->setText( 0, use.insert( 0, "+" ) );
        break;
    case ( Qt::PartiallyChecked ) :
        break;
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
    slotLoadUseFlags( cbVersionsUse->currentText() );
    slotLoadEbuild( cbVersionsEbuild->currentText() );
    slotLoadDependencies( cbVersionsDependencies->currentText() );
    slotLoadInstalledFiles( cbVersionsInstalled->currentText() );
    loadChangeLog();
}

/**
 * Load internal map with use flag description. @todo: /usr/portage/profiles/use.local.desc
 */
void PackageInspector::loadUseFlagDescription()
{
    QString useFile( KurooConfig::dirPortage() + "/profiles/use.desc" );
    QFile f( useFile );

    if ( f.open( QIODevice::ReadOnly ) ) {
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
        kError(0) << "Loading use flag description. Reading: " << useFile << LINE_INFO;
}

/**
 * View use flags for selected version.
 * @param version
 */
void PackageInspector::slotLoadUseFlags( const QString& version )
{
    // 	useView->setDisabled( true );
    useView->setColumnWidth( 0, 20 );

    if ( inspectorTabs->currentIndex() == 1 ) {
        QStringList useList;

        QMap<QString, PackageVersion*>::iterator itMap = m_portagePackage->versionMap().find( version );
        if ( itMap != m_portagePackage->versionMap().end() )
            useList = itMap.value()->useflags();

        // Get user set package use flags
        QStringList packageUseList = KurooDBSingleton::Instance()->packageUse( m_id ).split(" ");

        useView->clear();
        QStringList tmpUseList;
        foreach( QString use, useList ) {

            // Seems some use flags are duplicated, filter them out
            if( tmpUseList.contains( use ) ) {
                continue;
            } else {
                tmpUseList += use;
            }

            QString lines;
            QMap<QString, QString>::iterator itMap = m_useMap.find( use );
            if ( itMap != m_useMap.end() )
                lines = itMap.value();

            // Split long description into multiple lines
            QStringList description;
            if ( lines.length() <= 90 ) {
                description += lines;
            } else {
                while ( lines.length() > 90 ) {
                    int pos = ( lines.left(90) ).lastIndexOf(' ');
                    QString line = lines.left( pos + 1 );
                    lines = lines.right( lines.length() - line.length() );
                    description += line;
                }
                description += lines;
            }

            // Add use flag in use view
            QTreeWidgetItem* useItem = new QTreeWidgetItem( useView );
            useItem->setFlags( Qt::ItemIsUserCheckable ); //Type::CheckBox
            //useItem->setMultiLinesEnabled( true );
            useItem->setText( 0, use );
            useItem->setText( 1, description.join("\n") );
        }
    }
}

/**
 * Get this package changelog.
 */
void PackageInspector::loadChangeLog()
{
    changelogBrowser->clear();
    if ( inspectorTabs->currentIndex() == 2 ) {
        QString fileName = KurooDBSingleton::Instance()->packagePath( m_id ) + "/" + m_category + "/" + m_package + "/ChangeLog";
        QFile file( fileName );

        if ( file.open( QIODevice::ReadOnly ) ) {
            QTextStream stream( &file );
            QString textLines;
            QRegExp rx("#(\\d*)\\b");
            while ( !stream.atEnd() ) {
                QString line = stream.readLine();

                // Make bugs links to http://bugs.gentoo.org
                if ( rx.indexIn( line ) > -1 )
                    line.replace( rx.cap(0) , "<a href=\"http://bugs.gentoo.org/show_bug.cgi?id=" + rx.cap(1) + "\">" + rx.cap(0) + "</a>" );

                textLines += line + "<br>";
            }
            file.close();
            changelogBrowser->setText( textLines );
        }
        else {
            kError(0) << "Loading changelog. Reading: " << fileName << LINE_INFO;
            changelogBrowser->setText( i18n("%1No ChangeLog found.%2").arg("<font color=darkRed><b>").arg("</b></font>") );
        }
    }
}

/**
 * Get ebuild for selected version.
 * @param version
 */
void PackageInspector::slotLoadEbuild( const QString& version )
{
    ebuildBrowser->clear();
    if ( inspectorTabs->currentIndex() == 3 ) {
        QString fileName = KurooDBSingleton::Instance()->packagePath( m_id ) +
                           "/" + m_category + "/" + m_package + "/" + m_package + "-" + version + ".ebuild";
        QFile file( fileName );

        if ( file.open( QIODevice::ReadOnly ) ) {
            QTextStream stream( &file );
            QString textLines;
            while ( !stream.atEnd() )
                textLines += stream.readLine() + "<br>";
            file.close();
            ebuildBrowser->setText( textLines );
        }
        else {
            kError(0) << "Loading ebuild. Reading: " << fileName << LINE_INFO;
            ebuildBrowser->setText( i18n("%1No ebuild found.%2").arg("<font color=darkRed><b>").arg("</b></font>") );
        }
    }
}

/**
 * Get dependencies for selected version.
 * @param version
 */
void PackageInspector::slotLoadDependencies( const QString& version )
{
    //WARN: This won't work for anything but /usr/portage for now!
    dependencyView->clear();
    if ( inspectorTabs->currentIndex() == 4 ) {
        //package path ought to be /usr/portage
        QString fileName = KurooDBSingleton::Instance()->packagePath( m_id ) +
                           "/metadata/cache/" + m_category + "/" + m_package + "-" + version;
        QFile file( fileName );

        if ( file.open( QIODevice::ReadOnly ) ) {
            QTextStream stream( &file );
            QString textLines;
            int lineCount( 0 );

            // 			if ( KurooConfig::portageVersion21() )
            // 				while ( !stream.atEnd() ) {
            // 					QString line = stream.readLine();
            // 					if ( line.contains( "DEPEND=" ) && !line.endsWith( "DEPEND=" ) )
            // 						textLines += line + " ";
            // 				}
            // 			else
            //WARN: Portage seems to have down-graded to the older style flat cache file for the cache in the repository (/usr/portage).  Other caches (layman) may behave differently
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
            textLines.simplified();

            // Remove bootstrap and all it's duplicate
            textLines.remove( "!bootstrap? ( sys-devel/patch )" );

            dependencyView->insertDependAtoms( textLines.split(" ") );
        }
        else {
            kError(0) << "Loading dependencies. Reading: " << fileName << LINE_INFO;
            // 			dependencyBrowser->setText( i18n("%1No dependencies found.%2")
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
    installedFilesBrowser->clear();
    if ( !version.isEmpty() && inspectorTabs->currentIndex() == 5 ) {
        QString filename = KurooConfig::dirDbPkg() + "/" + m_category + "/" + m_package + "-" + version + "/CONTENTS";
        QFile file( filename );
        QString textLines;
        if ( file.open( QIODevice::ReadOnly ) ) {
            QTextStream stream( &file );
            while ( !stream.atEnd() ) {
                QString line = stream.readLine();
                if ( line.startsWith( "obj" ) )
                    textLines += line.section( "obj ", 1, 1 ).section( " ", 0, 0 ) + "\n";
            }
            file.close();
            installedFilesBrowser->setText( textLines );
        }
        else {
            kError(0) << "Loading installed files list. Reading: " << filename << LINE_INFO;
            installedFilesBrowser->setText( i18n("%1No installed files found.%2").arg("<font color=darkRed><b>").arg("</b></font>") );
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
    eProc = new KProcess();
    *eProc << "emerge" << "--columns" << "--nospinner" << "--color=n" << "-pv" << m_category + "/" + m_package;

    connect( eProc, SIGNAL( finished() ), this, SLOT( slotParsePackageUse() ) );
    connect( eProc, SIGNAL( readyReadStandardError() ), this, SLOT( slotCollectPretendOutput() ) );
    eProc->start( /*K3Process::NotifyOnExit, true*/ );
    SignalistSingleton::Instance()->setKurooBusy( true );

    if ( eProc->state() != QProcess::Running ) {
        LogSingleton::Instance()->writeLog( i18n("\nError: Could not calculate use flag for package %1/%2.")
                                            .arg( m_category ).arg( m_package ), ERROR );
        slotParsePackageUse();
    }
    else
        setDisabled( true );
}

/**
 * Collect emerge pretend output in m_pretendUseLines.
 * @param eProc
 */
void PackageInspector::slotCollectPretendOutput()
{
    QString line;
    while( !(line = eProc->readLine() ).isEmpty() )
        m_pretendUseLines += line;
}

/**
 * Parse emerge pretend output for all use flags, and return a List.
 * @param eProc
 */
void PackageInspector::slotParseTempUse()
{
    SignalistSingleton::Instance()->setKurooBusy( false );
    delete eProc;
    eProc = 0;

    QRegExp rxPretend = rxEmerge();

    // 	kDebug() << "m_pretendUseLines=" << m_pretendUseLines << LINE_INFO;

    QStringList tmpUseList;
    foreach ( QString pretend, m_pretendUseLines ) {
        if ( pretend.contains( m_category + "/" + m_package ) && rxPretend.indexIn( pretend ) > -1 ) {
            QString use = rxPretend.cap(7).simplified();
            tmpUseList = use.split(" ");
        }
    }
    // 	kDebug() << "tmpUseList=" << tmpUseList << LINE_INFO;

    useView->clear();
    if ( tmpUseList.isEmpty() ) {
        QTreeWidgetItem* item = new QTreeWidgetItem( useView );
        item->setText( 0, i18n("Use flags could not be calculated. Please check log for more information") );
        foreach ( QString pretend, m_pretendUseLines )
            LogSingleton::Instance()->writeLog( pretend, ERROR );
        return;
    }

    //recalculated use, now needs to check if a line in package.use is needed
    //do it better: check if a word is needed in package.use
    QStringList useList = m_useList.join(", ").remove( QRegExp("/b\\+|\\*|\\%") ).split(", ");
    foreach ( QString aux, tmpUseList ){
        //removes all * since it's not a characted admitted in use flags
        aux = aux.remove( QRegExp("/b\\+|\\*|\\%") );
        foreach ( QString aux2, m_useList ) {
            aux2 = aux2.remove( QRegExp("/b\\+|\\*|\\%") );
            if (aux == aux2 )
                useList = QRegExp(QString("^(?!").append( aux ).append(")") ).capturedTexts();
        }
    }
    //end of better
    // 	kDebug() << "useList=" << useList << LINE_INFO;

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
void PackageInspector::slotParsePackageUse()
{
    setDisabled( false );

    SignalistSingleton::Instance()->setKurooBusy( false );
    delete eProc;
    eProc = 0;

    QRegExp rxPretend = rxEmerge();

    // 	kDebug() << "m_pretendUseLines=" << m_pretendUseLines << LINE_INFO;

    QStringList pretendUseList;
    foreach( QString pretend, m_pretendUseLines ) {
        if ( pretend.contains( m_category + "/" + m_package ) && rxPretend.indexIn( pretend ) > -1 ) {
            QString use = rxPretend.cap(7).simplified();
            pretendUseList = use.split(" ");
        }
    }

    // 	kDebug() << "pretendUseList=" << pretendUseList << LINE_INFO;

    useView->clear();
    if ( pretendUseList.isEmpty() ) {
        QTreeWidgetItem* useItem = new QTreeWidgetItem( useView );
        useItem->setText( 0, i18n("Use flags could not be calculated. Please check log for more information") );
        foreach ( QString pretend, m_pretendUseLines )
            LogSingleton::Instance()->writeLog( pretend, ERROR );
        return;
    }
    else
        useView->setColumnWidth( 0, 20 );

    // Get user set package use flags
    foreach ( QString pretend, pretendUseList ) {
        QString lines;
        QString useFlag = pretend.remove( QRegExp("^\\+|^-|^\\(-|^\\(\\+|\\*$|\\*\\)$|\\)$") );
        QMap<QString, QString>::iterator itMap = m_useMap.find( useFlag );
        if ( itMap != m_useMap.end() )
            lines = itMap.value();

        // Split long description into multiple lines
        QStringList description;
        if ( lines.length() <= 90 )
            description += lines;
        else {
            while ( lines.length() > 90 ) {
                int pos = ( lines.left(90) ).lastIndexOf(' ');
                QString line = lines.left( pos + 1 );
                lines = lines.right( lines.length() - line.length() );
                description += line;
            }
            description += lines;
        }

        // Set CheckBox state
        if ( pretend.startsWith( "+" ) || ( KurooConfig::portageVersion21() && !pretend.startsWith( "-" ) ) ) {
            QTreeWidgetItem* useItem = new QTreeWidgetItem( useView );
            useItem->setText( 0, pretend );
            useItem->setFlags( Qt::ItemIsUserCheckable );
            //useItem->setMultiLinesEnabled( true );
            useItem->setText( 1, description.join("\n") );
            useItem->setCheckState( 0, Qt::Checked );
        }
        else
            if ( pretend.startsWith( "-" ) ) {
            QTreeWidgetItem* useItem = new QTreeWidgetItem( useView );
            useItem->setText( 0, pretend );
            useItem->setFlags( Qt::ItemIsUserCheckable );
            //useItem->setMultiLinesEnabled( true );
            useItem->setText( 1, description.join("\n") );
            useItem->setCheckState( 0, Qt::Unchecked );
        }
        else {
            QTreeWidgetItem* useItem = new QTreeWidgetItem( useView );
            useItem->setText( 0, pretend );
            //useItem->setMultiLinesEnabled( true );
            useItem->setText( 1, description.join("\n") );
        }
    }

    if ( KUser().isSuperUser() || EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy())
        useView->setDisabled( false );
}

#include "packageinspector.moc"
