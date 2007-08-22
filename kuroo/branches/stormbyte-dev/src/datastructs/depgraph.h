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
#ifndef __DepGraph__
#define __DepGraph__

#include "graph.h"

class DepGraph: private Graph {
	public:
		DepGraph();
		DepGraph(const DepGraph&);
		~DepGraph();
		DepGraph& operator=(const DepGraph&);
		bool AddEbuild(const Ebuild&);
		void ScanDependencies();
		void MergeFinished(const Ebuild&);
		void UpdateEmergeQueue(DataQueue<Ebuild>&);
};
#endif
