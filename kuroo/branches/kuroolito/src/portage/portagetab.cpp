/***************************************************************************
 *   Copyright (C) 2007 by Karye                                           *
 *   info@kuroo.org                                                        *
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
#include "search.h"
#include "categorieslistview.h"
#include "portagelistview.h"
#include "portagetab.h"
#include "packageversion.h"

#include <qlayout.h>
#include <qsplitter.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qtimer.h>
#include <qwhatsthis.h>

#include <kpushbutton.h>
#include <ktextbrowser.h>
#include <kpopupmenu.h>
#include <kdialogbase.h>
#include <kmessagebox.h>
#include <kuser.h>
#include <klineedit.h>
#include <kiconloader.h>
#include <kaccel.h>

/**
 * @class PortageTab
 * @short Package view with filters.
 */
PortageTab::PortageTab( QWidget* parent )
	: PortageBase( parent ), m_delayFilters( 0 )
{
	// Connect the filters
	connect( filterGroup, SIGNAL( released( int ) ), this, SLOT( slotFilters() ) );
	connect( searchFilter, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotFilters() ) );
	
	// Button actions.
	connect( pbClearFilter, SIGNAL( clicked() ), this, SLOT( slotClearFilter() ) );
	
	// Reload view after changes.
	connect( PortageSingleton::Instance(), SIGNAL( signalPortageChanged() ), this, SLOT( slotReload() ) );
	
	// Shortcut to enter filter with package name
	connect( SignalistSingleton::Instance(), SIGNAL( signalPackageClicked( const QString& ) ), this, SLOT( slotFillFilter( const QString& ) ) );
	
	// Load with current package info
	connect( packagesView, SIGNAL( selectionChanged() ), this, SLOT( slotPackage() ) );
	
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
	portageFrame->setPaletteBackgroundColor( colorGroup().base() );
	
	// Change select-color in summaryBrowser to get contrast
	QPalette summaryPalette;
	QColorGroup summaryColorGroup( colorGroup() );
	summaryColorGroup.setColor( QColorGroup::HighlightedText, colorGroup().dark() );
	summaryPalette.setActive( summaryColorGroup );
	summaryPalette.setInactive( summaryColorGroup );
	summaryPalette.setDisabled( summaryColorGroup );
	summaryBrowser->setPalette( summaryPalette );

	pbClearFilter->setIconSet( SmallIconSet("locationbar_erase") );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Package view slots
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Initialize category and subcategory views with the available categories and subcategories.
 */
void PortageTab::slotReload()
{
	disconnect( categoriesView, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotListSubCategories() ) );
	disconnect( subcategoriesView, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotListPackages() ) );
	
	categoriesView->init();
	subcategoriesView->init();
	
	connect( categoriesView, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotListSubCategories() ) );
	connect( subcategoriesView, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotListPackages() ) );
	
	categoriesView->loadCategories( KuroolitoDBSingleton::Instance()->portageCategories( filterGroup->selectedId(), searchFilter->text() ), false );
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
		categoriesView->loadCategories( KuroolitoDBSingleton::Instance()->portageCategories( filterGroup->selectedId(), searchFilter->text() ), true );
}

/**
 * List packages when clicking on category in installed.
 */
void PortageTab::slotListSubCategories()
{
	subcategoriesView->loadCategories( KuroolitoDBSingleton::Instance()->portageSubCategories( categoriesView->currentCategoryId(), 
		filterGroup->selectedId(), searchFilter->text() ) );
}

/**
 * List packages when clicking on subcategory.
 */
void PortageTab::slotListPackages()
{
	// Disable all buttons if query result is empty
	if ( packagesView->addSubCategoryPackages( KuroolitoDBSingleton::Instance()->portagePackagesBySubCategory( categoriesView->currentCategoryId(),
		subcategoriesView->currentCategoryId(), filterGroup->selectedId(), searchFilter->text() ) ) == 0 ) {
		
		summaryBrowser->clear();
		packagesView->showNoHitsWarning( i18n( "<font color=darkRed size=+1><b>No packages found with these filter settings</font><br>"
											   "<font color=darkRed>Please modify the filter settings you have chosen!<br>"
											   "Try to use more general filter options, so Kuroolito can find matching packages.</b></font>" ) );

		// Highlight text filter background in red if query failed
		if ( !searchFilter->text().isEmpty() )
			searchFilter->setPaletteBackgroundColor( QColor( KuroolitoConfig::noMatchColor() ) );
		else
			searchFilter->setPaletteBackgroundColor( Qt::white );
	}
	else {
		packagesView->showNoHitsWarning( QString::null );
		
		// Highlight text filter background in green if query successful
		if ( !searchFilter->text().isEmpty() )
			searchFilter->setPaletteBackgroundColor( QColor( KuroolitoConfig::matchColor() ) );
		else
			searchFilter->setPaletteBackgroundColor( Qt::white );
		
		slotPackage();
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
 * Process package and all it's versions.
 * Update summary and Inspector.
 */
void PortageTab::slotPackage()
{
	summaryBrowser->clear();
	
	// Multiple packages selected
	const QStringList selectedIdList = packagesView->selectedId();
	int count = selectedIdList.size();
	if ( count > 1 ) {
		
		// Build summary html-view
		QString lines = "<table width=100% border=0 cellpadding=0>";
		lines += "<tr><td bgcolor=#" + GlobalSingleton::Instance()->bgHexColor() + " colspan=2><font color=#";
		lines += GlobalSingleton::Instance()->fgHexColor() + " size=+1><b>";
		lines += QString::number( count )+ i18n(" packages selected") + "</b></font></td></tr>";
		lines += "<tr><td>";
		foreach ( selectedIdList ) {
			lines += packagesView->packageItemById( *it )->category() + "/";
			lines += packagesView->packageItemById( *it )->name() + " ";
		}
		lines = lines.left( lines.length() - 2 );
		lines += "</td></tr>";
		summaryBrowser->setText( lines + "</table>");
		
		return;
	}
	
	// Initialize the portage package object with the current package and it's versions data
	packagesView->currentPackage()->parsePackageVersions();
	QString linesInstalled = packagesView->currentPackage()->linesInstalled();
	QString linesAvailable = packagesView->currentPackage()->linesAvailable();
	QString linesUnavailable = packagesView->currentPackage()->linesUnavailable();
	QString linesEmerge = packagesView->currentPackage()->linesEmerge();
	
	// Build summary html-view
	QString lines = "<table width=100% border=0 cellpadding=0>";
			lines += "<tr><td bgcolor=#" + GlobalSingleton::Instance()->bgHexColor() + " colspan=2><b><font color=#";
			lines += GlobalSingleton::Instance()->fgHexColor() + "><font size=+1>" + packagesView->currentPackage()->name() + "</font> ";
			lines += "(" + packagesView->currentPackage()->category().section( "-", 0, 0 ) + "/";
			lines += packagesView->currentPackage()->category().section( "-", 1, 1 ) + ")</font></b></td></tr>";
	
	if ( packagesView->currentPackage()->isInPortage() ) {
		lines += "<tr><td colspan=2>" + packagesView->currentPackage()->description() + "</td></tr>";
		lines += "<tr><td colspan=2>" + i18n("<b>Homepage: </b>") + "<a href=\"" + packagesView->currentPackage()->homepage();
		lines += "\">" + packagesView->currentPackage()->homepage() + "</a></td></tr>";
	}
	else
		lines += i18n("%1Package not available in Portage tree anymore!%2")
					.arg("<tr><td colspan=2><font color=darkRed><b>")
					.arg("</b></font></td></tr>");
	
	// Construct installed verions line
	if ( !linesInstalled.isEmpty() )
		linesInstalled = i18n("%1Installed&nbsp;version:%2%3%4")
						.arg("<tr><td width=10%><b>")
						.arg("</b></font></td><td width=90%>")
						.arg( linesInstalled )
						.arg("</td></tr>");
	else
		linesInstalled = i18n("%1Installed&nbsp;version:%2%3%4")
						.arg("<tr><td width=10%><b>")
						.arg("</b></font></td><td width=90%>")
						.arg("Not installed")
						.arg("</td></tr>");
	
	if ( packagesView->currentPackage()->isInPortage() ) {
	
		// Construct emerge version line
		if ( !linesEmerge.isEmpty() ) {
			linesEmerge = i18n("%1Emerge&nbsp;version:%2%3%4")
				.arg("<tr><td width=10%><b>")
				.arg("</b></td><td width=90%>")
				.arg( linesEmerge )
				.arg("</td></tr>");
		}
		else {
			if ( packagesView->currentPackage()->isInArch() && linesAvailable.isEmpty() )
				linesEmerge = i18n("%1Emerge&nbsp;version:%2No version available on %3%4")
				.arg("<tr><td width=10%><b>")
				.arg("</font></td><td width=90%><font color=darkRed>")
				.arg( KuroolitoConfig::arch() )
				.arg("</b></td></tr>");
			else
				linesEmerge = i18n("%1Emerge&nbsp;version:%2No version available - please check package details%3")
				.arg("<tr><td width=10%><b>")
				.arg("</font></td><td width=90%><font color=darkRed>")
				.arg("</font></b></td></tr>");
		}
		
		// Construct available versions line
		if ( !linesAvailable.isEmpty() )
			linesAvailable = i18n("%1Available&nbsp;versions:%2%3%4")
			.arg("<tr><td width=10%><b>")
			.arg("</b></td><td width=90%>")
			.arg( linesAvailable )
			.arg("</b></td></tr>");
		else
			linesAvailable = i18n("%1Available&nbsp;versions:%2No versions available on %3%4")
			.arg("<tr><td width=10%><b>")
			.arg("</td><td width=90%><font color=darkRed>")
			.arg( KuroolitoConfig::arch() )
			.arg("</font></b></td></tr>");
		
		// Construct not available versions line
		if ( !linesUnavailable.isEmpty() )
			linesUnavailable = i18n("%1Not&nbsp;Available&nbsp;versions:%2%3%4")
			.arg("<tr><td width=10%><b>")
			.arg("</b></td><td width=90%>")
			.arg( linesUnavailable )
			.arg("</b></td></tr>");
		
		summaryBrowser->setText( lines + linesInstalled + linesEmerge + linesAvailable + linesUnavailable + "</table>");
	}
	else
		summaryBrowser->setText( lines + linesInstalled + "</table>");
}

#include "portagetab.moc"

