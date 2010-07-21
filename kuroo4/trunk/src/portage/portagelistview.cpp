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
	setUniformRowHeights(true);
	setAllColumnsShowFocus(true);

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
			PackageListItem *item = static_cast<PackageListItem*>(index.internalPointer());
			if (item)
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
	if (m_selectedPackages.count() != 1)
		return NULL;
	return m_selectedPackages.at(0);
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
		m_packageIndex[id]->setPackageIndex(m_packageIndex.count());
	}

	dynamic_cast<PackageListModel*>(model())->setPackages(items);
	
	QHeaderView *hh = header();
	//hh->setStretchLastSection(true);
	hh->resizeSections(QHeaderView::ResizeToContents);

	sortByColumn(0, Qt::AscendingOrder);
}

void PortageListView::mouseDoubleClickEvent(QMouseEvent *event)
{
	QModelIndex index = indexAt(event->pos());
	if (!index.isValid())
		return;
	
	PackageListItem *item = static_cast<PackageListItem*>(index.internalPointer());
	if (!item)
		return;

	emit doubleClickedSignal(item);
}

/**
 * Move to next package in listview.
 * @param isPrevious true is previous, false is next
 */
void PortageListView::nextPackage( const bool isPrevious )
{
	if ( isVisible() ) {
		QModelIndex item;
		if ( isPrevious )
			item = moveCursor(QAbstractItemView::MoveUp, Qt::NoModifier);
		else
			item = moveCursor(QAbstractItemView::MoveDown, Qt::NoModifier);
		
		if (item.isValid()) {
			//kDebug() << static_cast<PackageListItem*>(item.internalPointer())->index();
			scrollTo(item);
			setCurrentIndex(item);
		}
	}
}

