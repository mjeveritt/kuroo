//Authors: Karye; Fauconneau
#ifndef QUEUE_H
#define QUEUE_H

#include "queue/ui_queuebase.h"
class QueueModel;

/**
 * @class Queue
 * @short Object for packages to be emerged = installation queue.
 */
class Queue : public QWidget, public Ui::QueueBase
{
Q_OBJECT

public:
    Queue( QWidget *parent);

signals:
    void log( QString );

public slots:
    void selectPackage( QModelIndex index );
    void queue( QString package );
    void remove();

private:
    QueueModel* queueModel;
    QString currentPackage;
};

#endif
