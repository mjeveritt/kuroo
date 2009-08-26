#include "kuroo.h"
#include "portage/scan.h"
#include "package/package.h"
#include "history/history.h"
#include "queue/queue.h"
#include "log/log.h"

int main( int argc, char **argv )
{
    KAboutData about("kuroo", 0, ki18n("Kuroo"), "4.3", ki18n("Frontend to Gentoo Portage"),
                     KAboutData::License_GPL, ki18n("(C) 2006 karye") );
    about.addAuthor(ki18n("Karye"), ki18n("Original author"), "info@kuroo.org");
    about.addAuthor(ki18n("Fauconneau"), ki18n("Current maintainer"), "matthias.fauconneau@gmail.com");
    about.setHomepage("https://sourceforge.net/projects/kuroo/");
    KCmdLineArgs::init( argc, argv, &about );

    KApplication app;
    Kuroo* mainWindow = new Kuroo();
    mainWindow->show();
    return app.exec();
}

Kuroo::Kuroo() : KXmlGuiWindow()
{
    //open DB
    QDir().mkpath("/var/cache/kuroo");
    //QFile::remove("/var/cache/kuroo/portage.db"); //development
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/var/cache/kuroo/portage.db");
    db.open();

    //Status Bar
    KStatusBar* statusBar = new KStatusBar( this );
    QProgressBar* progress = new QProgressBar( this );
    progress->setMaximum( 100 );
    progress->hide();
    QLabel* status = new QLabel("", this );
    statusBar->addWidget( status );
    statusBar->addWidget( progress );
    setStatusBar( statusBar );

    //Systray
    /*if ( KurooConfig::isSystrayEnabled() ) systemTray->activate();
    connect( systemTray, SIGNAL( quitSelected() ), this, SLOT( slotQuit() ) );
    connect( systemTray, SIGNAL( signalPreferences() ), this, SLOT( slotPreferences() ) );*/

    //Pages
    KPageWidget* view = new KPageWidget(this);
    setCentralWidget( view );

    Package* package = new Package( this );
    KPageWidgetItem* pagePackage = view->addPage( package, i18n("Packages") );
    pagePackage->setIcon( KIcon("kuroo-view-portage") );

    Queue* queue = new Queue( this );
    KPageWidgetItem* pageQueue = view->addPage( queue, i18n("Queue") );
    pageQueue->setIcon( KIcon("kuroo-queue") );

    History* history = new History( this );
    KPageWidgetItem* pageHistory = view->addPage( history, i18n("History") );
    pageHistory->setIcon( KIcon("kuroo-history") );

    Log* log = new Log( this );
    KPageWidgetItem* pageLog = view->addPage( log, i18n("Log") );
    pageLog->setIcon( KIcon("kuroo-log") );

    view->setCurrentPage( pagePackage );

    connect( package, SIGNAL( queue(QString) ), queue, SLOT( queue(QString) ) );
    connect( queue, SIGNAL( log(QString) ), log, SLOT( append(QString) ) );

    Job* scan = new ScanPortageJob();
    //using thread-safe queued connection
    connect( scan, SIGNAL( status(QString) ), status, SLOT( setText(QString) ) );
    connect( scan, SIGNAL( progress(int) ), progress, SLOT( setValue(int) ) );
    connect( scan, SIGNAL( progress(int) ), progress, SLOT( show() ) );
    connect( scan, SIGNAL( finished() ), progress, SLOT( hide() ) );
    scan->start();

    queue->queue("app-portage/porthole");

    setupGUI();
}

#include "kuroo.moc"
