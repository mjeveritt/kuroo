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

#ifndef PACKAGEVERSION_H
#define PACKAGEVERSION_H

#include <qstring.h>
#include <qstringlist.h>
#include <qregexp.h>

#include <ksharedptr.h>

class PackageItem;

/**
 * PackageVersion is a class for managing package version information,
 * such as description, home page or use flags, and of course the
 * version string itself.
 *
 * @short  A class for storing package version information.
 */
class PackageVersion
{
public:
	friend class PackageItem; // the only one that may construct this directly
	
	QString version() const;
	PackageItem* package();
	
	/**
		* Returns true if this version is installed on the system, false if not.
		*/
	virtual bool isInstalled() const/* = 0*/;
	
	/**
		* Returns true if this version is available (as in: can be installed)
		* and false if not.
		*/
	virtual bool isAvailable() const/* = 0*/;
	
	/**
		* Returns true if this object's version is newer than the one
		* given as argument. As this depends on the package management system,
		* it can be overloaded by derived classes.
		* This default implementation just provides a simple string compare,
		* which is not likely to be sufficient in all cases.
		*/
	virtual bool isNewerThan( const QString& otherVersion ) const;
	
	bool isOlderThan( const QString& otherVersion ) const;
	
protected:
	PackageVersion( PackageItem* parent, const QString& version );
	virtual ~PackageVersion();
	
private:
	/** The package containing this version. */
	PackageItem* m_parent;
	
	/** The package version string. */
	QString m_version;
};

#endif
