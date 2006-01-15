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
#include "portagelistview.h"

#include <kdialogbase.h>

/**
 * @class PackageInspector
 * @short Specialized dialog 
 */
class PackageInspector : public KDialogBase
{
Q_OBJECT
public:
    PackageInspector( QWidget *parent = 0 );
    ~PackageInspector();
	
	/**
	 * Open use flags dialog.
	 * @param newPackage	selected package
	 */
	void							edit( PortageListView::PortageItem* portagePackage );
	void							showHardMaskInfo();
	
	InspectorBase					*dialog;
	
private slots:
	void							slotPreviousPackage();
	void							slotNextPackage();
	void							slotInstallVersion();
	void							slotActivateTabs();
	void							slotAdvancedToggle( bool isOn );
	
	void							slotGetInstalledFiles( const QString& version );
	void							slotApply();
	void							slotCancel();
	void							rollbackSetting();
	
	void							slotSetStability( int rbStability );
	void							slotSetSpecificVersion( const QString& version );
	void							slotAvailable( bool isAvailable );
	
	void							loadUseFlagDescription();
	void							slotGetEbuild( const QString& version );
	void							getChangeLog();
	void							slotGetDependencies( const QString& version );
	void							slotGetUseFlags( const QString& version );
	
private:
	bool							hasSettingsChanged;
	QString							category, package, packageId;
	QMap<QString, QString>			useMap;
	PortageListView::PortageItem* 	m_portagePackage;
	int								stability, stabilityBefore, stabilityAfter;
	QString							versionBefore;
	
signals:
	void							signalNextPackage( bool up );
	void							signalPackageChanged();
};

#endif
