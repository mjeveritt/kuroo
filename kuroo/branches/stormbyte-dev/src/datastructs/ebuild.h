/***************************************************************************
*   Copyright (C) 2004 by David C. Manuelda                               *
*   stormbyte@users.sourceforge.net                                       *
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
#ifndef __Ebuild__
#define __Ebuild__

#include <string>

using namespace std;

class Ebuild {
	public:
		inline Ebuild(const string& cat="", const string& pack="", const string& ver="", const bool record=false):
			category(cat), package(pack), version(ver), recordInWorld(record) { }
		inline Ebuild(const Ebuild& e) { category=e.category; package=e.package; version=e.version; recordInWorld=e.recordInWorld; }
		~Ebuild() { }
		Ebuild& operator=(const Ebuild&);
		inline bool operator==(const Ebuild& e) const { return (category==e.category && package==e.package && version==e.version
								&& recordInWorld==e.recordInWorld); }
		inline bool operator!=(const Ebuild& e) const { return !((*this)==e); }
		inline void SetCategory(const string& cat) { category=cat; }
		inline void SetPackageName(const string& pack) { package=pack; }
		inline void SetPackageVersion(const string& ver) { version=ver; }
		inline string GetCategory() const { return category; }
		inline string GetPackageName() const { return package; }
		inline string GetVersion() const { return version; }
		inline string GetCompleteName() const { return (category + "/" + package + "-" + version); }
		inline bool RecordInWorld() const { return recordInWorld; }
		inline void RecordInWorld(const bool& world) { recordInWorld=world; }

	private:
		string category, package, version;
		bool recordInWorld;
};
#endif
