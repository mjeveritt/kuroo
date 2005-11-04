
#ifndef PORTAGEPACKAGESVIEW_H
#define PORTAGEPACKAGESVIEW_H

#include <qobject.h>
#include <qvbox.h>

class PortageListView;

/**
 * @class PortagePackagesView
 * @short Connect portage package listview to searchline.
 */
class PortagePackagesView : public QVBox
{
Q_OBJECT
public:
	PortagePackagesView( QWidget *parent = 0, const char *name = 0 );
    ~PortagePackagesView();

	PortageListView *packagesView;
};

#endif
