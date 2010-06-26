/********************************************************************************
 *	Copyright (C) 2005 by Karye						*
 *	karye@users.sourceforge.net						*
 *										*
 *	This program is free software; you can redistribute it and/or modify	*
 *	it under the terms of the GNU General Public License as published by	*
 *	the Free Software Foundation; either version 2 of the License, or	*
 *	(at your option) any later version.					*
 *										*
 *	This program is distributed in the hope that it will be useful,		*
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of		*
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the		*
 *	GNU General Public License for more details.				*
 *										*
 *	You should have received a copy of the GNU General Public License	*
 *	along with this program; if not, write to the				*
 *	Free Software Foundation, Inc.,						*
 *	59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.		*
 ********************************************************************************/

#include <unistd.h>

#include "common.h"
#include "search.h"
#include "categorieslistview.h"
#include "portagetab.h"
#include "packageinspector.h"
#include "packageversion.h"
#include "versionview.h"
#include "uninstallinspector.h"
#include "ui_portagebase.h"
#include "packagelistmodel.h"
#include "packagelistitem.h"

#include <qlayout.h>
#include <qsplitter.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qtimer.h>
#include <qwhatsthis.h>
#include <qradiobutton.h>

/*#include <kpushbutton.h>
#include <ktextbrowser.h>
#include <kmenu.h>
#include <KDialog>
#include <kuser.h>
#include <klineedit.h>
#include <kiconloader.h>*/
//#include <kaccel.h>
#include <kmessagebox.h>

enum Focus {
		CATEGORYLIST,
		SUBCATEGORYLIST,
		PACKAGELIST
};

/**
 * @class PortageTab
 * @short Package view with filters.
 */
PortageTab::PortageTab( QWidget* parent, PackageInspector *packageInspector )
	: QWidget( parent ), m_focusWidget( PACKAGELIST ), m_delayFilters( 0 ),
	m_packageInspector( packageInspector ), m_uninstallInspector( 0 )
{
	setupUi( this );
	this->splitterH->setStretchFactor(0, 1);
	this->splitterH->setStretchFactor(1, 6);
	filterGroup->setSelected(0);
	//kdDebug() << "PortageTab.constructor categoryView minimumWidth=" << categoriesView->minimumWidth()
	//		<< "actual width=" << categoriesView->width() << LINE_INFO;
	// Connect What's this button
 	connect( pbWhatsThis, SIGNAL( clicked() ), parent->parent(), SLOT( whatsThis() ) );
	connect( pbWhatsThis, SIGNAL( clicked() ), this, SLOT( slotWhatsThis() ) );

	// Connect the filters
	connect( filterGroup, SIGNAL( released( int ) ), this, SLOT( slotFilters() ) );
	connect( searchFilter, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotFilters() ) );

	// Rmb actions.
	//connect( packagesView, SIGNAL( customContextMenuRequested(QPoint) ),
	//		 this, SLOT( slotContextMenu() ) );

	// Button actions.
	connect( pbQueue, SIGNAL( clicked() ), this, SLOT( slotEnqueue() ) );
	connect( pbUninstall, SIGNAL( clicked() ), this, SLOT( slotUninstall() ) );
	//TODO:reimplement
	connect( packagesView, SIGNAL( doubleClickedSignal(PackageListItem*) ), this, SLOT( slotAdvanced() ) );
	connect( pbAdvanced, SIGNAL( clicked() ), this, SLOT( slotAdvanced() ) );
	connect( pbClearFilter, SIGNAL( clicked() ), this, SLOT( slotClearFilter() ) );

	// Toggle Queue button between "add/remove" when after queue has been edited
	connect( QueueSingleton::Instance(), SIGNAL( signalQueueChanged( bool ) ), this, SLOT( slotInitButtons() ) );
	connect( SignalistSingleton::Instance(), SIGNAL( signalPackageQueueChanged() ), this, SLOT( slotButtons() ) );

	// Reload view after changes.
	connect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotReload() ) );

	// Enable/disable this view and buttons when kuroo is busy
	connect( SignalistSingleton::Instance(), SIGNAL( signalKurooBusy( bool ) ), this, SLOT( slotBusy() ) );

	// Enable/disable buttons
	connect( packagesView, SIGNAL( selectionChangedSignal() ), this, SLOT( slotButtons() ) );

	// Load Inspector with current package info
	connect( packagesView, SIGNAL( selectionChangedSignal() ), this, SLOT( slotPackage() ) );

	// Connect changes made in Inspector to this view so it gets updated
	connect( m_packageInspector, SIGNAL( signalPackageChanged() ), this, SLOT( slotPackage() ) );
	connect( m_packageInspector, SIGNAL( signalNextPackage( bool ) ), this, SLOT( slotNextPackage( bool ) ) );
	connect( m_packageInspector, SIGNAL( hidden() ), this, SLOT( slotButtons() ) );

	// Shortcut to enter filter with package name
	connect( SignalistSingleton::Instance(), SIGNAL( signalPackageClicked( const QString& ) ), this, SLOT( slotFillFilter( const QString& ) ) );

	slotInit();
}

PortageTab::~PortageTab()
{}

/**
 * Initialize Portage view.
 */
void PortageTab::slotInit()
{
	// Get color theme
	QPalette p;
	p.setColor(portageFrame->backgroundRole(), palette().base().color());
	portageFrame->setPalette(p);

	// Change select-color in summaryBrowser to get contrast
	QColorGroup colorGroup = QColorGroup(palette());
	QPalette summaryPalette;
	QColorGroup summaryColorGroup( colorGroup );
	summaryColorGroup.setColor( QColorGroup::HighlightedText, palette().dark().color() );

	/*summaryPalette.setActive( summaryColorGroup );
	summaryPalette.setInactive( summaryColorGroup );
	summaryPalette.setDisabled( summaryColorGroup );*/
	summaryPalette.setColor(QPalette::Active, QPalette::NoRole, summaryColorGroup.color(QPalette::Active, QPalette::NoRole));
	summaryPalette.setColor(QPalette::Inactive, QPalette::NoRole, summaryColorGroup.color(QPalette::Inactive, QPalette::NoRole));
	summaryPalette.setColor(QPalette::Disabled, QPalette::NoRole, summaryColorGroup.color(QPalette::Disabled, QPalette::NoRole));

	summaryBrowser->setPalette( summaryPalette );

	// Keyboard shortcuts
	/*KAccel* pAccel = new KAccel( this );
	pAccel->insert( "View package details...", i18n("View package details..."), i18n("View package details..."),
					Qt::Key_Return, this, SLOT( slotAdvanced() ) );*/

	// Initialize the uninstall dialog
	m_uninstallInspector = new UninstallInspector( this );

	pbClearFilter->setIcon( KIcon("edit-clear-locationbar-ltr") );
	pbQueue->setIcon( KIcon("kuroo_queue") );
	pbUninstall->setIcon( KIcon("list-remove") );
	//TODO: There is no icon in the pics folder for this, and I can't find any stock icon named options, so
	//I'm turning it off for now.
	//pbAdvanced->setIcon( KIcon("options") );
	pbWhatsThis->setIcon( KIcon("help-about") );

	slotBusy();
}

/**
 * What's this info explaning this tabs functionality.
 */
void PortageTab::slotWhatsThis()
{
	QWhatsThis::showText( QCursor::pos(), i18n( "<qt>"
			"This tab gives an overview of all packages available: in Portage, installed packages as well as package updates.<br>"
			"To keep your system in perfect shape (and not to mention install the latest security updates) you need to update your system regularly. "
			"Since Portage only checks the ebuilds in your Portage tree you first have to sync your Portage tree: "
			"Select 'Sync Portage' in the Portage menu.<br>"
			"After syncing Kuroo will search for newer version of the applications you have installed. "
			"However, it will only verify the versions for the applications you have explicitly installed - not the dependencies.<br>"
			"If you want to update every single package on your system, check the Deep checkbox in Kuroo Preferences.<br><br>"
			"When you want to remove a software package from your system, select a package and press 'Uninstall'. "
			"This will tell Portage to remove all files installed by that package from your system except the configuration files "
			"of that application if you have altered those after the installation. "
			"However, a big warning applies: Portage will not check if the package you want to remove is required by another package. "
			"It will however warn you when you want to remove an important package that breaks your system if you unmerge it.<br><br>"
			"Use the package Inspector to manage package specific version and use-flag settings: press 'Details' to open the Inspector.</qt>" ), this );
}

/**
 * Forward signal from next-buttons only if this tab is visible for user.
 * @param isNext
 */
void PortageTab::slotNextPackage( bool isNext )
{
	if ( !m_packageInspector->isParentView( VIEW_PORTAGE ) )
		return;

	packagesView->nextPackage( isNext );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Toggle button slots
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Reset queue button text when queue is refreshed.
 */
void PortageTab::slotInitButtons()
{
	pbQueue->setText( i18n("Add to Queue") );
}

/**
 * Disable/enable buttons when kuroo is busy.
 */
void PortageTab::slotBusy()
{
	// If no db no fun!
	if ( !SignalistSingleton::Instance()->isKurooReady() ) {
		pbUninstall->setDisabled( true );
		pbAdvanced->setDisabled( true );
		pbQueue->setDisabled( true );
		filterGroup->setDisabled( true );
		searchFilter->setDisabled( true );
		pbClearFilter->setDisabled( true );
	}
	else {
		filterGroup->setDisabled( false );
		searchFilter->setDisabled( false );
		pbClearFilter->setDisabled( false );
		slotButtons();
	}
}

/**
 * Toggle buttons states.
 */
void PortageTab::slotButtons()
{
	if ( m_packageInspector->isVisible() )
		return;
	else {
		filterGroup->setDisabled( false );
		searchFilter->setDisabled( false );
		pbClearFilter->setDisabled( false );
	}

	// No current package, disable all buttons
	if ( packagesView->selectedPackages().isEmpty()) {
		pbQueue->setDisabled( true );
		pbAdvanced->setDisabled( true );
		pbUninstall->setDisabled( true );
		return;
	}

	m_packageInspector->setDisabled( false );
	pbAdvanced->setDisabled( false );

	// When kuroo is busy disable queue and uninstall button
	if ( SignalistSingleton::Instance()->isKurooBusy() ) {
		pbQueue->setDisabled( true );
		pbUninstall->setDisabled( true );
		return;
	}
	else
		pbQueue->setDisabled( false );

	// Toggle queue button between add/remove
	if ( packagesView->currentPackage()->isInPortage() ) {
		pbQueue->setDisabled( false );

		if ( packagesView->currentPackage()->isQueued() )
			pbQueue->setText( i18n("Remove from Queue") );
		else
			pbQueue->setText( i18n("Add to Queue") );
	}
	else {
		pbQueue->setText( i18n("Add to Queue") );
		pbQueue->setDisabled( true );
	}

	// If user is su enable uninstall
	if ( packagesView->currentPackage()->isInstalled() && ::getuid() == 0 )
		pbUninstall->setDisabled( false );
	else
		pbUninstall->setDisabled( true );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Package view slots
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Initialize category and subcategory views with the available categories and subcategories.
 */
void PortageTab::slotReload()
{
 	m_packageInspector->setDisabled( true );
	pbAdvanced->setDisabled( true );

	disconnect( categoriesView, SIGNAL( currentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ), this, SLOT( slotListSubCategories() ) );
	disconnect( subcategoriesView, SIGNAL( currentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ), this, SLOT( slotListPackages() ) );

	//kdDebug() << "PortageTab.slotReload categoriesView.minWidth=" << categoriesView->minimumWidth()
	//		<< "actual width" << categoriesView->width() << LINE_INFO;
	categoriesView->init();
	subcategoriesView->init();

	connect( categoriesView, SIGNAL( currentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ), this, SLOT( slotListSubCategories() ) );
	connect( subcategoriesView, SIGNAL( currentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ), this, SLOT( slotListPackages() ) );

	categoriesView->loadCategories( KurooDBSingleton::Instance()->portageCategories( 1 << filterGroup->selected(), searchFilter->text() ), false );
}

void PortageTab::slotFillFilter( const QString& text )
{
	searchFilter->setText( text );
}

/**
 * Execute query based on filter and text. Add a delay of 250ms.
 */
void PortageTab::slotFilters()
{
	m_delayFilters++;
	QTimer::singleShot( 250, this, SLOT( slotActivateFilters() ) );
}

/**
 * Execute query based on filter and text.
 */
void PortageTab::slotActivateFilters()
{
	--m_delayFilters;
	if ( m_delayFilters == 0 )
		categoriesView->loadCategories( KurooDBSingleton::Instance()->portageCategories( 1 << filterGroup->selected(), searchFilter->text() ), true );
}

/**
 * List packages when clicking on category in installed.
 */
void PortageTab::slotListSubCategories()
{
	subcategoriesView->loadCategories( KurooDBSingleton::Instance()->portageSubCategories( categoriesView->currentCategoryId(),
		1 << filterGroup->selected(), searchFilter->text() ) );
}

/**
 * List packages when clicking on subcategory.
 */
void PortageTab::slotListPackages()
{
	int numberOfTerms = 0; //For singular or plural message
	// Disable all buttons if query result is empty
	QStringList dbres = KurooDBSingleton::Instance()->portagePackagesBySubCategory(categoriesView->currentCategoryId(),
										       subcategoriesView->currentCategoryId(),
										       1 << filterGroup->selected(),
										       searchFilter->text());

	packagesView->setPackages(dbres);
	if ( dbres.count() == 0) {
		m_packageInspector->hide();
		slotButtons();
		summaryBrowser->clear();
		if (radioUpdates->isChecked())
			numberOfTerms = -1;
		else {
			/*** We check number of terms to pass to showNoHitsWarning **/
			QChar space((char)32);
			bool iscounted=false;
			for (int i = 0; i < searchFilter->text().length(); i++) {
				if (searchFilter->text()[i] != space) {
					if (!iscounted) numberOfTerms++;
					iscounted = true;
				}
				else {
					iscounted = false;
				}
			}
		}
		kDebug(0) << numberOfTerms << LINE_INFO;
		//FIXME:implement this
		//packagesView->showNoHitsWarning( true , numberOfTerms );

		//TODO: This forces white instead of using the current color-scheme default
		// Highlight text filter background in red if query failed
		if ( !searchFilter->text().isEmpty() )
		{
			//searchFilter->setPaletteBackgroundColor( QColor( KurooConfig::noMatchColor() ) );
			QPalette p;
			p.setColor(searchFilter->backgroundRole(), QColor(KurooConfig::noMatchColor()));
			searchFilter->setPalette(p);

		}
		else
		{
			//searchFilter->setPaletteBackgroundColor( Qt::white );
			QPalette p;
			p.setColor(searchFilter->backgroundRole(), Qt::white);
			searchFilter->setPalette(p);
		}
	}
	else {
		//FIXME:implement this
		//packagesView->showNoHitsWarning( false , numberOfTerms );

		// Highlight text filter background in green if query successful
		if ( !searchFilter->text().isEmpty() )
		{
			//searchFilter->setPaletteBackgroundColor( QColor( KurooConfig::matchColor() ) );
			QPalette p;
			p.setColor(searchFilter->backgroundRole(), QColor(KurooConfig::matchColor()));
			searchFilter->setPalette(p);
		}
		else
		{
			//searchFilter->setPaletteBackgroundColor( Qt::white );
			QPalette p;
			p.setColor(searchFilter->backgroundRole(), Qt::white);
			searchFilter->setPalette(p);
		}
	}
}

/**
 * Reset text filter when clicking on clear button.
 */
void PortageTab::slotClearFilter()
{
	searchFilter->clear();
}

/**
 * Refresh packages list.
 */
void PortageTab::slotRefresh()
{
	switch( KMessageBox::questionYesNo( this,
		i18n( "<qt>Do you want to refresh the Packages view?<br>"
			  "This will take a couple of minutes...</qt>"), i18n( "Refreshing Packages" ),
										KStandardGuiItem::yes(), KStandardGuiItem::no(), "dontAskAgainRefreshPortage" ) ) {
		case KMessageBox::Yes:
			PortageSingleton::Instance()->slotRefresh();
	}
}

/**
 * Append or remove package to the queue.
 */
void PortageTab::slotEnqueue()
{
	if ( !EmergeSingleton::Instance()->isRunning() || !SignalistSingleton::Instance()->isKurooBusy() ) {
		const QStringList selectedIdsList = packagesView->selectedPackagesByIds();
		QStringList packageIdList;
		foreach( QString id, selectedIdsList )
			if ( packagesView->packageItemById( id )->isInPortage() && !packagesView->packageItemById( id )->isQueued() )
				packageIdList += id;
		QueueSingleton::Instance()->addPackageIdList( packageIdList );
	}
}

void PortageTab::slotDequeue()
{
	if ( !EmergeSingleton::Instance()->isRunning() || !SignalistSingleton::Instance()->isKurooBusy() ) {
		QueueSingleton::Instance()->removePackageIdList( packagesView->selectedPackagesByIds() );
	}
}

/**
 * Uninstall selected package.
 */
void PortageTab::slotUninstall()
{
	if ( !EmergeSingleton::Instance()->isRunning() || !SignalistSingleton::Instance()->isKurooBusy() || ::getuid() != 0 ) {
		const QStringList selectedIdsList = packagesView->selectedPackagesByIds();

		// Pick only installed packages
		QStringList packageList;
		foreach ( QString id, selectedIdsList ) {
			if ( packagesView->packageItemById( id )->isInstalled() ) {
				packageList += id;
				packageList += packagesView->packageItemById( id )->category() + "/" + packagesView->packageItemById( id )->name();
			}
		}

		m_uninstallInspector->view( packageList );
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Package slots
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Open advanced dialog with: ebuild, versions, use flags...
 */
void PortageTab::slotAdvanced()
{
	DEBUG_LINE_INFO;
	pbUninstall->setDisabled( true );
	pbAdvanced->setDisabled( true );
	pbQueue->setDisabled( true );
	filterGroup->setDisabled( true );
	searchFilter->setDisabled( true );
	pbClearFilter->setDisabled( true );

	if ( packagesView->currentPackage() )
		processPackage( true );
}

void PortageTab::slotPackage()
{
	if ( m_packageInspector->isVisible() )
		processPackage( true );
	else
		processPackage( false );
}

/**
 * Process package and all it's versions.
 * Update summary and Inspector.
 */
void PortageTab::processPackage(bool viewInspector)
{
	if ( m_packageInspector->isVisible() && !m_packageInspector->isParentView( VIEW_PORTAGE ) )
		return;

	summaryBrowser->clear();

	// Multiple packages selected
	const QStringList selectedIdsList = packagesView->selectedPackagesByIds();
	int count = selectedIdsList.size();
	if ( count > 1 ) {

		// Build summary html-view
		QString lines = "<table width=100% border=0 cellpadding=0>";
		lines += "<tr><td colspan=2><font";
		//bgcolor=#" + GlobalSingleton::Instance()->bgHexColor() + "
		//lines += "color=#"GlobalSingleton::Instance()->fgHexColor()
		lines += " size=+1><b>";
		lines += QString::number( count )+ i18n(" packages selected") + "</b></font></td></tr>";
		lines += "<tr><td>";
		foreach ( QString id, selectedIdsList ) {
			lines += packagesView->packageItemById( id )->name();
			lines += " (" + packagesView->packageItemById( id )->category().section( "-", 0, 0 ) + "/";
			lines += packagesView->packageItemById( id )->category().section( "-", 1, 1 ) + "), ";
		}
		lines = lines.left( lines.length() - 2 );
		lines += "</td></tr>";
		summaryBrowser->setText( lines + "</table>");

		pbAdvanced->setDisabled( true );
		return;
	}

	// Initialize the portage package object with the current package and it's versions data
	PackageListItem *package = packagesView->currentPackage();
	package->parsePackageVersions();
	QString linesInstalled = package->linesInstalled();
	QString linesAvailable = package->linesAvailable();
	QString linesEmerge = package->linesEmerge();

	// Build summary html-view
	QString lines = "<table width=100% border=0 cellpadding=0>";
			lines += "<tr><td";
			//"bgcolor=#" + GlobalSingleton::Instance()->bgHexColor();
			lines += " colspan=2><b>";
			//lines += "<font color=#" + GlobalSingleton::Instance()->fgHexColor() + ">"
			lines += "<font size=+1>" + package->name() + "</font> ";
			lines += "(" + package->category().section( "-", 0, 0 ) + "/";
			lines += package->category().section( "-", 1, 1 ) + ")</font></b></td></tr>";

	if ( package->isInPortage() ) {
		lines += "<tr><td colspan=2>" + package->description() + "</td></tr>";
		lines += "<tr><td colspan=2>" + i18n("<b>Homepage: </b>") + "<a href=\"" + package->homepage();
		lines += "\">" + package->homepage() + "</a></td></tr>";
	}
	else
		lines += "<tr><td colspan=2><font color=darkRed><b>"
				+ i18n("Package not available in Portage tree anymore!")
				+ "</b></font></td></tr>";

	// Construct installed verions line
	if ( !linesInstalled.isEmpty() )
		linesInstalled = "<tr><td width=10%><b>"
						+ i18n("Installed&nbsp;version:")
						+ "</b></font></td><td width=90%>"
						+ linesInstalled
						+ "</td></tr>";
	else
		linesInstalled = "<tr><td width=10%><b>"
						+ i18n("Installed&nbsp;version:")
						+ "</b></font></td><td width=90%>"
						+ "Not installed"
						+ "</td></tr>";

	if ( package->isInPortage() ) {

		// Construct emerge version line
		if ( !linesEmerge.isEmpty() ) {
			linesEmerge = "<tr><td width=10%><b>"
						+ i18n("Emerge&nbsp;version:")
						+ "</b></td><td width=90%>"
						+ linesEmerge
						+ "</td></tr>";
		}
		else {
			if ( package->isInArch() && linesAvailable.isEmpty() )
				linesEmerge = "<tr><td width=10%><b>"
							+ i18n("Emerge&nbsp;version:")
							+ "</font></td><td width=90%><font color=darkRed>"
							+ i18n("No version available on")
							+ KurooConfig::arch()
							+ "</b></td></tr>";
			else
				linesEmerge = "<tr><td width=10%><b>"
							+ i18n("Emerge&nbsp;version:")
							+ "</font></td><td width=90%><font color=darkRed>"
							+ i18n("No version available - please check package details")
							+ "</font></b></td></tr>";
		}

		// Construct available versions line
		if ( !linesAvailable.isEmpty() )
			linesAvailable = "<tr><td width=10%><b>"
							+ i18n("Available&nbsp;versions:")
							+ "</b></td><td width=90%>"
							+ linesAvailable
							+ "</b></td></tr>";
		else
			linesAvailable = "<tr><td width=10%><b>"
						+ i18n("Available&nbsp;versions:")
						+ "</td><td width=90%><font color=darkRed>"
						+ i18n("No versions available on %1", KurooConfig::arch())
						+ "</font></b></td></tr>";

		summaryBrowser->setText( lines + linesInstalled + linesEmerge + linesAvailable + "</table>");
	}
	else
		summaryBrowser->setText( lines + linesInstalled + "</table>");

	// Refresh inspector if visible
	if ( viewInspector )
		m_packageInspector->edit( package, VIEW_PORTAGE );
}

/**
 * Popup menu for actions like emerge.
 * @param item
 * @param point
 */
void PortageTab::slotContextMenu()
{
	DEBUG_LINE_INFO;
	//port KMenu to KDE4
	/*if ( !item )
		return;

	const QStringList selectedIdsList = packagesView->selectedIds();

	bool hasQueuedItems = false;
	bool hasUnqueuedItemsInPortage = false;
	bool hasItemsInWorld = false;
	bool hasItemsOutOfWorld = false;
	bool hasInstalledPackages = false;
	bool hasInstalledAndInPortagePackages = false;

	foreach( QString id, selectedIdsList ) {
		PackageItem *currentItem = packagesView->packageItemById( id );

		if( currentItem->isQueued() ) {
			hasQueuedItems = true;
		} else {
			hasUnqueuedItemsInPortage |= currentItem->isInPortage();
		}

		hasItemsInWorld |= currentItem->isInWorld();
		hasItemsOutOfWorld |= !currentItem->isInWorld();

		if( currentItem->isInstalled() ) {
			hasInstalledPackages = true;
			hasInstalledAndInPortagePackages |= currentItem->isInPortage();
		}

		if( hasQueuedItems && hasUnqueuedItemsInPortage && hasItemsInWorld && hasItemsOutOfWorld
			&& hasInstalledPackages && hasInstalledAndInPortagePackages )
			break;	//stop the loop if we already have everything

	}

	enum Actions { ADDQUEUE, DELQUEUE, UNINSTALL, ADDWORLD, DELWORLD, QUICKPACKAGE };

	KMenu menu( this );

	int enqueueMenuItem;
	enqueueMenuItem = menu.addAction( ImagesSingleton::Instance()->icon( QUEUED ), i18n("&Add to queue"), ADDQUEUE );
	menu.setEnabled( enqueueMenuItem, hasUnqueuedItemsInPortage );

	int dequeueMenuItem;
	dequeueMenuItem = menu.insertItem( ImagesSingleton::Instance()->icon( QUEUED ), i18n("&Remove from queue"), DELQUEUE );
	menu.setItemEnabled( dequeueMenuItem, hasQueuedItems );

	int detailsMenuItem;
	detailsMenuItem = menu.insertItem( ImagesSingleton::Instance()->icon( DETAILS ), i18n( "Details..." ), DETAILS );
	menu.setItemEnabled( detailsMenuItem, selectedIdsList.count() == 1 );

	int addWorldMenuItem;
	addWorldMenuItem = menu.insertItem( ImagesSingleton::Instance()->icon( WORLD ), i18n( "Add to world" ), ADDWORLD );
	menu.setItemEnabled( addWorldMenuItem, false );	//default to false, enable later if super user

	int delWorldMenuItem;
	delWorldMenuItem = menu.insertItem( ImagesSingleton::Instance()->icon( WORLD ), i18n( "Remove from world" ), DELWORLD );
	menu.setItemEnabled( delWorldMenuItem, false );	//default to false, enable later if super user

	int uninstallMenuItem;
	uninstallMenuItem = menu.insertItem( ImagesSingleton::Instance()->icon( REMOVE ), i18n("&Uninstall"), UNINSTALL );
	menu.setItemEnabled( uninstallMenuItem, hasInstalledPackages );

	int backupMenuItem;
	backupMenuItem = menu.insertItem( ImagesSingleton::Instance()->icon( QUICKPKG ), i18n("Backup Package"), QUICKPACKAGE );
	menu.setItemEnabled( backupMenuItem, hasInstalledAndInPortagePackages );

	// No change to Queue when busy @todo: something nuts here. Click once then open rmb to make it work!
	kDebug() << "EmergeSingleton::Instance()->isRunning()=" << EmergeSingleton::Instance()->isRunning();
	kDebug() << "SignalistSingleton::Instance()->isKurooBusy()=" << SignalistSingleton::Instance()->isKurooBusy();
	kDebug() << "hasQueuedItems=" << hasQueuedItems;
	kDebug() << "hasUnqueuedItemsInPortage=" << hasUnqueuedItemsInPortage;
	kDebug() << "hasItemsInWorld=" << hasItemsInWorld;
	kDebug() << "hasItemsOutOfWorld=" << hasItemsOutOfWorld;
	kDebug() << "hasInstalledPackages=" << hasInstalledPackages;
	kDebug() << "hasInstalledAndInPortagePackages=" << hasInstalledAndInPortagePackages;

	bool busy = EmergeSingleton::Instance()->isRunning() || SignalistSingleton::Instance()->isKurooBusy();
	if ( busy ) {
		menu.setItemEnabled( enqueueMenuItem, false );
		menu.setItemEnabled( dequeueMenuItem, false );
		menu.setItemEnabled( backupMenuItem, false );
	}

	// No uninstall when emerging or no privileges
	if ( busy || !KUser().isSuperUser() )
		menu.setItemEnabled( uninstallMenuItem, false );

	// Allow editing of World when superuser
	if ( KUser().isSuperUser() ) {
		menu.setItemEnabled( addWorldMenuItem, hasItemsOutOfWorld );
		menu.setItemEnabled( delWorldMenuItem, hasItemsInWorld );
	}

	if ( m_packageInspector->isVisible() ) {
		menu.setItemEnabled( enqueueMenuItem, false );
		menu.setItemEnabled( dequeueMenuItem, false );
		menu.setItemEnabled( detailsMenuItem, false );
		menu.setItemEnabled( addWorldMenuItem, false );
		menu.setItemEnabled( delWorldMenuItem, false );
		menu.setItemEnabled( uninstallMenuItem, false );
		menu.setItemEnabled( backupMenuItem, false );
	}

	switch( menu.exec( point ) ) {

		case ADDQUEUE:
			slotEnqueue();
			break;

		case DELQUEUE:
			slotDequeue();
			break;

		case UNINSTALL:
			slotUninstall();
			break;

		case DETAILS:
			slotAdvanced();
			break;

		case ADDWORLD: {
			QStringList packageList;
			foreach ( QString id, selectedIdsList )
				packageList += packagesView->packageItemById( id )->category() + "/" + packagesView->packageItemById( id )->name();
			PortageSingleton::Instance()->appendWorld( packageList );
			break;
		}

		case DELWORLD: {
			QStringList packageList;
			foreach ( QString id, selectedIdsList )
				packageList += packagesView->packageItemById( id )->category() + "/" + packagesView->packageItemById( id )->name();
			PortageSingleton::Instance()->removeFromWorld( packageList );
			break;
		}

		case QUICKPACKAGE: {
			QStringList packageList;
			foreach( QString id, selectedIdsList ) {
				if( packagesView->packageItemById( id )->isInstalled() && packagesView->packageItemById( id )->isInPortage() )
					packageList += packagesView->packageItemById( id )->category() + "/" + packagesView->packageItemById( id )->name();
			}
			EmergeSingleton::Instance()->quickpkg( packageList );
			break;
		}
	}*/
}

#include "portagetab.moc"
