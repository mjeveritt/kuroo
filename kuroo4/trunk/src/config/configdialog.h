/**************************************************************************
*   Copyright (C) 2004 by                                                 *
*   karye@users.sourceforge.net                                           *
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

#include <initializer_list>
#include <utility>

#include <QHash>
#include <QRegularExpression>

#include <KConfigDialog>
#include <KConfigSkeleton>

#include "ui_options1.h"
#include "ui_options2.h"
#include "ui_options3.h"
#include "ui_options4.h"
#include "settings.h"

/**
 * @class ConfigDialog
 * @short Kuroo preferences.
 */
class ConfigDialog : public KConfigDialog
{
Q_OBJECT

public:
	ConfigDialog( QWidget *parent, const QString& name, KConfigSkeleton *config );
	~ConfigDialog();

private:
	const QStringList 	readMakeConf();
	void 				parseMakeConf();
	bool 				saveMakeConf();
	static void			handleELogClasses( const QString& value );
	static void			handleELogSystem( const QString& value );

private slots:
	void 				slotSaveAll();
	void				slotDefaults();

private:
	bool				m_isDefault;
	Ui::Options1 form1;
	Ui::Options2 form2;
	Ui::Options3 form3;
	Ui::Options4 form4;

	inline static const QRegularExpression m_rxLine = QRegularExpression( "^(\\w*=)\\s*\"?([^\"#]*)\"?#*" );

	inline static const QHash<QString, void (*)(const QString&)> m_configHash = QHash<QString, void (*)(const QString&)>
		{{QString("ACCEPT_KEYWORDS="), KurooConfig::setAcceptKeywords},
		{QString("AUTOCLEAN="), NULL},
		{QString("BUILD_PREFIX="), KurooConfig::setBuildPrefix},
		{QString("CBUILD="), KurooConfig::setCBuild},
		{QString("CCACHE_SIZE="), KurooConfig::setCCacheSize},
		{QString("CFLAGS="), KurooConfig::setCFlags},
		{QString("CXXFLAGS="), KurooConfig::setCXXFlags},
		{QString("CHOST="), KurooConfig::setChost},
		{QString("CLEAN_DELAY="), KurooConfig::setCleanDelay},
		{QString("CONFIG_PROTECT="), KurooConfig::setConfigProtect},
		{QString("CONFIG_PROTECT_MASK="), KurooConfig::setConfigProtectMask},
		{QString("DEBUGBUILD="), KurooConfig::setDebugBuild},
		{QString("DISTDIR="), KurooConfig::setDirDist},
		{QString("FEATURES="), KurooConfig::setFeatures},
		{QString("FETCHCOMMAND="), KurooConfig::setFetchCommand},
		{QString("GENTOO_MIRRORS="), KurooConfig::setGentooMirrors},
		{QString("FTP_PROXY="), KurooConfig::setFtpProxy},
		{QString("HTTP_PROXY="), KurooConfig::setHttpProxy},
		{QString("MAKEOPTS="), KurooConfig::setMakeOpts},
		{QString("NOCOLOR="), NULL},
		{QString("PKG_TMPDIR="), KurooConfig::setDirPkgTmp},
		{QString("PKGDIR="), KurooConfig::setDirPkg},
		{QString("PORT_LOGDIR="), KurooConfig::setDirPortLog},
		{QString("PORTAGE_BINHOST="), KurooConfig::setPortageBinHost},
		{QString("PORTAGE_ELOG_CLASSES="), ConfigDialog::handleELogClasses},
		{QString("PORTAGE_ELOG_SYSTEM="), ConfigDialog::handleELogSystem},
		{QString("PORTAGE_ELOG_COMMAND="), KurooConfig::setElogCustomCmd},
		{QString("PORTAGE_ELOG_MAILURI="), KurooConfig::setElogMailURI},
		{QString("PORTAGE_ELOG_MAILFROM="), KurooConfig::setElogMailFromURI},
		{QString("PORTAGE_ELOG_MAILSUBJECT="), KurooConfig::setElogSubject},
		{QString("PORTAGE_NICENESS="), KurooConfig::setPortageNiceness},
		{QString("PORTAGE_TMPDIR="), KurooConfig::setDirPortageTmp},
		{QString("PORTDIR="), KurooConfig::setDirPortage},
		{QString("PORTDIR_OVERLAY="), KurooConfig::setDirPortageOverlay},
		{QString("RESUMECOMMAND="), KurooConfig::setResumeCommand},
		{QString("ROOT="), KurooConfig::setRoot},
		{QString("RSYNC_EXCLUDEFROM="), KurooConfig::setRsyncExcludeFrom},
		{QString("RSYNC_PROXY="), KurooConfig::setRsyncProxy},
		{QString("RSYNC_RETRIES="), KurooConfig::setRsyncRetries},
		{QString("RSYNC_RATELIMIT="), KurooConfig::setRsyncRateLimit},
		{QString("RSYNC_TIMEOUT="), KurooConfig::setRsyncTimeOut},
		{QString("RPMDIR="), KurooConfig::setDirRpm},
		{QString("SYNC="), KurooConfig::setSync},
		{QString("USE="), KurooConfig::setUse},
		{QString("USE_ORDER="), KurooConfig::setUseOrder}};
		
		inline static const QRegularExpression m_rxAnyConfigVars =
			QRegularExpression( "(" + QStringList( m_configHash.keys() ).replaceInStrings("=", "").join('|') + ")" );
};

#endif
