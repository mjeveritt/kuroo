#ifndef QUEUE_LIST_ITEM_H
#define QUEUE_LIST_ITEM_H

#include <QObject>
#include <QTimer>

#include "packagelistitem.h"

class QueueListItem : public PackageListItem
{
	Q_OBJECT
public:
	QueueListItem(QObject *parent = 0);
	QueueListItem(const QString& name, const QString& id, const QString& category, const int status, const int duration, QObject *parent = 0);

	~QueueListItem();

	void setVersion(const QString&);
	void setSize(const QString&);
	void setDuration(int);
	int duration() const {return m_duration;}
	int remainingDuration() const {if(m_hasStarted) {return m_duration - m_steps;} else {return m_duration;}}
	QString version() const {return m_version;}
	QString size() const {return m_size;}
	QString parentId() const {return m_parentId;}
	QueueListItem* parentItem() const {return m_parent;}
	bool hasStarted() const {return m_hasStarted;}
	bool isComplete() const {return m_isComplete;}
	int steps() const {return m_steps;}
	bool pretended() const {return m_pretended;}

	void setParentId(const QString&);
	void appendChild(QueueListItem*);
	QList<QueueListItem*> children() const {return m_children;}
	void setParentItem(QueueListItem* item);
	void setHasStarted(bool);
	void setIsComplete(bool);
	void setPretended(bool);

public slots:
	void oneStep();

private:
	int m_duration;
	QString m_size;
	QString m_version;
	QString m_parentId;
	QList<QueueListItem*> m_children;
	QueueListItem* m_parent;
	bool m_hasStarted;
	bool m_isComplete;
	bool m_pretended;
	int m_steps;
	QTimer *m_testTimer;
};

#endif
