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
	
	void							edit( PackageItem* portagePackage, int view );
	bool							isParentView( int view );
	void							showHardMaskInfo();
	
	InspectorBase					*dialog;
	
private:
	void							updateVersionData();
	void							rollbackSettings();
	void							loadUseFlagDescription();
	void							loadChangeLog();
	
private slots:
	void							slotHardMaskInfo();
	void							slotPreviousPackage();
	void							slotNextPackage();
	void							showSettings();
	void							slotRefreshTabs();
	void							slotLoadInstalledFiles( const QString& version );
	void							slotApply();
	void							slotCancel();
	void							slotOk();
	void							slotSetStability( int rbStability );
	void							slotSetSpecificVersion( const QString& version );
	void							slotSetUseFlags( QListViewItem* useItem );
	void							slotLoadEbuild( const QString& version );
	void							slotLoadDependencies( const QString& version );
	void							slotLoadUseFlags( const QString& version );
	void							slotCalculateUse();
	void							slotCollectPretendOutput( KProcIO* eProc );
	void							slotParsePackageUse( KProcess* eProc );
	void							slotParseTempUse( KProcess* eProc );
	void							slotQueue();
	void							slotWorld();
	
private:
	
	// Wiew that called the Inspector
	int								m_view;
	
	// Keep track when user changes any version masking settings
	bool							m_versionSettingsChanged;
	
	// Keep track when user changes use settings
	bool							m_useSettingsChanged;
	
	// Is this package settings untouched
	bool							m_isVirginState;
	
	QString							m_id, m_category, m_package, m_hardMaskComment;
	QMap<QString, QString>			m_useMap;
	PackageItem* 					m_portagePackage;
	int								m_stabilityBefore;
	QString							m_versionBefore;
	QStringList						m_pretendUseLines;
	QStringList 					m_useList;
	
signals:
	void							signalNextPackage( bool up );
	void							signalPackageChanged();
};

#endif
