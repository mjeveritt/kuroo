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


#ifndef _KUROO_H_
#define _KUROO_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "configdialog.h"
#include "mainwindow.h"
#include "kurooview.h"

#include <kapplication.h>

class KAction;
class KConfigDialog;
class KURL;
class IntroDlg;
class KCmdLineArgs;
class KurooInit;
class Message;
class Queue;
class Results;

/**
 * @class Kuroo
 * @short Create main window with menus, system tray icon and statusbar.
 */
class Kuroo : public MainWindow
{
Q_OBJECT
public:
	Kuroo();
	virtual ~Kuroo();
	
private slots:
	
	/**
	 * Dialogs:
	 * Show the wizard.
	 * Show the full emerge log 
	 * Kuroo preferences window.
	 */
	void 				introWizard();
	void 				slotPreferences();
	
	/**
	 * Disable buttons when kuroo is busy.
	 */
	void				slotBusy( bool b );
	
	/**
	 * Launch emerge portage sync.
	 */
	void				slotSync();
	
	/**
	 * Hide kuroo window when clicking in close button.
	 * Wait for the backup of the log is completed before terminating.
	 */
	bool 				queryClose();
	bool 				queryExit();
	void 				slotQuit();
	void				slotWait();
	void 				slotTerminate();
	
	void				slotNull();
	
	/**
	* Build mainwindow menus and toolbar.
	*/
private:
	void 				setupActions();
	
private:
	Results 			*kurooResults;
	Queue 				*kurooQueue;
	Message 			*kurooMessage;
	KurooInit 			*kurooInit;
	KurooView 			*m_view;
	KConfigDialog 		*prefDialog;
	IntroDlg 			*wizardDialog;
	bool 				m_shuttingDown;
	KAction 			*actionRefresh, *actionSync, *actionFind;
};

#endif // _KUROO_H_

