#include "portage/portage.h"
#include "queue.h"

/**
 * @class Queue
 * @short installation queue.
 */
Queue::Queue( QWidget* parent ) : QWidget( parent ) {
    setupUi( this );

    queueModel = new QueueModel();

    queueView->setModel( queueModel );
    queueView->setWordWrap( false );
    queueView->setShowGrid( false );
    queueView->setSelectionBehavior( QAbstractItemView::SelectRows );
    queueView->setSelectionMode( QAbstractItemView::SingleSelection );
    queueView->verticalHeader()->setVisible(false);
    queueView->verticalHeader()->setResizeMode( QHeaderView::ResizeToContents );
    queueView->verticalHeader()->setStretchLastSection( true );

    connect( queueView->selectionModel(), SIGNAL( currentChanged(QModelIndex,QModelIndex)),
             this, SLOT( selectPackage(QModelIndex) ) );
    connect( queueModel, SIGNAL( modelReset() ), queueView, SLOT( resizeColumnsToContents() ) );
    connect( queueModel, SIGNAL( log( QString ) ), this, SIGNAL( log( QString ) ) );
    connect( pbClear, SIGNAL( clicked() ), queueModel, SLOT( clear() ) );
    connect( pbRemove, SIGNAL( clicked() ), this, SLOT( remove() ) );
    connect( pbSync, SIGNAL( clicked() ), queueModel, SLOT( sync() ) );
    connect( pbUpdate, SIGNAL( clicked() ), queueModel, SLOT( update() ) );
    connect( pbApply, SIGNAL( clicked() ), queueModel, SLOT( apply() ) );
}

void Queue::queue( QString package ) { queueModel->queue( package ); }
void Queue::selectPackage( QModelIndex index ) { currentPackage = index.model()->index( index.row(), 1 ).data().toString(); }
void Queue::remove() { queueModel->remove( currentPackage ); }

#include "queue.moc"

