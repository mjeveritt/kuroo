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
 * @short The package Inspector dialog for all advanced settings.
 */
class PackageInspector : public KDialogBase
{
Q_OBJECT
public:
    PackageInspector( QWidget *parent = 0 );
    ~PackageInspector();
	
	void							edit( PackageItem* portagePackage, const QString& emergeVersion, int view );
	bool							isParentView( int view );
	void							showHardMaskInfo();
	
	InspectorBase					*dialog;
	
private slots:
	void							slotHardMaskInfo();
	void							slotPreviousPackage();
	void							slotNextPackage();
	void							showSettings();
	void							slotRefreshTabs();
	void							slotAdvancedToggle( bool isOn );
	void							slotLoadInstalledFiles( const QString& version );
	void							slotApply();
	void							slotCancel();
	void							slotOk();
	void							rollbackSettings();
	void							slotSetStability( int rbStability );
	void							slotSetSpecificVersion( const QString& version );
	void							slotSetAvailable( bool isAvailable );
	void							slotSetUseFlags( QListViewItem* useItem );
	void							loadUseFlagDescription();
	void							slotLoadEbuild( const QString& version );
	void							loadChangeLog();
	void							slotLoadDependencies( const QString& version );
	void							slotLoadUseFlags( const QString& version );
	void							slotCalculateUse();
	void							slotCollectPretendOutput( KProcIO* eProc );
	void							slotParsePackageUse( KProcess* eProc );
	void							slotParseTempUse( KProcess* eProc );
	
private:
	// Which view called the Inspector
	int								m_view;
	
	// Version which will be used by emerge
	QString							m_emergeVersion;
	
	bool							m_versionSettingsChanged, m_useSettingsChanged, m_isVirginState, m_isAvailableBefore;
	QString							m_id, m_category, m_package, m_hardMaskComment;
	QMap<QString, QString>			m_useMap;
	PackageItem* 					m_portagePackage;
	int								m_stabilityBefore;
	QString							m_versionBefore;
	QStringList						pretendUseLines;
	QStringList 					m_useList;
	
signals:
	void							signalNextPackage( bool up );
	void							signalPackageChanged();
};

#endif
