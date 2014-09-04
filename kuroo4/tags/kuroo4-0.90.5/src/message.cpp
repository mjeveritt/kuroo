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

#include <QLabel>
#include <QClipboard>
#include <QScrollBar>

#include <KTextBrowser>
#include <KApplication>

Message* Message::s_instance = 0;

/**
 * @class Message
 * @short Convenience singleton dialog for simple messages to user.
 */
Message::Message( QWidget *parent )
    : KDialog( parent )
{
	setupUi( mainWidget() );
	//i18n("Message"), false, i18n("Message"), , , false
	//enable the OK button and an extra button which we will use to copy the text to the clipboard
	setButtons( Ok | User1 );
	setDefaultButton( Ok );
	s_instance = this;
	setButtonText( User1, i18n("Copy text to clipboard") );
	showButton( User1, true );
	showButton( Cancel, false );

	//setTextFormat is deprecated, but I can't find a replacement
	//messageText->setTextFormat( Qt::LogText );
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
	//store the current scroll position so we can re-apply it after changing the text
	int scrollValue = messageText->verticalScrollBar()->value();
	setCaption( caption );
	setLabel( "<b>" + label + "</b>" );

	QString m_text = text;
	m_text.replace( "<br>", "<br/>");
	//fix issue with the log containing '<script>'
	m_text.replace( "<script>", "&lt;script&gt;");

    messageText->setHtml( m_text );
	//provided the text didn't get shorter since the last time the dialog was opened
	//(this happens if logrotate rotates the log), set the scroll position to the
	//previous position
	if (messageText->verticalScrollBar()->maximum() >= scrollValue)
	{
		messageText->verticalScrollBar()->setValue( scrollValue );
	}
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
