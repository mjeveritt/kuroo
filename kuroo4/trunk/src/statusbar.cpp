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
 * @class KurooStatusBar
 * @short Singleton object to build statusbar with label and progressbar.
 * 
 * The progressbar is either updated by a timer (eg when emerging packages) or by updated by count (eg when scanning packages).
 * The progressbar is hidden when inactive.
 * The messages in the label can be constant or transient. The label displays transient messages for 2 sec then returns to
 * last constant message if any. The message stack is fifo.
 */
KurooStatusBar::KurooStatusBar( QWidget *parent )
	: KStatusBar( parent ),	statusBarProgress( 0 ), statusBarLabel( 0 )
{
	s_instance = this;

    statusBarProgress = new QProgressBar( this );
    /*statusBarProgress->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0,
                                                   0, 0, statusBarProgress->sizePolicy().hasHeightForWidth() ) );*/
	
    statusBarLabel = new QLabel( "statusBarLabel", this );
    /*statusBarLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)2, (QSizePolicy::SizeType)5,
                                                0, 0, statusBarLabel->sizePolicy().hasHeightForWidth() ) );*/
	
    addWidget( statusBarLabel );
    addWidget( statusBarProgress );
	
    statusBarProgress->setMaximum( 100 );
	statusBarProgress->hide();
	
	// Clock timer for showing progress when emerging packages.
	m_internalTimer = new QTimer( this );
    connect( m_internalTimer, SIGNAL( timeout() ), this, SLOT( slotUpdateTime() ) );
    
	// Progress timer for activities when total duration is not specified.
	m_diffTimer = new QTimer( this );
    connect( m_diffTimer, SIGNAL( timeout() ), this, SLOT( slotAdvance() ) );
}

KurooStatusBar::~KurooStatusBar()
{}

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
	
	if ( !m_messageMap.contains( id ) ) {
		m_messageMap.insert( id, message );
		statusBarLabel->setText( message );
	}
	else {
        m_messageMap.remove( id );
		statusBarLabel->setText( message );
		QTimer::singleShot( 2000, this, SLOT( slotLastMessage() ) );
	}
}

/**
 * View last message.
 */
void KurooStatusBar::slotLastMessage()
{
	QMap<QString, QString>::Iterator it = m_messageMap.end();
	if ( m_messageMap.size() > 0 ) {
		it--;
        statusBarLabel->setText( it.value() );
	}
	else
		statusBarLabel->setText( i18n("Done.") );
}

/**
 * Set total for timer progress.
 */
void KurooStatusBar::setTotalSteps( int total )
{
	kDebug() << "total=" << total << LINE_INFO;
	
    stopTimer();
    statusBarProgress->setTextVisible( true );
    statusBarProgress->setMaximum( total );
	
	if ( total == 0 )
		statusBarProgress->hide();
	else
		if ( !statusBarProgress->isVisible() ) {
			statusBarProgress->show();
    		m_internalTimer->start( 1000 );
			startTimer();
		}
}

void KurooStatusBar::updateTotalSteps( int total )
{
    statusBarProgress->setMaximum( m_timerSteps + total );
    m_diffTimer->stop();
    statusBarProgress->setTextVisible( true );
    disconnect( m_internalTimer, SIGNAL( timeout() ), this, SLOT( slotOneStep() ) );
    connect( m_internalTimer, SIGNAL( timeout() ), this, SLOT( slotOneStep() ) );
}

/**
 * Set total for stepped progress.
 */
void KurooStatusBar::setThreadTotalSteps( int total )
{
    statusBarProgress->setTextVisible( true );
    statusBarProgress->setMaximum( total );
	
	if ( total == 0 )
		statusBarProgress->hide();
	else
		if ( !statusBarProgress->isVisible() )
			statusBarProgress->show();
}

/**
 * View progress.
 * @param steps		in %
 */
void KurooStatusBar::setProgress( int steps )
{
    statusBarProgress->setValue( steps );
}

/**
 * Launch internal timer used when emerging packages.
 */
void KurooStatusBar::startTimer()
{
    connect( m_internalTimer, SIGNAL( timeout() ), this, SLOT( slotOneStep() ) );
	m_timerSteps = 0;
}

/**
 * Stop internal timer.
 */
void KurooStatusBar::stopTimer()
{
    disconnect( m_internalTimer, SIGNAL( timeout() ), this, SLOT( slotOneStep() ) );
	m_diffTimer->stop();
    statusBarProgress->setValue( 0 );
    statusBarProgress->setMaximum( 100 );
    statusBarProgress->setTextVisible( true );
	statusBarProgress->hide();
	DEBUG_LINE_INFO;
}

/**
 * Increase progress by 1 second.
 */
void KurooStatusBar::slotOneStep()
{
    setProgress( m_timerSteps );
    if ( m_timerSteps > statusBarProgress->maximum() ) {
		stopTimer();
		startProgress();
	}
}

void KurooStatusBar::slotUpdateTime()
{
    m_timerSteps++;
}

long KurooStatusBar::elapsedTime()
{
	return m_timerSteps;
}

void KurooStatusBar::clearElapsedTime()
{
    m_diffTimer->stop();
    m_internalTimer->stop();
    m_timerSteps = 0;
}

/**
 * Start relative advance.
 */
void KurooStatusBar::startProgress()
{
	statusBarProgress->show();
    statusBarProgress->setMaximum( 0 );
    statusBarProgress->setTextVisible( false );
	m_diffTimer->start( 1000 );
}

/**
 * Pause Timers
 */
void KurooStatusBar::pauseTimers()
{
	m_diffTimer->stop();
	m_internalTimer->stop();
	statusBarLabel->setText( i18n("Paused...") );
}

/**
 * Unpause Timers
 */
void KurooStatusBar::unpauseTimers()
{
	m_diffTimer->start(1000);
	m_internalTimer->start( 1000 );
}

/**
 * Show relative advance progress.
 */
void KurooStatusBar::slotAdvance()
{
    m_timerSteps++;
    statusBarProgress->setValue( statusBarProgress->value() + 2 );
}

#include "statusbar.moc"
