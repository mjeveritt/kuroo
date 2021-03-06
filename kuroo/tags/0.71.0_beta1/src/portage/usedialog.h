/***************************************************************************
*   Copyright (C) 2004 by karye                                           *
*   karye@users.sourceforge.net                                           *
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

#ifndef USEDIALOG_H
#define USEDIALOG_H

#include "usebase.h"

#include <kdialogbase.h>

/**
 * @class UseDialog
 * @short Specialized dialog for editing Use Flags per package.
 */
class UseDialog : public KDialogBase
{
Q_OBJECT
public:
    UseDialog(QWidget *parent = 0, const char *name = 0);
    ~UseDialog();
	
	/**
	 * Open use flags dialog.
	 * @param newPackage	selected package
	 */
	void			edit( const QString& package );
	
private slots:
	void			slotUseDescription( QListBoxItem* item );
	void			slotApply();

private:
	QStringList		useList;
	UseBase		*dialog;
	QString		package;
	
};

#endif
