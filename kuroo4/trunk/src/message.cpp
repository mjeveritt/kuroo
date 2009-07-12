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
#include "message.h"
#include "ui_messagebase.h"

#include <qlabel.h>
#include <qclipboard.h>

#include <ktextbrowser.h>
#include <kapplication.h>

Message* Message::s_instance = 0;

/**
 * @class Message
 * @short Convenience singleton dialog for simple messages to user.
 */
Message::Message( QWidget *parent )
    : KDialog( parent ), m_text( QString::null )
{
    setupUi( mainWidget() );
    //i18n("Message"), false, i18n("Message"), KDialogBase::Ok | KDialogBase::User1, KDialogBase::Ok, false
	s_instance = this;
    setButtonText( KDialog::User1, i18n("Copy text to clipboard") );
    //showButtonCancel( false );
	
// 	base->messageText->setTextFormat( Qt::LogText );
}
Message::~Message() {}

/**
 * Show dialog with message content.
 * @param caption
 * @param label
 * @param text	the message
 */
void Message::prompt( const QString& caption, const QString& label, const QString& text )
{
	m_text = text;
	setCaption( caption );
	setLabel( "<b>" + label + "</b>" );
	
	QString m_text = text;
	m_text.replace( "<br>", "\x000a" );
	
    messageText->setText( text );
	setInitialSize( QSize(600, 300) );
	show();
}

void Message::setLabel( const QString& label )
{
    messageLabel->setText( label );
}

void Message::slotUser1()
{
	QClipboard *cb = QApplication::clipboard();
	cb->setText( m_text, QClipboard::Clipboard );
}

#include "message.moc"
