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
#include "statusbar.h"

#include <qtextcodec.h>

#include <kprocio.h>
#include <kmessagebox.h>

/**
 * @class Emerge
 * @short All Gentoo emerge command.
 */
Emerge::Emerge( QObject* m_parent )
	: QObject( m_parent ), m_error( false ), m_packageMessage( QString::null )
{
	QTextCodec *codec = QTextCodec::codecForName("utf8");
	eProc = new KProcIO( codec );
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
 * Abort the emerge process.
 * @return success
 */
bool Emerge::stop()
{
	if ( eProc->isRunning() && eProc->kill(9) ) {
		kdDebug() << i18n("Emerge process killed!") << endl;
		return true;
	}
	else
		return false;
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
EmergePackageList Emerge::packageList()
{
	return emergePackageList;
}

/**
 * Emerge list of packages.
 * @param packageList	
 * @return success
 */
bool Emerge::queue( const QStringList& packageList )
{
	blocks.clear();
	importantMessage = QString::null;
	unmasked = QString::null;
	lastEmergeList = packageList;
	etcUpdateCount = 0;
	QString sPack = packageList.join(" ");
	m_error = false;
	
	emergePackageList.clear();
	eProc->resetAll();
	*eProc << "emerge" << "--nospinner" << "--nocolor";
	
	// Add emerge options and packages
	foreach( packageList )
		*eProc << *it;
	
	eProc->start( KProcess::OwnGroup, true );
	connect( eProc, SIGNAL( readReady(KProcIO*) ), this, SLOT( slotEmergeOutput(KProcIO*) ) );
	connect( eProc, SIGNAL( processExited(KProcess*) ), this, SLOT( slotCleanupQueue(KProcess*) ) );
	SignalistSingleton::Instance()->setKurooBusy( true );
	
	if ( !eProc->isRunning() ) {
		LogSingleton::Instance()->writeLog( i18n("\nError: Emerge didn't start. "), ERROR );
		slotCleanupQueue( eProc );
		return false;
	}
	else {
		LogSingleton::Instance()->writeLog( i18n("\nEmerge %1 started...").arg( sPack ), KUROO );
		KurooStatusBar::instance()->setProgressStatus( "Emerge", i18n("Installing packages in queue...") );
		KurooStatusBar::instance()->startTimer();
		return true;
	}
}

/**
 * Emerge pretend list of packages.
 * @param packageList	
 * @return success
 */
bool Emerge::pretend( const QStringList& packageList )
{
	blocks.clear();
	importantMessage = QString::null;
	unmasked = QString::null;
	lastEmergeList = packageList;
	etcUpdateCount = 0;
	m_error = false;
	
	emergePackageList.clear();
	eProc->resetAll();
	*eProc << "emerge" << "--nospinner" << "--nocolor" << "-pv";
	
	// Add argument for each of the attached packages
	foreach( packageList )
		*eProc << *it;
	
	if ( !eProc->start( KProcess::OwnGroup, true ) ) {
		LogSingleton::Instance()->writeLog( i18n("\nError: Emerge didn't start. "), ERROR );
		return false;
	}
	else {
		connect( eProc, SIGNAL( readReady(KProcIO*) ), this, SLOT( slotEmergeOutput(KProcIO*) ) );
		connect( eProc, SIGNAL( processExited(KProcess*) ), this, SLOT( slotCleanupPretend(KProcess*) ) );
		SignalistSingleton::Instance()->setKurooBusy( true );
		LogSingleton::Instance()->writeLog( i18n("\nEmerge pretend %1 started...").arg( packageList.join(" ") ), KUROO );
		KurooStatusBar::instance()->setProgressStatus( "Emerge", i18n("Checking installation queue...") );
		KurooStatusBar::instance()->startProgress();
		return true;
	}
}

/**
 * Unmerge list of packages.
 * @param category
 * @param packageList	
 * @return success
 */
bool Emerge::unmerge( const QStringList& packageList )
{
	blocks.clear();
	importantMessage = QString::null;
	etcUpdateCount = 0;
	QString sPack = packageList.join(" ");
	emergePackageList.clear();
	m_error = false;
	
	eProc->resetAll();
	*eProc << "emerge" << "--unmerge" << "--nocolor" << "--nospinner";
	
	// Add argument for each of the attached packages
	foreach( packageList )
		*eProc << *it;
	
	if ( !eProc->start( KProcess::OwnGroup, true ) ) {
		LogSingleton::Instance()->writeLog( i18n("\nError: Emerge didn't start."), ERROR );
		return false;
	}
	else {
		connect( eProc, SIGNAL( readReady(KProcIO*) ), this, SLOT( slotEmergeOutput(KProcIO*) ) );
		connect( eProc, SIGNAL( processExited(KProcess*) ), this, SLOT( slotCleanupUnmerge(KProcess*) ) );
		SignalistSingleton::Instance()->setKurooBusy( true );
		LogSingleton::Instance()->writeLog( i18n("\nUnmerge %1 started...").arg( sPack ), KUROO );
		KurooStatusBar::instance()->setProgressStatus( "Emerge", i18n("Uninstalling packages...") );
		KurooStatusBar::instance()->startProgress();
		return true;
	}
}

/**
 * Synchronize Portage tree.
 * @return success
 */
bool Emerge::sync()
{
	blocks.clear();
	importantMessage = QString::null;
	etcUpdateCount = 0;
	emergePackageList.clear();
	m_error = false;
	
	eProc->resetAll();
	*eProc << "emerge" << "--sync" << "--quiet" << "--nocolor" << "--nospinner";
	
	if ( !eProc->start( KProcess::OwnGroup, true ) ) {
		LogSingleton::Instance()->writeLog( i18n("\nError: Emerge didn't start."), ERROR );
		return false;
	}
	else {
		connect( eProc, SIGNAL( readReady(KProcIO*) ), this, SLOT( slotEmergeOutput(KProcIO*) ) );
		connect( eProc, SIGNAL( processExited(KProcess*) ), this, SLOT( slotCleanupSync(KProcess*) ) );
		SignalistSingleton::Instance()->setKurooBusy( true );
		LogSingleton::Instance()->writeLog( i18n("\nEmerge synchronize Portage Tree started..."), KUROO );
		KurooStatusBar::instance()->setProgressStatus( "Emerge", i18n("Synchronizing portage tree...") );
		KurooStatusBar::instance()->startProgress();
		return true;
	}
}

/**
 * Check for updates of world and system.
 * @return success
 */
bool Emerge::checkUpdates()
{
	blocks.clear();
	importantMessage = QString::null;
	etcUpdateCount = 0;
	emergePackageList.clear();
	m_error = false;
	
	eProc->resetAll();
	*eProc << "emerge" << "-pvu" << "--nocolor" << "--nospinner";
	
	// Add deep if checked in gui
	if ( KurooConfig::updateDeep() )
		*eProc << "-D";
	
	// Remove dependencies if not checked in gui
// 	if ( !KurooConfig::updateDependency() )
// 		*eProc << "-O";
	
	*eProc << "world";

	if ( !eProc->start( KProcess::OwnGroup, true ) ) {
		LogSingleton::Instance()->writeLog( i18n("\nError: Emerge didn't start."), ERROR );
		return false;
	}
	else {
		connect( eProc, SIGNAL( readReady(KProcIO*) ), this, SLOT( slotEmergeOutput(KProcIO*) ) );
		connect( eProc, SIGNAL( processExited(KProcess*) ), this, SLOT( slotCleanupCheckUpdates(KProcess*) ) );
		SignalistSingleton::Instance()->setKurooBusy( true );
		LogSingleton::Instance()->writeLog( i18n("\nEmerge check package updates started..."), KUROO );
		KurooStatusBar::instance()->setProgressStatus( "Emerge", i18n("Checking for package updates...") );
		KurooStatusBar::instance()->startProgress();
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
	static bool completedFlag = false;
	static QString importantMessagePackage;
	QRegExp rxPackage( "^\\[ebuild([\\s|\\w]*)\\]\\s+"
	                   "((\\S+)/(\\S+))\\s*(?:\\[(\\S*)\\])*\\s*"
	                   "(?:USE=\")?([\\%\\-\\+\\w\\s\\(\\)\\*]*)\"?"
	                   "\\s+([\\d,]*)\\s+kB" );
	
	while ( proc->readln( line, true ) >= 0 ) {
		int logDone = 0;
		
		////////////////////////////////////////////////////////////////////////////////
		// Cleanup emerge output - remove damn escape sequences
		////////////////////////////////////////////////////////////////////////////////
		line.replace( QRegExp("\\x0007"), "\n" );
		int pos = 0;
		QRegExp rx( "(\\x0008)|(\\x001b\\[32;01m)|(\\x001b\\[0m)|(\\x001b\\[A)|(\\x001b\\[73G)|"
		            "(\\x001b\\[34;01m)|(\\x001b\\]2;)|(\\x001b\\[39;49;00m)|(\\x001b\\[01m.)" );
		while ( ( pos = rx.search(line) ) != -1 )
			line.replace( pos, rx.matchedLength(), "" );
		
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
			emergePackage.installedVersion = rxPackage.cap(5);
			emergePackage.useFlags = rxPackage.cap(6).simplifyWhiteSpace();
			emergePackage.size = rxPackage.cap(7);
			emergePackage.name = ( rxPackage.cap(4) ).section( rxPortageVersion, 0, 0 );
			emergePackage.version = ( rxPackage.cap(4) ).section( ( emergePackage.name + "-" ), 1, 1 );
			emergePackageList.prepend( emergePackage );
		}
		
		////////////////////////////////////////////////////////////////////////
		// Parse emerge output for correct log output
		////////////////////////////////////////////////////////////////////////
		QString lineLower = line.lower();
		if ( lineLower.contains( QRegExp("^>>>|^!!!") ) ) {
			
			if ( lineLower.contains( QRegExp("^>>> completed installing") ) ) {
				completedFlag = true;
				importantMessagePackage = line.section( "Completed installing ", 1, 1 ).section( " ", 0, 0 ) + ":<br>";
			}
			else
				if ( lineLower.contains( QRegExp("^>>> regenerating") ) )
					completedFlag = false;
			
			if ( lineLower.contains( QRegExp("^!!! error") ) ) {
				LogSingleton::Instance()->writeLog( line, ERROR );
				logDone++;
				m_error = true;
			}
			else
				if ( lineLower.contains( "etc-update" ) ) {
					LogSingleton::Instance()->writeLog( line, ERROR );
					logDone++;
				}
				else
					if ( lineLower.contains( QRegExp("^!!!") ) ) {
						LogSingleton::Instance()->writeLog( line, ERROR );
						importantMessage += line + "<br>";
						logDone++;
					}
					else
						if ( logDone == 0 && lineLower.contains(QRegExp("(^>>> (merging|unmerge|unmerging|clean|unpacking source|extracting|completed|regenerating))|(^ \\* important)|(^>>> unmerging in)")) ) {
							LogSingleton::Instance()->writeLog( line, EMERGE );
							logDone++;
						}
			
		}
		else
			if ( lineLower.contains("please tell me") ) {
				LogSingleton::Instance()->writeLog( line, EMERGE );
				logDone++;
			}
			else
				if ( lineLower.contains( "root access required" ) ) {
					LogSingleton::Instance()->writeLog( i18n("Root access required!"), ERROR );
					logDone++;
				}
				else
					if ( lineLower.contains("no ebuilds to satisfy") ) {
						QString missingPackage = line.section("no ebuilds to satisfy ", 1, 1);
						LogSingleton::Instance()->writeLog( i18n("There is no ebuilds to satisfy %1").arg( missingPackage ), ERROR );
						logDone++;
					}
					else
						if ( lineLower.startsWith("- ") && lineLower.contains( "masked by" ) ) {
							QString pName = line.section( " (masked by", 0, 0 );
							QString flags = line.section( " (masked by: ", 1, 1 );
							QString branch = flags.section( ")", 0, 0 );
							pName = pName.section( "- ", 1, 1 );
							QString cName = pName.section( "/", 0, 0 );
							pName = pName.section( "/", 1, 1 );
							unmasked = cName + "/" + pName + "%" + branch;
						}
						else
							if ( !unmasked.isEmpty() && line.startsWith("# ") )
								importantMessage += line.section( "# ", 1, 1 ) + "<br>";
	
		//////////////////////////////////////////////////////////////
		// Collect einfo and ewarn messages
		//////////////////////////////////////////////////////////////
		if ( completedFlag ) {
			
			QString eMessage = line.section( " * ", 1, 1 ) + line.section( "*** ", 1, 1 );
			if ( !eMessage.isEmpty() ) {

				eMessage = eMessage.replace( '>', "&gt;" ).replace( '<', "&lt;" ).replace('%', "&#37;") + "<br>";
				eMessage.remove( "!!!" );
				
				if ( !eMessage.isEmpty() ) {
					
					// Append package einfo
					if ( !importantMessagePackage.isEmpty() ) {
						if ( importantMessage.isEmpty() )
							importantMessage += importantMessagePackage + eMessage;
						else
							importantMessage += "<br>" + importantMessagePackage + eMessage;
						m_packageMessage = eMessage;
						importantMessagePackage = QString::null;
					}
					else {
						m_packageMessage += eMessage;
						importantMessage += eMessage;
					}
				}
			}
		}
		
		// Save to kuroo.log for debugging
		LogSingleton::Instance()->writeLog( line, TOLOG );
		
		// Collect blocking lines
		if ( line.contains( "is blocking" ) )
			blocks += line.section( "[blocks B     ]", 1, 1 ).replace( '>', "&gt;" ).replace( '<', "&lt;" );
		
		// Collect output line if user want full log verbose
		if ( logDone == 0 )
			LogSingleton::Instance()->writeLog( line, EMERGE );
		
		countEtcUpdates( lineLower );
	}
}

/**
 * Return einfo and ewarnings collected during emerge of the package.
 * @return message
 */
QString Emerge::packageMessage()
{
	QString message = m_packageMessage;
	m_packageMessage = QString::null;
	return message;
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
	KurooStatusBar::instance()->stopTimer();
	KurooStatusBar::instance()->setProgressStatus( "Emerge", i18n("Done.") );
	SignalistSingleton::Instance()->setKurooBusy( false );
	ResultsSingleton::Instance()->addPackageList( emergePackageList );
	
	if ( !blocks.isEmpty() )
		Message::instance()->prompt( i18n("Blocks"), i18n("Packages are blocking emerge, please correct!"), blocks.join("<br>") );

	if ( !unmasked.isEmpty() )
		askUnmaskPackage( unmasked );
	else
		if ( !importantMessage.isEmpty() )
			Message::instance()->prompt( i18n("Important"), i18n("Please check log for more information!"), importantMessage );
	
	if ( etcUpdateCount != 0 )
		EtcUpdateSingleton::Instance()->askUpdate( etcUpdateCount );
}

/**
 * Disconnect signals and signal termination to main thread.
 * @param proc	
 */
void Emerge::slotCleanupQueue( KProcess* proc )
{
	disconnect( proc, SIGNAL( readReady(KProcIO*) ), this, SLOT( slotEmergeOutput(KProcIO*) ) );
	disconnect( proc, SIGNAL( processExited(KProcess*) ), this, SLOT( slotCleanupQueue(KProcess*) ) );
	cleanup();
	HistorySingleton::Instance()->updateStatistics();
	emit signalEmergeComplete();
}

/**
 * Disconnect signals and signal termination to main thread.
 * @param proc	
 */
void Emerge::slotCleanupPretend( KProcess* proc )
{
	disconnect( proc, SIGNAL(readReady(KProcIO*) ), this, SLOT( slotEmergeOutput(KProcIO*) ) );
	disconnect( proc, SIGNAL(processExited(KProcess*) ), this, SLOT( slotCleanupPretend(KProcess*) ) );
	cleanup();
}

/**
 * Disconnect signals and signal termination to main thread.
 * @param proc	
 */
void Emerge::slotCleanupUnmerge( KProcess* proc )
{
	disconnect( proc, SIGNAL( readReady(KProcIO*) ), this, SLOT( slotEmergeOutput(KProcIO*) ) );
	disconnect( proc, SIGNAL( processExited(KProcess*) ), this, SLOT( slotCleanupUnmerge(KProcess*) ) );
	cleanup();
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
	
	KurooStatusBar::instance()->stopTimer();
	KurooStatusBar::instance()->setProgressStatus( "Emerge", i18n("Done.") );
	SignalistSingleton::Instance()->scanUpdatesComplete();
	
	if ( !blocks.isEmpty() )
		Message::instance()->prompt( i18n("Blocks"), i18n("Packages are blocking emerge, please correct!"), blocks.join("<br>")  );
	
	if ( !importantMessage.isEmpty() )
		Message::instance()->prompt( i18n("Important"), i18n("Please check log for more information!"), importantMessage );
}

/**
 * Check if package is masked, if so ask to unmask.
 * @param packageKeyword	concatenated string with package and mask
 */
void Emerge::askUnmaskPackage( const QString& packageKeyword )
{
	QString package = packageKeyword.section("%", 0, 0);
	QString keyword = (packageKeyword.section("%", 1, 1)).section(" keyword", 0, 0);
	QString name = package.section( rxPortageVersion, 0, 0 );
	
	if ( packageKeyword.contains( "missing keyword" ) ) {
		importantMessage += i18n("<b>missing keyword</b> means that the application has not been tested on your architecture yet. Ask the architecture porting team to test the package or test it for them and report your findings on Gentoo bugzilla website.");
		Message::instance()->prompt( i18n("Information"), i18n("<b>%1</b> is not available on your architecture %2!").arg( name ).arg(KurooConfig::arch()), importantMessage );
	}
	else
		if ( keyword.contains( "-*" ) ) {
			importantMessage += i18n("<br><b>-* keyword</b> means that the application does not work on your architecture. If you believe the package does work file a bug at Gentoo bugzilla website.");
			Message::instance()->prompt( i18n("Information"), i18n("<b>%1</b> is not available on your architecture %2!").arg( name ).arg(KurooConfig::arch()), importantMessage );
		}
		else {
			if ( !keyword.contains( KurooConfig::arch() ) && keyword.contains( "package.mask" ) ) {
				LogSingleton::Instance()->writeLog( i18n("Please add package to \"package.unmask\"."), ERROR );
				
				switch ( KMessageBox::questionYesNo( 0, i18n("<qt>Cannot emerge masked package!<br>Do you want to unmask <b>%1</b>?</qt>").arg( name ), i18n("Information"), KGuiItem::KGuiItem(i18n("Unmask")), KGuiItem::KGuiItem(i18n("Cancel"))) ) {
					case KMessageBox::Yes :
					if ( PortageSingleton::Instance()->unmaskPackage( name, KurooConfig::filePackageUserUnMask() ) )
							pretend( lastEmergeList );
						break;
					
				}
			}
			else {
				LogSingleton::Instance()->writeLog( i18n("Please add package to \"package.keywords\"."), ERROR );
				
				switch ( KMessageBox::questionYesNo( 0, i18n("<qt>Cannot emerge testing package!<br>Do you want to unmask <b>%1</b>?</qt>").arg( name ), i18n("Information"), i18n("Unmask"), i18n("Cancel")) ) {
					case KMessageBox::Yes :
					if ( PortageSingleton::Instance()->unmaskPackage( name, KurooConfig::filePackageKeywords() ) )
							pretend( lastEmergeList );
						break;
					
				}
			}
		}
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
		QRegExp rx("(\\d+)");
		int pos = rx.search(tmp);
		if ( pos > -1 )
			etcUpdateCount += (rx.cap(1)).toInt();
		
		return true;
	}
	else
		return false;
}

#include "emerge.moc"

