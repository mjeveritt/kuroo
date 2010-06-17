#ifndef PORTAGE_LIST_VIEW
#define PORTAGE_LIST_VIEW

#include <QTreeView>
#include <QStringList>

class PackageListItem;

class PortageListView : public QTreeView
{
	Q_OBJECT
public:
	PortageListView(QWidget *parent = 0);
	~PortageListView();

	void setPackages(const QStringList&);
	QList<PackageListItem*> selectedPackages() const;
	PackageListItem* currentPackage();
	PackageListItem* packageItemById(const QString& id);
	QStringList selectedPackagesByIds();

signals:
	void selectionChangedSignal();
	void doubleClickedSignal(PackageListItem*);

protected:
	void selectionChanged(const QItemSelection&, const QItemSelection&);
	void mouseDoubleClickEvent(QMouseEvent*);
private:
	QList<PackageListItem*> m_selectedPackages;
	QMap<QString, PackageListItem*> m_packageIndex;

};

#endif
