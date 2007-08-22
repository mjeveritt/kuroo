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
	in=gn.in;
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
		in=gn.in;
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
	int size=_list.Size();
	for (int i=0; i<size && !result;i++)
		if (_list[i].ebuild==e) result=true;
	if (!result) {
		Graph_Node temp(e);
		result=_list.Add(temp);
	}
	return result;
}

bool Graph::DelNode(const Ebuild& e) {
	bool result=false;
	int node_pos;
	for (node_pos=0; node_pos<_list.Size() && !result; node_pos++)
		if (_list[node_pos].GetEbuild()==e) result=true;
	if (result) {
		int max_in=_list[node_pos].in.Size(), max_out=_list[node_pos].out.Size();
		for (int l1=0; l1<max_in; l1++) {
			_list[node_pos].in[l1]->outDegree--;
			_list[node_pos].in[l1]->out.Del(&_list[node_pos]);
		}
		for (int l1=0; l1<max_out; l1++) {
			_list[node_pos].out[l1]->inDegree--;
			_list[node_pos].out[l1]->in.Del(&_list[node_pos]);
		}
		_list.Del(node_pos);
	}
	return result;
}

bool Graph::Search(const Ebuild& e) const {
	int size=_list.Size();
	bool result=false;
	for (int i=0; i<size && !result; i++)
		if (_list[i].ebuild==e) result=true;
	return result;
}

bool Graph::Search(const Ebuild& origin, const Ebuild& destination) const {
	Graph_Node o, d;
	bool result=false, finded_o=false, finded_d=false;
	int size=_list.Size();
	for (int i=0; i<size && (!finded_o || !finded_d); i++) {
		if (_list[i].ebuild==origin) { o=_list[i]; finded_o=true; }
		else if (_list[i].ebuild==destination) { d=_list[i]; finded_d=true; }
	}
	if (finded_o && finded_d) {
		//I am going to search an arc from origin to destination.
		//that can be found in out list of origin, or in in list of destination
		int o_out_max=o.out.Size();
		for (int i=0; i<o_out_max && !result; i++)
			if (o.out[i]->ebuild==destination) result=true;
	}
	return result;
}

bool Graph::AddArc(const Ebuild& origin, const Ebuild& destination) {
	bool result=false, finded_o=false, finded_d=false;
	int size=_list.Size();
	int o=-1, d=-1;
	for (int i=0; i<size && (!finded_o || !finded_d); i++) {
		if (_list[i].ebuild==origin) { o=i; finded_o=true; }
		else if (_list[i].ebuild==destination) { d=i; finded_d=true; }
	}
	if (finded_o && finded_d) {
		_list[o].outDegree++;
		_list[o].out.Add(&_list[d]);
		_list[d].inDegree++;
		_list[d].in.Add(&_list[o]);
		result=true;
	}
	return result;
}

bool Graph::DelArc(const Ebuild& origin, const Ebuild& destination) {
	bool result=false, finded_o=false, finded_d=false;
	int size=_list.Size();
	int o=-1, d=-1;
	for (int i=0; i<size && (!finded_o || !finded_d); i++) {
		if (_list[i].ebuild==origin) { o=i; finded_o=true; }
		else if (_list[i].ebuild==destination) { d=i; finded_d=true; }
	}
	if (finded_o && finded_d) {
		int o_out_size=_list[o].out.Size(), d_in_size=_list[d].in.Size();
		bool finded_in_arc=false, finded_out_arc=false;
		int o_arc_pos, d_arc_pos;
		for (o_arc_pos=0; o_arc_pos<o_out_size && !finded_out_arc; o_arc_pos++)
			if (_list[o].out[o_arc_pos]->ebuild==destination) finded_out_arc=true;
		for (d_arc_pos=0; d_arc_pos<d_in_size && !finded_in_arc; d_arc_pos++)
			if (_list[d].in[d_arc_pos]->ebuild==origin) finded_in_arc=true;
		if (finded_in_arc && finded_out_arc) {
			//All data is existing so we can remove..
			_list[o].outDegree--;
			_list[o].out.Del(o_arc_pos-1); //-1 because at exit of for it adds 1
			_list[d].inDegree--;
			_list[d].in.Del(d_arc_pos-1);
			result=true;
		}
	}
	return result;
}
