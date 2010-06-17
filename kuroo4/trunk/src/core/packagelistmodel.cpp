
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

QList<PackageListItem*> PackageListModel::packages()
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
	beginInsertRows(QModelIndex(), 0, packages.count() - 1);
	m_packages = packages;
	endInsertRows();
	kDebug() << "Insert" << m_packages.count() << "packages";
	//insertRows(0, m_packages.count());
}

// Implementation of pure virtual methods to be implemented.
int PackageListModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return 5;
}

QVariant PackageListModel::data(const QModelIndex& index, int role) const
{
	PackageListItem *p = m_packages[index.row()];
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
			return QVariant(KIcon("kuroo_queued"));

	case 2:
		if (role == Qt::DecorationRole && QueueSingleton::Instance()->isQueued(p->id()))
			return QVariant(KIcon("kuroo_queued"));

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
		return QVariant();
	}
	
	return QVariant();
}

QModelIndex PackageListModel::index(int row, int column, const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return createIndex(row, column, row);
}

bool PackageListModel::hasChildren(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return false;
}

QModelIndex PackageListModel::parent(const QModelIndex& index) const
{
	Q_UNUSED(index)
	return QModelIndex();
}

int PackageListModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
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
			return QVariant(i18n(QString("Package (%1)").arg(rowCount())));
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
		return QVariant();
	}
	
	return QVariant();
}
