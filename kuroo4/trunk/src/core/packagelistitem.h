#ifndef PACKAGE_LIST_ITEM
#define PACKAGE_LIST_ITEM

#include <QObject>
#include <QString>
#include <QMap>

#include "dependatom.h"
#include "packageversion.h"

class PackageListItem : public QObject
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
	QString homepage() const {return m_homepage;}
	int status() const {return m_status;}
	bool isInWorld() const {return m_isInWorld;}
	bool isQueued() const {return m_isQueued;}
	inline QList<PackageVersion*> versionList() const {return m_versions;}
	inline const QString& linesInstalled() const { return m_linesInstalled; }
	inline const QString& linesAvailable() const { return m_linesAvailable; }
	inline const QString& linesEmerge() const { return m_linesEmerge; }
	inline bool isInArch() const { return m_isInArch; }
	inline const QStringList& versionDataList() const { return m_versionsDataList; }
	inline const QString& emergeVersion() const { return m_emergeVersion; }
	//TODO:fix isFirst and isLast
	inline bool isLastPackage() const  { return (m_index == 1); }
	bool isFirstPackage() const {return false;}
	inline QMap<QString, PackageVersion*> versionMap() const { return m_versionMap; }
	
	void initVersions();
	QList<PackageVersion*> sortedVersionList();
	void parsePackageVersions();
	void setQueued(const bool);
	bool isInPortage() const;
	bool isInstalled() const;
	
	void resetDetailedInfo();

private:
	QString m_name;
	QString m_id;
	QString m_category;
	QString m_description;
	QString m_update;
	bool m_isInWorld;
	int m_status;
	int m_index;
	
	// True if package and its versions has been initialized with all data
	bool m_isInitialized;

	// Valuelist with all versions and their data
	QList<PackageVersion*> m_versions;

	// Alternatively map with all versions and their data
	QMap<QString, PackageVersion*> m_versionMap;

	// Atom object needed for versions stability
	PortageAtom* atom;

	// Formatted string
	QString m_linesInstalled, m_linesAvailable, m_linesEmerge;

	// Version used by emerge
	QString m_emergeVersion;

	// Latest versions homepage supposed to be most current
	QString m_homepage;

	// Versions list together with stability info etc...
	QStringList m_versionsDataList;

	// Is this package available in this arch?
	bool m_isInArch;
	bool m_isQueued;
};

#endif
