/***************************************************************************
 *	Copyright (C) 2004 by karye												*
 *	karye@users.sourceforge.net												*
 *																			*
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

#include "common.h"
#include "portagefiles.h"
#include "threadweaver/ThreadWeaver.h"
#include "LoadPackageHardMaskJob.h"
#include "LoadPackageKeywordsJob.h"
#include "LoadPackageUseJob.h"
#include "LoadPackageUserHardMaskJob.h"
#include "LoadPackageUserUnMaskJob.h"

bool mergeDirIntoFile( QString dirPath ) {
	//DEBUG_LINE_INFO;
	QDir mergeDir( dirPath );
	//TODO make sure this doesn't exist before we enter
	QFile tempFile( dirPath + ".temp" );
	QTextStream tempStream( &tempFile );
	if( !tempFile.open( QIODevice::WriteOnly ) ) {
		//kDebug(0) << "Opened " << tempFile.fileName() << " for writing." << LINE_INFO;
		//TODO handle failure
		return false;
	}

	QList<QFileInfo> infos = mergeDir.entryInfoList();
	QStringList lines;
	foreach( QFileInfo fi, infos ) {
		//kDebug(0) << "Processing " << fi.filePath() << LINE_INFO;
		if( fi.isDir() ) {
			//kDebug(0) << fi.filePath() << " is a dir." << LINE_INFO;
			if( fi.filePath().endsWith( "/." ) || fi.filePath().endsWith( "/.." ) ) {
				//kDebug(0) << fi.filePath() << " is ., skipping." << LINE_INFO;
			} else {
				//kDebug(0) << "Would recurse into " << fi.filePath() << LINE_INFO;
				//TODO handle failure
				if( !mergeDirIntoFile( fi.filePath() ) ) {
					return false;
				}
			}
		}

		QFile entryFile( fi.absoluteFilePath() );
		QTextStream streamFile( &entryFile );
		if ( !entryFile.open( QIODevice::ReadOnly ) ) {
			//kError(0) << "Parsing " << fi.filePath() << LINE_INFO;
		} else {
			while ( !streamFile.atEnd() )
				lines += streamFile.readLine();
			entryFile.close();
		}

		//Save the file as we go
		foreach( QString line, lines ) {
			tempStream << line << "\n";
		}

		if( !entryFile.remove() ) {
			//TODO handle failure
			return false;
		}
	}
	tempFile.close();
	//By the time we get out of here the directory should be empty, or else. . .
	if( mergeDir.rmdir( dirPath ) ) {
		//TODO handle failure
		return false;
	}

	//And write the new file in it's place
	KIO::file_move( dirPath + ".temp", dirPath, -1, KIO::Overwrite | KIO::HideProgressInfo );
	return true;
}

/**
* @class: SavePackageKeywordsJob
* @short: Thread for loading packages unmasked by user.
*/
class SavePackageKeywordsJob : public ThreadWeaver::Job
{
public:
	SavePackageKeywordsJob( QObject *dependent ) : Job( dependent ) {}

	virtual void run() {

		const QStringList lines = KurooDBSingleton::Instance()->query(
			"SELECT package.category, package.name, packageKeywords.keywords FROM package, packageKeywords "
			"WHERE package.id = packageKeywords.idPackage;" );
		kDebug() << "Found" << lines;
		if ( lines.isEmpty() ) {
			kWarning(0) << QString("No package keywords found. Saving to %1 aborted!")
				.arg( KurooConfig::defaultFilePackageKeywords() ) << LINE_INFO;
			return;
		}

		QFile file( KurooConfig::defaultFilePackageKeywords() );
		QTextStream stream( &file );

		if ( !file.open( QIODevice::WriteOnly ) ) {
			kError(0) << QString("Writing: %1.").arg( KurooConfig::defaultFilePackageKeywords() ) << LINE_INFO;
			return;
		}

		QStringListIterator it( lines );
		while( it.hasNext() ) {
			QString category = it.next();
			QString package = it.next();
			QString keywords = it.next();
			if ( !package.isEmpty() )
				stream << category << "/" << package << " " << keywords << "\n";
		}

		file.close();
//		return;
// 	}
//
// 	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( PACKAGE_KEYWORDS_SAVED );
	}
};

/**
* @class: SavePackageUserMaskJob
* @short: Thread for saving packages unmasked by user.
*/
class SavePackageUserMaskJob : public ThreadWeaver::Job
{
public:
	SavePackageUserMaskJob( QObject *dependent ) : Job( dependent ) {}

	virtual void run() {

		const QStringList lines = KurooDBSingleton::Instance()->query( "SELECT dependAtom FROM packageUserMask;" );
		if ( lines.isEmpty() ) {
			kWarning(0) << QString("No user mask depend atom found. Saving to %1 aborted!")
				.arg( KurooConfig::defaultFilePackageUserMask() ) << LINE_INFO;
			return;
		}

		QFile file( KurooConfig::defaultFilePackageUserMask() );
		QTextStream stream( &file );
		if ( !file.open( QIODevice::WriteOnly ) ) {
			kError(0) << QString("Writing: %1.").arg( KurooConfig::defaultFilePackageUserMask() ) << LINE_INFO;
			return;
		}

		foreach( QString line, lines )
			stream << line << "\n";

		file.close();
// 		return;
// 	}
//
// 	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( PACKAGE_USER_MASK_SAVED );
	}
};

/**
* @class: SavePackageUserMaskJob
* @short: Thread for saving packages unmasked by user.
*/
class SavePackageUserUnMaskJob : public ThreadWeaver::Job
{
public:
	SavePackageUserUnMaskJob( QObject *dependent ) : Job( dependent ) {}

	virtual void run() {

		const QStringList lines = KurooDBSingleton::Instance()->query( "SELECT dependAtom FROM packageUnMask ;" );
		if ( lines.isEmpty() ) {
			kWarning(0) << QString("No user unmask depend atom found. Saving to %1 aborted!")
				.arg( KurooConfig::defaultFilePackageUserUnMask() ) << LINE_INFO;
			return;
		}

		QFile file( KurooConfig::defaultFilePackageUserUnMask() );
		QTextStream stream( &file );
		if ( !file.open( QIODevice::WriteOnly ) ) {
			kError(0) << QString("Writing: %1.").arg( KurooConfig::defaultFilePackageUserUnMask() ) << LINE_INFO;
			return;
		}

		foreach ( QString line, lines )
			stream << line << "\n";

		file.close();
// 		return;
// 	}
//
// 	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( PACKAGE_USER_UNMASK_SAVED );
	}
};

/**
* @class: SavePackageUseJob
* @short: Thread for saving packages use-setting by user.
*/
class SavePackageUseJob : public ThreadWeaver::Job
{
public:
	SavePackageUseJob( QObject *dependent ) : Job( dependent ) {}

	virtual void run() {

		const QStringList lines = KurooDBSingleton::Instance()->query(
			"SELECT package.category, package.name, packageUse.use FROM package, packageUse "
			"WHERE package.id = packageUse.idPackage;" );
		if ( lines.isEmpty() ) {
			kWarning(0) << QString("No package use found. Saving to %1 aborted!").arg( KurooConfig::defaultFilePackageUserUse() ) << LINE_INFO;
			return;
		}

		QFile file( KurooConfig::defaultFilePackageUserUse() );
		QTextStream stream( &file );
		if ( !file.open( QIODevice::WriteOnly ) ) {
			kError(0) << QString("Writing: %1.").arg( KurooConfig::defaultFilePackageUserUse() ) << LINE_INFO;
			return;
		}

		QStringListIterator it( lines );
		while( it.hasNext() ) {
			QString category = it.next();
			QString package = it.next();
			QString use = it.next();
			QString tmpuse = use;
			if ( !tmpuse.remove(" ").isEmpty() )
				stream << category << "/" << package << " " << use << "\n";
		}

		file.close();
//		return;
// 	}
//
// 	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( PACKAGE_USER_USE_SAVED );
	}
};

/**
* Object for resulting list of packages from emerge actions.
*/
PortageFiles::PortageFiles( QObject *m_parent )
	: QObject( m_parent )
{}

PortageFiles::~PortageFiles()
{}

void PortageFiles::init( QObject *parent )
{
	m_parent = parent;
}

/**
* Forward signal to refresh results.
*/
void PortageFiles::refresh( int mask )
{
	if (LogSingleton::Instance()) {
		switch ( mask ) {
			case PACKAGE_KEYWORDS_SCANNED:
				LogSingleton::Instance()->writeLog( i18n( "Completed scanning for package keywords in %1.",
														KurooConfig::defaultFilePackageKeywords() ), KUROO );
				break;
			case PACKAGE_USER_UNMASK_SCANNED:
				LogSingleton::Instance()->writeLog(  i18n( "Completed scanning for unmasked packages in %1.",
														KurooConfig::defaultFilePackageUserUnMask() ), KUROO );
				break;
			case PACKAGE_HARDMASK_SCANNED:
				LogSingleton::Instance()->writeLog(  i18n( "Completed scanning for hardmasked packages in %1.",
														KurooConfig::filePackageHardMask() ), KUROO );
				break;
			case PACKAGE_USER_MASK_SCANNED:
				LogSingleton::Instance()->writeLog(  i18n( "Completed scanning for user masked packages in %1.",
														KurooConfig::defaultFilePackageUserMask() ), KUROO );
				break;
			case PACKAGE_KEYWORDS_SAVED:
				LogSingleton::Instance()->writeLog(  i18n( "Completed saving package keywords in %1.",
														KurooConfig::defaultFilePackageKeywords() ), KUROO );
				emit signalPortageFilesChanged();
				break;
			case PACKAGE_USER_MASK_SAVED:
				LogSingleton::Instance()->writeLog(  i18n( "Completed saving user masked packages in %1.",
														KurooConfig::defaultFilePackageUserMask() ), KUROO );
				emit signalPortageFilesChanged();
				break;
			case PACKAGE_USER_UNMASK_SAVED:
				LogSingleton::Instance()->writeLog(  i18n( "Completed saving user unmasked packages in %1.",
														KurooConfig::defaultFilePackageUserUnMask() ), KUROO );
				break;
			case PACKAGE_USER_USE_SCANNED:
				LogSingleton::Instance()->writeLog(  i18n( "Completed scanning user package use flags in %1.",
														KurooConfig::defaultFilePackageUserUse() ), KUROO );
				break;
			case PACKAGE_USER_USE_SAVED:
				LogSingleton::Instance()->writeLog(  i18n( "Completed saving user package use in %1.",
														KurooConfig::defaultFilePackageUserUse() ), KUROO );
		}
	}
}


/**
* Load all!
*/
void PortageFiles::loadPackageFiles()
{
	DEBUG_LINE_INFO;

	SignalistSingleton::Instance()->setKurooBusy(true);

	if (NULL != KurooStatusBar::instance())
		KurooStatusBar::instance()->setProgressStatus( QString::null, i18n("Loading portage files"));
	ThreadWeaver::Weaver::instance()->enqueue( new LoadPackageHardMaskJob( this ) );
	loadPackageUserMask();
	loadPackageUnmask();
	loadPackageKeywords();
	loadPackageUse();
	SignalistSingleton::Instance()->setKurooBusy(false);
	if (NULL != KurooStatusBar::instance())
		KurooStatusBar::instance()->setProgressStatus( QString::null, i18n("Done."));
}

void PortageFiles::loadPackageKeywords()
{
	ThreadWeaver::Weaver::instance()->enqueue( new LoadPackageKeywordsJob( this ) );
}

void PortageFiles::loadPackageUnmask()
{
	ThreadWeaver::Weaver::instance()->enqueue( new LoadPackageUserUnMaskJob( this ) );
}

void PortageFiles::loadPackageUserMask()
{
	ThreadWeaver::Weaver::instance()->enqueue( new LoadPackageUserMaskJob( this ) );
}

void PortageFiles::loadPackageUse()
{
	ThreadWeaver::Weaver::instance()->enqueue( new LoadPackageUseJob( this ) );
}

void PortageFiles::savePackageKeywords()
{
	ThreadWeaver::Weaver::instance()->enqueue( new SavePackageKeywordsJob( this ) );
}

void PortageFiles::savePackageUserUnMask()
{
	ThreadWeaver::Weaver::instance()->enqueue( new SavePackageUserUnMaskJob( this ) );
}

void PortageFiles::savePackageUserMask()
{
	ThreadWeaver::Weaver::instance()->enqueue( new SavePackageUserMaskJob( this ) );
}

void PortageFiles::savePackageUse()
{
	ThreadWeaver::Weaver::instance()->enqueue( new SavePackageUseJob( this ) );
}

#include "portagefiles.moc"
