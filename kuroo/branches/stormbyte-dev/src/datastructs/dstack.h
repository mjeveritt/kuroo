#ifndef __DataStack__
#define __DataStack__

#include "list.h"

template <class T> class DataStack {
	public:
		DataStack() { }
		DataStack(const DataStack& st) { _list=st._list; }
		~DataStack() { }
		DataStack& operator=(const DataStack& st) {
			if (this!=&st) _list=st._list;
			return (*this);
		}
		inline bool Push(const T& it) { return _list.HeadAdd(it); }
		T Pop() {
			T result;
			result=_list.GetFirst();
			_list.HeadDel();
			return result;
		}
		inline T GetHead() const { return _list.GetHead(); }
		inline int Size() const { return _list.Size(); }
		inline bool Search(const T& it) const { return _list.Search(it); }

	private:
		DataList<T> _list;
};

#endif
