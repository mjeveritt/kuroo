/**************************************************************************
*	Copyright (C) 2004 by													*
*	karye@users.sourceforge.net												*
*	Stefan Bogner <bochi@online.ms>											*
*	This program is free software; you can redistribute it and/or modify	*
*	it under the terms of the GNU General Public License as published by	*
*	the Free Software Foundation; either version 2 of the License, or		*
*	(at your option) any later version.										*
*																			*
*	This program is distributed in the hope that it will be useful,			*
*	but WITHOUT ANY WARRANTY; without even the implied warranty of			*
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the			*
*	GNU General Public License for more details.							*
*																			*
*	You should have received a copy of the GNU General Public License		*
*	along with this program; if not, write to the							*
*	Free Software Foundation, Inc.,											*
*	59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.				*
***************************************************************************/

#include <QCheckBox>
#include <QHash>
#include <QFile>
#include <QTextStream>

#include <KConfig>
#include <KConfigDialog>
#include <KConfigGroup>
#include <KMessageBox>

#include "common.h"
#include "configdialog.h"
#include "log.h"
#include "systemtray.h"

/**
* @class ConfigDialog
* @short Kuroo preferences.
*
* Build settings widget for kuroo and make.conf.
* Parses make.conf and tries to keep user-format when saving back settings.
*/
ConfigDialog::ConfigDialog( QWidget *parent, const QString& name, KConfigSkeleton *config )
	: KConfigDialog( parent, name, config ), m_isDefault( false )
{
	QWidget::setAttribute(Qt::WA_DeleteOnClose);

	setFaceType(KPageDialog::Tabbed);

	QDialog *opt1 = new QDialog();
	form1.setupUi( opt1 ); //, i18n("General") );
	QDialog *opt2 = new QDialog();
	form2.setupUi( opt2 ); //, i18n("make.conf") );
	QDialog *opt3 = new QDialog();
	form3.setupUi( opt3 ); //, i18n("Etc-update warnings") );
	QDialog *opt4 = new QDialog();
	form4.setupUi( opt4 ); //, i18n("Housekeeping") );

	addPage( opt1, i18n("General"), QString("kuroo"), i18n("General preferences") );
	addPage( opt2, i18n("make.conf"), QString("kuroo_makeconf"), i18n("Edit your make.conf file") );
	addPage( opt3, i18n("Etc-update warnings"), QString("dialog-warning"), i18n("Edit your etc-update warning file list") );
	addPage( opt4, i18n("Housekeeping"), QString("kuroo_housekeeping"), i18n("Control automatic file cleanup and rebuilding") );

	connect(this, &ConfigDialog::settingsChanged, this, &ConfigDialog::slotSaveAll);
	connect(buttonBox()->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &ConfigDialog::slotDefaults);
/*
	int selected = 0;
	for(int i = 0; i < KurooConfig::filePackageKeywords().count(); ++i)
	{
		QString f = KurooConfig::filePackageKeywords()[i];
		if (f == KurooConfig::defaultFilePackageKeywords())
			selected = i;
		form1.keywords->addItem(f);
	}
	form1.keywords->setCurrentIndex(selected);

	selected = 0;
	for(int i = 0; i < KurooConfig::filePackageUserUse().count(); ++i)
	{
		QString f = KurooConfig::filePackageUserUse()[i];
		if (f == KurooConfig::defaultFilePackageUserUse())
			selected = i;
		form1.use->addItem(f);
	}
	form1.use->setCurrentIndex(selected);

	selected = 0;
	for(int i = 0; i < KurooConfig::filePackageUserMask().count(); ++i)
	{
		QString f = KurooConfig::filePackageUserMask()[i];
		if (f == KurooConfig::defaultFilePackageUserMask())
			selected = i;
		form1.mask->addItem(f);
	}
	form1.mask->setCurrentIndex(selected);

	selected = 0;
	for(int i = 0; i < KurooConfig::filePackageUserUnMask().count(); ++i)
	{
		QString f = KurooConfig::filePackageUserUnMask()[i];
		if (f == KurooConfig::defaultFilePackageUserUnMask())
			selected = i;
		form1.unmask->addItem(f);
	}
	form1.unmask->setCurrentIndex(selected);
*/
	parseMakeConf();
}

ConfigDialog::~ConfigDialog()
{}

/**
* Reset to defaults.
*/
void ConfigDialog::slotDefaults()
{
	DEBUG_LINE_INFO;
	parseMakeConf();
	show();
}

/**
* Save settings when user press "Apply".
*/
void ConfigDialog::slotSaveAll()
{
	Log::buffer_MaxLines=KurooConfig::logLines();
	qDebug() << "Buffer changed to " << Log::buffer_MaxLines << "\n";
	DEBUG_LINE_INFO;
	qDebug() << "HACK\n";
	/*switch( activePageIndex() ) {
		// Activate the systray directly (not needing restarting kuroo)
		case 0: {
			if ( KurooConfig::isSystrayEnabled() )
				SystemTray::instance()->activate();
			else
				SystemTray::instance()->inactivate();

			SignalistSingleton::Instance()->fontChanged();
			break;
		}

		case 1:
			if ( !saveMakeConf() ) {
				parseMakeConf();
				show();
				KMessageBox::error( this, i18n( "Failed to save %1. Please run as root.", KurooConfig::fileMakeConf() ), i18n( "Saving" ));
			}
	}*/

	/*KurooConfig::setDefaultFilePackageKeywords( form1.keywords->currentText() );
	KurooConfig::setDefaultFilePackageUserUse( form1.use->currentText() );
	KurooConfig::setDefaultFilePackageUserMask( form1.mask->currentText() );
	KurooConfig::setDefaultFilePackageUserUnMask( form1.unmask->currentText() );
	*/
}

/**
* Read 'make.conf' into stringList by taking into account the different kind of extended lines.
* @return linesConcatenated
*/
const QStringList ConfigDialog::readMakeConf()
{
	QStringList linesConcatenated;
	if ( !QFile::exists( KurooConfig::fileMakeConf() ) ) {
		qCritical() << KurooConfig::fileMakeConf() << " doesn't exist. Trying /etc/portage/make.conf";
		KurooConfig::setFileMakeConf(QLatin1String( "/etc/portage/make.conf" ));
	}
	QFile makeconf( KurooConfig::fileMakeConf() );

	if ( makeconf.open( QIODevice::ReadOnly ) ) {
		QTextStream stream( &makeconf );
		QStringList lines;

		// Collect all lines
		while ( !stream.atEnd() )
			lines += stream.readLine().simplified();
		makeconf.close();

		// Concatenate extended lines
		QString extendedLine;
		QStringList linesCommented;
		foreach( QString line, lines ) {

			// Skip comment lines
			if ( line.isEmpty() || line.startsWith( '#' ) ) {
				linesCommented += line;
				continue;
			} else if ( line.endsWith( '\\' ) ) {
				//We're re-joining multiline vars into a single line with spaces, so the
				//trailing slash to indicate multi-line isn't needed
				line.chop( 1 );
			}

			//The first time we see '=' extendedLine should be empty, and it gets cleared and
			//set to the whole line.  Subsequent encounters with '=' will join previous lines
			//with spaces, add them to linesConcatenated, and reset extendedLine to the
			//current line
			if ( line.contains( '=' ) ) {
				linesConcatenated += extendedLine;
				extendedLine = line;
				//add the comments seen since last '=' to collected lines
				linesConcatenated += linesCommented;
				linesCommented.clear();
			} else {
				//Any non-comment lines seen since last '=' are appended with a space to the
				//extended line
				extendedLine += " " + line;
			}
		}

		linesConcatenated += extendedLine;
	} else {
		qCritical() << "Error reading: " << KurooConfig::fileMakeConf();
	}

	return linesConcatenated;
}

/**
* Parse /etc/make.conf.
*/
void ConfigDialog::parseMakeConf()
{
	DEBUG_LINE_INFO;
	QStringList linesConcatenated = readMakeConf();
	if ( !linesConcatenated.isEmpty() ) {
		// Clear old entries
		foreach( void (*value)(const QString&), m_configHash ) {
			if ( NULL != value )
				value( QString::null );
		}

		// Parse the lines
		foreach( QString line, linesConcatenated ) {

			// Skip comment lines
			if ( line.isEmpty() || line.startsWith( '#' ) )
				continue;

			QRegularExpressionMatch match = m_rxLine.match( line );
			if ( match.hasMatch() ) {
				QString name = match.captured( 1 );
				if ( m_configHash.contains( name ) ) {
					if ( NULL != m_configHash.value( name ) ) {
						m_configHash[name]( match.captured( 2 ) );
						qDebug() << "Parsed" << name << "as" << match.captured( 2 ) << "from make.conf";
					} else {
						qWarning() << "Parsing make.conf: found" << name << "but can't parse it";
					}
					continue;
				}
			}
		}
	}
}

void ConfigDialog::handleELogClasses( const QString& value ) {
	/* this is kind of messy, but it gets the job done */
	if( value.contains( "warn" ) )
		KurooConfig::setElogWarn( 1 );
	if( value.contains( "error" ) )
		KurooConfig::setElogError( 1 );
	if( value.contains( "info" ) )
		KurooConfig::setElogInfo( 1 );
	if( value.contains( "log" ) )
		KurooConfig::setElogLog( 1 );
	if( value.contains( "qa" ) )
		KurooConfig::setElogQa( 1 );
}

void ConfigDialog::handleELogSystem( const QString& value ) {
	/* this is also kindof messy, got a better way? */
	if( value.contains( "save") ) 
		KurooConfig::setElogSysSave( 1 );
	if( value.contains( "mail") ) 
		KurooConfig::setElogSysMail( 1 );
	if( value.contains( "syslog") ) 
		KurooConfig::setElogSysSyslog( 1 );
	if( value.contains( "custom") ) 
		KurooConfig::setElogSysCustom( 1 );
	if( value.contains( "save_summary" ) )
		KurooConfig::setElogSysSaveSum( 1 );
	if( value.contains( "mail_summary" ) )
		KurooConfig::setElogSysMailSum( 1 );
}

/**
* Save back /etc/make.conf.
* @return success
*/
bool ConfigDialog::saveMakeConf()
{
	QStringList linesConcatenated = readMakeConf();
	if ( linesConcatenated.isEmpty() ) {
		return false;
	}

	QString line;
	QStringList lines;
	QFile file( KurooConfig::fileMakeConf() );
	QMap<QString, QString> keywords;

	// Collect all keywords
	foreach ( line, linesConcatenated ) {

		if ( line.contains( QRegExp( "^\\s*(CHOST|CFLAGS|CXXFLAGS|MAKEOPTS|USE|GENTOO_MIRRORS|PORTDIR_OVERLAY|FEATURES|PORTDIR|PORTAGE_TMPDIR|"
						"DISTDIR|ACCEPT_KEYWORDS|AUTOCLEAN|BUILD_PREFIX|CBUILD|CCACHE_SIZE|CLEAN_DELAY|CONFIG_PROTECT|"
									"CONFIG_PROTECT_MASK|DEBUGBUILD|FETCHCOMMAND|HTTP_PROXY|FTP_PROXY|PKG_TMPDIR|PKGDIR|PORT_LOGDIR|PORTAGE_BINHOST|"
						"PORTAGE_NICENESS|RESUMECOMMAND|ROOT|RSYNC_EXCLUDEFROM|RSYNC_PROXY|RSYNC_RETRIES|RSYNC_RATELIMIT|"
						"RSYNC_TIMEOUT|RPMDIR|SYNC|USE_ORDER|NOCOLOR|"
						"PORTAGE_ELOG_MAILURI|PORTAGE_ELOG_MAILFROM|"
								"PORTAGE_ELOG_MAILSUBJECT|PORTAGE_ELOG_COMMAND)" ) ) ) {

			QRegularExpressionMatch match = m_rxLine.match( line );
			if ( match.hasMatch() ) {
				keywords[ match.captured(1) ] = match.captured(2);
			}
			else {
				qWarning() << QString("Parsing %1: can not match keyword %2.").arg( KurooConfig::fileMakeConf() ).arg( match.captured(1) );
			}
		}
	}

	// Update keywords from settings
	keywords[ "ACCEPT_KEYWORDS" ] = KurooConfig::acceptKeywords();

	keywords[ "CHOST" ] = KurooConfig::chost();

	keywords[ "CFLAGS" ] = KurooConfig::cFlags();

	keywords[ "CXXFLAGS" ] = KurooConfig::cXXFlags();

	keywords[ "MAKEOPTS" ] = KurooConfig::makeOpts();

	keywords[ "USE" ] = KurooConfig::use();

	keywords[ "CONFIG_PROTECT" ] = KurooConfig::configProtect();

	keywords[ "CONFIG_PROTECT_MASK" ] = KurooConfig::configProtectMask();

	keywords[ "FEATURES" ] = KurooConfig::features();

	keywords[ "USE_ORDER" ] = KurooConfig::useOrder();

	//Fix a BUG which stores NOCOLOR content translated to /etc/make.conf which should not be..
	if (KurooConfig::noColor()) {
		keywords[ "NOCOLOR" ] = "true";
	}
	else {
		keywords[ "NOCOLOR" ] = "false";
	}

	keywords[ "ROOT" ] = KurooConfig::root();

	keywords[ "PORTDIR" ] = KurooConfig::dirPortage();

	keywords[ "PORTDIR_OVERLAY" ] = KurooConfig::dirPortageOverlay();

	keywords[ "DISTDIR" ] = KurooConfig::dirDist();

	keywords[ "RPMDIR" ] = KurooConfig::dirRpm();

	keywords[ "PKG_TMPDIR" ] = KurooConfig::dirPkgTmp();

	keywords[ "PKGDIR" ] = KurooConfig::dirPkg();

	keywords[ "PORT_LOGDIR" ] = KurooConfig::dirPortLog();

	keywords[ "PORTAGE_BINHOST" ] = KurooConfig::portageBinHost();

	keywords[ "PORTAGE_NICENESS" ] = KurooConfig::portageNiceness();

	keywords[ "PORTAGE_TMPDIR" ] = KurooConfig::dirPortageTmp();

	//Fix a BUG which stores AUTOCLEAN content translated to /etc/make.conf which should not be..
	if (KurooConfig::autoClean()) {
		keywords[ "AUTOCLEAN" ] = "yes";
	}
	else {
		keywords[ "AUTOCLEAN" ] = "no";
	}

	keywords[ "BUILD_PREFIX" ] = KurooConfig::buildPrefix();

	keywords[ "CBUILD" ] = KurooConfig::cBuild();

	keywords[ "CCACHE_SIZE" ] = KurooConfig::cCacheSize();

	keywords[ "CLEAN_DELAY" ] = KurooConfig::cleanDelay();

	keywords[ "DEBUGBUILD" ] = KurooConfig::debugBuild();

	keywords[ "FETCHCOMMAND" ] = KurooConfig::fetchCommand();

	keywords[ "RESUMECOMMAND" ] = KurooConfig::resumeCommand();

	keywords[ "RSYNC_EXCLUDEFROM" ] = KurooConfig::rsyncExcludeFrom();

	keywords[ "HTTP_PROXY" ] = KurooConfig::httpProxy();

	keywords[ "FTP_PROXY" ] = KurooConfig::ftpProxy();

	keywords[ "GENTOO_MIRRORS" ] = KurooConfig::gentooMirrors();

	keywords[ "RSYNC_PROXY" ] = KurooConfig::rsyncProxy();

	keywords[ "RSYNC_RETRIES" ] = KurooConfig::rsyncRetries();

	keywords[ "RSYNC_RATELIMIT" ] = KurooConfig::rsyncRateLimit();

	keywords[ "RSYNC_TIMEOUT" ] = KurooConfig::rsyncTimeOut();

	QString elog_value = "";
	if( KurooConfig::elogWarn() ) elog_value.append("warn");
	if( KurooConfig::elogError() ) elog_value.append("error");
	if( KurooConfig::elogLog() ) elog_value.append("log");
	if( KurooConfig::elogInfo() ) elog_value.append("info");
	if( KurooConfig::elogQa() ) elog_value.append("qa");
	keywords[ "PORTAGE_ELOG_CLASSES" ] = elog_value;

	QString elog_sys = "";
	if( KurooConfig::elogSysSave() ) elog_sys.append("save");
	if( KurooConfig::elogSysMail() ) elog_sys.append("mail");
	if( KurooConfig::elogSysSyslog() ) elog_sys.append("syslog");
	if( KurooConfig::elogSysCustom() ) elog_sys.append("custom");
	if( KurooConfig::elogSysSaveSum() ) elog_sys.append("save_summary");
	if( KurooConfig::elogSysMailSum() ) elog_sys.append("mail_summary");
	keywords[ "PORTAGE_ELOG_SYSTEM" ] = elog_sys;

	keywords[ "PORTAGE_ELOG_COMMAND" ] = KurooConfig::elogCustomCmd();
	keywords[ "PORTAGE_ELOG_MAILURI" ] = KurooConfig::elogMailURI();
	keywords[ "PORTAGE_ELOG_MAILFROM" ] = KurooConfig::elogMailFromURI();
	keywords[ "PORTAGE_ELOG_MAILSUBJECT" ] = KurooConfig::elogSubject();
	keywords[ "SYNC" ] = KurooConfig::sync();

	// Write back everything
	if ( file.open( QIODevice::WriteOnly ) ) {
		QTextStream stream( &file );

		bool top( true );
		foreach ( line, linesConcatenated ) {

			// Skip first empty lines
			if ( top && line.isEmpty() ) {
				continue;
			}
			else {
				top = false;
			}

			if ( line.contains( QRegExp( "^\\s*(CHOST|CFLAGS|CXXFLAGS|MAKEOPTS|USE|GENTOO_MIRRORS|PORTDIR_OVERLAY|FEATURES|PORTDIR|PORTAGE_TMPDIR|"
										"DISTDIR|ACCEPT_KEYWORDS|AUTOCLEAN|BUILD_PREFIX|CBUILD|CCACHE_SIZE|CLEAN_DELAY|CONFIG_PROTECT|"
										"CONFIG_PROTECT_MASK|DEBUGBUILD|FETCHCOMMAND|HTTP_PROXY|FTP_PROXY|PKG_TMPDIR|PKGDIR|PORT_LOGDIR|"
										"PORTAGE_BINHOST|PORTAGE_NICENESS|RESUMECOMMAND|ROOT|RSYNC_EXCLUDEFROM|RSYNC_PROXY|RSYNC_RETRIES|"
										"RSYNC_RATELIMIT|RSYNC_TIMEOUT|RPMDIR|SYNC|USE_ORDER|NOCOLOR|"
										"PORTAGE_ELOG_CLASSES|PORTAGE_ELOG_SYSTEM|PORTAGE_ELOG_MAILURI|PORTAGE_ELOG_MAILFROM|"
										"PORTAGE_ELOG_MAILSUBJECT|PORTAGE_ELOG_COMMAND)" ) ) ) {

				QRegularExpressionMatch match = m_rxLine.match( line );
				if ( match.hasMatch() ) {
					QString keyword = match.captured(1);
					if ( !keywords[ keyword ].isEmpty() ) {
						stream << keyword << "=\"" << keywords[ keyword ] << "\"\n";
					}
					keywords.take( keyword );
				}
			}
			else {
				stream << line << endl;
			}
		}

		// Add the rest (new) entries into make.conf
		for ( QMap<QString, QString>::Iterator it = keywords.begin(), end = keywords.end(); it != end; ++it ) {
			if ( !it.value().isEmpty() ) {
				stream << it.key() << "=\"" << it.value() << "\"\n";
			}
		}

		file.close();
		return true;
	}
	else {
		qCritical() << QString( "Writing: %1" ).arg( KurooConfig::fileMakeConf() );
		return false;
	}
}



