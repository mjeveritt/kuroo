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
#include "filewatcher.h"

#include <kdirwatch.h>
#include <kmessagebox.h>

/**
 * @class FileWatcher.
 * @short Check important portage files for changes.
 */
FileWatcher::FileWatcher( QObject *parent )
	: QObject( m_parent ), watcher( 0 )
{
}

FileWatcher::~FileWatcher()
{
	delete watcher;
	watcher = 0;
}

void FileWatcher::init( QObject *parent )
{
	m_parent = parent;
	watcher = new KDirWatch( this );
	watcher->addDir( KurooConfig::dirDbPkg() + "/sys-apps" );
	connect( watcher, SIGNAL( dirty( const QString& ) ), this, SLOT( slotChanged( const QString& ) ) );
}

/**
 * Check for changes of portage version.
 * @param the new package
 */
void FileWatcher::slotChanged( const QString& package )
{
	kdDebug() << "FileWatcher::slotCreated package=" << package << endl;
	
	QString packageName = package.section( "/", 1 ).section( rxPortageVersion, 0, 0 );
	if ( packageName == "portage" ) {
		
		if ( package.section( "portage-", 1, 1 ).startsWith( "2.1" ) ) {
			KurooConfig::setPortageVersion21( true );
			KMessageBox::sorry( 0, i18n("Portage version is upgraded to 2.1.\n"
			                            "Please refresh package view."), i18n("Portage version") );
		}
	
	}
}

#include "filewatcher.moc"
