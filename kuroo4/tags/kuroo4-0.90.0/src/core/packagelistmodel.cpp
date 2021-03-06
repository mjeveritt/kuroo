
#include <KIcon>

#include "common.h"
#include "packagelistmodel.h"
#include "packagelistitem.h"

PackageListModel::PackageListModel(QObject *parent)
 : QAbstractItemModel(parent)
{
}

PackageListModel::~PackageListModel()
{
	while (!m_packages.isEmpty())
		delete m_packages.takeLast();
}

QList<PackageListItem*> PackageListModel::packages() const
{
	return m_packages;
}

void PackageListModel::setPackages(QList<PackageListItem*>& packages)
{
	// Remove all rows
	beginRemoveRows(QModelIndex(), 0, rowCount());
	while (!m_packages.isEmpty())
		delete m_packages.takeLast();
	endRemoveRows();
	if (packages.count() <= 0)
	{
		m_packages.clear();
		return;
	}
	beginInsertRows(QModelIndex(), 0, packages.count() - 1);
	m_packages = packages;
	endInsertRows();
}

// Implementation of pure virtual methods to be implemented.
int PackageListModel::columnCount(const QModelIndex& parent) const
{
	if (parent.isValid())
		return 0;
	return 5;
}

QVariant PackageListModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	PackageListItem *p = static_cast<PackageListItem*>(index.internalPointer());
	if (!p)
		return QVariant();

	switch(index.column())
	{
	case 0:
		if (role == Qt::DisplayRole)
			return QVariant(p->name());
		if (role == Qt::DecorationRole && p->status() & PACKAGE_AVAILABLE)
			return QVariant(KIcon("kuroo_package"));
		else if (role == Qt::DecorationRole)
			return QVariant(KIcon("kuroo_stable"));
		break;
	case 1:
		if (role == Qt::DecorationRole && p->isInWorld())
			return QVariant(KIcon("kuroo_world"));
		break;
	case 2:
		if (role == Qt::DecorationRole && QueueSingleton::Instance()->isQueued(p->id()))
			return QVariant(KIcon("kuroo_queue"));
		break;
	case 3:
		if (role == Qt::DisplayRole)
			return QVariant(p->update());
		break;
	case 4:
		if (role == Qt::DisplayRole)
			return QVariant(p->description());
		break;
	default:
		kdDebug() << "Error: invalid column!" << LINE_INFO;
		break;
	}

	return QVariant();
}

QModelIndex PackageListModel::index(int row, int column, const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	if (row >= 0 && row < m_packages.count())
		return createIndex(row, column, m_packages[row]);
	else
		return QModelIndex();
}

bool PackageListModel::hasChildren(const QModelIndex& parent) const
{
	//parent.isValid is true for actual items, which don't have children (we're flat), but false for the root
	//which needs to return true here
	if (parent.isValid())
		return false;

	return true;
}

QModelIndex PackageListModel::parent(const QModelIndex& index) const
{
	Q_UNUSED(index)
	return QModelIndex();
}

int PackageListModel::rowCount(const QModelIndex& parent) const
{
	if (parent.isValid())
		return 0;

	return m_packages.count();
}

QVariant PackageListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation != Qt::Horizontal)
		return QVariant();

	switch (section)
	{
	case 0:
		if (role == Qt::DisplayRole)
			return QVariant(i18n("Packages (%1)", rowCount()));
		break;
	case 1:
		if (role == Qt::DecorationRole)
			return QVariant(KIcon("kuroo_world_column"));
		break;
	case 2:
		if (role == Qt::DecorationRole)
			return QVariant(KIcon("kuroo_queued_column"));
		break;
	case 3:
		if (role == Qt::DisplayRole)
			return QVariant("Update");
		break;
	case 4:
		if (role == Qt::DisplayRole)
			return QVariant("Description");
		break;
	default:
		kdDebug() << "Error: invalid column!" << LINE_INFO;
		break;
	}

	return QVariant();
}

bool PackageListModel::packageLessThan(PackageListItem *p1, PackageListItem *p2)
{
	return p1->name() < p2->name();
}

bool PackageListModel::packageMoreThan(PackageListItem *p1, PackageListItem *p2)
{
	return p1->name() > p2->name();
}

void PackageListModel::sort(int column, Qt::SortOrder order)
{
	if (column != 0)
		return;

	if (order == Qt::AscendingOrder)
		qSort(m_packages.begin(), m_packages.end(), packageLessThan);
	else
		qSort(m_packages.begin(), m_packages.end(), packageMoreThan);
}
