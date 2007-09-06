
#include <qlabel.h>
#include <kinstance.h>
#include <qstring.h>
#include <qwidget.h>

#include "konqtest1.h"
#include "konqtest1.moc"

konqtest1::konqtest1(KInstance *inst,QObject *parent,QWidget *widgetParent, QString &desktopName, const char* name):
                   KonqSidebarPlugin(inst,parent,widgetParent,desktopName,name)
{
	widget=new QLabel("Init Value",widgetParent);
}


konqtest1::~konqtest1()
{
}

void konqtest1::handleURL(const KURL &url)
{
	widget->setText(QString("konqtest1")+"::"+url.url());
}

extern "C"
{
    void* create_konqsidebar_konqtest1(KInstance *instance,QObject *par,QWidget *widp,QString &desktopname,const char *name)
    {
        return new konqtest1(instance,par,widp,desktopname,name);
    }
};

extern "C" {
        KDE_EXPORT bool add_konqsidebar_konqtest1(QString* fn, QString* param, QMap<QString,QString> *map) {
                Q_UNUSED(param);
                map->insert("Type", "Link");
                map->insert("URL", "");
                map->insert("Icon", "kuroo");
                map->insert("Name", i18n("Portage"));
                map->insert("Open", "true");
                map->insert("X-KDE-KonqSidebarModule","konqsidebar_konqtest1");
                fn->setLatin1("konqtest1%1.desktop");
                return true;
        }
};