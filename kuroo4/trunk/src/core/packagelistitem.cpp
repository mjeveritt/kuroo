#include "common.h"
#include "packagelistitem.h"

PackageListItem::PackageListItem(QObject *parent)
 : QObject(parent)
{}

PackageListItem::PackageListItem(const QString& name, const QString& id, const QString& category, const QString& description, const int status, const QString& update, QObject *parent)
 : QObject(parent),
   m_name(name),
   m_id(id),
   m_category(category),
   m_description(description),
   m_status(status),
   m_update(update)
{
	if (PortageSingleton::Instance()->isInWorld(m_category + "/" + m_name))
		m_isInWorld = true;
	else
		m_isInWorld = false;
}

PackageListItem::~PackageListItem()
{}
