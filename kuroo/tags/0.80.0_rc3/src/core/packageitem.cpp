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
#include "packageitem.h"
#include "packageversion.h"
#include "dependatom.h"

#include <qpainter.h>

#include <klistview.h>

/**
 * @class PackageItem
 * @short Base class for package.
 */
PackageItem::PackageItem( QListView* parent, const char* name, const QString& id, const QString& category, const QString& description, const QString& status )
	: KListViewItem( parent, name ),
	m_parent( parent ), m_index( 0 ),
	m_id( id ), m_name( name ), m_status( status ), m_description( description ), m_category( category ), m_isQueued( false ), m_inWorld( false ),
	hasDetailedInfo( false )
{
}

PackageItem::PackageItem( QListViewItem* parent, const char* name, const QString& id, const QString& category, const QString& description, const QString& status )
	: KListViewItem( parent, name ),
	m_parent( parent->listView() ), m_index( 0 ),
	m_id( id ), m_name( name ), m_status( status ), m_description( description ), m_category( category ), m_isQueued( false ), m_inWorld( false ),
	hasDetailedInfo( false )
{
}

PackageItem::~PackageItem()
{}

/**
 * Register package index in parent listView.
 * @param index
 */
void PackageItem::setPackageIndex( int index )
{
	m_index = index;
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
 * Set icons when package is visible.
 */
void PackageItem::paintCell( QPainter* painter, const QColorGroup& colorgroup, int column, int width, int alignment )
{
	if ( this->isVisible() ) {
		QColorGroup m_colorgroup( colorgroup );
		
		// Optimizing - do not check for not relevant columns
		switch ( column ) {
			
			case 0 : {
				if ( m_status == FILTER_ALL_STRING )
					setPixmap( 0, ImagesSingleton::Instance()->icon( PACKAGE ) );
				else {
					if ( KurooConfig::installedColumn() ) {
						setPixmap( 0, ImagesSingleton::Instance()->icon( PACKAGE ) );
						setPixmap( 1, ImagesSingleton::Instance()->icon( VERSION_INSTALLED ) );
					}
					else
						setPixmap( 0, ImagesSingleton::Instance()->icon( INSTALLED ) );
					
					if ( m_status == FILTER_OLD_STRING ) {
						QFont font( painter->font() );
						font.setItalic( true );
						painter->setFont( font );
						m_colorgroup.setColor( QColorGroup::Text, Qt::gray );
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
				break;
			}
			
		}
		
		KListViewItem::paintCell( painter, m_colorgroup, column, width, alignment );
	}
}

/**
 * Package db id.
 * @return id
 */
QString PackageItem::id()
{
	return m_id;
}

/**
 * Package name as kuroo in app-portage/kuroo.
 * @return name
 */
QString PackageItem::name()
{
	return m_name;
}

/**
 * Package description.
 * @return description
 */
QString PackageItem::description()
{
	return m_description;
}

/**
 * Package status describing if this package is installed or not.
 * @return status
 */
QString PackageItem::status()
{
	return m_status;
}

/**
 * Is this package installed.
 * @return true if yes
 */
bool PackageItem::isInstalled()
{
	return ( m_status == FILTER_INSTALLED_STRING || m_status == FILTER_OLD_STRING );
}

/**
 * Is this package available in Portage tree?
 * @return true if yes
 */
bool PackageItem::isInPortage()
{
	return ( m_status != FILTER_OLD_STRING );
}

/**
 * Is this package is in the emerge queue?
 * @return true/false
 */
bool PackageItem::isQueued()
{
	return m_isQueued;
}

bool PackageItem::isInWorld()
{
	return m_inWorld;
}

void PackageItem::setDescription( const QString& description )
{
	m_description = description;
}

void PackageItem::setInstalled()
{
	m_status = FILTER_INSTALLED_STRING;
}

/**
 * Mark package as queued. Emit signal only if status is changed.
 * @param isQueued
 */
void PackageItem::setQueued( bool isQueued )
{
	if ( m_isQueued != isQueued ) {
		m_isQueued = isQueued;
		SignalistSingleton::Instance()->packageChanged();
	}
}

/**
 * Accessor for category.
 * @return the package category.
 */
QString PackageItem::category()
{
	return m_category;
}

void PackageItem::resetDetailedInfo()
{
	hasDetailedInfo = false;
}

/**
 * Initialize the package with all its versions and info. Executed when PortageItem get focus first time.
 */
void PackageItem::initVersions()
{
	if ( hasDetailedInfo )
		return;
	
	m_versions.clear();
	m_versionMap.clear();
	
// 	clock_t start = clock();
	
	// Get list of accepted keywords, eg if package is "untesting"
	QString acceptedKeywords = KurooDBSingleton::Instance()->packageKeywordsAtom( id() );
	
	const QStringList versionsList = KurooDBSingleton::Instance()->packageVersionsInfo( id() );
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
		version->setLicenses( QStringList::split( " ", licenses ) );
		version->setUseflags( QStringList::split( " ", useFlags ) );
		version->setSlot( slot );
		version->setKeywords( QStringList::split( " ", keywords ) );
		version->setAcceptedKeywords( QStringList::split( " ", acceptedKeywords ) );
		version->setSize( size );
		
		if ( status == FILTER_INSTALLED_STRING )
			version->setInstalled( true );
		
		m_versions.append( version );
		m_versionMap.insert( versionString, version );
	}
	
	// Initialize the 'atom' member variable
	atom = new DependAtom( this );
	
	// Check if any of this package versions are hardmasked
	const QStringList atomHardMaskedList = KurooDBSingleton::Instance()->packageHardMaskAtom( id() );
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
	
	// Initialize the 'atom' member variable
	atom = new DependAtom( this );
	
	// Check if any of this package versions are unmasked
	const QStringList atomUnmaskedList = KurooDBSingleton::Instance()->packageUnMaskAtom( id() );
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
	
	// Initialize the 'atom' member variable
	atom = new DependAtom( this );
	
	// Check if any of this package versions are user-masked
	const QStringList atomUserMaskedList = KurooDBSingleton::Instance()->packageUserMaskAtom( id() );
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
	
// 	clock_t finish = clock();
// 	const double duration = (double) ( finish - start ) / CLOCKS_PER_SEC;
// 	kdDebug() << "PortageListView::PortageItem::initVersions SQL-query (" << duration << "s): " << endl;
	
	// This package has collected all it's data
	hasDetailedInfo = true;
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
 * Return list of versions.
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

