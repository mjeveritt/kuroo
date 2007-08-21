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
