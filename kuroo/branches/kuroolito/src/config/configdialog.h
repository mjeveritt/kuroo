/**************************************************************************
*   Copyright (C) 2004 by                                                 *
*   info@kuroo.org                                                        *
*   Stefan Bogner <bochi@online.ms>                                       *
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

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <kconfigdialog.h>

class QComboBox;
class QGroupBox;
class QVBox;
class Options2;

/**
 * @class ConfigDialog
 * @short Kuroolito preferences.
 */
class ConfigDialog : public KConfigDialog
{
Q_OBJECT

public:
	ConfigDialog( QWidget *parent, const char *name, KConfigSkeleton *config );
	~ConfigDialog();
	
private:
	
private slots:

};

#endif
