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
