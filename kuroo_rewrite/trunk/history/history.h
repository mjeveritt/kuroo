//Authors: Karye; Fauconneau
#ifndef HISTORY_H
#define HISTORY_H

#include <QObject>
#include <QAbstractItemModel>
#include <QTreeView>
#include <QDateTime>
#include <QSqlField>

#include <KGlobal>
#include <KLocale>
#include <KIcon>

#include "history/ui_historybase.h"

/**
 * @class HistoryTab
 * @short Tabpage for emerge log browser, emerge history and portage directories sizes.
 */
class History : public QWidget, public Ui::HistoryBase
{
    Q_OBJECT
public:
    History(QWidget* parent);

};

#endif
