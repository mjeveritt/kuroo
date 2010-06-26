#ifndef QUEUE_LIST_MODEL
#define QUEUE_LIST_MODEL

#include <QObject>
#include <QAbstractItemModel>
#include <QList>

class QueueListItem;
class QueueListView;

class QueueListModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	QueueListModel(QObject *parent = 0);
	~QueueListModel();

	void setPackages(QList<QueueListItem*>&);

	// Declaration of pure virtual methods to be implemented.
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex& index) const;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	
	// Some virtual methods to reimplement.
	
	// Used for the header.
	virtual QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	bool hasChildren(const QModelIndex& parent = QModelIndex()) const;

	QList<QueueListItem*> packages();
	
	void updateItem(QueueListItem *item, QueueListView *listView);

private:
	QList<QueueListItem*> m_packages;
};

#endif
