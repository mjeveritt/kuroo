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

#include <qcombobox.h>
#include <qcheckbox.h>

#include <kactionselector.h>
#include <ktextbrowser.h>
#include <kmessagebox.h>
#include <kuser.h>
#include <klistview.h>

/**
 * Specialized dialog for editing Use Flags per package.
 */
PackageInspector::PackageInspector( QWidget *parent )
	: KDialogBase( KDialogBase::Swallow, i18n( "Package Inspector" ), KDialogBase::Apply | KDialogBase::Cancel, KDialogBase::Apply, parent, i18n( "Save" ), false ), category( NULL ), package( NULL ), packageId( NULL )
{
	dialog = new InspectorBase( this );
	setMainWidget( dialog );
	
	getUseFlagDescription();
	
	connect( dialog->cbVersionsEbuild, SIGNAL( activated( const QString& ) ), this, SLOT( slotGetEbuild( const QString& ) ) );
	connect( dialog->cbVersionsDependencies, SIGNAL( activated ( const QString& ) ), this, SLOT( getDependencies( const QString& ) ) );
	connect( dialog->cbVersionsInstalled, SIGNAL( activated ( const QString& ) ), this, SLOT( getInstalledFiles( const QString& ) ) );
// 	connect( dialog->cbVersionsUse, SIGNAL( activated ( const QString& ) ), this, SLOT( getUseFiles( const QString& ) ) );
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
// 	foreach ( useList ) {
// 		QString use = item->text();
// 		if ( use.startsWith( "-" ) )
// 			use = use.section( "-", 1, 1 );
// 		if ( ( *it ).startsWith( use ) )
// 			dialog->description->setText( *it );
// 	}
}

/**
 * Open use flags dialog.
 * @param newPackage	selected package
 */
void PackageInspector::edit( PortageListView::PortageItem* portagePackage )
{
	if ( !KUser().isSuperUser() )
		enableButtonApply( false );
	
	package = portagePackage->name();
	category = portagePackage->category();
	dialog->package->setText( "Package: " + category + "/" + package );
	
	getUseFlags( portagePackage, dialog->cbVersionsUse->currentText() );
	slotGetEbuild( dialog->cbVersionsEbuild->currentText() );
	getDependencies( dialog->cbVersionsDependencies->currentText() );
	getChangeLog();
	getInstalledFiles( dialog->cbVersionsInstalled->currentText() );
	
	show();
}

void PackageInspector::getUseFlagDescription()
{
	QString useFile( KurooConfig::dirPortage() + "/profiles/use.desc" );
	QFile f( useFile );
	
	if ( f.open( IO_ReadOnly ) ) {
		QTextStream stream( &f );
		
		while ( !stream.atEnd() ) {
			QString line = stream.readLine();
			if ( !line.startsWith( "#" ) && !line.isEmpty() ) {
				if ( !line.contains( QRegExp( "^alpha|^amd64|^arm|^hppa|^ia64|^mips|^ppc|^ppc64|^ppc-macos|^s390|^sh|^sparc|^x86" ) ) )
					useMap.insert( line.section( " -", 0, 0 ), line.section( " -", 1, 1 ) );
			}
		}
		f.close();
	}
	else
		kdDebug() << i18n( "Error reading: " ) << useFile << endl;
}

/**
 * @fixme: use map instead for versionList
 */
void PackageInspector::getUseFlags( PortageListView::PortageItem* portagePackage, const QString& version )
{
	QStringList useList;
	
	QValueList<PackageVersion*> versions = portagePackage->versionList();
	QValueList<PackageVersion*>::iterator versionIterator;
	for( versionIterator = versions.begin(); versionIterator != versions.end(); versionIterator++ ) {
		if ( ( *versionIterator )->version() == version ) {
			useList = ( *versionIterator )->useflags();
			break;
		}
	}

	dialog->useView->clear();
	foreach ( useList ) {
		QString description;
		
		QMap<QString, QString>::iterator itMap = useMap.find( *it );
		if ( itMap != useMap.end() )
			description = itMap.data();
		
		QCheckListItem* useItem = new QCheckListItem( dialog->useView, *it, QCheckListItem::CheckBox );
		useItem->setText( 1, description );
	}
}
/**
 * Save the new use flags created with ActionSelector
 */
void PackageInspector::slotApply()
{
// 	QString useFlags;
// 	QStringList lines;
// 
// 	for ( unsigned int i = 0; i < dialog->allUseFlags->selectedListBox()->count(); i++ ) {
// 		QListBoxItem *item = dialog->allUseFlags->selectedListBox()->item(i);
// 		useFlags += item->text() + " ";
// 	}
// 	
// 	// Get all lines and remove package
// 	QFile file( "/etc/portage/package.use" );
// 	if ( file.open( IO_ReadOnly ) ) {
// 		QTextStream stream( &file );
// 		while ( !stream.atEnd() ) {
// 			QString tmp = stream.readLine();
// 			QString eString = tmp.stripWhiteSpace();
// 			
// 			if ( !tmp.startsWith( category + "/" + package ) && !eString.isEmpty() )
// 				lines += tmp;
// 		}
// 		file.close();
// 		
// 		// Add package with updated use flags
// 		if ( !useFlags.isEmpty() )
// 			lines += category + "/" + package + " " + useFlags;
// 	}
// 	else
// 		kdDebug() << i18n("Error reading: /etc/portage/package.use") << endl;
// 	
// 	// Now write back
// 	if ( file.open( IO_WriteOnly ) ) {
// 		QTextStream stream( &file );
// 		foreach ( lines ) {
// 			stream << *it + "\n";
// 		}
// 		file.close();
// 	}
// 	else
// 		KMessageBox::error( this, i18n("Failed to save. Please run as root." ), i18n("Saving"));
	
	accept();
}

/**
 * Get this version ebuild.
 * @param id
 */
void PackageInspector::slotGetEbuild( const QString& version )
{
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

/**
 * Get this package changelog.
 * @param id
 */
void PackageInspector::getChangeLog()
{
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

/**
 * Get this package dependencies.
 * @param id
 */
void PackageInspector::getDependencies( const QString& version )
{
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

void PackageInspector::getInstalledFiles( const QString& version )
{
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
	else {
		kdDebug() << i18n( "Error reading: " ) << filename << endl;
		dialog->installedFilesBrowser->setText( i18n("<font color=darkGrey><b>Installed files list not found.</b></font>") );
	}
}

#include "packageinspector.moc"
