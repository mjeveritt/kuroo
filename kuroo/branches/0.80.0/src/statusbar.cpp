/***************************************************************************
 *   Copyright (C) 2005 by Karye   *
 *   karye@users.sourceforge.net   *
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
#include "statusbar.h"

#include <qlabel.h>

#include <qtimer.h>

KurooStatusBar* KurooStatusBar::s_instance = 0;

/**
 * Singleton object to build statusbar with progressbar in right corner.
 * The one and only instance created from this class can can be accessed from all classes.
 */
KurooStatusBar::KurooStatusBar( QWidget *parent )
: KStatusBar( parent ),	statusBarProgress( 0 ), statusBarLabel( 0 )
{
	s_instance = this;
	statusBarProgress = new KProgress( 0, "statusBarProgress" );
	statusBarLabel = new QLabel( 0, "statusBarLabel" );
	statusBarLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)2, (QSizePolicy::SizeType)5, 0, 0, statusBarLabel->sizePolicy().hasHeightForWidth() ) );
	addWidget( statusBarLabel, 1, 1 );
	addWidget( statusBarProgress, 0, true );
	
	setTotalSteps( 100 );
	
	// Clock timer for showing progress when emerging packages.
	internalTimer = new QTimer( this );
	connect( internalTimer, SIGNAL( timeout() ), SLOT( slotOneStep() ) );
	
	// Progress timer for activities when total duration is not specified.
	diffTimer = new QTimer( this );
	connect( diffTimer, SIGNAL( timeout() ), SLOT( slotAdvance() ) );
}

KurooStatusBar::~KurooStatusBar()
{
}

/**
 * Set label text in statusbar.
 */
void KurooStatusBar::setProgressStatus( const QString& id, const QString& message )
{
	if ( id.isEmpty() ) {
		statusBarLabel->setText( message );
		QTimer::singleShot( 2000, this, SLOT( slotLastMessage() ) );
		return;
	}
	
	if ( !messageMap.contains( id ) ) {
		messageMap.insert( id, message );
		statusBarLabel->setText( message );
	}
	else {
		messageMap.erase( id );
		statusBarLabel->setText( message );
		QTimer::singleShot( 2000, this, SLOT( slotLastMessage() ) );
	}
}

/**
 * View last message.
 */
void KurooStatusBar::slotLastMessage()
{
	QMap<QString, QString>::Iterator it = messageMap.end();
	if ( messageMap.size() > 0 ) {
		it--;
		statusBarLabel->setText( it.data() );
	}
	else
		statusBarLabel->setText( i18n("Done.") );
}

/**
 * Set total.
 */
void KurooStatusBar::setTotalSteps( int total )
{
	statusBarProgress->setTotalSteps( total );
}

/**
 * View progress.
 * @param steps		in %
 */
void KurooStatusBar::setProgress( int steps )
{
	statusBarProgress->setProgress( steps );
}

/**
 * Launch internal timer used when emerging packages.
 */
void KurooStatusBar::startTimer()
{
	internalTimer->start( 1000 );
	timerSteps = 0;
}

/**
 * Stop internal timer.
 */
void KurooStatusBar::stopTimer()
{
	internalTimer->stop();
	diffTimer->stop();
	setProgress( 0 );
	setTotalSteps( 100 );
	statusBarProgress->setTextEnabled( true );
}

/**
 * Increase progress by 1 second.
 */
void KurooStatusBar::slotOneStep()
{
	setProgress( timerSteps++ );
	
	if ( timerSteps > statusBarProgress->totalSteps() ) {
		stopTimer();
		startProgress();
	}
}

/**
 * Start relative advance.
 */
void KurooStatusBar::startProgress()
{
	setTotalSteps( 0 );
	statusBarProgress->setTextEnabled(false);
	diffTimer->start( 1000 );
}

/**
 * Show relative advance progress.
 */
void KurooStatusBar::slotAdvance()
{
	statusBarProgress->advance( 2 );
}

#include "statusbar.moc"
