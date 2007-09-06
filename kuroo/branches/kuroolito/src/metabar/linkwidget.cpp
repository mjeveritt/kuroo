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

#include <qlayout.h>
#include <kglobalsettings.h>
#include <kiconeffect.h>

#include "linkwidget.h"

LinkWidget::LinkWidget(QWidget *parent, const char *name) : QWidget(parent, name){
  m_link = new KURLLabel(this);
  m_link->setUnderline(false);
  m_link->setHighlightedColor(KGlobalSettings::textColor());
  m_link->setSelectedColor(KGlobalSettings::textColor());
  m_link->setFloat(true);
  
  m_icon = new KURLLabel(this);
  
  connect(m_link, SIGNAL(leftClickedURL (const QString &))  , this, SIGNAL(leftClickedURL (const QString &)));
  connect(m_link, SIGNAL(leftClickedURL ())                 , this, SIGNAL(leftClickedURL ()));
  connect(m_link, SIGNAL(rightClickedURL (const QString &)) , this, SIGNAL(rightClickedURL (const QString &)));
  connect(m_link, SIGNAL(rightClickedURL ())                , this, SIGNAL(rightClickedURL ()));
  connect(m_link, SIGNAL(middleClickedURL (const QString &)), this, SIGNAL(middleClickedURL (const QString &)));
  connect(m_link, SIGNAL(middleClickedURL ())               , this, SIGNAL(middleClickedURL ()));
  connect(m_link, SIGNAL(enteredURL ())                     , this, SLOT(activateIcon()));
  connect(m_link, SIGNAL(leftURL ())                        , this, SLOT(deactivateIcon()));
  
  connect(m_icon, SIGNAL(leftClickedURL (const QString &))  , this, SIGNAL(leftClickedURL (const QString &)));
  connect(m_icon, SIGNAL(leftClickedURL ())                 , this, SIGNAL(leftClickedURL ()));
  connect(m_icon, SIGNAL(rightClickedURL (const QString &)) , this, SIGNAL(rightClickedURL (const QString &)));
  connect(m_icon, SIGNAL(rightClickedURL ())                , this, SIGNAL(rightClickedURL ()));
  connect(m_icon, SIGNAL(middleClickedURL (const QString &)), this, SIGNAL(middleClickedURL (const QString &)));
  connect(m_icon, SIGNAL(middleClickedURL ())               , this, SIGNAL(middleClickedURL ()));
  connect(m_icon, SIGNAL(enteredURL ())                     , this, SLOT(activateIcon ()));
  connect(m_icon, SIGNAL(leftURL ())                        , this, SLOT(deactivateIcon()));

  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setSpacing(5);
  layout->addWidget(m_icon);
  layout->addWidget(m_link);
  layout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
}

LinkWidget::~LinkWidget(){
}

void LinkWidget::activateIcon()
{
  QPixmap pix = SmallIcon(icon_name);
  QImage img = pix.convertToImage();

  KIconEffect::toGamma(img, float(0.75));
  
  m_icon->setPixmap(QPixmap(img));
}

void LinkWidget::deactivateIcon()
{
  m_icon->setPixmap(SmallIcon(icon_name));
}

const QString& LinkWidget::icon() const{
 return icon_name;
}

const QString LinkWidget::text() const{
  return m_link->text();
}

const QString& LinkWidget::url() const{
  return m_link->url();
}
    
void LinkWidget::setText(const QString &text)
{
  m_link->setText(text);
}

void LinkWidget::setIcon(const QString &icn)
{
  icon_name = icn;
  m_icon->setPixmap(SmallIcon(icon_name));
}

void LinkWidget::setURL(const QString &url)
{
  m_link->setURL(url);
  m_icon->setURL(url);
}
