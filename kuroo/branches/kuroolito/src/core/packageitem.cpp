/***************************************************************************
*   Copyright (C) 2007 by Karye                                           *
*   info@kuroo.org                                                        *
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
#include "packageitem.h"
#include "packageversion.h"
#include "dependatom.h"

#include <qpainter.h>

#include <klistview.h>

/**
 * @class PackageItem
 * @short Base class for package.
 */
PackageItem::PackageItem( QListView* parent, const char* name, const QString& id, const QString& category, const QString& description, const int status )
	: KListViewItem( parent, name ),
	m_parent( parent ), m_index( 0 ),
	m_id( id ), m_name( name ), m_status( status ), m_description( description ), m_category( category ), m_isQueued( false ), m_inWorld( false ),
	m_isInitialized( false )
{
	if ( this->isVisible() && PortageSingleton::Instance()->isInWorld( m_category + "/" + m_name ) )
		m_inWorld = true;
}

PackageItem::PackageItem( QListViewItem* parent, const char* name, const QString& id, const QString& category, const QString& description, const int status )
	: KListViewItem( parent, name ),
	m_parent( parent->listView() ), m_index( 0 ),
	m_id( id ), m_name( name ), m_status( status ), m_description( description ), m_category( category ), m_isQueued( false ), m_inWorld( false ),
	m_isInitialized( false )
{
	if ( this->isVisible() && PortageSingleton::Instance()->isInWorld( m_category + "/" + m_name ) )
		m_inWorld = true;
}

PackageItem::~PackageItem()
{}

/**
 * Set icons when package is visible.
 */
void PackageItem::paintCell( QPainter* painter, const QColorGroup& colorgroup, int column, int width, int alignment )
{
	if ( this->isVisible() ) {
		QColorGroup m_colorgroup( colorgroup );
		QFont font( painter->font() );
		
		// Optimizing - check only relevant columns
		switch ( column ) {
			
			case 0 : {
				if ( m_status & PACKAGE_AVAILABLE )
					setPixmap( 0, ImagesSingleton::Instance()->icon( PACKAGES ) );
				else {
					if ( KuroolitoConfig::installedColumn() ) {
						setPixmap( 0, ImagesSingleton::Instance()->icon( PACKAGES ) );
						setPixmap( 1, ImagesSingleton::Instance()->icon( VERSION_INSTALLED ) );
					}
					else
						setPixmap( 0, ImagesSingleton::Instance()->icon( INSTALLED ) );
					
					if ( m_status & PACKAGE_OLD ) {
						font.setItalic( true );
						painter->setFont( font );
						m_colorgroup.setColor( QColorGroup::Text, m_colorgroup.dark() );
					}
				}
				break;
			}
			
			case 2 : {
				if ( PortageSingleton::Instance()->isInWorld( m_category + "/" + m_name ) ) {
					m_inWorld = true;
					setPixmap( 2, ImagesSingleton::Instance()->icon( WORLD ) );
				}
				else {
					m_inWorld = false;
					setPixmap( 2, ImagesSingleton::Instance()->icon( EMPTY ) );
				}
			}
		}
		
		KListViewItem::paintCell( painter, m_colorgroup, column, width, alignment );
	}
}


void PackageItem::resetDetailedInfo()
{
	m_isInitialized = false;
}

/**
 * Initialize the package with all its versions and info. Executed when PortageItem get focus first time.
 */
void PackageItem::initVersions()
{
	if ( m_isInitialized )
		return;
	
	m_versions.clear();
	m_versionMap.clear();

	// Get list of accepted keywords, eg if package is "untesting"
	QString acceptedKeywords = KuroolitoDBSingleton::Instance()->packageKeywordsAtom( id() );
	const QStringList versionsList = KuroolitoDBSingleton::Instance()->packageVersionsInfo( id() );
	foreach ( versionsList ) {
		QString versionString = *it++;
		QString description = *it++;
		QString homepage = *it++;
		QString status = *it++;
		QString licenses = *it++;
		QString useFlags = *it++;
		QString slot = *it++;
		QString keywords = *it++;
		QString size = *it;
		
		PackageVersion* version = new PackageVersion( this, versionString );
		version->setDescription( description );
		version->setHomepage( homepage );
		version->setLicenses( licenses );
		version->setUseflags( QStringList::split( " ", useFlags ) );
		version->setSlot( slot );
		version->setKeywords( QStringList::split( " ", keywords ) );
		version->setAcceptedKeywords( QStringList::split( " ", acceptedKeywords ) );
		version->setSize( size );
		
		if ( status == PACKAGE_INSTALLED_STRING )
			version->setInstalled( true );
		
		m_versions.append( version );
		m_versionMap.insert( versionString, version );
	}
	
	// Now that we have all available versions, sort out masked ones and leaving unmasked.
	
	// Check if any of this package versions are hardmasked
	atom = new DependAtom( this );
	const QStringList atomHardMaskedList = KuroolitoDBSingleton::Instance()->packageHardMaskAtom( id() );
// 	kdDebug() << "atomHardMaskedList=" << atomHardMaskedList << endl;
	foreach ( atomHardMaskedList ) {
		
		// Test the atom string on validness, and fill the internal variables with the extracted atom parts,
		// and get the matching versions
		if ( atom->parse( *it ) ) {
			QValueList<PackageVersion*> versions = atom->matchingVersions();
			QValueList<PackageVersion*>::iterator versionIterator;
			for( versionIterator = versions.begin(); versionIterator != versions.end(); versionIterator++ )
				( *versionIterator )->setHardMasked( true );
		}
	}
	delete atom;
	
	// Check if any of this package versions are user-masked
	atom = new DependAtom( this );
	const QStringList atomUserMaskedList = KuroolitoDBSingleton::Instance()->packageUserMaskAtom( id() );
// 	kdDebug() << "atomUserMaskedList=" << atomUserMaskedList << endl;
	foreach ( atomUserMaskedList ) {
		
		// Test the atom string on validness, and fill the internal variables with the extracted atom parts,
		// and get the matching versions
		if ( atom->parse( *it ) ) {
			QValueList<PackageVersion*> versions = atom->matchingVersions();
			QValueList<PackageVersion*>::iterator versionIterator;
			for( versionIterator = versions.begin(); versionIterator != versions.end(); versionIterator++ )
				( *versionIterator )->setUserMasked( true );
		}
	}
	delete atom;
	
	// Check if any of this package versions are unmasked
	atom = new DependAtom( this );
	const QStringList atomUnmaskedList = KuroolitoDBSingleton::Instance()->packageUnMaskAtom( id() );
// 	kdDebug() << "atomUnmaskedList=" << atomUnmaskedList << endl;
	foreach ( atomUnmaskedList ) {
		
		// Test the atom string on validness, and fill the internal variables with the extracted atom parts,
		// and get the matching versions
		if ( atom->parse( *it ) ) {
			QValueList<PackageVersion*> versions = atom->matchingVersions();
			QValueList<PackageVersion*>::iterator versionIterator;
			for( versionIterator = versions.begin(); versionIterator != versions.end(); versionIterator++ )
				( *versionIterator )->setUnMasked( true );
		}
	}
	delete atom;

	// This package has collected all it's data
	m_isInitialized = true;
}

/**
 * Return list of versions.
 * @return QValueList<PackageVersion*>
 */
QValueList<PackageVersion*> PackageItem::versionList()
{
	return m_versions;
}

/**
 * Return map of versions - faster find.
 * @return QMap<QString, PackageVersion*>
 */
QMap<QString, PackageVersion*> PackageItem::versionMap()
{
	return m_versionMap;
}

/**
 * Return a list of PackageVersion objects sorted by their version numbers,
 * with the oldest version at the beginning and the latest version at the end
 * of the list.
 * @return sortedVersions
 */
QValueList<PackageVersion*> PackageItem::sortedVersionList()
{
	QValueList<PackageVersion*> sortedVersions;
	QValueList<PackageVersion*>::iterator sortedVersionIterator;
	
	for( QValueList<PackageVersion*>::iterator versionIterator = m_versions.begin(); versionIterator != m_versions.end(); versionIterator++ ) {
		if ( versionIterator == m_versions.begin() ) {
			sortedVersions.append( *versionIterator );
			continue; // if there is only one version, it can't be compared
		}
		
		// reverse iteration through the sorted version list
		sortedVersionIterator = sortedVersions.end();
		while ( true ) {
			if ( sortedVersionIterator == sortedVersions.begin() ) {
				sortedVersions.prepend( *versionIterator );
				break;
			}
			
			sortedVersionIterator--;
			if ( (*versionIterator)->isNewerThan( (*sortedVersionIterator)->version() ) ) {
				sortedVersionIterator++; // insert after the compared one, not before
				sortedVersions.insert( sortedVersionIterator, *versionIterator );
				break;
			}
		}
	}
	return sortedVersions;
}

/**
 * Parse sorted list of versions for stability, installed, emerge versions ...
 */
void PackageItem::parsePackageVersions()
{
	if ( !m_isInitialized )
		initVersions();
	
	m_versionsDataList.clear();
	m_linesAvailable = QString::null;
	m_linesUnavailable = QString::null;
	m_linesEmerge = QString::null;
	m_linesInstalled = QString::null;
	
	// Iterate sorted versions list
	QString version;
	QValueList<PackageVersion*> sortedVersions = sortedVersionList();
	QValueList<PackageVersion*>::iterator sortedVersionIterator;
	for ( sortedVersionIterator = sortedVersions.begin(); sortedVersionIterator != sortedVersions.end(); sortedVersionIterator++ ) {
		
		version = (*sortedVersionIterator)->version();
		
		// Mark official version stability for version listview
		QString stability;
		if ( (*sortedVersionIterator)->isNotArch() )
			stability = i18n("Not on %1").arg( KuroolitoConfig::arch() );
		else {
			if ( (*sortedVersionIterator)->isOriginalHardMasked() ) {
				stability = i18n("Hardmasked");
				version = "<font color=darkRed><i>" + version + "</i></font>";
			}
			else {
				if ( (*sortedVersionIterator)->isOriginalTesting() ) {
					stability = i18n("Testing");
					version = "<i>" + version + "</i>";
				}
				else {
					if ( (*sortedVersionIterator)->isAvailable() )
						stability = i18n("Stable");
					else
						stability = i18n("Not available");
				}
			}
		}
		
// 		kdDebug() << "version="<< (*sortedVersionIterator)->version() << " isInstalled=" << (*sortedVersionIterator)->isInstalled() << 
// 			" stability=" << stability << LINE_INFO;
		
		// Versions data for use by Inspector in vewrsion view
		m_versionsDataList << (*sortedVersionIterator)->version() << stability << (*sortedVersionIterator)->size();
		
		// Create nice summary showing installed packages
		if ( (*sortedVersionIterator)->isInstalled() ) {
			m_versionsDataList << "1";
			version = "<b>" + version + "</b>";
			m_linesInstalled.prepend( version + " (" + stability + "), " );
		}
		else
			m_versionsDataList << "0";
		
		// Collect all available packages except those not in users arch
		if ( (*sortedVersionIterator)->isAvailable() ) {
			m_emergeVersion = (*sortedVersionIterator)->version();
			m_linesEmerge = version + " (" + stability + ")";
			m_linesAvailable.prepend( version + ", " );
		}
		else {
			if ( (*sortedVersionIterator)->isNotArch() ) {
				m_isInArch = false;
				m_linesUnavailable.prepend( version + ", " );
			}
			else
				m_linesAvailable.prepend( version + ", " );
		}
		
		// Get description and homepage from most recent version = assuming most correct
		m_description = (*sortedVersionIterator)->description();
		m_homepage = (*sortedVersionIterator)->homepage();
	}
	
	// Remove trailing commas
	m_linesInstalled.truncate( m_linesInstalled.length() - 2 );
	m_linesAvailable.truncate( m_linesAvailable.length() - 2 );
	m_linesUnavailable.truncate( m_linesUnavailable.length() - 2 );
}


///////////////////////////////////////////////////////////////////////////////
// Accessors
///////////////////////////////////////////////////////////////////////////////

/**
 * Register package index in parent listView.
 * @param index
 */
void PackageItem::setPackageIndex( int index )
{
	m_index = index;
}

void PackageItem::setDescription( const QString& description )
{
	m_description = description;
}

void PackageItem::setInstalled()
{
	m_status = PACKAGE_INSTALLED;
}

/**
 * Is this the first package in the listview. Since they are inserted in reverse order it means has the highest index.
 * @return true if first
 */
bool PackageItem::isFirstPackage()
{
	return ( m_index == m_parent->childCount() );
}

/**
 * Is this the last package?
 * @return true if last
 */
bool PackageItem::isLastPackage()
{
	return ( m_index == 1 );
}

/**
 * Is this package installed.
 * @return true if yes
 */
bool PackageItem::isInstalled()
{
	return ( m_status & ( PACKAGE_INSTALLED | PACKAGE_UPDATES | PACKAGE_OLD ) );
}

/**
 * Is this package available in Portage tree?
 * @return true if yes
 */
bool PackageItem::isInPortage()
{
	return ( m_status & ( PACKAGE_AVAILABLE | PACKAGE_INSTALLED | PACKAGE_UPDATES ) );
}

bool PackageItem::isInWorld()
{
	return m_inWorld;
}

/**
 * Package db id.
 * @return id
 */
const QString& PackageItem::id() const
{
	return m_id;
}

/**
 * Package name as kuroo in app-portage/kuroo.
 * @return name
 */
const QString& PackageItem::name() const
{
	return m_name;
}

/**
 * Accessor for category.
 * @return the package category.
 */
const QString& PackageItem::category() const
{
	return m_category;
}

/**
 * Package description.
 * @return description
 */
const QString& PackageItem::description() const
{
	return m_description;
}

/**
 * Package status describing if this package is installed or not.
 * @return status
 */
const int PackageItem::status() const
{
	return m_status;
}

const QString& PackageItem::homepage() const
{
	return m_homepage;
}

/**
 * Returns list of installed versions in html-format.
 */
const QString& PackageItem::linesInstalled() const
{
	return m_linesInstalled;
}

/**
 * Returns list of available versions in html-format.
 */
const QString& PackageItem::linesAvailable() const
{
	return m_linesAvailable;
}

/**
 * Returns list of unavailable versions in html-format.
 */
const QString& PackageItem::linesUnavailable() const
{
	return m_linesUnavailable;
}

/**
 * Returns list emergeable versions in html-format.
 */
const QString& PackageItem::linesEmerge() const
{
	return m_linesEmerge;
}

const bool PackageItem::isInArch() const
{
	return m_isInArch;
}

/**
 * Return versions list together with stability info etc...
 */
const QStringList& PackageItem::versionDataList() const
{
	return m_versionsDataList;
}

/**
 * Return version used by emerge.
 */
const QString& PackageItem::emergeVersion() const
{
	return m_emergeVersion;
}
