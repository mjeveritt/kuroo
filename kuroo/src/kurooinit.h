/***************************************************************************
 *   Copyright (C) 2005 by Karye   *
 *   karye@users.sourceforge.net   *
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

#ifndef KUROOINIT_H
#define KUROOINIT_H

#include <qobject.h>

class KCmdLineArgs;
class IntroDlg;

/**
 * @class KurooInit
 * @short Checks that kuroo environment is correctly setup.
 */
class KurooInit : public QObject
{
Q_OBJECT
public:
	KurooInit( QObject *parent = 0, const char *name = 0 );
    ~KurooInit();
	
private slots:
	void 			firstTimeWizard();
	bool 			getEnvironment();
	void			checkUser();
	
private:
	IntroDlg 		*wizardDialog;
};

#endif
