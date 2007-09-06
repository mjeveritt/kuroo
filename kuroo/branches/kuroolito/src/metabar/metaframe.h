/***************************************************************************
 *   Copyright (C) 2005 by Florian Roth   *
 *   florian@synatic.net   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 
#ifndef _METAFRAME_H_
#define _METAFRAME_H_

#include <qframe.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qevent.h>
#include <qtimer.h>
#include <kpushbutton.h>
#include <kconfig.h>

class MetaFrame : public QFrame
{
  Q_OBJECT
  
  public:
    MetaFrame(KConfig *config, QWidget *parent = 0, const char *name = 0);
    ~MetaFrame();
    
    void addWidget(QWidget *widget, bool adjust = true);
    
    const QString title();
    bool isExpanded();
    QWidget* mainWidget();
  
  public slots:
    void setTitle(const QString &title);
    void setExpanded(bool b = true);
    void collapse(bool b);
    void animateResize();
    void resize();
  
  private:
    QWidget *main;
    QVBoxLayout *layout;
    
    KPushButton *titleButton;
    
    KConfig *config;
    
    QTimer *timer;
    int wishedHeight;
    
  private slots:
    void animateResizeImpl();
    
  signals:
    void toggled(bool);
};

#endif
