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

#ifndef _LINKWIDGET_H_
#define _LINKWIDGET_H_

#include <qwidget.h>
#include <qpixmap.h>
#include <kurllabel.h>
#include <kiconloader.h>

class LinkWidget : public QWidget
{
  Q_OBJECT

  public:
    LinkWidget(QWidget *parent = 0, const char *name = 0);
    ~LinkWidget();
    
    const QString& icon() const;
    const QString text() const;
    const QString& url () const;
    
  public slots:
    void setText(const QString &text);
    void setIcon(const QString &icn);
    void setURL(const QString &url);
    
  protected:
    KURLLabel *m_icon;
    KURLLabel *m_link;
    
    QString icon_name;
    
  private slots:
    void activateIcon();
    void deactivateIcon();
    
  signals:
    void  leftClickedURL(const QString &url);
    void  leftClickedURL();
    void  rightClickedURL(const QString &url);
    void  rightClickedURL();
    void  middleClickedURL(const QString &url);
    void  middleClickedURL();
};

#endif
