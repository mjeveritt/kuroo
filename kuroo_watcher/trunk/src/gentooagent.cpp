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

#include <qtooltip.h>
#include <qtimer.h>

#include <kapplication.h>
#include <klocale.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <klistview.h>
#include <kpopupmenu.h>
#include <ksystemtray.h>
#include <kpassivepopup.h>
#include <kprocess.h>
#include <krun.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kprocess.h>
#include <kdialogbase.h>
#include <ktextedit.h>

#include "gentooagent.h"
#include "glsacheckbase.h"
#include "watchersettings.h"

/** 
 * GentooAgent systray widget.
 */
GentooAgent::GentooAgent(QWidget *parent, const char *name)
	:GentooWatcherBase(parent, name)
{
	internalTimer = 0;
	client = 0;
	internalTimer = new QTimer( this );
	connect( internalTimer, SIGNAL(timeout()), SLOT( sendToKuroo()) );
	
	// get our DCOP client and attach so that we may use it
	client = new DCOPClient();
	client->attach();
}

GentooAgent::~GentooAgent()
{
	client->detach();
	delete client;
	client = 0;
	delete internalTimer;
	internalTimer = 0;
}

/**
 * Minimize window.
 */
void GentooAgent::closeEvent( QCloseEvent *ce )
{
	this->hide();
}

/**
 * Start kuroo.
 * @param item
 */
void GentooAgent::addPackageKuroo( QListViewItem *item )
{
	if ( item ) {
		QString eString = item->text(0);
		package = eString.section( " ", 0, 0 ) + "-" + eString.section( " ", 1, 1 );
		
		if ( client->isApplicationRegistered("kuroo") ){
			sendToKuroo();
		}
		else {
			
			// Start Kuroo if not running
			KProcess *eProc = new KProcess;
			*eProc << "kdesu" << "--nonewdcop" << "-u" << "root" << "-c" << "kuroo";
			eProc->start();
			internalTimer->start( 5000 );
			QTimer::singleShot( 25000, this, SLOT(stopTimer()) );
	
			KPassivePopup::message( i18n("Starting Kuroo..."), this );
		}
	}
}

/**
 * Stop trying after 25 sec = 5 times.
 */
void GentooAgent::stopTimer()
{
	internalTimer->stop();
}

/**
 * Launch emerge pretend in kuroo via dcop.
 */
void GentooAgent::sendToKuroo()
{
	DCOPRef emergeKuroo("kuroo", "KurooIface");
	
	if ( !emergeKuroo.isNull() ) {
		if ( emergeKuroo.call("slotEmergePretend(QString)", package) ){
			internalTimer->stop();
			KPassivePopup::message( "Gentoo Watcher: " + package + i18n(" sent to Kuroo..."), this );
		}
		else 
			QToolTip::add(this, i18n("Gentoo Watcher: Kuroo is busy"));
	}
}

/**
 * Open page in browser.
 * @param item
 */
void GentooAgent::sentToBrowser( QListViewItem* item )
{
	QString url(item->text(3));
	if (!url.isEmpty())
		kapp->invokeBrowser(url);
}

/**
 * Lauch glsa-check.
 * @param item
 */
void GentooAgent::viewGlsa( QListViewItem* item )
{
	QString glsa = item->text(0).section("GLSA ", 1, 1);
	package = glsa.section(": ", 1, 1);
	glsa = glsa.section(" (", 0, 0);
	glsaText = "";
	
	KProcIO * eProc1 = new KProcIO;
	*eProc1 << "glsa-check" << "-d" << glsa;
	eProc1->start(KProcess::NotifyOnExit, true);
	connect( eProc1, SIGNAL(readReady( KProcIO*)), this, SLOT(readFromStdout( KProcIO*)));
	connect( eProc1, SIGNAL(processExited(KProcess*)), this, SLOT(popupGlsa(KProcess*)) );
}

/**
 * Collect output from "glsa-check".
 * @param proc
 */
void GentooAgent::readFromStdout( KProcIO* proc )
{
	QString eString;
	while ( proc->readln(eString, true) != -1)
		glsaText += eString + "\n";
	
	KPassivePopup::message( i18n("glsa-check -d ") + package + "...", this );
}

/**
 * Open dialog for "glsa-check".
 */
void GentooAgent::popupGlsa( KProcess* )
{
	KDialogBase *dial = new KDialogBase( KDialogBase::Swallow, i18n("Experimental glsa-check!"), KDialogBase::Ok, KDialogBase::Ok, this, "glsa-check -d package",true);
	
	glsacheckOutput *glsacheckDialog = new glsacheckOutput();
	dial->setMainWidget(glsacheckDialog);
	glsacheckDialog->show();
	glsacheckDialog->glsaCheck->setTextFormat(LogText);
	glsacheckDialog->glsaCheck->setText(glsaText);
	dial->exec();
}

/**
 * 
 */
void GentooAgent::openGlsa( QListViewItem* item )
{
	
	QString glsa = item->text(0).section("GLSA ", 1, 1);
	package = glsa.section(": ", 1, 1);
	package.stripWhiteSpace();
	
	if (!package.isEmpty()) {
		if(client->isApplicationRegistered("kuroo")){
			sendToKuroo();
		}
		else {
			
			// Start Kuroo if not running
			KProcess *eProc = new KProcess;
			*eProc << "kdesu" << "--nonewdcop" << "-u" << "root" << "-c" << "kuroo";
			if ( !eProc->isRunning() )
			eProc->start();
			
			internalTimer = new QTimer( this );
			connect( internalTimer, SIGNAL(timeout()), SLOT( sendToKuroo()) );
			internalTimer->start( 5000 );
			QTimer::singleShot( 25000, this, SLOT(stopTimer()) );
			
			KPassivePopup::message( i18n("Starting Kuroo..."), this );
		}
	}
}

/**
 * Rmb.
 */
void GentooAgent::contextMenu( KListView* listView, QListViewItem* item, const QPoint& point )
{
	QString category;
	QString lv( listView->name() );
	QString tmp;
	
	if( item ) {
		QString sPack( item->text(0) );
		menu = new KPopupMenu(this);
		enum Actions { SEND, VIEW, SORT, CHECK, OPEN};
		
		if ( lv.contains( "packagesStable" ) ) menu->insertItem( i18n("&Emerge package now"), SEND );
		if ( lv.contains( "packagesTesting" ) ) menu->insertItem( i18n("&Emerge package now"), SEND );
		if ( lv.contains( "packagesGlsa" ) ) menu->insertItem( i18n("&Open Glsa in browser"), VIEW );
		if ( lv.contains( "packages" ) ) menu->insertItem( i18n("&Sort by pubdate"), SORT );
		if ( lv.contains( "packagesGlsa" ) ) menu->insertItem( i18n("Open in &Kuroo"), OPEN );
		
		if (watcherSettings::glsaCheck())
			if ( lv.contains( "packagesGlsa" ) ) menu->insertItem( i18n("&Glsa-check now"), CHECK );
		
		switch( menu->exec( point ) ) {
				case SEND:
			addPackageKuroo(item);
			break;
				case VIEW:
			sentToBrowser(item);
			break;
				case SORT:
			listView->setSorting(2, true);
			break;
				case CHECK:
			viewGlsa(item);
			break;
				case OPEN:
			openGlsa(item);
			break;
		}
	}
}

#include "gentooagent.moc"
