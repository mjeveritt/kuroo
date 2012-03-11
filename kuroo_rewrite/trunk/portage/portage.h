//Authors: Karye; Fauconneau
#ifndef PORTAGE_H
#define PORTAGE_H

#include <QString>
#include <QMap>
#include <QThread>
#include <QObject>
#include <QTextStream>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QAbstractListModel>
#include <QDir>
#include <KMessageBox>
#include <KDebug>
#include <KStringHandler>
#include <QProcess>

QString statusToIcon( QString status );

/**
 * @class Job
 * @short QThread with status and progress support
 */
class Job : public QThread
{
    Q_OBJECT

protected:
    void setStatus( QString status );
    void setDone();

    void newProgress( uint totalSteps );
    void setProgress( uint steps );
    void incProgress();

signals:
    void status( QString status );
    void progress( int percent );

private:
    uint count,total;
};

/**
 * @class Query
 * @short QSqlQuery with field expansion and error handling
 */
class Query : public QSqlQuery {
public:
    Query();
    Query( QString query );

    QString expand( QString text );
    bool check();

    static QString escape( QString raw );
};

/**
 * @class VersionList
 * @short QStringList with version sorting and HTML formatting
 */
class VersionList : public QStringList {
public:
    VersionList();
    VersionList( const QStringList& );
    void sortOldestFirst();
    void sortNewestFirst();
    QString toHTML();
    QString toPlainText();
};

/**
 * @class CategoryModel
 * @short display categories or subcategories matching filter
 */
class CategoryModel : public QAbstractListModel {
    Q_OBJECT
public:
    int rowCount( const QModelIndex & parent = QModelIndex() ) const;
    QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;

public slots:
    void selectCategory( QModelIndex current );
    void refresh();

private:
    QString filter;
    QStringList categories;
};

/**
 * @class PackageModel
 * @short display packages matching search terms
 */
class PackageModel : public QAbstractTableModel {
    Q_OBJECT
public:
    PackageModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data( const QModelIndex & item, int role = Qt::DisplayRole ) const;
public slots:
    void selectCategory( QModelIndex current );
    void selectSubcategory( QModelIndex current );
    void selectFilter( QString filter );

    void selectAll();
    void selectAvailable();
    void selectInstalled();
    void selectWorld();
    void selectUpdate();

    void refresh();
private:
    Query* query;
    QString category,subcategory,status,search;
};

/**
 * @class HistoryModel
 * @short display emerge history.
 */
class HistoryModel : public QAbstractTableModel {
Q_OBJECT
public:
    HistoryModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data( const QModelIndex & item, int role = Qt::DisplayRole ) const;
public slots:
    void selectUnmerge( bool unmerge );
    void selectFilter( QString filter );
    void refresh();

private:
    Query* query;
    QString search,action;
};

/**
 * @class QueueModel
 * @short start emerge and display emerge output
 */
class QueueModel : public QAbstractTableModel {
Q_OBJECT
public:
    QueueModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data( const QModelIndex & item, int role = Qt::DisplayRole ) const;

signals:
    void log( QString );

public slots:
    void queue( QString package );
    void remove( QString package );
    void clear();
    void refresh();
    void read();
    void scan();

    void sync();
    void update();
    void apply();

private:
    QStringList packages;
    QProcess emerge;
    QList<QStringList> table;
    QTextStream stdout;
};

#endif
