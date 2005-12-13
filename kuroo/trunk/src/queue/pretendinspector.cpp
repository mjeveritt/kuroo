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

#include "pretendinspector.h"
#include "resultlistview.h"
#include "common.h"

#include <qregexp.h>

#include <kactionselector.h>
#include <ktextbrowser.h>
#include <kmessagebox.h>
#include <kuser.h>

/**
 * Specialized dialog for editing Use Flags per package.
 */
PretendInspector::PretendInspector( QWidget *parent, const char *name )
	: KDialogBase( KDialogBase::Swallow, i18n( "Emerge pretend result" ), KDialogBase::User1 | KDialogBase::Ok , KDialogBase::Ok, parent, i18n( "Save" ), false )
{
	setButtonText( KDialogBase::User1, i18n( "Append all to Install Queue" ) );
	dialog = new EmergePretendBase( this );
	setMainWidget( dialog );
	
// 	if ( !KUser().isSuperUser() )
// 		enableButtonApply( false );
// 	
// 	QString useFile( KurooConfig::dirPortage() + "/profiles/use.desc" );
// 	QFile f( useFile );
// 	
// 	if ( f.open( IO_ReadOnly ) ) {
// 		QTextStream stream( &f );
// 		
// 		while ( !stream.atEnd() ) {
// 			QString line = stream.readLine();
// 			if ( !line.startsWith( "#" ) && !line.isEmpty() ) {
// 				if ( !line.contains( QRegExp( "^alpha|^amd64|^arm|^hppa|^ia64|^mips|^ppc|^ppc64|^ppc-macos|^s390|^sh|^sparc|^x86" ) ) )
// 					useList += line;
// 			}
// 		}
// 		f.close();
// 		
// 		foreach ( useList ) {
// 			dialog->allUseFlags->availableListBox()->insertItem( "-" + ( *it ).section( " -", 0, 0 ) );
// 		}
// 		foreach ( useList ) {
// 			dialog->allUseFlags->availableListBox()->insertItem( ( *it ).section( " -", 0, 0 ) );
// 		}
// 		
// 		connect( dialog->allUseFlags->availableListBox(), SIGNAL( currentChanged( QListBoxItem* ) ), this, SLOT( slotUseDescription( QListBoxItem* ) ) );
// 		connect( dialog->allUseFlags->selectedListBox(), SIGNAL( currentChanged( QListBoxItem* ) ), this, SLOT( slotUseDescription( QListBoxItem* ) ) );
// 	}
// 	else
// 		kdDebug() << i18n( "Error reading: " ) << useFile << endl;
}

PretendInspector::~PretendInspector()
{
}

void PretendInspector::showResults()
{
	if ( dialog->resultView->loadFromDB() )
		show();
}

/**
 * Save the new use flags created with ActionSelector
 */
void PretendInspector::slotOk()
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
// 			if ( !tmp.startsWith( package ) && !eString.isEmpty() )
// 				lines += tmp;
// 		}
// 		file.close();
// 		
// 		// Add package with updated use flags
// 		if ( !useFlags.isEmpty() )
// 			lines += package + " " + useFlags;
// 	}
// 	else
// 		kdDebug() << i18n("Error reading: /etc/portage/package.use") << endl;
// 	
// 	// Now write back
// 	if ( file.open( IO_WriteOnly ) ) {
// 		QTextStream stream( &file );
// 		for ( QStringList::Iterator it = lines.begin(); it != lines.end(); ++it ) {
// 			stream << *it + "\n";
// 		}
// 		file.close();
// 	}
// 	else
// 		KMessageBox::error( this, i18n("Failed to save. Please run as root." ), i18n("Saving"));
// 	
	accept();
}

#include "pretendinspector.moc"
