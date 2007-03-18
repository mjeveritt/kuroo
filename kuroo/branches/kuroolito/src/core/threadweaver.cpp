// From amaroK
// Author: Max Howell (C) Copyright 2004
// Copyright: See COPYING file that comes with this distribution
//

// the asserts we use in this module prevent crashes, so best to abort the application if they fail
#define QT_FATAL_ASSERT
#define DEBUG_PREFIX "ThreadWeaver"

#include "common.h"
#include "statusbar.h"
#include "threadweaver.h"

#include <qapplication.h>

#include <kcursor.h>
#include <kdebug.h>


/**
 * @class ThreadWeaver
 * @short For creating threaded objects.
 */
ThreadWeaver::ThreadWeaver()
{
// 	kdDebug() << "ThreadWeaver::ThreadWeaver " << LINE_INFO;
    startTimer( 60 * 1000 ); // prunes the thread pool every 5 minutes
}

ThreadWeaver::~ThreadWeaver()
{
	for ( ThreadList::Iterator it = m_threads.begin(), end = m_threads.end(); it != end; ++it ) {
// 		kdDebug() << "ThreadWeaver::~ThreadWeaver Waiting on thread..." << LINE_INFO;
		(*it)->wait();
// 		kdDebug() << "ThreadWeaver::~ThreadWeaver finished" << LINE_INFO;
	}
}

uint ThreadWeaver::jobCount( const QCString &name )
{
	uint count = 0;

	for ( JobList::Iterator it = m_jobs.begin(), end = m_jobs.end(); it != end; ++it )
		if ( name == (*it)->name() )
			count++;

	return count;
}

int ThreadWeaver::queueJob( Job *job )
{
	if ( !job )
		return -1;

	// this list contains all pending and running jobs
	m_jobs += job;

	const uint count = jobCount( job->name() );

	if ( count == 1 )
		gimmeThread()->runJob( job );

	return count;
}

int ThreadWeaver::queueJobs( const JobList &jobs )
{
	if ( jobs.isEmpty() )
		return -1;

	m_jobs += jobs;

	const QCString name = jobs.front()->name();
	const uint count = jobCount( name );

	if ( count == jobs.count() )
		gimmeThread()->runJob( jobs.front() );
	
	return count;
}

void ThreadWeaver::onlyOneJob( Job *job )
{
	const QCString name = job->name();

	// first cause all current jobs with this name to be aborted
	abortAllJobsNamed( name );

	// now queue this job.
	// if there is a running Job of its type this one will be
	// started when that one returns to the GUI thread.
	m_jobs += job;

	// if there weren't any jobs of this type running, we must
	// start this job.
	if ( jobCount( name ) == 1 )
		gimmeThread()->runJob( job );
}

int ThreadWeaver::abortAllJobsNamed( const QCString &name )
{
	int count(0);

	for ( JobList::Iterator it = m_jobs.begin(), end = m_jobs.end(); it != end; ++it )
		if ( name == (*it)->name() ) {
			count++;
			(*it)->abort();
		}

	return count;
}

ThreadWeaver::Thread* ThreadWeaver::gimmeThread()
{
	for ( ThreadList::ConstIterator it = m_threads.begin(), end = m_threads.end(); it != end; ++it )
		if ( !(*it)->running() && (*it)->job() == 0 )
			return *it;

	Thread *thread = new Thread;
	m_threads += thread;
	return thread;
}

bool ThreadWeaver::event( QEvent *e )
{
	switch( e->type() )
	{
	case JobEvent: {
		Job *job = (Job*)e;
		const QCString name = job->name();
		Thread *thread = job->m_thread;

		QApplication::postEvent(
				ThreadWeaver::instance(),
				new QCustomEvent( ThreadWeaver::RestoreOverrideCursorEvent ) );

		if ( !job->isAborted() ) {
// 			kdDebug() << "Job completed" << ": " << name << LINE_INFO;
			job->completeJob();
		}
		else 
			kdWarning(0) << "Job aborted" << ": " << name << LINE_INFO;

		m_jobs.remove( job );
// 		kdDebug() << "Jobs pending: " << jobCount( name ) << LINE_INFO;

		for( JobList::ConstIterator it = m_jobs.begin(), end = m_jobs.end(); it != end; ++it )
			if ( name == (*it)->name() ) {
				thread->runJob( (*it) );
				return true;
			}
	
		// this thread is done
		thread->m_job = 0;
	
		break;
	}

	case QEvent::Timer:
// 		kdDebug() << "Threads in pool: " << m_threads.count() << LINE_INFO;

// 		for( ThreadList::Iterator it = m_threads.begin(), end = m_threads.end(); it != end; ++it )
// 			if ( (*it)->readyForTrash() ) {
// 				m_threads.remove( it );
// 				delete *it;
// 				break; // only delete 1 thread every 5 minutes
// 			}
		break;

	case OverrideCursorEvent:
		QApplication::setOverrideCursor( KCursor::workingCursor() );
		break;

	case RestoreOverrideCursorEvent:
		QApplication::restoreOverrideCursor();
		break;

	default:
		return false;
	}

	return true;
}



/// @class ThreadWeaver::Thread
ThreadWeaver::Thread::Thread()
	: QThread()
{}

ThreadWeaver::Thread::~Thread()
{
	Q_ASSERT( finished() );
}

void ThreadWeaver::Thread::runJob( Job *job )
{
	job->m_thread = this;

	if ( job->isAborted() )
		QApplication::postEvent( ThreadWeaver::instance(), job );
	else {
		m_job = job;
//         start( Thread::IdlePriority ); //will wait() first if necessary
		start( Thread::HighestPriority );
		
		QApplication::postEvent(
				ThreadWeaver::instance(),
				new QCustomEvent( ThreadWeaver::OverrideCursorEvent ) );
	}
}

void ThreadWeaver::Thread::run()
{
	// BE THREAD-SAFE!

	m_job->m_aborted |= !m_job->doJob();

	if ( m_job )
		QApplication::postEvent( ThreadWeaver::instance(), m_job );

	// almost always the thread doesn't finish until after the
	// above event is already finished processing
}



/// @class ProgressEvent
/// @short Used by ThreadWeaver::Job internally
class ProgressEvent : public QCustomEvent {
public:
	ProgressEvent( int progress )
			: QCustomEvent( 30303 )
			, progress( progress ) {}

	const int progress;
};



/// @class ThreadWeaver::Job
ThreadWeaver::Job::Job( const char *name )
		: QCustomEvent( ThreadWeaver::JobEvent )
		, m_name( name )
		, m_thread( 0 )
		, m_percentDone( 0 )
		, m_progressDone( 0 )
		, m_totalSteps( 1 ) // no divide by zero
{}

ThreadWeaver::Job::~Job()
{
	if ( m_thread->running() && m_thread->job() == this )
		kdWarning(0) << "Deleting a job before its thread has finished with it!" << LINE_INFO;
}

void ThreadWeaver::Job::setProgressTotalSteps( uint steps )
{
	if ( steps == 0 ) {
		kdWarning(0) << "You can't set steps to 0!" << LINE_INFO;
		QApplication::postEvent( this, new ProgressEvent( -2 ) );
		steps = 1;
	}
	else
		QApplication::postEvent( this, new ProgressEvent( -1 ) );
	
	m_totalSteps = steps;
}

void ThreadWeaver::Job::setProgress( uint steps )
{
	m_progressDone = steps;

	uint newPercent = uint( (100 * steps) / m_totalSteps);

	if ( newPercent != m_percentDone ) {
		m_percentDone = newPercent;
		QApplication::postEvent( this, new ProgressEvent( newPercent ) );
	}
}

void ThreadWeaver::Job::setStatus( const QString& id, const QString& status )
{
	m_id = id;
	m_status = status;

	QApplication::postEvent( this, new ProgressEvent( -3 ) );
}

void ThreadWeaver::Job::incrementProgress()
{
	setProgress( m_progressDone + 1 );
}

void ThreadWeaver::Job::customEvent( QCustomEvent *e )
{
	int progress = static_cast<ProgressEvent*>(e)->progress;
	
	switch( progress ) {
		case -3:
			KuroolitoStatusBar::instance()->setProgressStatus( m_id, m_status );
			break;
	
		case -2:
			KuroolitoStatusBar::instance()->setThreadTotalSteps( 0 );
			break;
		
		case -1:
			KuroolitoStatusBar::instance()->setThreadTotalSteps( 100 );
			break;
	
		default:
			KuroolitoStatusBar::instance()->setProgress( progress );
	}
}

ThreadWeaver::DependentJob::DependentJob( QObject *dependent, const char *name )
	: Job( name ), m_dependent( dependent )
{
	connect( dependent, SIGNAL(destroyed()), SLOT(abort()) );

	QApplication::postEvent( dependent, new QCustomEvent( JobStartedEvent ) );
}

void ThreadWeaver::DependentJob::completeJob()
{
	//synchronous, so we don't get deleted twice
	QApplication::sendEvent( m_dependent, this );
}

#include "threadweaver.moc"
#undef QT_FATAL_ASSERT //enable-final
