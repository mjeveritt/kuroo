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

#ifndef PACKAGEINSPECTOR_H
#define PACKAGEINSPECTOR_H

#include "inspectorbase.h"

#include <kdialogbase.h>

/**
 * @class PackageInspector
 * @short Specialized dialog 
 */
class PackageInspector : public KDialogBase
{
Q_OBJECT
public:
    PackageInspector( QWidget *parent = 0, const char *name = 0 );
    ~PackageInspector();
	
	/**
	 * Open use flags dialog.
	 * @param newPackage	selected package
	 */
	void			edit( const QString& packageId );
	
private slots:
	void			loadEbuild( const QString& packageId );
	void			changelog( const QString& packageId );
	void			dependencies( const QString& packageId );
	void			loadVersions( const QString& packageId );
	void			loadUseFlags( const QString& packageId );
	void			slotUseDescription( QListBoxItem* item );
	void			slotApply();

private:
	QStringList		useList;
	InspectorBase	*dialog;
	
};

#endif
