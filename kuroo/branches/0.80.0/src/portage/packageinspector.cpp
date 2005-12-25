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

#include "packageinspector.h"
#include "common.h"

#include <qregexp.h>

#include <kactionselector.h>
#include <ktextbrowser.h>
#include <kmessagebox.h>
#include <kuser.h>
#include <klistview.h>

/**
 * Specialized dialog for editing Use Flags per package.
 */
PackageInspector::PackageInspector( QWidget *parent, const char *name )
	: KDialogBase( KDialogBase::Swallow, i18n( "Package Inspector" ), KDialogBase::Apply | KDialogBase::Cancel, KDialogBase::Apply, parent, i18n( "Save" ), false ), category( NULL ), package( NULL ), packageId( NULL )
{
	dialog = new InspectorBase( this );
	setMainWidget( dialog );
}

PackageInspector::~PackageInspector()
{
}

/**
 * View Use Flag description.
 * @param item
 */
void PackageInspector::slotUseDescription( QListBoxItem* item )
{
	foreach ( useList ) {
		QString use = item->text();
		if ( use.startsWith( "-" ) )
			use = use.section( "-", 1, 1 );
		if ( ( *it ).startsWith( use ) )
			dialog->description->setText( *it );
	}
}

/**
 * Open use flags dialog.
 * @param newPackage	selected package
 */
void PackageInspector::edit( const QString& id )
{
	if ( !KUser().isSuperUser() )
		enableButtonApply( false );
	
	packageId = id;
	package = PortageSingleton::Instance()->package( packageId );
	category = PortageSingleton::Instance()->category( packageId );
	dialog->package->setText( "Package: " + category + "/" + package );
	
	loadVersions();
	loadUseFlags();
	loadEbuild();
	loadDependencies();
	loadChangeLog();
	
	show();
}

void PackageInspector::loadUseFlags()
{
	QString useFile( KurooConfig::dirPortage() + "/profiles/use.desc" );
	QFile f( useFile );
	
	if ( f.open( IO_ReadOnly ) ) {
		QTextStream stream( &f );
		
		while ( !stream.atEnd() ) {
			QString line = stream.readLine();
			if ( !line.startsWith( "#" ) && !line.isEmpty() ) {
				if ( !line.contains( QRegExp( "^alpha|^amd64|^arm|^hppa|^ia64|^mips|^ppc|^ppc64|^ppc-macos|^s390|^sh|^sparc|^x86" ) ) )
					useList += line;
			}
		}
		f.close();
		
		foreach ( useList ) {
			dialog->allUseFlags->availableListBox()->insertItem( "-" + ( *it ).section( " -", 0, 0 ) );
		}
		foreach ( useList ) {
			dialog->allUseFlags->availableListBox()->insertItem( ( *it ).section( " -", 0, 0 ) );
		}
		
		connect( dialog->allUseFlags->availableListBox(), SIGNAL( currentChanged( QListBoxItem* ) ), this, SLOT( slotUseDescription( QListBoxItem* ) ) );
		connect( dialog->allUseFlags->selectedListBox(), SIGNAL( currentChanged( QListBoxItem* ) ), this, SLOT( slotUseDescription( QListBoxItem* ) ) );
	}
	else
		kdDebug() << i18n( "Error reading: " ) << useFile << endl;

	QFile file( "/etc/portage/package.use" );
	dialog->allUseFlags->selectedListBox()->clear();
	
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		while ( !stream.atEnd() ) {
			QString line = stream.readLine();
			if ( !line.isEmpty() ) {
				if ( line.section( " ", 0, 0 ) == ( category + "/" + package ) ) {
					QString eString = line.section( " ", 1, 1 );
					
					int i = 2;
					while ( !eString.isEmpty() ) {
						dialog->allUseFlags->selectedListBox()->insertItem( eString );
						QListBoxItem *selectedUseFlag = dialog->allUseFlags->availableListBox()->findItem( eString, Qt::ExactMatch );
						dialog->allUseFlags->availableListBox()->takeItem( selectedUseFlag );
						eString = line.section( " ", i, i );
						eString = eString.stripWhiteSpace();
						i++;
					};
				}
			}
		}
		file.close();
	}
	dialog->allUseFlags->availableListBox()->setCurrentItem( dialog->allUseFlags->availableListBox()->topItem() );
}

/**
 * Save the new use flags created with ActionSelector
 */
void PackageInspector::slotApply()
{
	QString useFlags;
	QStringList lines;

	for ( unsigned int i = 0; i < dialog->allUseFlags->selectedListBox()->count(); i++ ) {
		QListBoxItem *item = dialog->allUseFlags->selectedListBox()->item(i);
		useFlags += item->text() + " ";
	}
	
	// Get all lines and remove package
	QFile file( "/etc/portage/package.use" );
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		while ( !stream.atEnd() ) {
			QString tmp = stream.readLine();
			QString eString = tmp.stripWhiteSpace();
			
			if ( !tmp.startsWith( category + "/" + package ) && !eString.isEmpty() )
				lines += tmp;
		}
		file.close();
		
		// Add package with updated use flags
		if ( !useFlags.isEmpty() )
			lines += category + "/" + package + " " + useFlags;
	}
	else
		kdDebug() << i18n("Error reading: /etc/portage/package.use") << endl;
	
	// Now write back
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		foreach ( lines ) {
			stream << *it + "\n";
		}
		file.close();
	}
	else
		KMessageBox::error( this, i18n("Failed to save. Please run as root." ), i18n("Saving"));
	
	accept();
}

/**
 * Get this version ebuild.
 * @param id
 */
void PackageInspector::loadEbuild()
{
	QString fileName = KurooConfig::dirPortage() + "/" + category + "/" + package.section( rxPortageVersion, 0, 0 ) + "/" + package + ".ebuild";
	QFile file( fileName );
	
	if ( !file.exists() ) {
		fileName = KurooConfig::dirPortageOverlay() + "/" + category + "/" + package.section( rxPortageVersion, 0, 0 ) + "/" + package + ".ebuild";
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

/**
 * Get this package changelog.
 * @param id
 */
void PackageInspector::loadChangeLog()
{
	QString fileName = KurooConfig::dirPortage() + "/" + category + "/" + package.section(rxPortageVersion, 0, 0) + "/ChangeLog";
	QFile file( fileName );
	
	if ( !file.exists() ) {
		fileName = KurooConfig::dirPortageOverlay() + "/" + category + "/" + package.section(rxPortageVersion, 0, 0) + "/ChangeLog";
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

/**
 * Get this package dependencies.
 * @param id
 */
void PackageInspector::loadDependencies()
{
	QString fileName = KurooConfig::dirEdbDep() + "/usr/portage/" + category + "/" + package;
	QFile file( fileName );
	
	if ( !file.exists() ) {
		fileName = KurooConfig::dirEdbDep() + "/usr/local/portage/" + category + "/" + package;
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

void PackageInspector::loadVersions()
{
	dialog->versionsView->clear();
	
	const QStringList versionList = PortageSingleton::Instance()->packageVersionsInfo( packageId );
	foreach ( versionList ) {
		QString version = *it++;
		QString meta = *it++;
		QString licenses = *it++;
		QString useFlags = *it++;
		QString slot = *it++;
		QString branches = *it++;
		QString size = *it;
		
		if ( meta == FILTERINSTALLED )
			new KListViewItem( dialog->versionsView, version, branches, size );
		else
			new KListViewItem( dialog->versionsView, version, branches, size );
	}
}

#include "packageinspector.moc"
