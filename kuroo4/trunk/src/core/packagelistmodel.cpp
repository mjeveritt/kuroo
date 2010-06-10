
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

}

void PackageListModel::setPackages(QList<PackageListItem*>& packages)
{
	// Remove all rows
	beginRemoveRows(QModelIndex(), 0, rowCount());
	m_packages.clear();
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
		return QVariant();
	case 1:
		if (role == Qt::DecorationRole && QueueSingleton::Instance()->isQueued(p->id()))
			return QVariant(KIcon("kuroo_queued"));

		return QVariant();
	case 2:
		if (role == Qt::DecorationRole && p->isInWorld())
			return QVariant(KIcon("kuroo_queued"));

		return QVariant();
	case 3:
		if (role == Qt::DisplayRole)
			return QVariant(p->update());
		return QVariant();
	case 4:
		if (role == Qt::DisplayRole)
			return QVariant(p->description());
		return QVariant();
	}
	
	return QVariant();
}

QModelIndex PackageListModel::index(int row, int column, const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return createIndex(row, column, row);
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
		return QVariant();
	case 1:
		if (role == Qt::DecorationRole)
			return QVariant(KIcon("kuroo_world_column"));
		return QVariant();
	case 2:
		if (role == Qt::DecorationRole)
			return QVariant(KIcon("kuroo_queued_column"));
		return QVariant();
	case 3:
		if (role == Qt::DisplayRole)
			return QVariant("Update");
		return QVariant();
	case 4:
		if (role == Qt::DisplayRole)
			return QVariant("Description");
		return QVariant();
	default:
		return QVariant();
	}
}
