#ifndef METABAR_H
#define METABAR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <konqsidebarplugin.h>
#include <qstring.h>
#include <qscrollview.h>

// #include "metabar_widget.h"
// #include "metascrollview.h"

class Metabar : public KonqSidebarPlugin
{
    Q_OBJECT
    
public:
  Metabar(KInstance *inst,QObject *parent,QWidget *widgetParent, QString &desktopName, const char* name=0);
  ~Metabar();
    
  virtual QWidget *getWidget(){return view;}
  virtual void *provides(const QString &) {return 0;}

protected:
	QWidget		*widget;
	QScrollView	*view;
//   MetabarWidget *widget;
//   MetaScrollView *view;
  
  virtual void handleURL(const KURL &url);
  virtual void handlePreview(const KFileItemList & items);

};

#endif
