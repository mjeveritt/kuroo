//Authors: Karye; Fauconneau
#ifndef _KUROO_H_
#define _KUROO_H_

#include <QProgressBar>
#include <QLabel>
#include <QMenu>
#include <QSqlDatabase>

#include <KLocale>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KApplication>
#include <KXmlGuiWindow>
//#include <KProcess>
#include <KUser>
#include <KAction>
#include <KActionCollection>
#include <KStandardAction>
#include <KPageWidget>
#include <KStatusBar>
#include <KMessageBox>
#include <KToolBar>
#include <KMenuBar>

/**
 * @class Kuroo
 * @short Main window with menus, systray icon, statusbar and pages
 */
class Kuroo : public KXmlGuiWindow
{
    Q_OBJECT
public:
	Kuroo();
};

#endif // _KUROO_H_
