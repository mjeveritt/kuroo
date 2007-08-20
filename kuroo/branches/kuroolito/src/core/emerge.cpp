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
#include "statusbar.h"

#include <qtextcodec.h>

#include <kprocio.h>
#include <kmessagebox.h>
#include <kuser.h>
#include <kdeversion.h>

/**
 * @class Emerge
 * @short All Gentoo emerge command.
 * 
 * Handles emerge, unmerge, check-for-updates, sync...
 */
Emerge::Emerge( QObject* m_parent )
	: QObject( m_parent ), m_packageMessage( QString::null ), m_completedFlag( false ), m_importantMessagePackage( QString::null )
{
	QTextCodec *codec = QTextCodec::codecForName("utf8");
	eProc = new KProcIO( codec );
	eProc->setUseShell( true, "/bin/bash" );
	
	#if KDE_VERSION >= KDE_MAKE_VERSION(3,5,2)
	eProc->setComm( KProcess::Communication( KProcess::Stdout | KProcess::MergedStderr | KProcess::Stdin ) );
	#endif
}

Emerge::~Emerge()
{
	delete eProc;
	eProc = 0;
}

void Emerge::init( QObject *parent )
{
	m_parent = parent;
}

/**
 * Convenience flag.
 * @return true if emerging.
 */
bool Emerge::isRunning()
{
	return eProc->isRunning();
}

/**
 * @return list of packages parsed out from emerge output.
 */
const EmergePackageList Emerge::packageList()
{
	return m_emergePackageList;
}

/**
 * Synchronize Portage tree.
 * @return success
 */
bool Emerge::sync()
{
// 	m_blocks.clear();
	m_importantMessage = QString::null;
// 	m_etcUpdateCount = 0;
	m_emergePackageList.clear();
	
	eProc->resetAll();
	*eProc << "emerge" << "--sync" << "--quiet" << "--nocolor" << "--nospinner";
	
	if ( !eProc->start( KProcess::OwnGroup, true ) ) {
// 		KuroolitoStatusBar::instance()->setProgressStatus( "Error", i18n("emerge --sync didn't start.") );
		return false;
	}
	else {
		connect( eProc, SIGNAL( readReady(KProcIO*) ), this, SLOT( slotEmergeOutput(KProcIO*) ) );
		connect( eProc, SIGNAL( processExited(KProcess*) ), this, SLOT( slotCleanupSync(KProcess*) ) );
		SignalistSingleton::Instance()->setKuroolitoBusy( true );
// 		/*KuroolitoStatusBar*/::instance()->setProgressStatus( "Emerge", i18n("Synchronizing portage tree...") );
// 		KuroolitoStatusBar::instance()->setTotalSteps( KuroolitoDBSingleton::Instance()->getKuroolitoDbMeta( "syncDuration" ).toInt() );
		return true;
	}
}

/**
 * Check for updates of world and system.
 * @return success
 */
bool Emerge::checkUpdates()
{
// 	m_blocks.clear();
	m_importantMessage = QString::null;
// 	m_etcUpdateCount = 0;
	m_emergePackageList.clear();
	
	eProc->resetAll();
	*eProc << "emerge" << "-pvu" << "--color=n" << "--nospinner" << "--columns";
	
	// Add deep if checked in gui
	if ( KuroolitoConfig::updateDeep() )
		*eProc << "-D";
	
	*eProc << "world";

	if ( !eProc->start( KProcess::OwnGroup, true ) ) {
// 		KuroolitoStatusBar::instance()->setProgressStatus( "Error", i18n("emerge --update didn't start.") );
		return false;
	}
	else {
		connect( eProc, SIGNAL( readReady(KProcIO*) ), this, SLOT( slotEmergeOutput(KProcIO*) ) );
		connect( eProc, SIGNAL( processExited(KProcess*) ), this, SLOT( slotCleanupCheckUpdates(KProcess*) ) );
		SignalistSingleton::Instance()->setKuroolitoBusy( true );
// 		KuroolitoStatusBar::instance()->setProgressStatus( "Emerge", i18n("Checking for package updates...") );
// 		KuroolitoStatusBar::instance()->startProgress();
		return true;
	}
}

/**
 * Parse emerge process output for messages and packages.
 * @param proc	
 */
void Emerge::slotEmergeOutput( KProcIO *proc )
{
	QString line;
	QRegExp rxPackage = GlobalSingleton::Instance()->rxEmerge();
	
	while ( proc->readln( line, true ) >= 0 ) {
		int logDone( 0 );
		
		////////////////////////////////////////////////////////////////////////////////
		// Cleanup emerge output - remove damn escape sequences
		////////////////////////////////////////////////////////////////////////////////
		line.replace( QRegExp("\\x0007"), "\n" );
		int pos( 0 );
		QRegExp rx( "(\\x0008)|(\\x001b\\[32;01m)|(\\x001b\\[0m)|(\\x001b\\[A)|(\\x001b\\[73G)|"
		            "(\\x001b\\[34;01m)|(\\x001b\\]2;)|(\\x001b\\[39;49;00m)|(\\x001b\\[01m.)" );
		while ( ( pos = rx.search(line) ) != -1 )
			line.replace( pos, rx.matchedLength(), QString::null );
		
		if ( line.isEmpty() )
			continue;
		
		////////////////////////////////////////////////////////////////////////////
		// Parse out package and info
		////////////////////////////////////////////////////////////////////////////
		if ( rxPackage.search( line ) > -1 ) {
			EmergePackage emergePackage;
			emergePackage.updateFlags = rxPackage.cap(1);
			emergePackage.package = rxPackage.cap(2);
			emergePackage.category = rxPackage.cap(3);
			emergePackage.name = rxPackage.cap(4);
			emergePackage.version = rxPackage.cap(5);
			emergePackage.installedVersion = rxPackage.cap(6);
			emergePackage.useFlags = rxPackage.cap(7).simplifyWhiteSpace();
			emergePackage.size = rxPackage.cap(8);
			m_emergePackageList.prepend( emergePackage );
		}
		
		// Collect lines
		m_importantMessage += line + "<br>";
	}
}


//////////////////////////////////////////////////////////////////////////////
// Post emerge stuff
//////////////////////////////////////////////////////////////////////////////

/**
 * Cleanup when emerge process is done.
 * Stop statusbar progress and set kuroo to ready.
 * Present eventual messages collected during emerge to the user.
 */
void Emerge::cleanup()
{
// 	KuroolitoStatusBar::instance()->stopTimer();
// 	KuroolitoStatusBar::instance()->setProgressStatus( "Emerge", i18n("Done.") );
	SignalistSingleton::Instance()->setKuroolitoBusy( false );
}

/**
 * Disconnect signals and signal termination to main thread.
 * @param proc	
 */
void Emerge::slotCleanupSync( KProcess* proc )
{
	disconnect( proc, SIGNAL( readReady(KProcIO*) ), this, SLOT( slotEmergeOutput(KProcIO*) ) );
	disconnect( proc, SIGNAL( processExited(KProcess*) ), this, SLOT( slotCleanupSync(KProcess*) ) );
	cleanup();
}

/**
 * Disconnect signals and signal termination to main thread.
 * @param proc	
 */
void Emerge::slotCleanupCheckUpdates( KProcess* proc )
{
	disconnect( proc, SIGNAL( readReady(KProcIO*) ), this, SLOT( slotEmergeOutput(KProcIO*) ) );
	disconnect( proc, SIGNAL( processExited(KProcess*) ), this, SLOT( slotCleanupCheckUpdates(KProcess*) ) );
	
// 	KuroolitoStatusBar::instance()->stopTimer();
// 	KuroolitoStatusBar::instance()->setProgressStatus( "Emerge", i18n("Done.") );
	SignalistSingleton::Instance()->scanUpdatesComplete();
	
// 	if ( !m_importantMessage.isEmpty() )
// 		Message::instance()->prompt( i18n("Important"), i18n("Important message!"), m_importantMessage );
}

#include "emerge.moc"

