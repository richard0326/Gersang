#pragma once

template <typename T>
class CList
{
public:
	struct Node
	{
		T _data;
		Node* _Prev;
		Node* _Next;
	};

	class iterator
	{
		friend iterator CList<T>::erase(iterator iter);
	private:
		Node* _node;
	public:
		iterator(Node* node = nullptr)
		{
			//인자로 들어온 Node 포인터를 저장
			_node = node;
		}

		iterator operator ++(int)
		{
			//현재 노드를 다음 노드로 이동
			iterator ret(_node);
			_node = _node->_Next;
			return ret;
		}

		iterator& operator++()
		{
			_node = _node->_Next;
			return *this;
		}

		iterator operator--(int)
		{
			iterator ret(_node);
			_node = _node->_Prev;
			return ret;
		}

		iterator& operator--()
		{
			_node = _node->_Prev;
			return *this;
		}

		T& operator*()
		{
			//현재 노드의 데이터를 뽑음
			return _node->_data;
		}
		bool operator==(const iterator& other)
		{
			if (_node == other._node)
			{
				return true;
			}
			return false;
		}
		bool operator!=(const iterator& other)
		{
			if (_node != other._node)
			{
				return true;
			}
			return false;
		}
	};

public:
	CList();
	~CList();

	iterator begin()
	{
		//첫번째 노드를 가리키는 이터레이터 리턴		
		iterator ret(_head._Next);
		return ret;
	}
	iterator end()
	{
		//Tail 노드를 가리키는(데이터가 없는 진짜 더미 끝 노드) 이터레이터를 리턴
		//또는 끝으로 인지할 수 있는 이터레이터를 리턴
		iterator ret(&_tail);
		return ret;
	}

	iterator rbegin()
	{
		//첫번째 노드를 가리키는 이터레이터 리턴
		iterator ret(_tail._Prev);
		return ret;
	}
	iterator rend()
	{
		//Tail 노드를 가리키는(데이터가 없는 진짜 더미 끝 노드) 이터레이터를 리턴
		//또는 끝으로 인지할 수 있는 이터레이터를 리턴
		iterator ret(&_head);
		return ret;
	}

	void push_front(T data);
	void push_back(T data);
	T pop_front();
	T pop_back();
	void clear();
	int size() { return _size; }
	bool empty() { return _size == 0; }

	iterator erase(iterator iter)
	{
		iterator ret(iter._node->_Next);
		iter._node->_Prev->_Next = iter._node->_Next;
		iter._node->_Next->_Prev = iter._node->_Prev;
		
		_size--;

		delete iter._node;
		return ret;
	}
	//- 이터레이터의 그 노드를 지움.
	//- 그리고 지운 노드의 다음 노드를 카리키는 이터레이터 리턴

	void remove(T Data);

private:
	int _size = 0;
	Node _head;
	Node _tail;
};

template <typename T>
CList<T>::CList()
{
	_head._Next = &_tail;
	_tail._Prev = &_head;

	_head._Prev = _tail._Next = 0;
	_head._data = _tail._data = 0;
}

template <typename T>
CList<T>::~CList()
{
	clear();
}

template <typename T>
void CList<T>::push_front(T data)
{
	Node* pNode = new Node();
	pNode->_data = data;
	pNode->_Next = _head._Next;
	pNode->_Prev = &_head;

	_head._Next->_Prev = pNode;
	_head._Next = pNode;

	_size++;
}

template <typename T>
void CList<T>::push_back(T data)
{
	Node* pNode = new Node();
	pNode->_data = data;
	pNode->_Prev = _tail._Prev;
	pNode->_Next = &_tail;

	_tail._Prev->_Next = pNode;
	_tail._Prev = pNode;

	_size++;
}

template <typename T>
T CList<T>::pop_front()
{
	if (_size == 0)
		return 0;

	Node* node = _head._Next;
	T ret = node->_data;

	node->_Next->_Prev = &_head;
	_head._Next = node->_Next;

	delete node;

	_size--;
	return ret;
}

template <typename T>
T CList<T>::pop_back()
{
	if (_size == 0)
		return 0;

	Node* node = _tail._Prev;
	T ret = node->_data;

	node->_Prev->_Next = &_tail;
	_tail._Prev = node->_Prev;

	delete node;

	_size--;
	return ret;
}

template <typename T>
void CList<T>::clear()
{
	Node* tmp = _head._Next;
	while (tmp != &_tail)
	{
		Node* ttmp = tmp->_Next;
		delete tmp;
		tmp = ttmp;
	}

	_head._Next = &_tail;
	_tail._Prev = &_head;
	_size = 0;
}

template <typename T>
void CList<T>::remove(T Data)
{
	Node* tmp = _head._Next;
	while (tmp != &_tail)
	{
		if (tmp->_data == Data)
		{
			Node* ttmp = tmp->_Next;

			ttmp->_Prev = tmp->_Prev;
			ttmp->_Prev->_Next = ttmp;

			delete tmp;
			tmp = ttmp;

			_size--;
		}
		else 
		{
			tmp = tmp->_Next;
		}
	}
}