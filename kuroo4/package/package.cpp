#include "package.h"
#include "portage/portage.h"
#include "config.h"

/**
 * @class Package
 * @short Package view with filters.
 */
Package::Package( QWidget* parent) : QWidget( parent ) {
	setupUi( this );
    if( !Config::browserSplitter().isEmpty() ) browserSplitter->setSizes( Config::browserSplitter() );
    if( !Config::packageSplitter().isEmpty() ) packageSplitter->setSizes( Config::packageSplitter() );

    CategoryModel* categoryModel = new CategoryModel();
    CategoryModel* subcategoryModel = new CategoryModel();
    PackageModel* packageModel = new PackageModel();

    categoryModel->refresh();
    subcategoryModel->refresh();

    categoryView->setModel( categoryModel );
    subcategoryView->setModel( subcategoryModel );
    packageView->setModel( packageModel );

    packageView->setWordWrap( false );
    packageView->setShowGrid(false);
    packageView->setSelectionBehavior( QAbstractItemView::SelectRows );
    packageView->setSelectionMode( QAbstractItemView::SingleSelection );
    packageView->verticalHeader()->setVisible(false);
    packageView->verticalHeader()->setResizeMode( QHeaderView::ResizeToContents );
    packageView->verticalHeader()->setStretchLastSection( true );

    //filter subcategories
    connect( categoryView->selectionModel(), SIGNAL( currentChanged(QModelIndex,QModelIndex)),
             subcategoryModel, SLOT( selectCategory(QModelIndex) ) );
    //filter packages
    connect( categoryView->selectionModel(), SIGNAL( currentChanged(QModelIndex,QModelIndex)),
             packageModel, SLOT( selectCategory(QModelIndex) ) );
    connect( subcategoryView->selectionModel(), SIGNAL( currentChanged(QModelIndex,QModelIndex)),
             packageModel, SLOT( selectSubcategory(QModelIndex) ) );
    connect( radioAll, SIGNAL( clicked() ), packageModel, SLOT( selectAll() ) );
    connect( radioAvailable, SIGNAL( clicked() ), packageModel, SLOT( selectAvailable() ) );
    connect( radioInstalled, SIGNAL( clicked() ), packageModel, SLOT( selectInstalled() ) );
    connect( radioWorld, SIGNAL( clicked() ), packageModel, SLOT( selectWorld() ) );
    connect( radioUpdate, SIGNAL( clicked() ), packageModel, SLOT( selectUpdate() ) );
    connect( searchLine, SIGNAL( completion(QString) ), packageModel, SLOT( selectFilter(QString) ) );
    connect( searchLine, SIGNAL( userTextChanged(QString) ), packageModel, SLOT( selectFilter(QString) ) );
    connect( packageModel, SIGNAL( modelReset() ), packageView, SLOT( resizeColumnsToContents() ) );
    //select package
    connect( packageView->selectionModel(), SIGNAL( currentChanged(QModelIndex,QModelIndex)),
             this, SLOT( selectPackage(QModelIndex) ) );
    //connect buttons
        connect( pbInstall, SIGNAL( clicked() ), this, SLOT( install() ) );
}

Package::~Package() {
    Config::setBrowserSplitter( browserSplitter->sizes() );
    Config::setPackageSplitter( packageSplitter->sizes() );
    Config::self()->writeConfig();
}

void Package::selectPackage( QModelIndex current ) {
    //fetch additional information
    QString rowid = current.data( Qt::UserRole ).toString();
    Query query("SELECT category,subcategory,name,description,versions,status FROM package WHERE rowid="+rowid);
    // store current package for buttons
    currentPackage = query.expand("$category-$subcategory/$name");
    // Build summary html-view
    summaryBrowser->setHtml( query.expand(
            "<table width=100% border=0 cellpadding=0>"
            "<tr><td bgcolor=#8080FF colspan=2>"
            "<img src=\""+KIconLoader::global()->iconPath( statusToIcon( query.expand("$status") ), KIconLoader::Small )+"\" />"
            "<b><font size=+1> $name</font> ($category-$subcategory)</b></td></tr>"
            "<tr><td colspan=2>$description</td></tr>"
            "<tr><td><b>Homepage: </b></td><td><a href=\"$homepage\">$homepage</a></td></tr>"
            "<tr><td><b>Versions: </b></td><td>"+VersionList( query.expand("$versions").split(" ") ).toHTML()+"</td></tr>"
            "</table>"));
}

void Package::install() { emit queue( currentPackage ); }

#include "package.moc"
