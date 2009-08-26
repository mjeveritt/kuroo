//Authors: Karye; Fauconneau
#ifndef LOG_H
#define LOG_H

#include <QFile>
#include <QDebug>
#include "log/ui_logbase.h"

/**
 * @class Log
 * @short display emerge log.
 */
class Log : public QWidget, public Ui::LogBase
{
Q_OBJECT
public:
    Log( QWidget *parent = 0 );
public slots:
    void append( QString );
private:
    QFile* stdout;
};

#endif
