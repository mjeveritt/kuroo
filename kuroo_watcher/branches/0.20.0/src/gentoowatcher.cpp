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

#include <qcursor.h>
#include <qfile.h>
#include <qtooltip.h>
#include <qstring.h>
#include <qtimer.h>
#include <qdir.h>
#include <qeventloop.h>
#include <qregexp.h>
#include <qmap.h>

#include <kconfigdialog.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <ksystemtray.h>
#include <klistview.h>
#include <kapplication.h>
#include <kpassivepopup.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <dcopref.h>
#include <kdebug.h>
#include <kurl.h>

#include "gentoowatcher.h"
#include "gentooagent.h"
#include "watchersettings.h"
#include "watchersettingswidget.h"
#include "watchersettingswidget2.h"
#include "watcher_portagedb.h"

#define foreach( x ) \
for( QStringList::ConstIterator it = x.begin(), end = x.end(); it != end; ++it )

/** 
 * GentooWatcher is a system tray class checking for latest packages in the Portage tree and Gentoo GLSA.
 * Gentoo Watcher fetches the xml documents every x hour.
 * The documents are then parsed and packages inserted to the listviews of m_window.
 */
GentooWatcher::GentooWatcher()
	: KSystemTray( 0, "gentoo_watcher" )
{
	internalTimer = new QTimer( this );
	checkGlsaTimer = new QTimer( this );
	
	KIconLoader *ldr = KGlobal::iconLoader();
	pxGlsa0 = ldr->loadIcon("kuroo_glsa", KIcon::NoGroup, KIcon::SizeSmall, KIcon::DefaultState, NULL, true);
	pxGlsa1 = ldr->loadIcon("kuroo_glsawarning", KIcon::NoGroup, KIcon::SizeSmall, KIcon::DefaultState, NULL, true);
	pxGlsa2 = ldr->loadIcon("kuroo_watcher", KIcon::NoGroup, KIcon::SizeSmall, KIcon::DefaultState, NULL, true);
	pxGlsa3 = ldr->loadIcon("kuroo_watcherwarning", KIcon::NoGroup, KIcon::SizeSmall, KIcon::DefaultState, NULL, true);
	pxPackage = ldr->loadIcon("kuroo_package", KIcon::NoGroup, KIcon::SizeSmall, KIcon::DefaultState, NULL, true);
	pxTesting = ldr->loadIcon("kuroo_testing", KIcon::NoGroup, KIcon::SizeSmall, KIcon::DefaultState, NULL, true);
	pxInstalled = ldr->loadIcon("kuroo_emerged", KIcon::NoGroup, KIcon::SizeSmall, KIcon::DefaultState, NULL, true);
	
	QToolTip::add(this, i18n("Gentoo Watcher"));
	
	this->contextMenu()->changeTitle( -2, KSystemTray::loadIcon( "kuroo_watcher" ), QString("Gentoo Watcher"));
	this->contextMenu()->insertItem( QString(i18n("&Configure Watcher...")), this, SLOT( configureAgent() ) );
	this->contextMenu()->insertItem( QString(i18n("&Check updates")), this, SLOT( updateDocuments() ) );
	connect( this, SIGNAL( quitSelected() ), this, SLOT( queryClose() ) );
	this->show();
	
	m_window = new GentooAgent();
	m_window->setCaption(i18n("Latest Gentoo packages and Security Advisories"));
	setupGui();
	
	// Start rssservice if not running
	if(!kapp->dcopClient()->isApplicationRegistered("rssservice")) {
		KProcess *eProc = new KProcess;
		*eProc << "rssservice";
		eProc->start();
	}
	
	QTimer::singleShot( 25000, this, SLOT(initDocuments()));
	connect( checkGlsaTimer, SIGNAL(timeout()), SLOT(checkGlsa()));
	connectDCOPSignal( 0, 0, "added(QString)", "documentAdded(QString)", false );
	
	// refresh timer is used to refresh the rss documents.
	connect( internalTimer, SIGNAL(timeout()), this, SLOT(updateDocuments()) );
}

GentooWatcher::~GentooWatcher()
{
	delete internalTimer;
	internalTimer = 0;
	delete checkGlsaTimer;
	checkGlsaTimer = 0;
}

/**
 * Set column width for nice gui and more
 */
void GentooWatcher::setupGui()
{
	this->setPixmap( KSystemTray::loadIcon( "kuroo_watcher" ) );
	m_window->setIcon( pxGlsa2 );
	
	m_window->packagesTesting->setColumnWidthMode(0, QListView::Manual);
	m_window->packagesTesting->setColumnWidthMode(1, QListView::Manual);
	m_window->packagesTesting->setColumnWidth(0, 200);
	m_window->packagesTesting->setColumnWidth(1, 600);
	m_window->packagesTesting->setSorting(2, true);
	m_window->packagesTesting->setColumnWidthMode(2, QListView::Manual);
	m_window->packagesTesting->hideColumn(2);
	
	m_window->packagesStable->setColumnWidthMode(0, QListView::Manual);
	m_window->packagesStable->setColumnWidthMode(1, QListView::Manual);
	m_window->packagesStable->setColumnWidth(0, 200);
	m_window->packagesStable->setColumnWidth(1, 600);
	m_window->packagesStable->setSorting(2, true);
	m_window->packagesStable->setColumnWidthMode(2, QListView::Manual);
	m_window->packagesStable->hideColumn(2);
	
	m_window->packagesGlsa->setColumnWidthMode(0, QListView::Manual);
	m_window->packagesGlsa->setColumnWidthMode(1, QListView::Manual);
	m_window->packagesGlsa->setColumnWidthMode(2, QListView::Manual);
	m_window->packagesGlsa->setColumnWidthMode(3, QListView::Manual);
	m_window->packagesGlsa->setColumnWidth(0, 350);
	m_window->packagesGlsa->setColumnWidth(1, 440);
	m_window->packagesGlsa->setSorting(0, false);
	m_window->packagesGlsa->hideColumn(2);
	m_window->packagesGlsa->hideColumn(3);
	
	m_window->sfKurooNews->setColumnWidthMode(0, QListView::Manual);
	m_window->sfKurooNews->setColumnWidthMode(1, QListView::Manual);
	m_window->sfKurooNews->setColumnWidthMode(2, QListView::Manual);
	m_window->sfKurooNews->setColumnWidthMode(3, QListView::Manual);
	m_window->sfKurooNews->setColumnWidth(0, 200);
	m_window->sfKurooNews->setColumnWidth(1, 600);
	m_window->sfKurooNews->setSorting(2, true);
	m_window->sfKurooNews->hideColumn(2);
	m_window->sfKurooNews->hideColumn(3);
}

/**
 * Add rss urls to rssservice and get feeds
 */
void GentooWatcher::initDocuments()
{
	mFeeds.clear();
	
	DCOPRef dcopCall("rssservice", "RSSService");
	QStringList urls;
	
	urls.append( watcherSettings::urlGlsa() );
	urls.append( watcherSettings::urlTesting() );
	urls.append( watcherSettings::urlStable() );
	urls.append( watcherSettings::urlSF() );
	dcopCall.send( "add(QString)", urls[0] );
	dcopCall.send( "add(QString)", urls[1] );
	dcopCall.send( "add(QString)", urls[2] );
	dcopCall.send( "add(QString)", urls[3] );
	
	QStringList::Iterator it;
	for ( it = urls.begin(); it != urls.end(); ++it ) {
		DCOPRef feedRef = dcopCall.call("document(QString)", *it);
		
		Feed feed;
		feed.ref = feedRef;
		feedRef.call( "title()" ).get( feed.title );
		feedRef.call( "link()" ).get( feed.url );
		mFeeds.append( feed );
		
		connectDCOPSignal( "rssservice", feedRef.obj(), "documentUpdated(DCOPRef)", "documentUpdated(DCOPRef)", false );
		
		qApp->processEvents( QEventLoop::ExcludeUserInput | QEventLoop::ExcludeSocketNotifiers );
	}
	
	updateDocuments();
}

/**
 * Start timer to fetch rss every x hour
 */
void GentooWatcher::updateDocuments()
{
	this->setPixmap( KSystemTray::loadIcon( "kuroo_watcher" ) );
	this->contextMenu()->changeTitle( -2, pxGlsa2, QString("Gentoo Watcher") );
	m_window->setIcon( pxGlsa2 );
	
	internalTimer->start( 3600000 );
	FeedList::Iterator it;
	for ( it = mFeeds.begin(); it != mFeeds.end(); ++it )
		(*it).ref.send( "refresh()" );
	
	int rssInterval = watcherSettings::rssInterval() * 3600000;
	if ( rssInterval < 3600000 )
		rssInterval = 3600000;
	internalTimer->changeInterval( rssInterval );
	
	checkGlsaInterval = 150000;
	checkGlsaTimer->start( checkGlsaInterval );
}

/**
 * Catch signal from rssservice that articles are fetched and ready for display
 */
void GentooWatcher::documentUpdated( DCOPRef feedRef )
{
	static uint feedCounter = 0;
	ArticleMap map;
	
	int numArticles = feedRef.call( "count()" );
	for ( int i = 0; i < numArticles; ++i ) {
		DCOPRef artRef = feedRef.call( "article(int)", i );
		QString title, url, description;
		
		qApp->processEvents( QEventLoop::ExcludeUserInput | QEventLoop::ExcludeSocketNotifiers );
		
		artRef.call( "title()" ).get( title );
		artRef.call( "link()" ).get( url );
		artRef.call( "description()" ).get( description );
		description += "%%" + url;
		
		QPair<QString, QString> article(title, description );
		map.append( article );
	}
	
	FeedList::Iterator it;
	for ( it = mFeeds.begin(); it != mFeeds.end(); ++it )
		if ( (*it).ref.obj() == feedRef.obj() ) {
			(*it).map = map;
			if ( (*it).title.isEmpty() )
				feedRef.call( "title()" ).get( (*it).title );
			if ( (*it).url.isEmpty() )
				feedRef.call( "link()" ).get( (*it).url );
		}
	
	feedCounter++;
	if ( feedCounter == mFeeds.count() ) {
		feedCounter = 0;
		updateView();
	}
}

/**
 * Catch signal from rssservice that articles are fetched and ready for display
 */
void GentooWatcher::documentAdded( QString )
{
	updateView();
}

/**
 * Add packages to listViews
 */
void GentooWatcher::updateView()
{
	FeedList::Iterator it;
	for ( it = mFeeds.begin(); it != mFeeds.end(); ++it ) {
	
		if ( (*it).title.contains("stable") ) {
			m_window->packagesStable->clear();
			ArticleMap articles = (*it).map;
			ArticleMap::Iterator artIt;
			int numArticles = 0;
			for ( artIt = articles.begin(); artIt != articles.end() && numArticles < 20; ++artIt ) {
				QString itemTitle = (*artIt).first.replace(' ', '-');
				QString itemDescription = ((*artIt).second).section("%%", 0, 0);
				
				KListViewItem* itemOne = new KListViewItem( m_window->packagesStable, itemTitle );
				itemOne->setPixmap( 0, pxPackage );
				itemOne->setText( 1, itemDescription );
				itemOne->setText( 2, QString::number(numArticles).rightJustify(2, '0') );
				
				numArticles++;
			}
		}
	
		if ( (*it).title.contains("testing") ) {
			m_window->packagesTesting->clear();
			ArticleMap articles = (*it).map;
			ArticleMap::Iterator artIt;
			int numArticles = 0;
			for ( artIt = articles.begin(); artIt != articles.end() && numArticles < 20; ++artIt ) {
				QString itemTitle = (*artIt).first.replace(' ', '-');
				QString itemDescription = ((*artIt).second).section("%%", 0, 0);
				
				KListViewItem* itemOne = new KListViewItem( m_window->packagesTesting, itemTitle );
				itemOne->setPixmap( 0, pxPackage );
				itemOne->setText( 1, itemDescription );
				itemOne->setText( 2, QString::number(numArticles).rightJustify(2, '0') );
				
				numArticles++;
			}
		}
		
		if ( (*it).title.contains("Advisories") ) {
			m_window->packagesGlsa->clear();
			ArticleMap articles = (*it).map;
			ArticleMap::Iterator artIt;
			int numArticles = 0;
			for ( artIt = articles.begin(); artIt != articles.end() && numArticles < 200; ++artIt ) {
				QString itemTitle = ((*artIt).first).section(" ", 0, 3);
				QString itemLink = ((*artIt).second).section("%%", 1, 1);
				QString itemDescription = ((*artIt).second).section("%%", 0, 0);
				
				KListViewItem* itemOne = new KListViewItem( m_window->packagesGlsa, itemTitle );
				itemOne->setPixmap( 0, pxPackage );
				itemOne->setText( 1, itemDescription );
				itemOne->setText( 2, QString::number(numArticles).rightJustify(2, '0') );
				itemOne->setText( 3, itemLink );
				
				numArticles++;
			}
		}
		
		if ( (*it).title.contains("SourceForge.net") ) {
			m_window->sfKurooNews->clear();
			ArticleMap articles = (*it).map;
			ArticleMap::Iterator artIt;
			int numArticles = 0;
			for ( artIt = articles.begin(); artIt != articles.end() && numArticles < 200; ++artIt ) {
				QString itemTitle = ((*artIt).first).section( " released", 0, 0 );
				QString itemLink = ((*artIt).second).section( "%%", 1, 1 );
				QString itemDescription = ((*artIt).second).section( "%%", 0, 0 );
				itemDescription.replace( "<br />", " " );
				itemDescription.remove( QRegExp("<[^>]*>") );
				
				KListViewItem* itemOne = new KListViewItem( m_window->sfKurooNews, itemTitle );
				itemOne->setPixmap( 0, pxPackage );
				itemOne->setText( 1, itemDescription );
				itemOne->setText( 2, QString::number(numArticles).rightJustify(2, '0') );
				itemOne->setText( 3, itemLink );
				
				numArticles++;
			}
		}
	}
	
	// Wait to get all rss - move time forward before checking glsa
	checkGlsaInterval += 50000;
	checkGlsaTimer->changeInterval( checkGlsaInterval );
}

/**
 * Gentoo watcher settings dialog.
 */
void GentooWatcher::configureAgent()
{
	if(KConfigDialog::showDialog(i18n("settings")))
		return;
	KConfigDialog *dialog = new KConfigDialog(this, i18n("settings"), watcherSettings::self(), KDialogBase::IconList);
	watcherSettingsGeneral *m_connectionSettings = new watcherSettingsGeneral(0, i18n("Connection"));
	watcherSettingsFeedback *m_feedbackSettings = new watcherSettingsFeedback(0, i18n("Connection"));
	dialog->addPage( m_connectionSettings, i18n("General"), "package_settings", i18n("General settings") );
	dialog->addPage( m_feedbackSettings, i18n("Advanced"), "package_settings", i18n("Glsa and alert settings") );
	dialog->show();
}

/**
 * Reset Gentoo Watcher icon.
 */
void GentooWatcher::slotSysTrayView() 
{
	m_window->show();
}

/**
 * Show window with listviews when user click in system tray
 */
void GentooWatcher::mousePressEvent( QMouseEvent* ev )
{
	if (ev->button() == Qt::RightButton )
		contextMenu()->exec(QCursor::pos());
	else 
		if (ev->button() == Qt::LeftButton)
			if ( m_window->isVisible() )
				m_window->hide();
			else
				m_window->show();
}

/**
 * Save settings when closing
 */
bool GentooWatcher::queryClose()
{
	watcherSettings::writeConfig();
	return true;
}

/**
 * Start experimental glsa-check for local glsa packages
 */
void GentooWatcher::checkGlsa()
{
	checkGlsaTimer->stop();
	
	KProcIO * eProc = new KProcIO;
	*eProc << "glsa-check" << "-t" << "all";
	eProc->start(KProcess::NotifyOnExit, true);
	connect( eProc, SIGNAL(readReady( KProcIO*)), this, SLOT(readFromStdout( KProcIO* )));
	connect( eProc, SIGNAL(processExited( KProcess* )), this, SLOT(checkUpdates()));
}

/**
 * Collect glsa packages in stringlist
 */
void GentooWatcher::readFromStdout(KProcIO *proc)
{
	QString eString;
	
	while ( proc->readln( eString, true ) != -1 ) {
		if ( eString.startsWith("200") )
			glsaList += eString;
	}
}

/**
 * Check for updates of installed packages
 */
void GentooWatcher::checkUpdates()
{
	KLocale *loc = KGlobal::locale();
	QString tooltip;
	QString allTooltip("<b>Gentoo Watcher</b> ");
	QString checkTime = loc->formatTime(QTime::currentTime());
	
	allTooltip += checkTime;
	
	m_window->setCaption( i18n("Latest Gentoo packages and Security Advisories %1").arg(checkTime) );
	QListViewItemIterator it = m_window->packagesGlsa;
	bool found = false;
	
	// Mark Glsa for installed packages
	for ( ; it.current(); ++it ) {
		QString package = it.current()->text(0);
		for( QStringList::Iterator it2 = glsaList.begin(); it2 != glsaList.end(); ++it2 ) {
			if ( package.contains(*it2) ) {
				it.current()->setPixmap( 0, pxGlsa1 );
				it.current()->setVisible(true);
				this->setPixmap( KSystemTray::loadIcon( "kuroo_watcherwarning" ) );
				this->contextMenu()->changeTitle( -2, pxGlsa3, QString("Gentoo Watcher"));
				m_window->setIcon( pxGlsa3 );
				if ( watcherSettings::alertGlsa() )
					tooltip += "<br>(" + package.section( " (", 1, 1 );
				found = true;
				break;
			}
			else {
				if ( watcherSettings::glsaAll() ) {
					it.current()->setPixmap( 0, pxGlsa0 );
					it.current()->setVisible(true);
				}
				else
					it.current()->setVisible(false);
			}
		}
	}
	
	int count = 0;
	it = m_window->packagesGlsa;
	for ( ; it.current(); ++it ) {
		if ( it.current()->isVisible() )
			if ( count++ > watcherSettings::rowsGlsa() )
				it.current()->setVisible(false);
	}
	
	if ( watcherSettings::alertGlsa() && found )
		allTooltip += i18n("<br><br><b>Latest GLSA</b>") + tooltip;
	
	it = m_window->packagesStable;
	found = false;
	tooltip = "";
	
	// Mark updates for installed packages
	for ( ; it.current(); ++it ) {
		QString package = it.current()->text(0);
		if ( KurooWatcherDB::instance()->isInstalled( package.section(pv, 0, 0) ) ) {
			it.current()->setPixmap( 0, pxInstalled );
			if ( watcherSettings::alertStable() )
				tooltip += "<br>" + package;
			found = true;
			continue;
		}
		else
			it.current()->setPixmap( 0, pxPackage );
	}
	
	if ( watcherSettings::alertStable() && found )
		allTooltip += i18n("<br><br><b>Latest stable packages</b>") + tooltip;
	
	it = m_window->packagesTesting;
	found = false;
	tooltip = "";
	
	// Mark updates for testing packages
	for ( ; it.current(); ++it ) {
		QString package = it.current()->text(0);
		if ( KurooWatcherDB::instance()->isInstalled( package.section(pv, 0, 0) ) ) {
			it.current()->setPixmap( 0, pxTesting );
			if ( watcherSettings::alertTesting() )
				tooltip += "<br>" + package;
			found = true;
			continue;
		}
		else
			it.current()->setPixmap( 0, pxPackage );
	}
	
	if ( watcherSettings::alertTesting() && found )
		allTooltip += i18n("<br><br><b>Latest testing packages</b>") + tooltip;
	
	if ( watcherSettings::alertTesting() || watcherSettings::alertStable() || watcherSettings::alertGlsa())
		KPassivePopup::message( allTooltip, this );
	
	QToolTip::add( this, allTooltip );
}

#include "gentoowatcher.moc"

