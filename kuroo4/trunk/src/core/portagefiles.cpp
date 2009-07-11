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
#include "portagefiles.h"
#include "LoadPackageHardMaskJob.h"
#include "LoadPackageKeywordsJob.h"
#include "LoadPackageUseJob.h"
#include "LoadPackageUserHardMaskJob.h"
#include "LoadPackageUserUnMaskJob.h"

/**
 * @class: SavePackageKeywordsJob
 * @short: Thread for loading packages unmasked by user.
 */
class SavePackageKeywordsJob : public ThreadWeaver::DependentJob
{
public:
	SavePackageKeywordsJob( QObject *dependent ) : DependentJob( dependent, "DBJob1" ) {}
	
	virtual bool doJob() {
		
		const QStringList lines = KurooDBSingleton::Instance()->query( 
			"SELECT package.category, package.name, packageKeywords.keywords FROM package, packageKeywords "
			"WHERE package.id = packageKeywords.idPackage;" );
		if ( lines.isEmpty() ) {
			kWarning(0) << QString("No package keywords found. Saving to %1 aborted!")
				.arg( KurooConfig::filePackageKeywords() ) << LINE_INFO;
			return false;
		}
		
		QFile file( KurooConfig::filePackageKeywords() );
        QTextStream stream( &file );
		if ( !file.open( QIODevice::WriteOnly ) ) {
			kError(0) << QString("Writing: %1.").arg( KurooConfig::filePackageKeywords() ) << LINE_INFO;
			return false;
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
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( ThreadWeaver::PACKAGE_KEYWORDS_SAVED );
	}
};

/**
 * @class: SavePackageUserMaskJob
 * @short: Thread for saving packages unmasked by user.
 */
class SavePackageUserMaskJob : public ThreadWeaver::DependentJob
{
public:
	SavePackageUserMaskJob( QObject *dependent ) : DependentJob( dependent, "DBJob2" ) {}
	
	virtual bool doJob() {
		
		const QStringList lines = KurooDBSingleton::Instance()->query( "SELECT dependAtom FROM packageUserMask;" );
		if ( lines.isEmpty() ) {
			kWarning(0) << QString("No user mask depend atom found. Saving to %1 aborted!")
				.arg( KurooConfig::filePackageUserMask() ) << LINE_INFO;
			return false;
		}
		
		QFile file( KurooConfig::filePackageUserMask() );
        QTextStream stream( &file );
		if ( !file.open( QIODevice::WriteOnly ) ) {
			kError(0) << QString("Writing: %1.").arg( KurooConfig::filePackageUserMask() ) << LINE_INFO;
			return false;
		}
		
        foreach( QString line, lines )
            stream << line << "\n";
		
		file.close();
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( ThreadWeaver::PACKAGE_USER_MASK_SAVED );
	}
};

/**
 * @class: SavePackageUserMaskJob
 * @short: Thread for saving packages unmasked by user.
 */
class SavePackageUserUnMaskJob : public ThreadWeaver::DependentJob
{
public:
	SavePackageUserUnMaskJob( QObject *dependent ) : DependentJob( dependent, "DBJob3" ) {}
	
	virtual bool doJob() {
		
		const QStringList lines = KurooDBSingleton::Instance()->query( "SELECT dependAtom FROM packageUnMask ;" );
		if ( lines.isEmpty() ) {
			kWarning(0) << QString("No user unmask depend atom found. Saving to %1 aborted!")
				.arg( KurooConfig::filePackageUserUnMask() ) << LINE_INFO;
			return false;
		}
		
		QFile file( KurooConfig::filePackageUserUnMask() );
        QTextStream stream( &file );
		if ( !file.open( QIODevice::WriteOnly ) ) {
			kError(0) << QString("Writing: %1.").arg( KurooConfig::filePackageUserUnMask() ) << LINE_INFO;
			return false;
		}
		
        foreach ( QString line, lines )
            stream << line << "\n";
		
		file.close();
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( ThreadWeaver::PACKAGE_USER_UNMASK_SAVED );
	}
};

/**
 * @class: SavePackageUseJob
 * @short: Thread for saving packages use-setting by user.
 */
class SavePackageUseJob : public ThreadWeaver::DependentJob
{
public:
	SavePackageUseJob( QObject *dependent ) : DependentJob( dependent, "DBJob4" ) {}
	
	virtual bool doJob() {
		
		const QStringList lines = KurooDBSingleton::Instance()->query( 
			"SELECT package.category, package.name, packageUse.use FROM package, packageUse "
			"WHERE package.id = packageUse.idPackage;" );
		if ( lines.isEmpty() ) {
			kWarning(0) << QString("No package use found. Saving to %1 aborted!").arg( KurooConfig::filePackageUserUse() ) << LINE_INFO;
			return false;
		}
		
		QFile file( KurooConfig::filePackageUserUse() );
        QTextStream stream( &file );
		if ( !file.open( QIODevice::WriteOnly ) ) {
			kError(0) << QString("Writing: %1.").arg( KurooConfig::filePackageUserUse() ) << LINE_INFO;
			return false;
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
		return true;
	}
	
	virtual void completeJob() {
		PortageFilesSingleton::Instance()->refresh( ThreadWeaver::PACKAGE_USER_USE_SAVED );
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
	switch ( mask ) {
		case ThreadWeaver::PACKAGE_KEYWORDS_SCANNED:
			LogSingleton::Instance()->writeLog( i18n("Completed scanning for package keywords in %1.")
			                                    .arg( KurooConfig::filePackageKeywords() ), KUROO );
			break;
		case ThreadWeaver::PACKAGE_USER_UNMASK_SCANNED:
			LogSingleton::Instance()->writeLog(  i18n("Completed scanning for unmasked packages in %1.")
												.arg( KurooConfig::filePackageUserUnMask() ), KUROO );
			break;
		case ThreadWeaver::PACKAGE_HARDMASK_SCANNED:
			LogSingleton::Instance()->writeLog(  i18n("Completed scanning for hardmasked packages in %1.")
												.arg( KurooConfig::filePackageHardMask() ), KUROO );
			break;
		case ThreadWeaver::PACKAGE_USER_MASK_SCANNED:
			LogSingleton::Instance()->writeLog(  i18n("Completed scanning for user masked packages in %1.")
												.arg( KurooConfig::filePackageUserMask() ), KUROO );
			break;
		case ThreadWeaver::PACKAGE_KEYWORDS_SAVED:
			LogSingleton::Instance()->writeLog(  i18n("Completed saving package keywords in %1.")
												.arg( KurooConfig::filePackageKeywords() ), KUROO );
			emit signalPortageFilesChanged();
			break;
		case ThreadWeaver::PACKAGE_USER_MASK_SAVED:
			LogSingleton::Instance()->writeLog(  i18n("Completed saving user masked packages in %1.")
												.arg( KurooConfig::filePackageUserMask() ), KUROO );
			emit signalPortageFilesChanged();
			break;
		case ThreadWeaver::PACKAGE_USER_UNMASK_SAVED:
			LogSingleton::Instance()->writeLog(  i18n("Completed saving user unmasked packages in %1.")
												.arg( KurooConfig::filePackageUserUnMask() ), KUROO );
			break;
		case ThreadWeaver::PACKAGE_USER_USE_SCANNED:
			LogSingleton::Instance()->writeLog(  i18n("Completed scanning user package use flags in %1.")
												.arg( KurooConfig::filePackageUserUse() ), KUROO );
			break;
		case ThreadWeaver::PACKAGE_USER_USE_SAVED:
			LogSingleton::Instance()->writeLog(  i18n("Completed saving user package use in %1.")
												.arg( KurooConfig::filePackageUserUse() ), KUROO );
	}
}


/**
 * Load all!
 */
void PortageFiles::loadPackageFiles()
{
	DEBUG_LINE_INFO;
	
	ThreadWeaver::instance()->queueJob( new LoadPackageHardMaskJob( this ) );
	loadPackageUserMask();
	loadPackageUnmask();
	loadPackageKeywords();
	loadPackageUse();
}

void PortageFiles::loadPackageKeywords()
{
	ThreadWeaver::instance()->queueJob( new LoadPackageKeywordsJob( this ) );
}

void PortageFiles::loadPackageUnmask()
{
	ThreadWeaver::instance()->queueJob( new LoadPackageUserUnMaskJob( this ) );
}

void PortageFiles::loadPackageUserMask()
{
	ThreadWeaver::instance()->queueJob( new LoadPackageUserMaskJob( this ) );
}

void PortageFiles::loadPackageUse()
{
	ThreadWeaver::instance()->queueJob( new LoadPackageUseJob( this ) );
}

void PortageFiles::savePackageKeywords()
{
	ThreadWeaver::instance()->queueJob( new SavePackageKeywordsJob( this ) );
}

void PortageFiles::savePackageUserUnMask()
{
	ThreadWeaver::instance()->queueJob( new SavePackageUserUnMaskJob( this ) );
}

void PortageFiles::savePackageUserMask()
{
	ThreadWeaver::instance()->queueJob( new SavePackageUserMaskJob( this ) );
}

void PortageFiles::savePackageUse()
{
	ThreadWeaver::instance()->queueJob( new SavePackageUseJob( this ) );
}

#include "portagefiles.moc"
