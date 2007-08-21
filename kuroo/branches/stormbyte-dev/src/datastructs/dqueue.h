#ifndef __DataQueue__
#define __DataQueue__

#include "list.h"

template <class T> class DataQueue {
	public:
		DataQueue() { }
		DataQueue(const DataQueue& q) { _list=q._list; }
		~DataQueue() { }
		DataQueue& operator=(const DataQueue& q) {
			if (this!=&q) _list=q._list;
			return (*this);
		}
		inline bool EnDataQueue(const T& it) { return _list.TailAdd(it); }
		T UnDataQueue() {
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
		DataList<T> _list;
};

#endif
