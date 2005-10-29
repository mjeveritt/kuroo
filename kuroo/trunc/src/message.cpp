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

#include "message.h"
#include "messagebase.h"

#include <qlabel.h>

#include <ktextbrowser.h>
#include <klocale.h>
#include <kdebug.h>

Message* Message::s_instance = 0;

/**
 * @class Message
 * @short Convenience singleton dialog for simple messages to user.
 */
Message::Message( QWidget *parent, const char *name )
	: KDialogBase( parent, name, false ), base(0)
{
	s_instance = this;
	base = new MessageBase(this);
	showButtonApply(false);
	showButtonCancel(false);
	setMainWidget(base);
}

Message::~Message()
{
}

bool Message::prompt( const QString& caption, const QString& label, const QStringList& text )
{
	setCaption( caption );
	setLabel( label );
	setText( text );
	setInitialSize( QSize(600, 300) );
	show();
}

void Message::setText( const QStringList& text )
{
	base->messageText->setText( text.join("\n") );
}

void Message::setLabel( const QString& label )
{
	base->messageLabel->setText( label );
}

#include "message.moc"
