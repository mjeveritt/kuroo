// From amaroK
// Author: Max Howell (C) Copyright 2004
// Copyright: See COPYING file that comes with this distribution
//

// the asserts we use in this module prevent crashes, so best to abort the application if they fail
#define QT_FATAL_ASSERT
#define DEBUG_PREFIX "ThreadWeaver"

#include <QApplication>
#include <QRegExp>
#include <QFileInfo>
#include <QDir>
#include <QCustomEvent>
#include <QEvent>

#include <kcursor.h>
#include <kdebug.h>

#include "common.h"
#include "statusbar.h"
#include "threadweaver.h"

QRegExp ThreadWeaver::DependentJob::rxAtom = QRegExp(	
		"^"    									// Start of the string
		"(!)?" 									// "Block these packages" flag, only occurring in ebuilds
		"(~|(?:<|>|=|<=|>=))?" 							// greater-than/less-than/equal, or "all revisions" prefix
		"((?:[a-z]|[0-9])+)-((?:[a-z]|[0-9])+)/"   				// category and subcategory
		"((?:[a-z]|[A-Z]|[0-9]|-(?=\\d+dpi)|-(?!\\d)|\\+|_)+)" 			// package name
		"("           								// start of the version part
		"(?:-\\d*(?:\\.\\d+)*[a-z]?)" 						// base version number, including wildcard version matching (*)
		"(?:_(?:alpha|beta|pre|rc|p)\\d*)?" 					// version suffix
		"(?:-r\\d*)?"  								// revision
		"\\*?)?$"          							// end of the (optional) version part and the atom string
	);
/**
 * @class ThreadWeaver
 * @short For creating threaded objects.
 */
ThreadWeaver::ThreadWeaver()
{
// 	kDebug() << "ThreadWeaver::ThreadWeaver " << LINE_INFO;
    startTimer( 60 * 1000 ); // prunes the thread pool every 5 minutes
}

ThreadWeaver::~ThreadWeaver()
{
	for ( ThreadList::Iterator it = m_threads.begin(), end = m_threads.end(); it != end; ++it ) {
// 		kDebug() << "ThreadWeaver::~ThreadWeaver Waiting on thread..." << LINE_INFO;
		(*it)->wait();
// 		kDebug() << "ThreadWeaver::~ThreadWeaver finished" << LINE_INFO;
	}
}

uint ThreadWeaver::jobCount( const QString &name )
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

    const QString name = jobs.front()->name();
    const int count = jobCount( name );

	if ( count == jobs.count() )
		gimmeThread()->runJob( jobs.front() );
	
	return count;
}

void ThreadWeaver::onlyOneJob( Job *job )
{
    const QString name = job->name();

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

int ThreadWeaver::abortAllJobsNamed( const QString &name )
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
        if ( !(*it)->isRunning() && (*it)->job() == 0 )
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
        const QString& name = job->name();
		Thread *thread = job->m_thread;

		QApplication::postEvent(
				ThreadWeaver::instance(),
                new QEvent( (QEvent::Type)RestoreOverrideCursorEvent ) );

		if ( !job->isAborted() ) {
// 			kDebug() << "Job completed" << ": " << name << LINE_INFO;
			job->completeJob();
		}
		else 
			kWarning(0) << "Job aborted" << ": " << name << LINE_INFO;

        m_jobs.removeAll( job );
// 		kDebug() << "Jobs pending: " << jobCount( name ) << LINE_INFO;

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
// 		kDebug() << "Threads in pool: " << m_threads.count() << LINE_INFO;

// 		for( ThreadList::Iterator it = m_threads.begin(), end = m_threads.end(); it != end; ++it )
// 			if ( (*it)->readyForTrash() ) {
// 				m_threads.remove( it );
// 				delete *it;
// 				break; // only delete 1 thread every 5 minutes
// 			}
		break;

	case OverrideCursorEvent:
		QApplication::setOverrideCursor( Qt::BusyCursor );
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
        Q_ASSERT( isFinished() );
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
                new QEvent( (QEvent::Type)OverrideCursorEvent ) );
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
class ProgressEvent : public QEvent {
public:
	ProgressEvent( int progress )
            : QEvent( (QEvent::Type)30303 )
			, progress( progress ) {}

	const int progress;
};



/// @class ThreadWeaver::Job
ThreadWeaver::Job::Job( const QString& name )
        : QEvent( (QEvent::Type)JobEvent )
		, m_name( name )
		, m_thread( 0 )
		, m_percentDone( 0 )
		, m_progressDone( 0 )
		, m_totalSteps( 1 ) // no divide by zero
{}

ThreadWeaver::Job::~Job()
{
    if ( m_thread->isRunning() && m_thread->job() == this )
		kWarning(0) << "Deleting a job before its thread has finished with it!" << LINE_INFO;
}

void ThreadWeaver::Job::setProgressTotalSteps( uint steps )
{
	if ( steps == 0 ) {
		//This isn't really an error, just a special case
		//kWarning(0) << "You can't set steps to 0!" << LINE_INFO;
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

void ThreadWeaver::Job::customEvent( QEvent *e )
{
	int progress = static_cast<ProgressEvent*>(e)->progress;
	
	switch( progress ) {
		case -3:
			KurooStatusBar::instance()->setProgressStatus( m_id, m_status );
			break;
	
		case -2:
			KurooStatusBar::instance()->setThreadTotalSteps( 0 );
			break;
		
		case -1:
			KurooStatusBar::instance()->setThreadTotalSteps( 100 );
			break;
	
		default:
			KurooStatusBar::instance()->setProgress( progress );
	}
}

ThreadWeaver::DependentJob::DependentJob( QObject *dependent, const char *name )
	: Job( name ), m_dependent( dependent )
{
	connect( dependent, SIGNAL(destroyed()), SLOT(abort()) );

    QApplication::postEvent( dependent, new QEvent( (QEvent::Type)JobStartedEvent ) );
}

bool ThreadWeaver::DependentJob::mergeDirIntoFile( QString dirPath ) {
	DEBUG_LINE_INFO;
	QDir mergeDir( dirPath );
	//TODO make sure this doesn't exist before we enter
	QFile tempFile( dirPath + ".temp" );
    QTextStream tempStream( &tempFile );
	if( !tempFile.open( QIODevice::WriteOnly ) ) {
        kDebug(0) << "Opened " << tempFile.fileName() << " for writing." << LINE_INFO;
		//TODO handle failure
		return false;
	}

    QList<QFileInfo> infos = mergeDir.entryInfoList();
	QStringList lines;
    foreach( QFileInfo fi, infos ) {
        kDebug(0) << "Processing " << fi.filePath() << LINE_INFO;
        if( fi.isDir() ) {
            kDebug(0) << fi.filePath() << " is a dir." << LINE_INFO;
            if( fi.filePath().endsWith( "/." ) || fi.filePath().endsWith( "/.." ) ) {
                kDebug(0) << fi.filePath() << " is ., skipping." << LINE_INFO;
			} else {
                kDebug(0) << "Would recurse into " << fi.filePath() << LINE_INFO;
				//TODO handle failure
                if( !mergeDirIntoFile( fi.filePath() ) ) {
					return false;
				}
			}
		}

        QFile entryFile( fi.absoluteFilePath() );
        QTextStream streamFile( &entryFile );
		if ( !entryFile.open( QIODevice::ReadOnly ) ) {
            kError(0) << "Parsing " << fi.filePath() << LINE_INFO;
		} else {
			while ( !streamFile.atEnd() )
				lines += streamFile.readLine();
			entryFile.close();
		}

		//Save the file as we go
        foreach( QString line, lines ) {
            tempStream << line << "\n";
		}

		if( !entryFile.remove() ) {
			//TODO handle failure
			return false;
		}
	}
	tempFile.close();
	//By the time we get out of here the directory should be empty, or else. . .
	if( mergeDir.rmdir( dirPath ) ) {
		//TODO handle failure
		return false;
	}

	//And write the new file in it's place
	KIO::file_move( dirPath + ".temp", dirPath, -1, KIO::Overwrite | KIO::HideProgressInfo );
	return true;
}

void ThreadWeaver::DependentJob::completeJob()
{
	//synchronous, so we don't get deleted twice
	QApplication::sendEvent( m_dependent, this );
}

#include "threadweaver.moc"
#undef QT_FATAL_ASSERT //enable-final
