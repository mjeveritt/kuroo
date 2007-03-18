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

KuroolitoStatusBar* KuroolitoStatusBar::s_instance = 0;

/**
 * @class KuroolitoStatusBar
 * @short Singleton object to build statusbar with label and progressbar.
 * 
 * The progressbar is either updated by a timer (eg when emerging packages) or by updated by count (eg when scanning packages).
 * The progressbar is hidden when inactive.
 * The messages in the label can be constant or transient. The label displays transient messages for 2 sec then returns to
 * last constant message if any. The message stack is fifo.
 */
KuroolitoStatusBar::KuroolitoStatusBar( QWidget *parent )
	: KStatusBar( parent ),	statusBarProgress( 0 ), statusBarLabel( 0 )
{
	s_instance = this;

	statusBarProgress = new KProgress( this, "statusBarProgress" );
	statusBarProgress->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 
	                                               0, 0, statusBarProgress->sizePolicy().hasHeightForWidth() ) );
	
	statusBarLabel = new QLabel( this, "statusBarLabel" );
	statusBarLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)2, (QSizePolicy::SizeType)5, 
	                                            0, 0, statusBarLabel->sizePolicy().hasHeightForWidth() ) );
	
	addWidget( statusBarLabel, 1, 1 );
	addWidget( statusBarProgress, 0, true );
	
	statusBarProgress->setTotalSteps( 100 );
	statusBarProgress->hide();
	
	// Clock timer for showing progress when emerging packages.
	m_internalTimer = new QTimer( this );
    connect( m_internalTimer, SIGNAL( timeout() ), this, SLOT( slotUpdateTime() ) );
    
	// Progress timer for activities when total duration is not specified.
	m_diffTimer = new QTimer( this );
    connect( m_diffTimer, SIGNAL( timeout() ), this, SLOT( slotAdvance() ) );
}

KuroolitoStatusBar::~KuroolitoStatusBar()
{}

/**
 * Set label text in statusbar.
 */
void KuroolitoStatusBar::setProgressStatus( const QString& id, const QString& message )
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
		m_messageMap.erase( id );
		statusBarLabel->setText( message );
		QTimer::singleShot( 2000, this, SLOT( slotLastMessage() ) );
	}
}

/**
 * View last message.
 */
void KuroolitoStatusBar::slotLastMessage()
{
	QMap<QString, QString>::Iterator it = m_messageMap.end();
	if ( m_messageMap.size() > 0 ) {
		it--;
		statusBarLabel->setText( it.data() );
	}
	else
		statusBarLabel->setText( i18n("Done.") );
}

/**
 * Set total for timer progress.
 */
void KuroolitoStatusBar::setTotalSteps( int total )
{
	kdDebug() << "total=" << total << LINE_INFO;
	
    stopTimer();
	statusBarProgress->setTextEnabled( true );
	statusBarProgress->setTotalSteps( total );
	
	if ( total == 0 )
		statusBarProgress->hide();
	else
		if ( !statusBarProgress->isVisible() ) {
			statusBarProgress->show();
    		m_internalTimer->start( 1000 );
			startTimer();
		}
}

void KuroolitoStatusBar::updateTotalSteps( int total )
{
    statusBarProgress->setTotalSteps( m_timerSteps + total );
    m_diffTimer->stop();
    statusBarProgress->setTextEnabled( true );
    disconnect( m_internalTimer, SIGNAL( timeout() ), this, SLOT( slotOneStep() ) );
    connect( m_internalTimer, SIGNAL( timeout() ), this, SLOT( slotOneStep() ) );
}

/**
 * Set total for stepped progress.
 */
void KuroolitoStatusBar::setThreadTotalSteps( int total )
{
	statusBarProgress->setTextEnabled( true );
	statusBarProgress->setTotalSteps( total );
	
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
void KuroolitoStatusBar::setProgress( int steps )
{
	statusBarProgress->setProgress( steps );
}

/**
 * Launch internal timer used when emerging packages.
 */
void KuroolitoStatusBar::startTimer()
{
    connect( m_internalTimer, SIGNAL( timeout() ), this, SLOT( slotOneStep() ) );
	m_timerSteps = 0;
}

/**
 * Stop internal timer.
 */
void KuroolitoStatusBar::stopTimer()
{
    disconnect( m_internalTimer, SIGNAL( timeout() ), this, SLOT( slotOneStep() ) );
	m_diffTimer->stop();
	statusBarProgress->setProgress( 0 );
	statusBarProgress->setTotalSteps( 100 );
	statusBarProgress->setTextEnabled( true );
	statusBarProgress->hide();
	DEBUG_LINE_INFO;
}

/**
 * Increase progress by 1 second.
 */
void KuroolitoStatusBar::slotOneStep()
{
    setProgress( m_timerSteps );
	if ( m_timerSteps > statusBarProgress->totalSteps() ) {
		stopTimer();
		startProgress();
	}
}

void KuroolitoStatusBar::slotUpdateTime()
{
    m_timerSteps++;
}

long KuroolitoStatusBar::elapsedTime()
{
	return m_timerSteps;
}

void KuroolitoStatusBar::clearElapsedTime()
{
    m_diffTimer->stop();
    m_internalTimer->stop();
    m_timerSteps = 0;
}

/**
 * Start relative advance.
 */
void KuroolitoStatusBar::startProgress()
{
    statusBarProgress->show();
	statusBarProgress->setTotalSteps( 0 );
	statusBarProgress->setTextEnabled( false );
	m_diffTimer->start( 1000 );
}

/**
 * Show relative advance progress.
 */
void KuroolitoStatusBar::slotAdvance()
{
    m_timerSteps++;
	statusBarProgress->advance( 2 );
}

#include "statusbar.moc"
