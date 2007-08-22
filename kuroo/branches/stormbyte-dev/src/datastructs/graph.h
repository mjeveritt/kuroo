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
#ifndef __Graph__
#define __Graph__

//#include "dlist.h"
#include "dqueue.h" //dqueue.h already includes dlist.h
#include "ebuild.h"


class Graph_Node {
	friend class Graph;
	public:
		Graph_Node(const Ebuild& e=Ebuild::Ebuild());
		Graph_Node(const Graph_Node&);
		~Graph_Node();
		Graph_Node& operator=(const Graph_Node&);
		inline Ebuild GetEbuild() const { return ebuild; }

	private:
		Ebuild ebuild;
		DataList<Graph_Node*> in, out;
		int inDegree, outDegree; //These are always kept updated due to performance issues
};

class Graph {
	public:
		Graph();
		Graph(const Graph&);
		~Graph();
		Graph& operator=(const Graph&);
		bool AddNode(const Ebuild&);
		bool DelNode(const Ebuild&);
		bool Search(const Ebuild&) const;
		bool Search(const Ebuild&, const Ebuild&) const;
		bool AddArc(const Ebuild&, const Ebuild&);
		bool DelArc(const Ebuild&, const Ebuild&);
		inline bool isEmpty() const { return _list.isEmpty(); }
		int InDegree(const Ebuild&) const;
		int OutDegree(const Ebuild&) const;

	private:
		DataList<Graph_Node> _list;
};

#endif
