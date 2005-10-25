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

#include <qregexp.h>

#include <kprocio.h>
#include <kmessagebox.h>

/**
 * Object handling the Gentoo emerge command.
 */
Emerge::Emerge( QObject* parent )
	: QObject( parent )
{
	eProc = new KProcIO();
}

Emerge::~Emerge()
{
	delete eProc;
	eProc = 0;
}

void Emerge::init( QObject *myParent )
{
	parent = myParent;
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
 * Emerge packages.
 * @param category
 * @param packageList	
 * @return success
 */
bool Emerge::queue( const QString& category, const QStringList& packageList )
{
	blocks.clear();
	importantMessage = "";
	unmasked = "";
	lastEmergeList = packageList;
	etcUpdateCount = 0;
	QString sPack = packageList.join(" ");
	
	emergePackageList.clear();
	eProc->resetAll();
	*eProc << "emerge" << "--nospinner";
	
	// Add emerge options and packages
	foreach( packageList ) {
		if ( (*it).startsWith("-") )
			*eProc << *it;
		else
			*eProc << "=" + category + "/" + *it;
	}
	
	eProc->start( KProcess::OwnGroup, true );
	connect( eProc, SIGNAL( readReady(KProcIO*) ), this, SLOT( readFromStdout(KProcIO*) ) );
	connect( eProc, SIGNAL( processExited(KProcess*) ), this, SLOT( cleanupQueue(KProcess*) ) );
	SignalistSingleton::Instance()->setKurooBusy(true);
	
	if ( !eProc->isRunning() ) {
		LogSingleton::Instance()->writeLog( i18n("\nError: Emerge didn't start. "), ERROR );
		cleanupQueue(eProc);
		return false;
	}
	else {
		LogSingleton::Instance()->writeLog( i18n("\nEmerge %1 started...").arg(sPack), KUROO );
		KurooStatusBar::instance()->setProgressStatus( i18n("Emerging packages in queue...") );
		KurooStatusBar::instance()->startTimer();
		return true;
	}
}

/**
 * Emerge packages.
 * @param packageList	
 * @return success
 */
bool Emerge::queue( const QStringList& packageList )
{
	blocks.clear();
	importantMessage = "";
	unmasked = "";
	lastEmergeList = packageList;
	etcUpdateCount = 0;
	QString sPack = packageList.join(" ");
	
	emergePackageList.clear();
	eProc->resetAll();
	*eProc << "emerge" << "--nospinner";
	
	// Add emerge options and packages
	foreach( packageList ) {
		if ( (*it).startsWith("-") )
			*eProc << *it;
		else
			*eProc << "=" + *it;
	}
	
	eProc->start( KProcess::OwnGroup, true );
	connect( eProc, SIGNAL( readReady(KProcIO*) ), this, SLOT( readFromStdout(KProcIO*) ) );
	connect( eProc, SIGNAL( processExited(KProcess*) ), this, SLOT( cleanupQueue(KProcess*) ) );
	SignalistSingleton::Instance()->setKurooBusy(true);
	
	if ( !eProc->isRunning() ) {
		LogSingleton::Instance()->writeLog(i18n("\nError: Emerge didn't start. "), ERROR);
		cleanupQueue(eProc);
		return false;
	}
	else {
		LogSingleton::Instance()->writeLog( i18n("\nEmerge %1 started...").arg(sPack), KUROO );
		KurooStatusBar::instance()->setProgressStatus( i18n("Emerging packages in queue...") );
		KurooStatusBar::instance()->startTimer();
		return true;
	}
}

/**
 * Emerge pretend list of packages.
 * @param category
 * @param packageList	
 * @return success
 */
bool Emerge::pretend( const QString& category, const QStringList& packageList )
{
	blocks.clear();
	importantMessage = "";
	unmasked = "";
	lastEmergeList = packageList;
	etcUpdateCount = 0;
	
	emergePackageList.clear();
	eProc->resetAll();
	*eProc << "emerge" << "--nospinner" << "-pv";
	
	// Add argument for each of the attached packages
	foreach( packageList ) {
		*eProc << "=" + category + "/" + *it;
	}
	
	if ( !eProc->start(KProcess::OwnGroup, true) ) {
		LogSingleton::Instance()->writeLog(i18n("\nError: Emerge didn't start. "), ERROR);
		return false;
	}
	else {
		connect( eProc, SIGNAL( readReady(KProcIO*) ), this, SLOT( readFromStdout(KProcIO*) ) );
		connect( eProc, SIGNAL( processExited(KProcess*) ), this, SLOT( cleanupPretend(KProcess*) ) );
		SignalistSingleton::Instance()->setKurooBusy(true);
		LogSingleton::Instance()->writeLog(i18n("\nEmerge pretend %1 started...").arg(packageList.join(" ")), KUROO);
		KurooStatusBar::instance()->setProgressStatus( i18n("Running emerge pretend...") );
		KurooStatusBar::instance()->startProgress();
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
	importantMessage = "";
	unmasked = "";
	lastEmergeList = packageList;
	etcUpdateCount = 0;
	
	emergePackageList.clear();
	eProc->resetAll();
	*eProc << "emerge" << "--nospinner" << "-pv";
	
	// Add argument for each of the attached packages
	foreach( packageList ) {
		*eProc << "=" + *it;
	}
	
	if ( !eProc->start(KProcess::OwnGroup, true) ) {
		LogSingleton::Instance()->writeLog(i18n("\nError: Emerge didn't start. "), ERROR);
		return false;
	}
	else {
		connect( eProc, SIGNAL( readReady(KProcIO*) ), this, SLOT( readFromStdout(KProcIO*) ) );
		connect( eProc, SIGNAL( processExited(KProcess*) ), this, SLOT( cleanupPretend(KProcess*) ) );
		SignalistSingleton::Instance()->setKurooBusy(true);
		LogSingleton::Instance()->writeLog(i18n("\nEmerge pretend %1 started...").arg(packageList.join(" ")), KUROO);
		KurooStatusBar::instance()->setProgressStatus( i18n("Running emerge pretend...") );
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
bool Emerge::unmerge( const QString& category, const QStringList& packageList )
{
	blocks.clear();
	importantMessage = "";
	etcUpdateCount = 0;
	QString sPack = packageList.join(" ");
	emergePackageList.clear();
	
	eProc->resetAll();
	*eProc << "emerge" << "-C";
	
	// Add argument for each of the attached packages
	foreach( packageList ) {
		*eProc << "=" + category + "/" + *it;
	}
	
	if ( !eProc->start(KProcess::OwnGroup, true) ) {
		LogSingleton::Instance()->writeLog( i18n("\nError: Emerge didn't start."), ERROR );
		return false;
	}
	else {
		connect( eProc, SIGNAL( readReady(KProcIO*) ), this, SLOT( readFromStdout(KProcIO*) ) );
		connect( eProc, SIGNAL( processExited(KProcess*) ), this, SLOT( cleanupUnmerge(KProcess*) ) );
		SignalistSingleton::Instance()->setKurooBusy(true);
		LogSingleton::Instance()->writeLog( i18n("\nUnmerge %1 started...").arg(sPack), KUROO );
		KurooStatusBar::instance()->setProgressStatus( i18n("Unmerging packages...") );
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
	importantMessage = "";
	etcUpdateCount = 0;
	emergePackageList.clear();
	
	eProc->resetAll();
	*eProc << "emerge" << "--sync" << "--quiet";
	
	if ( !eProc->start(KProcess::OwnGroup, true) ) {
		LogSingleton::Instance()->writeLog( i18n("\nError: Emerge didn't start."), ERROR );
		return false;
	}
	else {
		connect( eProc, SIGNAL( readReady(KProcIO*) ), this, SLOT( readFromStdout(KProcIO*) ) );
		connect( eProc, SIGNAL( processExited(KProcess*) ), this, SLOT( cleanupSync(KProcess*) ) );
		SignalistSingleton::Instance()->setKurooBusy(true);
		LogSingleton::Instance()->writeLog( i18n("\nEmerge synchronize Portage Tree started..."), KUROO );
		KurooStatusBar::instance()->setProgressStatus( i18n("Synchronizing portage tree...") );
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
	importantMessage = "";
	etcUpdateCount = 0;
	emergePackageList.clear();
	
	eProc->resetAll();
	*eProc << "emerge" << "-pvu";
	
	// Add deep if checked in gui
	if ( KurooConfig::updateDeep() )
		*eProc << "-D";
	
	// Remove dependencies if not checked in gui
	if ( !KurooConfig::updateDependency() )
		*eProc << "-O";
	
	*eProc << "world";

	if ( !eProc->start(KProcess::OwnGroup, true) ) {
		LogSingleton::Instance()->writeLog( i18n("\nError: Emerge didn't start."), ERROR );
		return false;
	}
	else {
		connect( eProc, SIGNAL( readReady(KProcIO*) ), this, SLOT( readFromStdout(KProcIO*) ) );
		connect( eProc, SIGNAL( processExited(KProcess*) ), this, SLOT( cleanupCheckUpdates(KProcess*) ) );
		SignalistSingleton::Instance()->setKurooBusy(true);
		LogSingleton::Instance()->writeLog( i18n("\nEmerge check package updates started..."), KUROO );
		KurooStatusBar::instance()->setProgressStatus( i18n("Checking updates...") );
		KurooStatusBar::instance()->startProgress();
		return true;
	}
}

/**
 * Parse emerge process output for messages and packages.
 * @param proc	
 */
void Emerge::readFromStdout( KProcIO *proc )
{
	QString line;
	static QString lineOver("");
	static bool lastLineFlag = false;
	static bool completedFlag = false;
	static QString importantMessagePackage;
	bool unterm = false;
	
	while ( proc->readln(line, false, &unterm) >= 0 ) {
		int logDone = 0;

		// Concatenate not completed lines
		if ( !unterm ) {
			if ( !line.contains(QRegExp("^>>>|^!!!")) )
				line = lineOver + line;
			lineOver = "";
		}
		else {
			lineOver = line;
			break;
		}
		
		/////////////////////////////////
		// Catch ewarn and einfo messages
		/////////////////////////////////
		line.replace(QRegExp("\\x001b\\[33;01m*"), "****");
		line.replace(QRegExp("\\x001b\\[32;01m*"), "****");
		
		///////////////////////////////////////////////////////
		// Cleanup emerge output - remove damn escape sequences
		///////////////////////////////////////////////////////
		line.replace(QRegExp("\\x0007"), "\n");
		int pos = 0;
		QRegExp rx("(\\x0008)|(\\x001b\\[32;01m)|(\\x001b\\[0m)|(\\x001b\\[A)|(\\x001b\\[73G)|(\\x001b\\[34;01m)|(\\x001b\\]2;)|(\\x001b\\[39;49;00m)|(\\x001b\\[01m.)");
		while ( (pos = rx.search(line)) != -1 ) {
			line.replace(pos, rx.matchedLength(), "");
		}
		
		if ( line.isEmpty() )
			continue;
		
		/////////////////////////
		// Parse out packages and info
		/////////////////////////
		if ( line.contains(QRegExp("^\\[ebuild")) ) {
			
			EmergePackage emergePackage;
			rx.setPattern("\\s\\S+/\\S+\\s");
			int pos = rx.search(line);
			
			if ( pos > -1 ) {
				QString parsedPackage = rx.cap(0).stripWhiteSpace();
				
				emergePackage.category = parsedPackage.section("/", 0, 0);
				emergePackage.package = parsedPackage.section("/", 1, 1);
				emergePackage.name = emergePackage.package.section(pv, 0, 0);
				emergePackage.version = parsedPackage.section((emergePackage.name + "-"), 1, 1);
				emergePackage.updateFlags = line.left(14).section(QRegExp("^\\[ebuild"), 1, 1);
				
				QString temp = line.section(emergePackage.package, 1, 1);
				temp = temp.section( " kB", 0, 0 );
				emergePackage.installedVersion = temp.section("[", 1, 1).section("]", 0, 0);

				// Parse out USE flags
				const QStringList field = QStringList::split(" ", temp);
				QStringList useList;
				foreach ( field ) {
					if ( (*it).startsWith("+") || (*it).startsWith("-") )
						useList += *it;
				}
				emergePackage.useFlags = useList.join(" ");
				emergePackageList.append(emergePackage);
			}
		}
		
		/////////////////////////////////////////////
		// Parse emerge output for correct log output
		/////////////////////////////////////////////
		QString lineLower = line.lower();
		if ( lineLower.contains(QRegExp("^>>>|^!!!")) ) {
			
			if ( lineLower.contains(QRegExp("^>>> completed installing")) ) {
				completedFlag = true;
				importantMessagePackage = "<b>" + line.section("Completed installing ", 1, 1).section(" ", 0, 0) + "</b>:<br>";
			}
			else
				if ( lineLower.contains(QRegExp("^>>> regenerating")) ) {
					completedFlag = false;
				}
			
			if ( lineLower.contains(QRegExp("^!!! error")) ) {
				LogSingleton::Instance()->writeLog( line, ERROR );
				logDone++;
			}
			else
				if ( lineLower.contains("etc-update") ) {
					LogSingleton::Instance()->writeLog( line, ERROR );
					logDone++;
				}
				else
					if ( lineLower.contains(QRegExp("^!!!")) ) {
						LogSingleton::Instance()->writeLog( line, ERROR );
						logDone++;
					}
					else
						if ( logDone == 0 && lineLower.contains(QRegExp("(^>>> (merging|unmerge|unmerging|clean|unpacking source|extracting|completed|regenerating))|(^ \\* important)|(^>>> unmerging in)")) ) {
							LogSingleton::Instance()->writeLog(line, EMERGE);
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
						LogSingleton::Instance()->writeLog( i18n("There is no ebuilds to satisfy %1").arg(missingPackage), ERROR );
						logDone++;
					}
					else
						if ( lineLower.startsWith("- ") && lineLower.contains("masked by") ) {
							QString pName = line.section(" (masked by", 0, 0);
							QString flags = line.section(" (masked by: ", 1, 1);
							QString branch = flags.section(")", 0, 0);
							pName = pName.section("- ", 1, 1);
							QString cName = pName.section("/", 0, 0);
							pName = pName.section("/", 1, 1);
							unmasked = cName + "/" + pName + "%" + branch;
						}
						else
							if ( !unmasked.isEmpty() && line.startsWith("# ") )
								importantMessage += line.section("# ", 1, 1) + "\n";
	
		////////////////////////////////////
		// Collect einfo and ewarn messages
		////////////////////////////////////
		if ( completedFlag && ( line.contains( QRegExp(KurooConfig::noticeRegExp())) || lastLineFlag || line.contains("**** ") ) ) {
			QString cleanLine = line.replace('>', "&gt;").replace('<', "&lt;") + "<br>";
			cleanLine.remove("!!!");
			
			kdDebug() << "cleanLine=" << cleanLine << endl;
			
			if ( line.endsWith(":") )
				lastLineFlag = true;
				else
					lastLineFlag = false;
			
			if ( line.contains("**** ") ) {
				cleanLine = cleanLine.section("**** ", 1, 1);
			}
			
			if ( !cleanLine.isEmpty() ) {
				if ( !importantMessagePackage.isEmpty() ) {
					importantMessage = importantMessagePackage + cleanLine;
					importantMessagePackage = "";
				}
				else
					importantMessage += cleanLine;
			}
		}
		
		// Save to kuroo.log for debugging
		LogSingleton::Instance()->writeLog( line, TOLOG );
		
		// Collect blocking lines
		if ( line.contains("is blocking") ) {
			blocks += line.section("[blocks B     ]", 1, 1);
		}
		
		// Collect output line if user want full log verbose
		if ( logDone == 0 )
			LogSingleton::Instance()->writeLog( line, EMERGE );
		
		countEtcUpdates( lineLower );
	}
	proc->ackRead();
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
	KurooStatusBar::instance()->setProgressStatus( i18n("Done.") );
	SignalistSingleton::Instance()->setKurooBusy(false);
	ResultsSingleton::Instance()->addPackageList( emergePackageList );
	
	if ( !blocks.isEmpty() ) {
		Message::instance()->prompt( i18n("Blocks"), i18n("Packages are blocking emerge, please correct!"), blocks );
	}
	
	if ( !unmasked.isEmpty() )
		askUnmaskPackage( unmasked );
	else
		if ( !importantMessage.isEmpty() )
			Message::instance()->prompt( i18n("Important"), i18n("Please check log for more information!"), importantMessage );
	
	if ( etcUpdateCount != 0 && !SignalistSingleton::Instance()->isKurooBusy() )
		EtcUpdateSingleton::Instance()->askUpdate( etcUpdateCount );
	
	EtcUpdateSingleton::Instance()->askUpdate( etcUpdateCount );
}

/**
 * Disconnect signals and signal termination to main thread.
 * @param proc	
 */
void Emerge::cleanupQueue( KProcess* proc )
{
	disconnect( proc, SIGNAL( readReady(KProcIO*) ), this, SLOT( readFromStdout(KProcIO*) ) );
	disconnect( proc, SIGNAL( processExited(KProcess*) ), this, SLOT( cleanupQueue(KProcess*) ) );
	cleanup();
}

/**
 * Disconnect signals and signal termination to main thread.
 * @param proc	
 */
void Emerge::cleanupPretend( KProcess* proc )
{
	disconnect( proc, SIGNAL(readReady(KProcIO*) ), this, SLOT( readFromStdout(KProcIO*) ) );
	disconnect( proc, SIGNAL(processExited(KProcess*) ), this, SLOT( cleanupPretend(KProcess*) ) );
	cleanup();
}

/**
 * Disconnect signals and signal termination to main thread.
 * @param proc	
 */
void Emerge::cleanupUnmerge( KProcess* proc )
{
	disconnect( proc, SIGNAL( readReady(KProcIO*) ), this, SLOT( readFromStdout(KProcIO*) ) );
	disconnect( proc, SIGNAL( processExited(KProcess*) ), this, SLOT( cleanupUnmerge(KProcess*) ) );
	cleanup();
}

/**
 * Disconnect signals and signal termination to main thread.
 * @param proc	
 */
void Emerge::cleanupSync( KProcess* proc )
{
	disconnect( proc, SIGNAL(readReady(KProcIO*)), this, SLOT(readFromStdout(KProcIO*)) );
	disconnect( proc, SIGNAL(processExited(KProcess*)), this, SLOT(cleanupSync(KProcess*)) );
	cleanup();
}

/**
 * Disconnect signals and signal termination to main thread.
 * @param proc	
 */
void Emerge::cleanupCheckUpdates( KProcess* proc )
{
	disconnect( proc, SIGNAL(readReady(KProcIO*)), this, SLOT(readFromStdout(KProcIO*)) );
	disconnect( proc, SIGNAL(processExited(KProcess*)), this, SLOT(cleanupCheckUpdates(KProcess*)) );
	
	KurooStatusBar::instance()->stopTimer();
	KurooStatusBar::instance()->setProgressStatus( i18n("Done.") );
	SignalistSingleton::Instance()->scanUpdatesComplete();
	
	if ( !blocks.isEmpty() )
		Message::instance()->prompt( i18n("Blocks"), i18n("Packages are blocking emerge, please correct!"), blocks );
	
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
	
	if ( packageKeyword.contains( "missing keyword" ) ) {
		importantMessage += i18n("\n<b>missing keyword</b> means that the application has not been tested on your architecture yet. Ask the architecture porting team to test the package or test it for them and report your findings on Gentoo bugzilla website.");
		Message::instance()->prompt( i18n("Information"), i18n("<b>%1</b> is not available on your architecture %2!").arg(package.section(pv, 0, 0)).arg(KurooConfig::arch()), importantMessage );
	}
	else
		if ( keyword.contains( "-*" ) ) {
			importantMessage += i18n("\n<b>-* keyword</b> means that the application does not work on your architecture. If you believe the package does work file a bug at Gentoo bugzilla website.");
			Message::instance()->prompt( i18n("Information"), i18n("<b>%1</b> is not available on your architecture %2!").arg(package.section(pv, 0, 0)).arg(KurooConfig::arch()), importantMessage );
		}
		else {
			if ( !keyword.contains( KurooConfig::arch() ) && keyword.contains( "package.mask" ) ) {
				LogSingleton::Instance()->writeLog( i18n("Please add package to \"package.unmask\"."), ERROR );
				
				switch ( KMessageBox::questionYesNo( 0, i18n("<qt>Cannot emerge testing package!<br>Do you want to unmask <b>%1</b>?</qt>").arg(package.section(pv, 0, 0)), i18n("Information"), KGuiItem::KGuiItem(i18n("Unmask")), KGuiItem::KGuiItem(i18n("Cancel"))) ) {
					case KMessageBox::Yes : {
						if ( PortageSingleton::Instance()->unmaskPackage( package.section(pv, 0, 0), KurooConfig::dirPackageUnmask() ) ) {
							pretend( lastEmergeList );
						}
						break;
					}
				}
			}
			else {
				LogSingleton::Instance()->writeLog( i18n("Please add package to \"package.keywords\"."), ERROR );
				
				switch ( KMessageBox::questionYesNo( 0, i18n("<qt>Cannot emerge testing package!<br>Do you want to unmask <b>%1</b>?</qt>").arg(package.section(pv, 0, 0)), i18n("Information"), i18n("Unmask"), i18n("Cancel")) ) {
					case KMessageBox::Yes : {
						if ( PortageSingleton::Instance()->unmaskPackage( package.section(pv, 0, 0) + " " + keyword, KurooConfig::dirPackageKeywords() ) ) {
							PortageSingleton::Instance()->loadUnmaskedList();
							pretend( lastEmergeList );
						}
						break;
					}
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
		if ( pos > -1 ) {
			etcUpdateCount += (rx.cap(1)).toInt();
		}
		
		return true;
	}
	else
		return false;
}

#include "emerge.moc"

