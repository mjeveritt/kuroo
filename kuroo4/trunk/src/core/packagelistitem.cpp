#include "packagelistitem.h"

PackageListItem::PackageListItem(QObject *parent)
 : QObject(parent)
{}

PackageListItem::PackageListItem(const QString& name, const QString& id, const QString& category, const QString& description, const int status, QObject *parent)
 : QObject(parent),
   m_name(name),
   m_id(id),
   m_category(category),
   m_description(description),
   m_status(status)
{}

PackageListItem::~PackageListItem()
{}
