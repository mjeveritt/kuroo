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
	setButtonText( KDialogBase::User1, i18n( "Add selected to Install Queue" ) );
	dialog = new EmergePretendBase( this );
	setMainWidget( dialog );
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
 * Append packages to Queue.
 */
void PretendInspector::slotUser1()
{
	QueueSingleton::Instance()->addPackageIdList( dialog->resultView->selectedId() );
}

#include "pretendinspector.moc"
