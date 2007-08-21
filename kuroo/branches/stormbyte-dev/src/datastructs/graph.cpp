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
#include "graph.h"

Graph_Node::Graph_Node(const Ebuild& e) {
	ebuild=e;
	inDegree=outDegree=0;
}

Graph_Node::Graph_Node(const Graph_Node& gn) {
	ebuild=gn.ebuild;
	out=gn.out;
	inDegree=gn.inDegree;
	outDegree=gn.outDegree;
}

Graph_Node::~Graph_Node() {
	inDegree=outDegree=-1;
}

Graph_Node& Graph_Node::operator=(const Graph_Node& gn) {
	if (this!=&gn) {
		ebuild=gn.ebuild;
		out=gn.out;
		inDegree=gn.inDegree;
		outDegree=gn.outDegree;
	}
	return (*this);
}

/***************************************************************************/

/*
	Graph();
	Graph(const Graph&);
	~Graph();
	Graph& operator=(const Graph&);
	bool AddNode(const Ebuild&);
	bool DelNode();
	bool Search(const Ebuild&) const;
	bool Search(const Ebuild&, const Ebuild&) const;
	bool AddArc(const Ebuild&, const Ebuild&);
	bool DelArc(const Ebuild&, const Ebuild&);
	inline bool isEmpty() const { return _list.isEmpty(); }
	int InDegree(const Ebuild&) const;
	int OutDegree(const Ebuild&) const;

private:
	DataList<Graph_Node> _list;
	*/
Graph::Graph() { }

Graph::Graph(const Graph& gr) {
	_list=gr._list;
}

Graph::~Graph() { }

Graph& Graph::operator=(const Graph& gr) {
	if (this!=&gr) _list=gr._list;
	return (*this);
}


bool Graph::AddNode(const Ebuild& e) {
	//Can't have repeated nodes!
	bool result=false;
	ListIterator<Graph_Node> it=_list.First();
	while (!it.isEmpty() && !result) {
		if (it.GetItem().ebuild==e) result=true;
		else it=it.Next();
	}
	if (!result) {
		Graph_Node temp(e);
		result=_list.Add(temp);
	}
	return result;
}

bool Graph::DelNode(const Ebuild& e) {
#warning IMPLEMENT THIS
}
