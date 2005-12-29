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

#include "common.h"
#include "packageversion.h"

/**
 * Initialize the version with its version string.
 * Protected so that only PackageItem can construct a PackageVersion object.
 */
PackageVersion::PackageVersion( PackageItem* parent, const QString& version )
	: m_parent( parent ), m_version( version )
{
}

/**
 * Destruct this version object.
 */
PackageVersion::~PackageVersion()
{
}

/**
 * Returns the package version string.
 */
QString PackageVersion::version() const
{
	return m_version;
}

/**
 * Returns the package that this version belongs to.
 */
PackageItem* PackageVersion::package()
{
	return m_parent;
}

/**
 * Find out if this version has a higher version number than another one.
 *
 * @param otherVersion  Version string of the version that should be compared
 *                      to this one.
 * @return  true if this version is newer than the one given in the argument.
 *          false if the other version is newer (or if the strings are equal).
 */
bool PackageVersion::isNewerThan( const QString& otherVersion ) const
{
	kdDebug() << "PackageVersion::isNewerThan" << endl;
	if ( QString::compare(m_version, otherVersion) > 0 )
		return true; // this number is bigger than the other one
	else
		return false;
}


/**
 * Find out if this version has a lower version number than another one.
 * As this function is just using isNewerThan, it doesn't make sense to be
 * overloaded in derived classes (which is why it's not declared virtual).
 *
 * @param otherVersion  Version string of the version that should be compared
 *                      to this one.
 * @return  true if this version is older than the one given in the argument.
 *          false if the other version is older (or if the strings are equal).
 */
bool PackageVersion::isOlderThan( const QString& otherVersion ) const
{
	kdDebug() << "PackageVersion::isOlderThan" << endl;
	if ( m_version == otherVersion || isNewerThan(otherVersion) == true )
		return false;
	else
		return true;
}

bool PackageVersion::isAvailable() const
{
	kdDebug() << "PackageVersion::isAvailable" << endl;
}

bool PackageVersion::isInstalled() const
{
	kdDebug() << "PackageVersion::isInstalled" << endl;
}
