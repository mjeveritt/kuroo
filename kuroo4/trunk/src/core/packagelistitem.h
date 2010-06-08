#ifndef PACKAGE_LIST_ITEM
#define PACKAGE_LIST_ITEM

#include <QObject>
#include <QString>

class PackageListItem : private QObject
{
	Q_OBJECT
public:
	PackageListItem(QObject *parent = 0);
	PackageListItem(const QString& name, const QString& id, const QString& category, const QString& description, const int status, QObject *parent = 0);

	QString name() const {return m_name;}
	QString id() const {return m_id;}
	QString category() const {return m_category;}
	QString description() const {return m_description;}
	int status() const {return m_status;}

	~PackageListItem();
private:
	QString m_name;
	QString m_id;
	QString m_category;
	QString m_description;
	int m_status;
	
};

#endif
