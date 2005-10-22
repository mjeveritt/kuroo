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

#include "common.h"
#include "scaninstalledjob.h"

/**
 * Thread for adding packages into installed in db.
 */
class AddInstalledPackageJob : public ThreadWeaver::DependentJob
{
public:
	AddInstalledPackageJob( QObject *dependent, const QString& package ) : DependentJob( dependent, "DBJob" ), m_package( package ) {}
	
	virtual bool doJob() {
		QString category = m_package.section("/", 0, 0);
		QString name = (m_package.section("/", 1, 1)).section(pv, 0, 0);
		QString version = m_package.section(name + "-", 1, 1);
		
		QString idCategory = KurooDBSingleton::Instance()->query(QString("SELECT id FROM category WHERE name = '%1';").arg(category)).first();
		QString packageId = KurooDBSingleton::Instance()->query(QString("SELECT id FROM package WHERE idCategory = '%1' AND name  = '%2' AND version = '%3';").arg(idCategory).arg(name).arg(version)).first();
		KurooDBSingleton::Instance()->query(QString("UPDATE package SET installed = '1' WHERE id = '%1';").arg(packageId));
	
		return true;
	}
	
	virtual void completeJob() {
		InstalledSingleton::Instance()->slotChanged();
	}
	
private:
	const QString m_package;
};

/**
 * Thread for removing packages from installed in db.
 */
class RemoveInstalledPackageJob : public ThreadWeaver::DependentJob
{
public:
	RemoveInstalledPackageJob( QObject *dependent, const QString& package ) : DependentJob( dependent, "DBJob" ), m_package( package ) {}
	
	virtual bool doJob() {
		QString category = m_package.section("/", 0, 0);
		QString name = (m_package.section("/", 1, 1)).section(pv, 0, 0);
		QString version = m_package.section(name + "-", 1, 1);

		QString idCategory = KurooDBSingleton::Instance()->query(QString("SELECT id FROM category WHERE name = '%1';").arg(category)).first();

		QString installedFlag = KurooDBSingleton::Instance()->query(QString("SELECT installed FROM package WHERE name = '%1' AND idCategory = '%2' AND version = '%3';").arg(name).arg(idCategory).arg(version)).first();

		// Mark package as uninstalled or remove it if old
		if ( installedFlag == "1" )
			KurooDBSingleton::Instance()->query(QString("UPDATE package SET installed = '0' WHERE idCategory = '%1' AND name  = '%2' AND version = '%3';").arg(idCategory).arg(name).arg(version));
		else
			KurooDBSingleton::Instance()->query(QString("DELETE FROM package WHERE idCategory = '%1' AND name  = '%2' AND version = '%3';").arg(idCategory).arg(name).arg(version));

		// Remove package from world file
		QFile file( KurooConfig::dirWorldFile() );
		QStringList lines;
		if ( file.open( IO_ReadOnly ) ) {
			QTextStream stream( &file );
			while ( !stream.atEnd() )
				lines += stream.readLine();
			file.close();
			
			if ( file.open( IO_WriteOnly ) ) {
				QTextStream stream( &file );
				foreach ( lines ) {
					if ( *it != ( category + "/" + name ) )
						stream << *it << endl;
				}
				file.close();
			}
			else
				kdDebug() << i18n("Error writing: ") << KurooConfig::dirWorldFile() << endl;
		}
		else
			kdDebug() << i18n("Error reading: ") << KurooConfig::dirWorldFile() << endl;
		
		return true;
	}
	
	virtual void completeJob() {
		InstalledSingleton::Instance()->slotChanged();
	}
	
private:
	const QString m_package;
};

/**
 * Object for installed packages.
 */
Installed::Installed( QObject *parent )
	: QObject( parent )
{
	connect( SignalistSingleton::Instance(), SIGNAL( signalScanInstalledComplete() ), this, SLOT( slotChanged() ) );
}

Installed::~Installed()
{
}

void Installed::init( QObject *myParent )
{
	parent = myParent;
}

/**
 * Forward signal after a new scan.
 */
void Installed::slotChanged()
{
	emit signalInstalledChanged();
}

/**
 * Clear packages.
 */
void Installed::slotReset()
{
	KurooDBSingleton::Instance()->query("DELETE FROM package WHERE installed = '1';");
	emit signalInstalledReset();
}

/**
 * Start scan of installed packages.
 * @return bool
 */
bool Installed::slotRefresh()
{
	kdDebug() << "Installed::slotRefresh" << endl;
	SignalistSingleton::Instance()->scanStarted();
	ThreadWeaver::instance()->queueJob( new ScanInstalledJob( this ) );
	return true;
}

/**
 * Launch emerge pretend of packages.
 * @param category
 * @param packageList
 */
void Installed::pretendPackage( const QString& category, const QStringList& packageList )
{
	EmergeSingleton::Instance()->pretend( category, packageList );
}

/**
 * Launch unmerge of packages
 * @param category
 * @param packageList
 */
void Installed::uninstallPackage( const QString& category, const QStringList& packageList )
{
	EmergeSingleton::Instance()->unmerge( category, packageList );
}

/**
 * @fixme: Check for failure.
 * Add package as installed in db.
 * @param package
 */
void Installed::addPackage( const QString& package )
{
	ThreadWeaver::instance()->queueJob( new AddInstalledPackageJob( this, package ) );
}

/**
 * @fixme: Check for failure.
 * Remove packages from db.
 * @param packageIdList
 */
void Installed::removePackage( const QString& package )
{
	ThreadWeaver::instance()->queueJob( new RemoveInstalledPackageJob( this, package ) );
}

/**
 * Get list of all categories for installed packages.
 * @return category list
 */
QStringList Installed::categories()
{
	return KurooDBSingleton::Instance()->installedCategories();
}

/**
 * Get list of packages in this category from database.
 * @param category
 * @return category list
 */
QStringList Installed::packagesInCategory( const QString& category )
{
	QString idCategory = KurooDBSingleton::Instance()->portageCategoryId( category ).first();
	return KurooDBSingleton::Instance()->installedPackagesByCategory( idCategory );
}

/**
 * Find packages by name or description.
 * @param text		string
 * @param isName	find in name or description
 */
void Installed::findPackage( const QString& text, const bool& isName )
{
	QStringList packageIdList;
	
	if ( isName )
		packageIdList = KurooDBSingleton::Instance()->findInstalledPackagesDescription( text );
	else
		packageIdList = KurooDBSingleton::Instance()->findInstalledPackagesName( text );
	
	if ( !packageIdList.isEmpty() )
		ResultsSingleton::Instance()->addPackageIdList( packageIdList );
	else
		LogSingleton::Instance()->writeLog( i18n("\nNo packages found matching: %1").arg(text), KUROO );
}

/**
 * Return info for package as description, homepage ...
 * @param package id
 * @return info
 */
Info Installed::packageInfo( const QString& packageId )
{
	Info info;
	
	QStringList packageList = KurooDBSingleton::Instance()->installedPackageInfo( packageId );
	QStringList::Iterator it = packageList.begin();
	info.description = *it++;
	info.size = *it++;
	info.keywords = *it++;
	info.homepage = *it++;
	info.licenses = *it++;
	info.useFlags = *it++;
	info.packageSlots = *it;
	return info;
}

/**
 * Count installed packages.
 * @return count
 */
QString Installed::count()
{
	QStringList total = KurooDBSingleton::Instance()->query("SELECT COUNT(id) FROM package WHERE installed != 0 LIMIT 1;");
	return total.first();
}

/**
 * Get installed files list for this package by parsing CONTENTS in dirDbPkg().
 * @param category 
 * @param package
 * @return List of files 
 */
QString Installed::installedFiles( const QString& packageId )
{
	QString package(PortageSingleton::Instance()->package( packageId ));
	QString category(PortageSingleton::Instance()->category( packageId ));
	
	QString filename = KurooConfig::dirDbPkg() + "/" + category + "/" + package.section("*", 0, 0) + "/CONTENTS";
	QFile file( filename );
	QString textLines;
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		while ( !stream.atEnd() ) {
			QString line = stream.readLine();
			if ( line.startsWith("obj") )
				textLines += line.section("obj ", 1, 1).section(" ", 0, 0) + "\n";
		}
		file.close();
		return textLines;
	}
	else {
		kdDebug() << i18n("Error reading: ") << filename << endl;
		return i18n("na");
	}
}

/**
 * Get info for selected package.
 * @param category 
 * @param package
 * @return Summary text 
 */
QString Installed::installedSummary( const QString& packageId )
{
	QString package(PortageSingleton::Instance()->package( packageId ));
	QString category(PortageSingleton::Instance()->category( packageId ));
	Info info( packageInfo( packageId ) );
	
	QString textLines = "<font size=\"+2\">" + category + "/" + package + "</font>";
	QString time = HistorySingleton::Instance()->packageTime( category + "/" + package.section(pv, 0, 0) );
	
	if ( info.size.isEmpty() )
		textLines += " <font color=red>(Version not available in Portage)</font>";
	
	textLines += "<br>";
	
	QString ebuild = KurooConfig::dirDbPkg() + "/" + category + "/" + package + "/" + package + ".ebuild";
	QFile file( ebuild );
	
	if ( file.open(IO_ReadOnly) ) {
		QTextStream stream( &file );
		QFileInfo fil( ebuild );
		
		// Get installation date
		QDateTime Date = fil.created();
		
		while ( !stream.atEnd() ) {
			QString line = stream.readLine();
			
			if ( line.startsWith("DESCRIPTION=" )) {
				line = line.section("\"", 1, 1);
				textLines += line + "<br>";
			}
			if ( line.startsWith( "HOMEPAGE=" )) {
				line = line.section("\"", 1, 1);
				textLines += "<a href=\"" + line + "\">" + line + "</a><br>";
			}
			if ( line.startsWith( "LICENSE=" )) {
				line = line.section("\"", 1, 1);
				textLines += i18n("<b>License:</b> ") + line + "<br>";
			}
			if ( line.startsWith( "SLOT=" )) {
				line = line.section("\"", 1, 1);
				textLines += i18n("<b>Slot:</b> ") + line + "<br>";
			}
			if ( line.startsWith( "KEYWORDS=" )) {
				line = line.section("\"", 1, 1);
				textLines += i18n("<b>Branches:</b> ") + line + "<br>";
			}
		}
		textLines += i18n("<b>Use flags:</b> ") + info.useFlags + "<br>";
		textLines += i18n("<b>Size:</b> ") + info.size + "<br>";
		textLines += i18n("<b>Emerge date: </b>") + Date.toString("MMM dd yyyy hh:mm") + "<br>";
		textLines += i18n("<b>Emerge time (average): </b>") + timeFormat( time );
		
		file.close();
		return textLines;
	}
	else {
		kdDebug() << i18n("Error reading: ") << ebuild << endl;
		return i18n("na");
	}
}

/**
 * Convert emerge duration from seconds to format hh:mm:ss.
 * @param time
 * @return formated time
 */
QString Installed::timeFormat( const QString& time )
{
	if ( !time.isEmpty() ) {
		QTime emergeTime(0, 0, 0);
		emergeTime = emergeTime.addSecs(time.toInt());
		return emergeTime.toString(Qt::TextDate);
	}
	else
		return i18n("na");
}

#include "installed.moc"
