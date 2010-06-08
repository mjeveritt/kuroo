
#include <KIcon>

#include "common.h"
#include "packagelistmodel.h"
#include "packagelistitem.h"

PackageListModel::PackageListModel(QObject *parent)
{

}

PackageListModel::~PackageListModel()
{

}

void PackageListModel::setPackages(QList<PackageListItem*>& packages)
{
	m_packages = packages;
}

// Implementation of pure virtual methods to be implemented.
int PackageListModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return 5;
}

QVariant PackageListModel::data(const QModelIndex& index, int role) const
{

}

QModelIndex PackageListModel::index(int row, int column, const QModelIndex& parent) const
{

}

QModelIndex PackageListModel::parent(const QModelIndex& index) const
{

}

int PackageListModel::rowCount(const QModelIndex& parent) const
{

}

QVariant PackageListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation != Qt::Horizontal)
		return QVariant();
	
	switch (section)
	{
	case 0:
		return QVariant(i18n("Package (num)"));
	case 1:
		return QVariant(KIcon("kuroo_world_column"));
	case 2:
		return QVariant(KIcon("kuroo_queued_column"));
	default:
		return QVariant("Header");
		
	}
}
