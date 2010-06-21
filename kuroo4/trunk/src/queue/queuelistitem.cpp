#include "common.h"
#include "queuelistitem.h"

QueueListItem::QueueListItem(QObject *parent)
 : PackageListItem(parent)
{
}

QueueListItem::QueueListItem(const QString& name, const QString& id, const QString& category, const int status, const int duration, QObject *parent)
 : PackageListItem(name, id, category, QString(), status, QString(), parent)
{
	setDuration(duration);
	m_parentId = "";
	m_parent = NULL;
	m_hasStarted = false;
	m_isComplete = false;
	m_steps = 0;
}

QueueListItem::~QueueListItem()
{
	foreach(QueueListItem *i, m_children)
		delete i;
}

void QueueListItem::setVersion(const QString& v)
{
	m_version = v;
}

void QueueListItem::setDuration(int d)
{
	m_duration = d;
}

void QueueListItem::setSize(const QString& s)
{
	m_size = s;
}

/*QString QueueListItem::duration()
{
	if (m_duration == -1)
		return i18n("na");
	
	return formatTime(m_duration);
}*/

void QueueListItem::setParentId(const QString& pid)
{
	m_parentId = pid;
}

void QueueListItem::appendChild(QueueListItem* item)
{
	m_children << item;
	item->setParentItem(this);
}

void QueueListItem::setParentItem(QueueListItem* item)
{
	m_parent = item;
}

void QueueListItem::setHasStarted(bool h)
{
	m_hasStarted = h;
}

void QueueListItem::setIsComplete(bool h)
{
	m_isComplete = h;
}

void QueueListItem::oneStep()
{
	m_steps++;
	//TODO:update index in view.
}
