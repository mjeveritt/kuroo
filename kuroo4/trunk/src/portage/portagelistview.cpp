#include <QHeaderView>

#include "common.h"
#include "portagelistview.h"
#include "packagelistmodel.h"
#include "packagelistitem.h"

PortageListView::PortageListView(QWidget *parent)
 : QTreeView(parent)
{
	PackageListModel *m = new PackageListModel(this);
	setModel(m);
	QHeaderView *hh = header();
	hh->setStretchLastSection(true);
	hh->resizeSections(QHeaderView::ResizeToContents);
	setRootIsDecorated(false);

	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setAlternatingRowColors(true);
}

PortageListView::~PortageListView()
{}

void PortageListView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	foreach(const QModelIndex index, deselected.indexes())
	{
		if (index.column() == 0 && index.data().canConvert(QVariant::String))
		{
			//PackageListItem *item = dynamic_cast<PackageListModel*>(model())->packages().at(index.row());
			for(int j = 0; j < m_selectedPackages.count(); j++)
				if (m_selectedPackages[j]->name() == index.data().toString())
					m_selectedPackages.removeAt(j);
		}
	}
	foreach(const QModelIndex index, selected.indexes())
	{
		if (index.column() == 0 && index.data().canConvert(QVariant::String))
		{
			PackageListItem *item = dynamic_cast<PackageListModel*>(model())->packages().at(index.row());
			m_selectedPackages << item;
		}
	}

	emit selectionChangedSignal();
}

QList<PackageListItem*> PortageListView::selectedPackages() const
{
	return m_selectedPackages;
}

PackageListItem* PortageListView::currentPackage()
{
	return dynamic_cast<PackageListModel*>(model())->packages().at(currentIndex().row());
}

PackageListItem* PortageListView::packageItemById(const QString& id)
{
	return m_packageIndex[id];
}

QStringList PortageListView::selectedPackagesByIds()
{
	QStringList ret;
	foreach(PackageListItem *item, m_selectedPackages)
		ret << item->id();
	
	return ret;
}

void PortageListView::setPackages(const QStringList& packages)
{
	QList<PackageListItem*> items;
	QListIterator<QString> it(packages);
	while( it.hasNext() ) {
		QString id = it.next();
		QString name = it.next();
		QString category = it.next();
		QString description = it.next();
		QString status = it.next();
		QString update = it.next();

		items << new PackageListItem(name, id, category, description, status.toInt(), update, this);
		m_packageIndex.insert(id, items.last());
	}

	dynamic_cast<PackageListModel*>(model())->setPackages(items);
	
	QHeaderView *hh = header();
	//hh->setStretchLastSection(true);
	hh->resizeSections(QHeaderView::ResizeToContents);
}

void PortageListView::mouseDoubleClickEvent(QMouseEvent *event)
{
	QModelIndex index = indexAt(event->globalPos());
	if (!index.isValid())
		return;
	PackageListItem *item = static_cast<PackageListItem*>(index.internalPointer());
	if (!item)
		return;

	emit doubleClickedSignal(item);
}
