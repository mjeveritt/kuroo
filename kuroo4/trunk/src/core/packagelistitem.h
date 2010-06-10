#ifndef PACKAGE_LIST_ITEM
#define PACKAGE_LIST_ITEM

#include <QObject>
#include <QString>

class PackageListItem : private QObject
{
	Q_OBJECT
public:
	PackageListItem(QObject *parent = 0);
	PackageListItem(const QString& name, const QString& id, const QString& category, const QString& description, const int status, const QString& update, QObject *parent = 0);

	~PackageListItem();

	QString name() const {return m_name;}
	QString id() const {return m_id;}
	QString category() const {return m_category;}
	QString description() const {return m_description;}
	QString update() const {return m_update;}
	int status() const {return m_status;}
	bool isInWorld() const {return m_isInWorld;}
private:
	QString m_name;
	QString m_id;
	QString m_category;
	QString m_description;
	QString m_update;
	bool m_isInWorld;
	int m_status;
	
};

#endif
