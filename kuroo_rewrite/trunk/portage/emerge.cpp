#include <signal.h>

#include <qtextcodec.h>
#include <QProgressBar>
#include <QLabel>

#include <kmessagebox.h>
#include <kuser.h>
#include <kdeversion.h>
#include <KLocale>
#include <KDebug>

#include "config.h"
#include "emerge.h"
#include "portagedb.h"
#include "portage.h"

/**
 * @class Emerge
 * @short All Gentoo emerge command.
 * 
 * Handles emerge, unmerge, check-for-updates, sync...
 */
Emerge::Emerge() :
        m_completedFlag( false ), m_backupComplete( false ), m_backingUp( false ),
        m_importantMessagePackage( QString::null ), m_packageMessage( QString::null )
{
    eProc = new KProcess();
}

Emerge::~Emerge()
{
	delete eProc;
	eProc = 0;
}


/**
 * Send text to stdIn.
 * @param text
 */
void Emerge::inputText( const QString& text )
{
    if ( eProc->state() == QProcess::Running ) {
        eProc->write( text.toAscii() );
        //kuroo->log->emerge( text );
    } else {
        //kuroo->log->error( i18n("Can not process input! Emerge is not running.") );
    }
}

/**
 * Abort the emerge process.
 * @return success
 */
bool Emerge::stop()
{
    if ( eProc->state() == QProcess::Running && kill( eProc->pid(), 9 /*FIXME: magic number*/ ) ) {
		kWarning(0) << "Emerge process killed!";
		return true;
	}
	else
		return false;
}

/**
 * Checks if emerge is running.
 * @return true if emerging, false otherwise.
 */
bool Emerge::isRunning() const { return eProc->state() == QProcess::Running; }

/**
 * Pause the eproc
 */
void Emerge::slotPause()
{
    if ( !eProc->state() == QProcess::Running || !m_pausable ) {
        //kuroo->log->error( i18n("\nEmerge is not running or cannot be paused...") );
	}
	else {
        ////kuroo->statusBar->pauseTimers();
        //kuroo->queue->pauseEmerge();
        kill( eProc->pid(), SIGSTOP );
		m_isPaused = true;
	}
}

/**
 * Unpause the eproc
 */
void Emerge::slotUnpause()
{
	if ( !m_isPaused ) {
        //kuroo->log->error( i18n("\nEmerge is not paused...") );
	}
    //kuroo->status->setText( i18n("Installing packages in queue...") );
    ////kuroo->statusBar->unpauseTimers();
    //kuroo->queue->unpauseEmerge();
    kill( eProc->pid(), SIGCONT );
	m_isPaused = false;
}

bool Emerge::emerge( const QStringList& args, QString /*status*/, const QStringList& packageList ) {
    m_blocks.clear();
    m_importantMessage = QString::null;
    m_unmasked = QString::null;
    m_lastEmergeList = packageList;
    m_etcUpdateCount = 0;
    m_emergePackageList.clear();
    
    *eProc << args << packageList;
    eProc->start();
    connect( eProc, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotEmergeOutput() ) );
    connect( eProc, SIGNAL( finished() ), this, SLOT( slotCleanup() ) );

    if ( !eProc->state() == QProcess::Running ) {
        //kuroo->log->error( i18n("\nError: Emerge didn't start. ") );
        slotDone();
        return false;
    } else {
        //emit busy( status );
        return true;
    }
}

/**
 * Quickpkg something
 * @param packageList
 * @return success
 */
bool Emerge::quickpkg( const QStringList& packageList )
{
    QStringList args("quickpkg");
    return emerge( args, i18n("Building binary packages"), packageList );
}

/**
 * Emerge list of packages.
 * @param packageList
 * @return success
 */
bool Emerge::queue( const QStringList& packageList )
{
    if( Config::backupPkg() && !m_backupComplete ) {
        m_backingUp = true;
        return quickpkg( packageList );
    } else {
        QStringList args("emerge");
        args << "--nospinner" << "--columns" << "--color=n";
        return emerge( args, i18n("Installing packages in queue"), packageList );
    }
}

/**
 * Emerge pretend list of packages.
 * @param packageList	
 * @return success
 */
bool Emerge::pretend( const QStringList& packageList )
{
    QStringList args("emerge");
    args << "--nospinner" << "--color=n" << "--columns" << "-pv";
    if( Config::updateBuilddeps() )
        args << "--with-bdeps=y";
    emerge( args, i18n("Checking installation queue..."), packageList );
    return true;
}

/**
 * Unmerge list of packages.
 * @param category
 * @param packageList	
 * @return success
 */
bool Emerge::unmerge( const QStringList& packageList )
{
    QStringList args("emerge");
    args << "--unmerge" << "--color=n" << "--nospinner";
    emerge( args, i18n("Uninstalling packages..."), packageList );
    return true;
}

/**
 * Synchronize Portage tree.
 * @return success
 */
bool Emerge::sync()
{
    QStringList args("emerge");
    args << "--sync" << "--quiet" << "--color=n" << "--nospinner";
    emerge( args, i18n("Synchronizing portage tree...") );
    //kuroo->progress->setMaximum( //kuroo->db->getKurooDbMeta( "syncDuration" ).toInt() );
    return true;
}

/**
 * Check for updates of world and system.
 * @return success
 */
bool Emerge::checkUpdates()
{
    QStringList args("emerge");
    args << "-pvu" << "--color=n" << "--nospinner" << "--columns" << "world";
	
    if ( Config::updateDeep() )
        args << "-D";
    if( Config::updateNewUse() )
        args << "-N";
    if( Config::updateBuilddeps() )
        args << "--with-bdeps=y";

    emerge( args, i18n("Checking for package updates...") );
    return true;
}

/**
 * Regexp to parse emerge output.
 */
const QRegExp Emerge::rxEmerge()
{
    if ( Config::portageVersion21() )
        return QRegExp( "^\\[ebuild([\\s\\w]*)\\]\\s+"
                        "((\\S+)/(\\S+))"
                        "(?:\\s*\\[([^\\]]*)\\])?"
                        "(?:\\s*\\[([^\\]]*)\\])?"
                        "(?:\\s*USE=\"([^\"]*)\")?"
                        "(?:\\s*LINGUAS=\"(?:[^\"]*)\")?"
                        "(?:\\s(\\d*,?\\d*)\\skB)?" );
    else
        return QRegExp( "^\\[ebuild([\\s\\w]*)\\]\\s+"
                        "((\\S+)/(\\S+))"
                        "(?:\\s*\\[([^\\]]*)\\])?"
                        "(?:\\s*\\[([^\\]]*)\\])?"
                        "((?:\\s*[\\(\\-\\+]+\\w+[\\)%]?)*)"
                        "(?:\\s(\\d*,?\\d*)\\skB)?" );
}


/**
 * Parse emerge process output for messages and packages.
 * @param proc	
 */

void Emerge::slotEmergeOutput()
{
	QString line;	
    QRegExp rxPackage = rxEmerge();
    while( !( line = eProc->readLine() ).isEmpty() ) {
		int logDone( 0 );
		
		////////////////////////////////////////////////////////////////////////////////
		// Cleanup emerge output - remove damn escape sequences
		////////////////////////////////////////////////////////////////////////////////
		line.replace( QRegExp("\\x0007"), "\n" );
		int pos( 0 );
		QRegExp rx( "(\\x0008)|(\\x001b\\[32;01m)|(\\x001b\\[0m)|(\\x001b\\[A)|(\\x001b\\[73G)|"
		            "(\\x001b\\[34;01m)|(\\x001b\\]2;)|(\\x001b\\[39;49;00m)|(\\x001b\\[01m.)" );
                while ( ( pos = rx.indexIn(line) ) != -1 )
			line.replace( pos, rx.matchedLength(), QString::null );
		
		if ( line.isEmpty() )
			continue;
		
// 		kDebug() << "line=" << line;
		
		////////////////////////////////////////////////////////////////////////////
		// Parse out package and info
		////////////////////////////////////////////////////////////////////////////
        if ( rxPackage.indexIn( line ) > -1 ) {
			EmergePackage emergePackage;
			emergePackage.updateFlags = rxPackage.cap(1);
			emergePackage.package = rxPackage.cap(2);
			emergePackage.category = rxPackage.cap(3);
			emergePackage.name = rxPackage.cap(4);
			emergePackage.version = rxPackage.cap(5);
			emergePackage.installedVersion = rxPackage.cap(6);
			emergePackage.useFlags = rxPackage.cap(7).simplified();
			emergePackage.size = rxPackage.cap(8);
			m_emergePackageList.prepend( emergePackage );
		}
		
		////////////////////////////////////////////////////////////////////////
		// Parse emerge output for correct log output
		////////////////////////////////////////////////////////////////////////
		QString lineLower = line.toLower();
		if ( lineLower.contains( QRegExp("^>>>|^!!!") ) ) {
			if ( lineLower.contains( QRegExp("^>>> completed installing") ) ) {
				m_completedFlag = true;
            } else if ( lineLower.contains( QRegExp("^>>> regenerating") ) ) {
                m_completedFlag = false;
            }
			
			if ( lineLower.contains( QRegExp("^!!! error") ) ) {
                //kuroo->log->error( line );
				logDone++;
            } else if ( lineLower.contains( QRegExp("^!!!") ) ) {
                //kuroo->log->error( line );
                logDone++;
            } else if( logDone == 0 && lineLower.contains( QRegExp(
                    "(^>>> (merging|unmerge|unmerging|clean|unpacking source|extracting|completed|regenerating))|"
                    "(^ \\* IMPORTANT)|(^>>> unmerging in)")) ) {
                //kuroo->log->emerge( line );
                logDone++;
            }
        } else {
			if ( lineLower.contains("please tell me") ) {
                //kuroo->log->emerge( line );
				logDone++;
            } else if ( lineLower.contains( "root access required" ) ) {
                //kuroo->log->error( i18n("Root access required!") );
                logDone++;
            } else if ( lineLower.contains("no ebuilds to satisfy") ) {
                QString missingPackage = line.section("no ebuilds to satisfy ", 1, 1);
                //kuroo->log->error( i18n("There is no ebuilds to satisfy %1").arg( missingPackage ) );
                logDone++;
            } else if ( lineLower.contains( " (masked by: " ) ) {
                m_unmasked = line.section( "- ", 1 ).section( ")", 0 );
            } else if ( !m_unmasked.isEmpty() && line.startsWith("# ") ) {
                m_importantMessage += line.section( "# ", 1, 1 ) + "<br>";
            }
        }

		// Collect blocking lines
		if ( line.contains( "is blocking" ) )
			m_blocks += line.section( "[blocks B     ]", 1, 1 ).replace( '>', "&gt;" ).replace( '<', "&lt;" );
		
		// Collect output line if user want full log verbose
		if ( logDone == 0 )
            //kuroo->log->emerge( line );
		
		countEtcUpdates( lineLower );
	}
}

/**
 * Return einfo and ewarnings collected during emerge of the package.
 * @return message
 */
const QString Emerge::packageMessage()
{
	QString message = m_packageMessage;
	m_packageMessage = QString::null;
	return message;
}


//////////////////////////////////////////////////////////////////////////////
// Post emerge stuff
//////////////////////////////////////////////////////////////////////////////

/**
 * Revdep-rebuild Complete
 * @param proc
 */
void Emerge::slotRevdepRebuildComplete()
{
    disconnect( ioRevdepRebuild, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotEmergeOutput() ) );
    disconnect( ioRevdepRebuild, SIGNAL( finished() ), this, SLOT( slotRevdepRebuildComplete() ) );
    //kuroo->done();
    delete ioRevdepRebuild; // cleanup that memory
}


/**
 * Run an eclean for packages if necessary
 * @param proc
 */
void Emerge::slotEmergeDistfilesComplete()
{
    disconnect( eClean1, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotEmergeOutput() ) );
    disconnect( eClean1, SIGNAL( finished() ), this, SLOT( slotEmergeDistfilesComplete() ) );
    //kuroo->done();
    delete eClean1; // cleanup that memory

    if( Config::ecleanDistfiles() )
    {
        //QTextCodec *codec = QTextCodec::codecForName("utf8");
        eClean2 = new KProcess();
        //eClean2->setUseShell( true, "/bin/bash" );
        //eClean2->setComm( K3Process::Communication( K3Process::Stdout | K3Process::MergedStderr | K3Process::Stdin ) );
        eClean2->close();
        *eClean2 << "eclean";
        if( Config::ecleanTimeLimit().isEmpty() )
        {
            *eClean2 << "-t" << Config::ecleanTimeLimit();
        }

        *eClean2 << "--nocolor" << "packages";

       eClean2->start( /*K3Process::OwnGroup, true*/ );
       connect( eClean2, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotEmergeOutput() ) );
       connect( eClean2, SIGNAL( finished() ), this, SLOT( slotEClean2Complete() ) );
       //kuroo->setBusy("Cleaning distfiles");
       return;
   } else startRevdepRebuild();

}

void Emerge::startRevdepRebuild() {
    if( m_dorevdeprebuild )
    {
        ioRevdepRebuild = new KProcess();
        *ioRevdepRebuild << "revdep-rebuild" << "--no-color" << "--ignore";
        ioRevdepRebuild->start();
        connect( ioRevdepRebuild, SIGNAL( readReady(KProcIO*) ), this, SLOT( slotEmergeOutput(KProcIO*) ) );
        connect( ioRevdepRebuild, SIGNAL( processExited(KProcess*) ), this, SLOT( slotRevdepRebuildComplete(KProcess*) ) );
        //kuroo->setBusy("revdep-rebuild");
        m_dorevdeprebuild = false;
    } /* end revdeprebuild section */
}

void Emerge::slotEClean2Complete()
{
    disconnect( eClean2, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotEmergeOutput() ) );
    disconnect( eClean2, SIGNAL( finished() ), this, SLOT( slotEClean2Complete() ) );
    //kuroo->done();
    delete eClean2;
    startRevdepRebuild();
}

/**
 * Disconnect signals and signal termination to main thread.
 * @param proc	
 */
void Emerge::slotBackupComplete()
{
    disconnect( eProc, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotEmergeOutput() ) );
    disconnect( eProc, SIGNAL( finished() ), this, SLOT( slotBackupComplete() ) );
    //kuroo->history->updateStatistics();
    m_backupComplete = 1;
    Emerge::queue(m_lastEmergeList);
}

/**
 * Disconnect signals and signal termination to main thread.
 * Stop statusbar progress and set kuroo to ready.
 * Present eventual messages collected during emerge to the user.
 */
void Emerge::slotDone()
{
    disconnect( eProc, SIGNAL( readyReadStandardOutput() ), this, SLOT( slotEmergeOutput() ) );
    disconnect( eProc, SIGNAL( finished() ), this, SLOT( slotDone() ) );

    //kuroo->done();
    //kuroo->queue->addPackageList( m_emergePackageList );

    if ( !m_unmasked.isEmpty() ) {
        if ( KUser().isSuperUser() )
            askUnmaskPackage( m_unmasked );
        else
            KMessageBox::information( 0, i18n("You must run Kuroo as root to unmask packages!"), i18n("Auto-unmasking packages") );
    }

    /* if m_doclean then perform an eclean */
    if( m_doeclean ) {
        if( Config::ecleanDistfiles() ) {
            QString ecleanCOMMAND;
            //QTextCodec *codec = QTextCodec::codecForName("utf8");
            eClean1 = new KProcess();
            //eClean1->setUseShell( true, "/bin/bash" );
            //eClean1->setComm( K3Process::Communication( K3Process::Stdout | K3Process::MergedStderr | K3Process::Stdin ) );
            eClean1->close(); //resetAll();
            *eClean1 << "eclean";
            ecleanCOMMAND="eclean ";
            if( !Config::ecleanTimeLimit().isEmpty() ) {
                *eClean1 << "-t" << Config::ecleanTimeLimit();
                ecleanCOMMAND+="-t";
                ecleanCOMMAND+=Config::ecleanTimeLimit();
                ecleanCOMMAND+=" ";
            }
            if( Config::ecleanDestructive() )
            {
                *eClean1 << "--destructive";
                ecleanCOMMAND+="--destructive ";
            }
            *eClean1 << "--nocolor";
            ecleanCOMMAND+="--nocolor ";

            if( Config::ecleanFetchRestrict() && Config::ecleanDestructive())
            {
                *eClean1 << "--fetch-restricted";
                ecleanCOMMAND+="--fetch-restricted ";
            }
            *eClean1 << "distfiles ";
            if( Config::ecleanSizeLimit().isEmpty() )
            {
                *eClean1 << "-s" << Config::ecleanSizeLimit();
                ecleanCOMMAND+="-s"+Config::ecleanSizeLimit()+" ";
            }
            ecleanCOMMAND+="distfiles";
            kDebug(0) << "ECLEAN COMMAND: " << ecleanCOMMAND << "\n";
            eClean1->start( /*K3Process::OwnGroup, true*/ );

            connect( eClean1, SIGNAL( readReady(K3ProcIO*) ), this, SLOT( slotEmergeOutput(K3ProcIO*) ) );
            connect( eClean1, SIGNAL( processExited(K3Process*) ), this, SLOT( slotEmergeDistfilesComplete(K3Process*) ) );
            //kuroo->setBusy( i18n("Cleaning distfiles...") );
            m_doeclean = false;
        } /* end mclean section */
    } else {
        startRevdepRebuild();
    }
    //kuroo->history->updateStatistics();
}

/**
 * Check if package is masked, if so ask to unmask.
 * @param packageKeyword	concatenated string with package and mask
 */
void Emerge::askUnmaskPackage( const QString& packageKeyword )
{
    QString package = packageKeyword.section( "(masked by: ", 0, 0 );
    QString keyword = ( packageKeyword.section( "(masked by: ", 1, 1) ).section( " keyword", 0, 0 );

    if ( packageKeyword.contains( "missing keyword" ) ) {
        m_importantMessage += i18n("%1 is not available on your architecture %2!<br><br>").arg( package ).arg( Config::arch() );
        m_importantMessage += i18n("<b>missing keyword</b> means that the application has not been tested on your architecture yet.<br>"
                                   "Ask the architecture porting team to test the package or test it for them and report your "
                                   "findings on Gentoo bugzilla website.");
        KMessageBox::information( 0, "<qt>" + m_importantMessage + "</qt>", i18n( "Missing Keyword") );
    }
    else {
        if ( keyword.contains( "-*" ) ) {
            m_importantMessage += i18n("%1 is not available on your architecture %2!<br><br>").arg( package ).arg( Config::arch() );
            m_importantMessage += i18n("<br><b>-* keyword</b> means that the application does not work on your architecture.<br>"
                                       "If you believe the package does work file a bug at Gentoo bugzilla website.");
            KMessageBox::information( 0, "<qt>" + m_importantMessage + "</qt>", i18n( "-* Keyword" ) );
        }
        else {
            if ( !keyword.contains( Config::arch() ) && keyword.contains( "package.mask" ) ) {
                //kuroo->log->error( i18n("Please add package to \"package.unmask\".") );

                switch ( KMessageBox::questionYesNoWId( NULL,
                                                        i18n("<qt>Cannot emerge masked package!<br>Do you want to unmask <b>%1</b>?</qt>").arg( package ),
                                                        i18n("Information"), KGuiItem( i18n("Unmask") ), KGuiItem( i18n("Cancel") ) ) ) {
                case KMessageBox::Yes :
                    //kuroo->db->setPackageUnMasked( //kuroo->db->packageId( package ) );
                    ////kuroo->portage->savePackageUserUnMask();
                    /*disconnect( kuroo->portage, SIGNAL( signalPortageFilesChanged() ), this, SLOT( slotTryEmerge() ) );
                    connect( kuroo->portage, SIGNAL( signalPortageFilesChanged() ), this, SLOT( slotTryEmerge() ) );*/
                    break;
                }
            }
            else {
                //kuroo->log->error( i18n("Please add package to \"package.keywords\".") );

                switch ( KMessageBox::questionYesNoWId( NULL,
                                                        i18n("<qt>Cannot emerge testing package!<br>Do you want to unmask <b>%1</b>?</qt>").arg( package ),
                                                        i18n("Information"), KGuiItem( i18n("Unmask") ), KGuiItem( i18n("Cancel") ) ) ) {
                case KMessageBox::Yes :
                    //kuroo->db->setPackageUnTesting( //kuroo->db->packageId( package ) );
                    ////kuroo->portage->savePackageKeywords();
                    /*disconnect( kuroo->portage, SIGNAL( signalPortageFilesChanged() ), this, SLOT( slotTryEmerge() ) );
                    connect( kuroo->portage, SIGNAL( signalPortageFilesChanged() ), this, SLOT( slotTryEmerge() ) );*/
                    break;
                }
            }
        }
    }
}

/**
 * After package is auto-unmasked try remerging the list again to find next package to unmask.
 */
void Emerge::slotTryEmerge()
{
    pretend( m_lastEmergeList );
    //disconnect( kuroo->portage, SIGNAL( signalPortageFilesChanged() ), this, SLOT( slotTryEmerge() ) );
}

/**
 * count etc-files to merge.
 * @param emerge line
 * @return success
 */
bool Emerge::countEtcUpdates( const QString& line )
{
    // count etc-files to merge
    if ( line.contains(" need updating") ) {
        QString tmp = line.section("config files", 0, 0);
        QRegExp rx("^\\d+\\)");
        int pos = rx.indexIn(tmp);
        if ( pos > -1 )
            m_etcUpdateCount += ( rx.cap(1) ).toInt();

        return true;
    }
    else
        return false;
}

#include "emerge.moc"
