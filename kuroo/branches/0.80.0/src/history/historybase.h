/****************************************************************************
** Form interface generated from reading ui file '/home/karye/repository/kuroo-svn/kuroo/branches/0.80.0/src/history/historybase.ui'
**
** Created: Tue Nov 29 17:00:00 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.4   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef HISTORYBASE_H
#define HISTORYBASE_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class HistoryListView;
class QCheckBox;
class KListViewSearchLineWidget;

class HistoryBase : public QWidget
{
    Q_OBJECT

public:
    HistoryBase( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~HistoryBase();

    HistoryListView* historyView;
    QCheckBox* kcfg_viewUnmerges;
    KListViewSearchLineWidget* searchLineHistory;

protected:
    QGridLayout* HistoryBaseLayout;
	QFrame* 	 line1;
	QGridLayout* layout1;
	QGridLayout* layout3;
    QSpacerItem* spacer3;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;

};

#endif // HISTORYBASE_H
