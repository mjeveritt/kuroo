#include "portage/portage.h"
#include "history.h"

History::History(QWidget* parent) : QWidget(parent) {
    setupUi( this );
    HistoryModel* historyModel = new HistoryModel();
    historyView->setModel( historyModel );
    historyModel->refresh();

    historyView->setWordWrap( false );
    historyView->setShowGrid(false);
    historyView->setSelectionBehavior( QAbstractItemView::SelectRows );
    historyView->setSelectionMode( QAbstractItemView::SingleSelection );
    historyView->verticalHeader()->setVisible(false);
    historyView->verticalHeader()->setResizeMode( QHeaderView::ResizeToContents );
    historyView->verticalHeader()->setStretchLastSection( true );

    //filter entries
    connect( viewUnmerge, SIGNAL( toggled( bool ) ), historyModel, SLOT( selectUnmerge( bool ) ) );
    connect( searchLine, SIGNAL( completion(QString) ), historyModel, SLOT( selectFilter(QString) ) );
    connect( searchLine, SIGNAL( userTextChanged(QString) ), historyModel, SLOT( selectFilter(QString) ) );
    connect( historyModel, SIGNAL( modelReset() ), historyView, SLOT( resizeColumnsToContents() ) );

}

#include "history.moc"
