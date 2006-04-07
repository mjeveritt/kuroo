/***************************************************************************
*   Copyright (C) 2005 by Karye   *
*   karye@users.sourceforge.net   *
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
#include "threadweaver.h"

#ifndef LOADPORTAGEJOB_H
#define LOADPORTAGEJOB_H

#include <qobject.h>

class DbConnection;

/**
 * @class LoadPortageJob
 * @short Collect portage packages from portage api.
 */
class LoadPortageJob : public ThreadWeaver::DependentJob
{
Q_OBJECT
public:
	LoadPortageJob( QObject *parent = 0 );
	~LoadPortageJob();

private:
	bool 								doJob();
	void 								completeJob();
	bool								insertCategories();
	int								loadPackages();
	
	QString escapeString( QString string ) {
		return string.replace('\'', "''").replace('%', "&#37;");
	}
	
private:
	bool								m_aborted;
	DbConnection* const 				m_db;
	
	struct Data {
		QString							description;
		QString							homepage;
		QString							status;
		QString							licenses;
		QString							iuse;
		QString							slot;
		QString							size;
		QString							keywords;
		QString							path;
	};
	
	typedef QMap<QString, Data>			PortageVersions;
	struct Versions {
		QString							status;
		QString							description;
		PortageVersions					versions;
	};
	
	typedef QMap<QString, Versions>		PortagePackages;
	struct Categories {
		QString							category;
		QString							subCategory;
		QString							idCategory;
		QString							idSubCategory;
		QString							idCatSubCategory;
		PortagePackages					packages;
	};
	
	typedef QMap<QString, Categories>	PortageCategories;
	PortageCategories					m_categories;
};

#endif
