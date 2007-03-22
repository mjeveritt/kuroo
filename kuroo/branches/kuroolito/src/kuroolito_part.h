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

#ifndef KUROOLITO_PART_H
#define KUROOLITO_PART_H

#include "portagetab.h"

#include <kparts/part.h>
#include <kparts/factory.h>

class KActionMenu;
class KConfigDialog;
class KURL;
class IntroDlg;
class KCmdLineArgs;
class KuroolitoInit;
class Message;
class Queue;
class Results;
class SystemTray;

class KuroolitoPart : public KParts::ReadWritePart
{
	Q_OBJECT
public:
    KuroolitoPart( QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name );

    virtual ~KuroolitoPart();

protected:
    /**
     * This must be implemented by each part
     */
    virtual bool 		openFile();

    /**
     * This must be implemented by each read-write part
     */
    virtual bool 		saveFile();

protected slots:
    void 				fileOpen();
    void 				fileSaveAs();

private slots:
	void 				slotPreferences();

private:
	void 				setupActions();
	
private:
	SystemTray			*systemTray;
	Results 			*kurooResults;
	Queue 				*kurooQueue;
	Message 			*kurooMessage;
	KuroolitoInit 		*kurooInit;
	KConfigDialog 		*prefDialog;
	IntroDlg 			*wizardDialog;
	bool 				m_shuttingDown;
	KActionMenu			*actionRefreshPortage, *actionRefreshUpdates;
	PortageTab 			*viewPortage;
};

class KInstance;
class KAboutData;

class KuroolitoPartFactory : public KParts::Factory
{
    Q_OBJECT
public:
    KuroolitoPartFactory();
    virtual ~KuroolitoPartFactory();
    virtual KParts::Part* createPartObject( QWidget *parentWidget, const char *widgetName,
                                            QObject *parent, const char *name,
                                            const char *classname, const QStringList &args );
    static KInstance* instance();
 
private:
    static KInstance* s_instance;
    static KAboutData* s_about;
};

#endif
