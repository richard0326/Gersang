#pragma once

template <typename T>
class CList
{
public:
	struct Node
	{
		Node* _Next;
		Node* _Prev;
		T _data;
	};

	class iterator
	{
		template <typename T> friend class CList;
	private:
		Node* _node;
	public:
		iterator() = default;

		iterator operator ++(int)
		{
			//현재 노드를 다음 노드로 이동
			iterator ret;
			ret._node = _node;
			_node = _node->_Next;
			return ret;
		}

		__forceinline iterator& operator++()
		{
			_node = _node->_Next;
			return *this;
		}

		iterator operator--(int)
		{
			iterator ret;
			ret._node = _node;
			_node = _node->_Prev;
			return ret;
		}

		__forceinline iterator& operator--()
		{
			_node = _node->_Prev;
			return *this;
		}

		__forceinline T& operator*()
		{
			//현재 노드의 데이터를 뽑음
			return _node->_data;
		}
		__forceinline bool operator==(const iterator& other)
		{
			if (_node == other._node)
			{
				return true;
			}
			return false;
		}
		__forceinline bool operator!=(const iterator& other)
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

	__forceinline iterator begin()
	{
		//첫번째 노드를 가리키는 이터레이터 리턴
		iterator ret;
		ret._node = _head._Next;
		return ret;
	}
	__forceinline iterator end()
	{
		//Tail 노드를 가리키는(데이터가 없는 진짜 더미 끝 노드) 이터레이터를 리턴
		//또는 끝으로 인지할 수 있는 이터레이터를 리턴
		iterator ret;
		ret._node = &_tail;
		return ret;
	}

	__forceinline iterator rbegin()
	{
		//첫번째 노드를 가리키는 이터레이터 리턴
		iterator ret;
		ret._node = _tail._Prev;
		return ret;
	}
	__forceinline iterator rend()
	{
		//Tail 노드를 가리키는(데이터가 없는 진짜 더미 끝 노드) 이터레이터를 리턴
		//또는 끝으로 인지할 수 있는 이터레이터를 리턴
		iterator ret;
		ret._node = &_head;
		return ret;
	}
	__forceinline void push_front(T data);
	__forceinline void push_back(T data);
	__forceinline T pop_front();
	__forceinline T pop_back();

private:
	__forceinline void free_push_front(Node* data);
	__forceinline Node* free_pop_front()
	{
		//if (_freeSize == 0)
		//	return nullptr;

		if (&_freeHead == _freeTail._Prev)
			return nullptr;

		Node* node = _freeHead._Next;

		node->_Next->_Prev = &_freeHead;
		_freeHead._Next = node->_Next;
#ifdef MY_DEBUG
		_freeSize--;
#endif
		return node;
	}

public:
	__forceinline void allocFreeNode(int size)
	{
		for (int i = 0; i < size; ++i)
		{
			free_push_front(new Node);
		}
	}

	__forceinline void clear();
	__forceinline int size() { return _size; }
#ifdef MY_DEBUG
	__forceinline int freesize() { return _freeSize; }
#endif
	__forceinline bool empty() { return _size == 0; }

	__forceinline iterator erase(iterator& iter)
	{
		iterator ret;
		ret._node = iter._node->_Next;

		iter._node->_Prev->_Next = iter._node->_Next;
		iter._node->_Next->_Prev = iter._node->_Prev;

		--_size;

		free_push_front(iter._node);
		return ret;
	}
	//- 이터레이터의 그 노드를 지움.
	//- 그리고 지운 노드의 다음 노드를 카리키는 이터레이터 리턴

	__forceinline void remove(T Data);
	__forceinline void remove_one(T Data); // 첫번째로 만나는 같은 데이터를 삭제합니다.

	// 특정 index의 Data를 가지고 옵니다.
	__forceinline bool at(T* outData, int idx);

	// quick sort
	void quick_sort(iterator beginIter, iterator endIter, bool (*cmpFunc)(T leftVal, T rightVal) = nullptr)
	{
		if (beginIter._node == endIter._node || beginIter._node->_Next == endIter._node)
			return;

		Node* pivot = beginIter._node;
		Node* left = pivot->_Next;
		Node* right = endIter._node->_Prev;

		// compare 함수가 없는 경우
		if (cmpFunc == nullptr) {

			while (true)
			{
				// left->_Prev != right 인 이유는 left하고 right가 교차하는 것을 원하기 때문. 
				// (left가 pivot보다 무조건 커야하고, right가 pivot보다 무조건 작아야하기 때문)
				// >= 로 해야만 된다. >는 무한 루프를 돈다. 
				// (pivot과 중복된 값을 넘어갈 수 없어서...)

				// left를 오른쪽으로...
				while (left->_data <= pivot->_data && left != right)
				{
					left = left->_Next;
				}

				// right를 왼쪽으로...
				while (right->_data >= pivot->_data && left->_Prev != right)
				{
					right = right->_Prev;
				}

				// 피벗이 교차되는 순간					
				if (left->_Prev == right || left == right)
				{
					T tmp = right->_data;
					right->_data = pivot->_data;
					pivot->_data = tmp;

					iterator retLeftEnd;
					retLeftEnd._node = left;
					quick_sort(beginIter, retLeftEnd, cmpFunc);

					iterator retRightBegin;
					retRightBegin._node = left;
					quick_sort(retRightBegin, endIter, cmpFunc);

					break;
				}

				T tmp = left->_data;
				left->_data = right->_data;
				right->_data = tmp;
			}
		}
		// compare 함수가 있는 경우
		else
		{
			while (true)
			{
				// left->_Prev != right 인 이유는 left하고 right가 교차하는 것을 원하기 때문. 
				// (left가 pivot보다 무조건 커야하고, right가 pivot보다 무조건 작아야하기 때문)
				// >= 로 해야만 된다. >는 무한 루프를 돈다. 
				// (pivot과 중복된 값을 넘어갈 수 없어서...)

				// left를 오른쪽으로...
				while (cmpFunc(pivot->_data, left->_data) && left != right)
				{
					left = left->_Next;
				}

				// right를 왼쪽으로...
				while (cmpFunc(right->_data, pivot->_data) && left->_Prev != right)
				{
					right = right->_Prev;
				}

				// 피벗이 교차되는 순간					
				if (left->_Prev == right || left == right)
				{
					T tmp = right->_data;
					right->_data = pivot->_data;
					pivot->_data = tmp;

					iterator retLeftEnd;
					retLeftEnd._node = left;
					quick_sort(beginIter, retLeftEnd, cmpFunc);

					iterator retRightBegin;
					retRightBegin._node = left;
					quick_sort(retRightBegin, endIter, cmpFunc);

					break;
				}

				T tmp = left->_data;
				left->_data = right->_data;
				right->_data = tmp;
			}
		}
	}

private:
	// write Area
	alignas(64) int _size = 0;

	Node _freeHead;
	Node _freeTail;
	Node _head;
	Node _tail;
	
#ifdef MY_DEBUG
	int _freeSize = 0;
#endif
};

template <typename T>
CList<T>::CList()
{
	_head._Next = &_tail;
	_tail._Prev = &_head;

	_freeHead._Next = &_freeTail;
	_freeTail._Prev = &_freeHead;

	_head._Prev = _tail._Next = 0;
	_freeHead._Prev = _freeTail._Next = 0;
}

template <typename T>
CList<T>::~CList()
{
	clear();
}

template <typename T>
void CList<T>::push_front(T data)
{
	Node* pNode = free_pop_front();
	if (pNode == nullptr)
		pNode = new Node();

	pNode->_data = data;
	pNode->_Prev = &_head;
	pNode->_Next = _head._Next;

	_head._Next->_Prev = pNode;
	_head._Next = pNode;

	++_size;
}

template <typename T>
void CList<T>::push_back(T data)
{
	Node* pNode = free_pop_front();
	if (pNode == nullptr)
		pNode = new Node();

	pNode->_data = data;
	pNode->_Prev = _tail._Prev;
	pNode->_Next = &_tail;

	_tail._Prev->_Next = pNode;
	_tail._Prev = pNode;

	++_size;
}

template <typename T>
T CList<T>::pop_front()
{
	//if (_size == 0)
	//	return 0;
	if (&_tail == _head._Next)
		return 0;

	Node* node = _head._Next;
	T ret = node->_data;

	node->_Next->_Prev = &_head;
	_head._Next = node->_Next;

	free_push_front(node);

	_size--;
	return ret;
}

template <typename T>
T CList<T>::pop_back()
{
	//if (_size == 0)
	//	return 0;
	if (&_tail == _head._Next)
		return 0;

	Node* node = _tail._Prev;
	T ret = node->_data;

	node->_Prev->_Next = &_tail;
	_tail._Prev = node->_Prev;

	free_push_front(node);

	_size--;
	return ret;
}

template <typename T>
void CList<T>::free_push_front(Node* pNode)
{
	pNode->_Next = _freeHead._Next;
	pNode->_Prev = &_freeHead;

	_freeHead._Next->_Prev = pNode;
	_freeHead._Next = pNode;
#ifdef MY_DEBUG
	++_freeSize;
#endif
}

template <typename T>
void CList<T>::clear()
{
	Node* tmp = _head._Next;
	while (tmp != &_tail)
	{
		Node* ttmp = tmp->_Next;
		free_push_front(ttmp);
		tmp = ttmp;
	}

	for(;;)
	{
		Node* delNode = free_pop_front();
		if (delNode == nullptr)
			break;

		delete delNode;
	}

	_head._Next = &_tail;
	_tail._Prev = &_head;
	_size = 0;
}

template <typename T>
void CList<T>::remove(T Data)
{
	Node* tmp = _head._Next;
	Node* ttmp;
	while (tmp != &_tail)
	{
		if (tmp->_data == Data)
		{
			ttmp = tmp->_Next;

			ttmp->_Prev = tmp->_Prev;
			ttmp->_Prev->_Next = ttmp;

			free_push_front(tmp);
			tmp = ttmp;

			--_size;
			//break;
			// remove는 원래 중복된 모든 값을 제거한다.
		}
		else
		{
			tmp = tmp->_Next;
		}
	}
}

template <typename T>
void CList<T>::remove_one(T Data)
{
	Node* tmp = _head._Next;
	Node* ttmp;
	while (tmp != &_tail)
	{
		if (tmp->_data == Data)
		{
			ttmp = tmp->_Next;

			ttmp->_Prev = tmp->_Prev;
			ttmp->_Prev->_Next = ttmp;

			free_push_front(tmp);
			tmp = ttmp;

			--_size;
			break;
			// remove는 원래 중복된 모든 값을 제거한다.
			// remove_one은 첫번째로 만나는 같은 값을 제거 한다.
		}
		else
		{
			tmp = tmp->_Next;
		}
	}
}

template <typename T>
bool CList<T>::at(T* outData, int idx)
{
	Node* tmp = _head._Next;
	Node* pTail = &_tail;
	for (int i = 0; tmp != pTail; ++i)
	{
		if (i == idx)
		{
			*outData = tmp->_data;
			return true;
		}
		tmp = tmp->_Next;
	}
	return false;
}