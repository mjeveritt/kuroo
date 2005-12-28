/***************************************************************************
 *   Copyright (C) 2005 by Jakob Petsovits                                 *
 *   jpetso@gmx.at                                                         *
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

#include "dependatom.h"
#include "common.h"

// For more info on DEPEND atoms, see the DEPEND Atoms section of man 5 ebuild

// capture positions inside the regexp. (like rxAtom.cap(POS_CALLSIGN))
#define POS_CALLSIGN    1
#define POS_PREFIX      2
#define POS_CATEGORY    3
#define POS_SUBCATEGORY 4
#define POS_PACKAGE     5
#define POS_VERSION     6

// The complete atom regexp in non-escaped form (for testing, or similar):
// ^(!)?(~|(?:<|>|=|<=|>=))?((?:[a-z]|[0-9])+)-((?:[a-z]|[0-9])+)/((?:[a-z]|[A-Z]|[0-9]|-|\+|_)+)((?:\*$|-\d+(?:\.\d+)*[a-z]?(?:\*$)?)(?:_(?:alpha|beta|pre|rc|p)\d+)?(?:-r\d+)?)?$

/**
 * Initialize this object.
 * @param portageTree  The portage tree which contains the packages
 *                     that will be filtered out.
 */
DependAtom::DependAtom( QObject* parent )
	: QObject( parent ),
	rxAtom("^"    // Start of the string
         "(!)?" // "Block these packages" flag, only occurring in ebuilds
         "(~|(?:<|>|=|<=|>=))?" // greater-than/less-than/equal, or "all revisions" prefix
         "((?:[a-z]|[0-9])+)-((?:[a-z]|[0-9])+)/"   // category and subcategory
         "((?:[a-z]|[A-Z]|[0-9]|-|\\+|_)+)" // package name
         "("            // start of the version part
         "(?:\\*$|-\\d+(?:\\.\\d+)*[a-z]?(?:\\*$)?)" // base version number,
                                   // including wildcard version matching (*)
         "(?:_(?:alpha|beta|pre|rc|p)\\d+)?" // version suffix
         "(?:-r\\d+)?"  // revision
         ")?$"          // end of the (optional) version part and the atom string
         )
{
// 	tree = portageTree;
	matches = false;
	callsign = false;
}

/**
 * Test the atom string on validness, and fill the internal variables with
 * the extracted atom parts like category, package name and version. If the
 * atom is valid, you can afterwards call matchingPackages() and isBlocking().
 * If it's invalid, the result of the these functions is undefined.
 * So, make sure to check the return value.
 *
 * @param atom  The atom string that should be parsed. This string is expected
 *              not to have leading or trailing whitespaces, otherwise it
 *              will not be considered valid in any case.
 * @return  true if the atom is valid, false otherwise.
 */
bool DependAtom::parse( const QString& atom )
{
	// Do the regexp match, which also prepares for text capture
	if( rxAtom.exactMatch(atom) == false ) {
		matches = false;
		return false;
	}

	// Get the captured strings
	callsign    = rxAtom.cap( POS_CALLSIGN ).isEmpty() ? false : true;
	prefix      = rxAtom.cap( POS_PREFIX );
	category    = rxAtom.cap( POS_CATEGORY );
	subcategory = rxAtom.cap( POS_SUBCATEGORY );
	package     = rxAtom.cap( POS_PACKAGE );
	version     = rxAtom.cap( POS_VERSION );

	// Additional check: If there is a version, there also must be a prefix
	if( version.isEmpty() != prefix.isEmpty() ) {
		matches = false;
		return false;
	}

	// Strip the hyphen at the start of the version, except when it's a "*"
	if( !version.isEmpty() && version[0] == '-' )
		version = version.mid(1);

	// Not yet returned false, so it's a valid atom
	matches = true;
	return true;
}


/**
 * Retrieve the set of package versions that is matching the atom.
 * The searched packages are the ones from the portage tree.
 * If no matching package versions are found, an empty list is returned.
 */
QStringList DependAtom::matchingVersions( const QString& versionList )
{
	QStringList matchingVersions;

// 	if ( tree == NULL || matches == false )
// 		return matchingVersions;

// 	if ( tree->hasPackage(category, subcategory, package) == false )
// 		return matchingVersions;
// 
// 	Package* pkg = tree->package( category, subcategory, package );

	bool matchAllVersions;
	if ( version.isEmpty() || version == "*" )
		matchAllVersions = true;
	else
		matchAllVersions = false;

	bool matchBaseVersion;
	if ( version.endsWith("*") ) {
		// remove the trailing star
		version = version.left( version.length() - 1 );
		matchBaseVersion = true;
	}
	else {
		matchBaseVersion = false;
	}

	// When this is set true, it will match all versions
	// with exactly the same version string as the parsed one.
	bool matchEqual = prefix.endsWith("=");

	// When this is set true, it will match all versions greater than
	// (but not equalling) the parsed version from the atom.
	// When false, it will match all versions less than
	// (but not equalling, too) the parsed version.
	bool matchGreaterThan = prefix.startsWith(">");


	// So, let's iterate through the versions to check if they match or not
	foreach ( versionList ) {
		if ( ( matchAllVersions == true ) ||
		    ( matchBaseVersion == true  && (*it).startsWith(version) ) ||
		    ( matchEqual       == true  && (*it) == version   ) ||
		    ( matchEqual == false && matchGreaterThan == true  && (*it).isNewerThan(version) ) ||
		    ( matchEqual == false && matchGreaterThan == false && (*it).isOlderThan(version) )
		  ) {
			matchingVersions.append( *versionIterator );
			continue;
		}
	}
	return matchingVersions;

} // end of matchingVersions()


/**
 * Return true if the atom begins with a call sign ("!") which means that
 * this package is blocking another one. This is only used inside ebuilds,
 * where it looks, for example, like DEPEND="!app-cdr/dvdrtools".
 * If there is no call sign, the function returns false.
 */
bool DependAtom::isBlocking()
{
	return callsign;
}

#include "dependatom.moc"
