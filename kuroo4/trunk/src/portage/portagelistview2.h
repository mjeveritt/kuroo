
#include <QTreeView>
#include <QStringList>

class PackageListItem;

class PortageListView2 : public QTreeView
{
	Q_OBJECT
public:
	PortageListView2(QWidget *parent = 0);
	~PortageListView2();

	void setPackages(const QStringList&);
	QList<PackageListItem*> selectedPackages() const;
	PackageListItem* currentPackage();
	PackageListItem* packageItemById(const QString& id);
	QStringList selectedPackagesByIds();

signals:
	void selectionChangedSignal();

protected:
	void selectionChanged(const QItemSelection&, const QItemSelection&);
private:
	QList<PackageListItem*> m_selectedPackages;
	QMap<QString, PackageListItem*> m_packageIndex;

};
