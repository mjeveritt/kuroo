#ifndef __Stack__
#define __Stack__

#include "list.h"

template <class T> class Stack {
	public:
		Stack() { }
		Stack(const Stack& st) { _list=st._list; }
		~Stack() { }
		Stack& operator=(const Stack& st) {
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
		List<T> _list;
};

#endif
