
#ifndef KONQTEST1_H
#define KONQTEST1_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <konqsidebarplugin.h>
#include <qstring.h>
#include <kconfig.h>


class konqtest1 : public KonqSidebarPlugin
{
    Q_OBJECT
    
public:
    /**
     * Construct a @ref KonqSidebarPlugin.
     *
     * @param inst The sidebar's kinstance class.
     * @param parent The sidebar internal button info class responsible for this plugin.
     * @param widgetParent The container which will contain the plugins widget.
     * @param desktopName The filename of the configuration file.
     * @param name A Qt object name for your plugin.
     **/
     konqtest1(KInstance *inst,QObject *parent,QWidget *widgetParent, QString &desktopName, const char* name=0);

    /** destructor */
    ~konqtest1();
    
   virtual QWidget *getWidget(){return widget;}
   virtual void *provides(const QString &) {return 0;}

protected:
	/*Example widget only. You use whichever widget you want. You onl have to care that getWidget returns some kind of widget */
	QWidget *widget;
    virtual void handleURL(const KURL &url);
		
private:
	KParts::ReadWritePart *m_part;

};

#endif
