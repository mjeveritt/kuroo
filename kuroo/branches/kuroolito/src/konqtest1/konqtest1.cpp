
#include <qlabel.h>
#include <kinstance.h>
#include <qstring.h>
#include <qwidget.h>

#include "konqtest1.h"
#include "konqtest1.moc"
#include <klibloader.h>

konqtest1::konqtest1(KInstance *inst,QObject *parent,QWidget *widgetParent, QString &desktopName, const char* name):
                   KonqSidebarPlugin(inst,parent,widgetParent,desktopName,name)
{
	KLibFactory *factory = KLibLoader::self()->factory("libkuroolitopart");
    if (factory) {
        // now that the Part is loaded, we cast it to a Part to get
        // our hands on it
        m_part = static_cast<KParts::ReadWritePart *>(factory->create(this, "kuroolito_part", "KParts::ReadWritePart" ));

        if (m_part) {
            // tell the KParts::MainWindow that this is indeed the main widget
            widget = m_part->widget();

            // and integrate the part's GUI with the shell's
//             createGUI(m_part);
        }
    }
    else {
		widget = new QLabel("Init Value",widgetParent);
	}
}

konqtest1::~konqtest1()
{
}

void konqtest1::handleURL(const KURL &url)
{
// 	widget->setText(QString("konqtest1")+"::"+url.url());
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
                map->insert("Name", "Portage");
                map->insert("Open", "true");
                map->insert("X-KDE-KonqSidebarModule","konqsidebar_konqtest1");
                fn->setLatin1("konqtest1%1.desktop");
                return true;
        }
};
