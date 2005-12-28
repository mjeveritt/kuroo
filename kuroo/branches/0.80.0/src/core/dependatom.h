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

#include <qregexp.h>
#include <qobject.h>

/**
 * This class is the provides capabilities to parse DEPEND atoms and get
 * the appropriate matching Package objects out of a given portage tree.
 * DEPEND atoms are the strings in files like package.keywords, and are
 * also (all should we say mainly) used in ebuilds to describe dependencies.
 *
 * @short  A depend atom parser and matching package retriever.
 */
class DependAtom : public QObject
{
Q_OBJECT
public:
	DependAtom( QObject *parent );
	~DependAtom();
	
	bool parse( const QString& atom );

	QStringList matchingVersions( const QString& versionList );

	bool isBlocking();

protected:
	//! A pointer to the portage tree from which the packages are retrieved.
// 	PortageTree* tree;

private:
	//! The regular expression for the whole atom.
	QRegExp rxAtom;
	//! This is set to the result of parse().
	bool matches;

	// These are the extracted parts of the atom.

	//! true if the callsign prefix ("blocked by this package" in ebuild dependencies) is there.
	bool callsign;
	//! A compare sign (greater than / less than / equal) or the "all revisions" prefix ("~").
	QString prefix;
	//! The main category of the package.
	QString category;
	//! The subcategory of the package.
	QString subcategory;
	//! The package name.
	QString package;
	//! The complete version string.
	QString version;
};

#endif // DEPENDATOM_H
