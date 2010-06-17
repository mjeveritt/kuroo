#ifndef QUEUE_LIST_VIEW_H
#define QUEUE_LIST_VIEW_H

#include <QTreeView>
#include <QStringList>

class QueueListItem;

class QueueListView2 : public QTreeView
{
	Q_OBJECT
public:
	QueueListView2(QWidget *parent = 0);
	~QueueListView2();

	QList<QueueListItem*> selectedPackages() const;
	QueueListItem* currentPackage();
	QueueListItem* packageItemById(const QString& id);
	QStringList selectedPackagesByIds();
	
	void insertPackageList( bool hasCheckedQueue );
	long totalDuration();
	const QStringList allPackagesNoChildren();
	
	const QStringList allId() const;
	const QString count() const { return QString::number( m_packageIndex.count() ); }
	void nextPackage( const bool& isPrevious );
signals:
	void selectionChangedSignal();

protected:
	void selectionChanged(const QItemSelection&, const QItemSelection&);
private:
	QList<QueueListItem*> m_selectedPackages;
	QMap<QString, QueueListItem*> m_packageIndex;
	
	int m_sumSize;
	const QString formatSize( const QString& sizeString );
	void addSize( const QString& size );
	void indexPackage( const QString& id, QueueListItem *item );

};
#endif
