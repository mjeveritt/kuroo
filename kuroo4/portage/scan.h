//Authors: Karye; Fauconneau
#ifndef SCAN_H
#define SCAN_H

#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QEvent>
#include <QSqlError>

#include <klocalizedstring.h>
#include <KGlobal>
#include <KDebug>

#include "portage.h"

//Thread for scanning local portage tree.
class ScanPortageJob : public Job
{
    Q_OBJECT
private:
    void 						run();
};

#endif
