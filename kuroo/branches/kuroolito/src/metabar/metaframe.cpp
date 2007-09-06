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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
 
#include <kglobalsettings.h>
#include <qfont.h>
#include <qpainter.h>
#include <kdebug.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <kiconloader.h>
#include <kconfig.h>

#include "metaframe.h"

MetaFrame::MetaFrame(KConfig *config, QWidget *parent, const char *name) : QFrame(parent, name)
{
  MetaFrame::config = config;

  timer = new QTimer();
  connect(timer, SIGNAL(timeout()), this, SLOT(animateResizeImpl()));

  setPaletteBackgroundColor(KGlobalSettings::baseColor());
  //setLineWidth(0);
  setFrameShape(QFrame::StyledPanel);
  setFrameShadow(QFrame::Raised);

  
  titleButton = new KPushButton(this);
  titleButton->setToggleButton(true);
  connect(titleButton, SIGNAL(toggled(bool)), this, SLOT(collapse(bool)));
  connect(titleButton, SIGNAL(toggled(bool)), this, SIGNAL(toggled(bool)));
  
  main = new QWidget(this);
  main->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  
  layout = new QVBoxLayout(main, 4, 5);
  
  QVBoxLayout *mainLayout = new QVBoxLayout(this, 2, 1);
  mainLayout->addWidget(titleButton);
  mainLayout->addWidget(main);
}

MetaFrame::~MetaFrame(){
}

void MetaFrame::addWidget(QWidget *w, bool adjust)
{
  layout->addWidget(w);
  
  if(adjust){
    resize();
  }
}
    
void MetaFrame::setTitle(const QString &title)
{  
  titleButton->setText(title);
}

const QString MetaFrame::title()
{
  return titleButton->text();
}

void MetaFrame::setExpanded(bool b)
{
  titleButton->setOn(!b);

  if(b){
    main->show();
  }

  resize();
}

void MetaFrame::collapse(bool b)
{
  setExpanded(!b);
}

bool MetaFrame::isExpanded()
{
  return !titleButton->isOn();
}

QWidget* MetaFrame::mainWidget()
{
  return main;
}

void MetaFrame::resize()
{
  main->updateGeometry();
  
  if(config->readBoolEntry("AnimateResize", false)){
    animateResize();
  }
  else{
    main->setFixedHeight(isExpanded() ? main->sizeHint().height() : 0);
  }
}

void MetaFrame::animateResize()
{
  main->updateGeometry();
  wishedHeight = isExpanded() ? main->sizeHint().height() : 0;
  
  if(!timer->isActive()){
    main->setFixedHeight(main->height());
    
    timer->start(5);
  }
}

void MetaFrame::animateResizeImpl()
{
  int currentHeight = main->height();
  int scrollAmount = currentHeight < wishedHeight ? 3 : -3;
  
  
  int newHeight = scrollAmount > 0 ? kMin(currentHeight + scrollAmount, wishedHeight) : kMax(currentHeight + scrollAmount, wishedHeight); 
  
  main->setFixedHeight(newHeight);
    
  if((scrollAmount / kAbs(scrollAmount)) *newHeight >= wishedHeight){
    if(!isExpanded()){
      main->hide();
    }
    
    timer->stop();
  }
}
