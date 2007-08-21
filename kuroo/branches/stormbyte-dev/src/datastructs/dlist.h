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
#ifndef __DataList__
#define __DataList__

#include <new.h>

//template <class T> class DataList_Node;

template <class T> class DataList;
template <class T> class ListIterator;

template <class T> class DataList_Node {
	friend class DataList<T>;
	friend class ListIterator<T>;
	public:
		inline DataList_Node(const T& it) {
			item=it;
			next=NULL;
		}
		inline DataList_Node(const DataList_Node& nod) {
			item=nod.item;
			next=nod.next;
		}
		inline ~DataList_Node() {
			next=NULL;
		}
		DataList_Node& operator=(const DataList_Node& nod) {
			if (this!=&nod) {
				item=nod.item;
				next=nod.next;
			}
			return (*this);
		}

	private:
		T item;
		DataList_Node* next;
};

template <class T> class ListIterator {
	friend class DataList<T>;
	public:
		inline ListIterator() { item=NULL; }
		inline ListIterator(const ListIterator& lt) { item=lt.item; }
		inline ~ListIterator() { item=NULL; }
		ListIterator& operator=(const ListIterator& lt) {
			if (this!=&lt) item=lt.item;
			return (*this);
		}
		ListIterator Next() {
			ListIterator<T> result;
			if (item!=NULL) result.item=item->next;
			return result;
		}
		inline bool isEmpty() { return item==NULL; }
		inline T GetItem() const { return item->item; }

	private:
		DataList_Node<T>* item;
};

template <class T> class DataList {
	public:
		inline DataList() { first=NULL; }
		DataList(const DataList& list) {
			first=NULL;
			DataList_Node<T> *nodo=NULL, *nav=list.first, *ant=NULL;
			while (nav!=NULL) {
				nodo=new DataList_Node<T>(*nav);
				if (ant==NULL) first=nodo; else ant->next=nodo;
				ant=nodo;
				if (nav->next=NULL) last=nodo;
				nodo=NULL;
				nav=nav->next;
			}
		}
		~DataList() {
			if (first!=NULL) {
				DataList_Node<T> *nav=first;
				while (nav!=NULL) {
					nav=first->next;
					delete first;
					first=nav;
				}
				last=NULL;
			}
		}
		DataList& operator=(const DataList& list) {
			if (this!=&list) {
				//Primero borro si hace falta
				if (first!=NULL) {
					DataList_Node<T> *nav=first;
					while (nav!=NULL) {
						nav=first->next;
						delete first;
						first=nav;
					}
					last=NULL;
				}
				//Ahora copio..
				DataList_Node<T> *nodo=NULL, *nav=list.first, *ant=NULL;
				while (nav!=NULL) {
					nodo=new DataList_Node<T>(*nav);
					if (ant==NULL) first=nodo; else ant->next=nodo;
					ant=nodo;
					if (nav->next==NULL) last=nodo;
					nodo=NULL;
					nav=nav->next;
				}
			}
			return (*this);
		}
		bool HeadAdd(const T& it) {
			bool result=true;
			DataList_Node<T>* nod=NULL;
			nod=new DataList_Node<T>(it);
			if (first==NULL) first=last=nod;
			else { nod->next=first; first=nod; }
			result=false;
			return result;
		}
		bool TailAdd(const T& it) {
			bool result=true;
			DataList_Node<T>* nod=NULL;
			nod=new DataList_Node<T>(it);
			if (first==NULL) first=last=nod;
			else { last->next=nod; last=nod; }
			return result;
		}
		inline bool Add(const T& it) { return TailAdd(it); }
		bool HeadDel() {
			bool result=true;
			if (first!=NULL) {
				DataList_Node<T>* nod=NULL;
				nod=first->next;
				delete first;
				first=nod;
				if (first==NULL) last=NULL;
				else if (first->next==NULL) last=first;
			}
			else result=false;
			return result;
		}
		bool TailDel() {
			bool result=true;
			if (last!=NULL) {
				DataList_Node<T>* nod=first;
				while (nod->next!=NULL && nod->next!=last)
					nod=nod->next;
				delete last;
				last=nod;
				if (last==NULL) first=NULL;
				else if (first->next==NULL) first=last;
			}
			else result=false;
			return result;
		}
		bool Del(const T& it) {
			DataList_Node<T> *ant=NULL, *nav=first;
			bool finded=false;
			while (nav!=NULL && !finded) {
				if (nav->item==it) finded=true;
				else { ant=nav; nav=nav->next; }
			}
			if (finded) {
				if (first==nav) {
					first=nav->next;
					delete nav;
					if (first==NULL) last=first;
					else if (first->next==NULL) last=first;
				}
				else {
					ant->next=nav->next;
					if (nav==last) last=ant;
					delete nav;
				}
			}
			return finded;
		}
		inline bool isEmpty() const { return (first==NULL); }
		bool Search(const T& it) const {
			DataList_Node<T> *nav=first;
			bool finded=false;
			while (nav!=NULL && !finded) {
				if (nav->item==it) finded=true;
				else nav=nav->next;
			}
			return finded;
		}
		int Size() const {
			DataList_Node<T> *nav=first;
			int result=0;
			while (nav!=NULL) {
				result++;
				nav=nav->next;
			}
			return result;
		}
		inline T GetFirst() const { 
			T result;
			if (first!=NULL) result=(first->item);
			return result;
		}
		inline T GetLast() const {
			T result=NULL;
			if (last!=NULL) result=(last->item);
		}
		ListIterator<T> First() const {
			ListIterator<T> it;
			if (first!=NULL) it.item=first;
			return it;
		}
		
		ListIterator<T> Last() const {
			ListIterator<T> it;
			if (last!=NULL) it.item=last;
			return it;
		}

	private:
		DataList_Node<T>* first;
		DataList_Node<T>* last;
};
#endif


