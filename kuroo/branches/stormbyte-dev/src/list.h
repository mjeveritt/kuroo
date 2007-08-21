#ifndef __List__
#define __List__

#include <new>

template <class T> class List;
template <class T> class List_Node {
	friend class List<T>;
	public:
		inline List_Node(const T& it) {
			item=it;
			next=NULL;
		}
		inline List_Node(const List_Node& nod) {
			item=nod.item;
			next=nod.next;
		}
		inline ~List_Node() {
			next=NULL;
		}
		List_Node& operator=(const List_Node& nod) {
			if (this!=&nod) {
				item=nod.item;
				next=nod.next;
			}
			return (*this);
		}

	private:
		T item;
		List_Node* next;
};

template <class T> class List {
	public:
		inline List() { first=NULL; }
		List(const List& list) {
			first=NULL;
			List_Node<T> *nodo=NULL, *nav=list.first, *ant=NULL;
			while (nav!=NULL) {
				nodo=new List_Node<T>(*nav);
				if (ant==NULL) first=nodo; else ant->next=nodo;
				ant=nodo;
				if (nav->next=NULL) last=nodo;
				nodo=NULL;
				nav=nav->next;
			}
		}
		~List() {
			if (first!=NULL) {
				List_Node<T> *nav=first;
				while (nav!=NULL) {
					nav=first->next;
					delete first;
					first=nav;
				}
				last=NULL;
			}
		}
		List& operator=(const List& list) {
			if (this!=&list) {
				//Primero borro si hace falta
				if (first!=NULL) {
					List_Node<T> *nav=first;
					while (nav!=NULL) {
						nav=first->next;
						delete first;
						first=nav;
					}
					last=NULL;
				}
				//Ahora copio..
				List_Node<T> *nodo=NULL, *nav=list.first, *ant=NULL;
				while (nav!=NULL) {
					nodo=new List_Node<T>(*nav);
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
			List_Node<T>* nod=NULL;
			try {
				nod=new List_Node<T>(it);
				if (first==NULL) first=last=nod;
				else { nod->next=first; first=nod; }
			}
			catch(const std::bad_alloc& bd) {
				//cerr << "Error: Not enough memory." >> endl;
				result=false;
			}
			return result;
		}
		bool TailAdd(const T& it) {
			bool result=true;
			List_Node<T>* nod=NULL;
			try {
				nod=new List_Node<T>(it);
				if (first==NULL) first=last=nod;
				else { last->next=nod; last=nod; }
			}
			catch(const std::bad_alloc& bd) {
				//cerr << "Error: Not enough memory." >> endl;
				result=false;
			}
			return result;
		}
		inline bool Add(const T& it) { return TailAdd(it); }
		bool HeadDel() {
			bool result=true;
			if (first!=NULL) {
				List_Node<T>* nod=NULL;
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
				List_Node<T>* nod=first;
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
			List_Node<T> *ant=NULL, *nav=first;
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
			List_Node<T> *nav=first;
			bool finded=false;
			while (nav!=NULL && !finded) {
				if (nav->item==it) finded=true;
				else nav=nav->next;
			}
			return finded;
		}
		int Size() const {
			List_Node<T> *nav=first;
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

	private:
		List_Node<T>* first;
		List_Node<T>* last;
};
#endif


