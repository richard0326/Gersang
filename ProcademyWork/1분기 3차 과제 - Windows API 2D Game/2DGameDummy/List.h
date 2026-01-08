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
		friend void CList<T>::quick_sort(iterator beginIter, iterator endIter, bool (*cmpFunc)(T a, T b));
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

	// 특정 index의 Data를 가지고 옵니다.
	bool at(T* outData, int idx);

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

					iterator retLeftEnd(left);
					quick_sort(beginIter, retLeftEnd, cmpFunc);

					iterator retRightBegin(left);
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

					iterator retLeftEnd(left);
					quick_sort(beginIter, retLeftEnd, cmpFunc);

					iterator retRightBegin(left);
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

	++_size;
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

	++_size;
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

template <typename T>
bool CList<T>::at(T* outData, int idx)
{
	Node* tmp = _head._Next;
	for (int i = 0; tmp != &_tail; ++i)
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