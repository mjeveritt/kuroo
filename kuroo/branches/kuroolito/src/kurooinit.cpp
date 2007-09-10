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
#include "kurooinit.h"
#include "introdlg.h"

#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>

#include <qdir.h>
#include <qtextcodec.h>

#include <kcmdlineargs.h>
#include <kstringhandler.h>
#include <kuser.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kprocio.h>

/**
 * @class KuroolitoInit
 * @short KuroolitoInit checks that kuroo environment is correctly setup.
 * 
 * And launch intro wizard whenever a new version of kuroo is installed.
 * Set ownership for directories and files to portage:portage.
 * Check that user is in portage group.
 */
KuroolitoInit::KuroolitoInit( QObject *parent, const char *name )
	: QObject( parent, name ), wizardDialog( 0 )
{
	getEnvironment();
	
	// Run intro if new version is installed or no DirHome directory is detected.
	QDir d( GlobalSingleton::Instance()->kurooDir() );
	if ( KuroolitoConfig::version() != KuroolitoConfig::hardVersion() || !d.exists() || KuroolitoConfig::wizard() )
		firstTimeWizard();
	
	kdDebug() << "KuroolitoInit::KuroolitoInit" << endl;
	
	// Setup kuroo environment
// 	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	if ( KuroolitoConfig::init() ) {

		// Create DirHome dir
		if ( !d.exists() ) {
			if ( !d.mkdir( GlobalSingleton::Instance()->kurooDir() ) ) {
				KMessageBox::error( 0, i18n("<qt>Cannot create kuroolito home directory.<br>"
				                            "Please correct and try again!</qt>"), i18n("Initialization") );
				exit(0);
			}
			
			d.setCurrent( GlobalSingleton::Instance()->kurooDir() );
		}
	}
	
	KuroolitoConfig::setVersion( KuroolitoConfig::hardVersion() );
	KuroolitoConfig::writeConfig();
	
	// Initialize the database
	QString databaseFile = KuroolitoDBSingleton::Instance()->init( this );
	QString database = GlobalSingleton::Instance()->kurooDir() + KuroolitoConfig::databas();
	QString dbVersion = KuroolitoDBSingleton::Instance()->getKuroolitoDbMeta( "kurooVersion" );
	
	// Check for conflicting db design or new install
	if ( KuroolitoConfig::version().section( "_db", 1, 1 ) != dbVersion ) {
		
		// Backup history if there's old db version
		if ( !dbVersion.isEmpty() ) {
			remove( database );
			kdWarning(0) << QString("Database structure is changed. Deleting old version of database %1").arg( database ) << LINE_INFO;
			
			// and recreate with new structure
			KuroolitoDBSingleton::Instance()->init( this );
		}
		
		KuroolitoDBSingleton::Instance()->setKuroolitoDbMeta( "kurooVersion", KuroolitoConfig::version().section( "_db", 1, 1 ) );
	}
	
	// Initialize singletons objects
	GlobalSingleton::Instance()->init( this );
	ImagesSingleton::Instance()->init( this );
	SignalistSingleton::Instance()->init( this );
	PortageSingleton::Instance()->init( this );
	PortageFilesSingleton::Instance()->init( this );
}

KuroolitoInit::~KuroolitoInit()
{
	KuroolitoConfig::setInit( false );
}

/**
 * Run "emerge --info" to collect system info like "ACCEPT_KEYWORDS" and "CONFIG_PROTECT".
 */
void KuroolitoInit::getEnvironment()
{
	DEBUG_LINE_INFO;
	QTextCodec *codec = QTextCodec::codecForName("utf8");
	KProcIO* eProc = new KProcIO( codec );
	*eProc << "emerge" << "--info";
	
	if ( !eProc->start( KProcess::NotifyOnExit, KProcess::All ) ) {
		kdError(0) << "Cannot run emerge --info, quitting!" << LINE_INFO;
		exit(0);
	}
	connect( eProc, SIGNAL( processExited( KProcess* ) ), this, SLOT( slotEmergeInfo( KProcess* ) ) );
	connect( eProc, SIGNAL( readReady( KProcIO* ) ), this, SLOT( slotCollectOutput( KProcIO* ) ) );
	
	KProcIO* eProc1 = new KProcIO( codec );
	eProc1->setUseShell( true, "/bin/bash" );
	*eProc1 << "find" << KuroolitoConfig::dirEdbDep() << "-name" << "*.sqlite";
	eProc1->start( KProcess::NotifyOnExit, KProcess::All );
	connect( eProc1, SIGNAL( readReady( KProcIO* ) ), this, SLOT( slotCollectOutput2( KProcIO* ) ) );
	connect( eProc, SIGNAL( processExited( KProcess* ) ), this, SLOT( slotSqliteDb( KProcess* ) ) );
}

void KuroolitoInit::slotCollectOutput( KProcIO* eProc )
{
	QString line;
	while ( eProc->readln( line, true ) >= 0 )
		m_emergeInfoLines += line;
}

void KuroolitoInit::slotCollectOutput2( KProcIO* eProc )
{
	QString line;
	while ( eProc->readln( line, true ) >= 0 ) {
		GlobalSingleton::Instance()->addSqliteFile( line );
		kdDebug() << "line=" << line << endl;
	}
}

void KuroolitoInit::slotEmergeInfo( KProcess* )
{
	foreach ( m_emergeInfoLines ) {
		if ( (*it).startsWith( "ACCEPT_KEYWORDS=" ) ) {
			QString arch = (*it).section( "\"", 1, 1 );
			
			// When testing we have two keywords, only pick one
			if ( arch.contains( "~" ) )
				arch = arch.section( "~", 1, 1 );
			
			KuroolitoConfig::setArch( arch );
		}
	}

	KuroolitoConfig::writeConfig();
}

void KuroolitoInit::slotSqliteDb( KProcess* )
{
	if ( GlobalSingleton::Instance()->sqliteFileList().empty() )
		KMessageBox::error(0, i18n("Cannot find any portage sqlite dabatase!\nIn order to activate the sqlite module you must uncomment #portdbapi.auxdbmodule = cache.sqlite.database in /etc/portage/modules."), i18n("Initialization"));
}

/**
 * Run wizard to inform user of latest changes and collect user settings like kuroo DirHome directory,
 * and overlay location.
 * If user aborts the wizard it will be relaunched again next time.
 */
void KuroolitoInit::firstTimeWizard()
{
	IntroDlg wizardDialog;
	
	if ( wizardDialog.exec() != QDialog::Accepted )
		exit(0);
	else
		KuroolitoConfig::setWizard( false );
	
	KuroolitoConfig::setInit( true );
}

#include "kurooinit.moc"
