
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
