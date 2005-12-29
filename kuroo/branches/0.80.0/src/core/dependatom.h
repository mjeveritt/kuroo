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

#ifndef DEPENDATOM_H
#define DEPENDATOM_H

// #include "packagelist.h"

#include <qregexp.h>
#include <qvaluelist.h>

// class PortagePackage;
// class PortageCategory;
class PackageVersion;

/**
 * This class is the provides capabilities to parse DEPEND atoms and get
 * the appropriate matching Package objects out of a given portage tree.
 * DEPEND atoms are the strings in files like package.keywords, and are
 * also (all should we say mainly) used in ebuilds to describe dependencies.
 *
 * @short  A depend atom parser and matching package retriever.
 */
class DependAtom
{
public:
	DependAtom( /*TemplatedPackageList<PortagePackage>* packages*/ );
	~DependAtom();
	
	bool parse( const QString& atom );
	
	QValueList<PackageVersion*> matchingVersions();
	
	bool isBlocking();
	
private:
	//! A pointer to the portage tree from which the packages are retrieved.
// 	TemplatedPackageList<PortagePackage>* m_packages;
	//! The regular expression for the whole atom.
	QRegExp m_rxAtom;
	//! This is set to the result of parse().
	bool m_matches;
	
	// These are the extracted parts of the atom.
	
	//! true if the callsign prefix ("blocked by this package" in ebuild dependencies) is there.
	bool m_callsign;
	//! A compare sign (greater than / less than / equal) or the "all revisions" prefix ("~").
	QString m_prefix;
	//! The main category of the package.
// 	PortageCategory* m_category;
	//! The package name.
	QString m_package;
	//! The complete version string.
	QString m_version;
};

#endif
