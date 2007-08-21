#ifndef __Queue__
#define __Queue__

#include "list.h"

template <class T> class Queue {
	public:
		Queue() { }
		Queue(const Queue& q) { _list=q._list; }
		~Queue() { }
		Queue& operator=(const Queue& q) {
			if (this!=&q) _list=q._list;
			return (*this);
		}
		inline bool EnQueue(const T& it) { return _list.TailAdd(it); }
		T UnQueue() {
			T result=_list.GetFirst();
			_list.HeadDel();
			return result;
		}
		T GetHead() const {
			T result=_list.GetFirst();
			return result;
		}
		inline bool Search(const T& it) const { return _list.Search(it); }
		inline int Size() const { return _list.Length(); }
		inline bool isEmpty() const { return _list.isEmpty; }

	private:
		List<T> _list;
};

#endif
