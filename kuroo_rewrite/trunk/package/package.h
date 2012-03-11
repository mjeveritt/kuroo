//Authors: Karye; Fauconneau
#ifndef PACKAGE_H
#define PACKAGE_H

#include <QObject>
#include <QSortFilterProxyModel>
#include <KLineEdit>
#include <KStandardDirs>
#include "package/ui_packagebase.h"

class Portage;

/**
 * @class PortageTab
 * @short Package view with filters.
 */
class Package : public QWidget, public Ui::PortageBase
{
    Q_OBJECT
public:
    Package( QWidget *parent );
    ~Package();
	
signals:
    void queue(QString);

public slots:
    void selectPackage(QModelIndex);
    void install();

private:
    QString currentPackage;
};

#endif
