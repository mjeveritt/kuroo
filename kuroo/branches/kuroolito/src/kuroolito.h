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


#ifndef _KUROOLITO_H_
#define _KUROOLITO_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "configdialog.h"
#include "portagetab.h"

#include <kapplication.h>
#include <kparts/mainwindow.h>

class KAction;
class KConfigDialog;
class KURL;
class IntroDlg;
class KCmdLineArgs;
class KuroolitoInit;
class Message;
class Queue;
class Results;
class SystemTray;

/**
 * @class Kuroolito
 * @short Main kde window with menus, system tray icon and statusbar.
 */
class Kuroolito : public KParts::MainWindow
{
Q_OBJECT
public:
	Kuroolito();
	virtual ~Kuroolito();
	
private slots:
	void 				slotQuit();
	bool 				queryClose();
	
private:
	void 				setupActions();
	
private:
	KParts::ReadWritePart *m_part;
	SystemTray			*systemTray;
	IntroDlg 			*wizardDialog;
	bool 				m_shuttingDown;
	KAction 			*actionRefreshPortage, *actionRefreshUpdates, *actionSyncPortage;
	PortageTab 			*viewPortage;
};

#endif // _KUROOLITO_H_

