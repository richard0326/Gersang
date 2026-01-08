#pragma once

template <typename F, typename T>
class COrderedmap
{
	enum NODE_COLOR {
		RED,
		BLACK,

		LEFT,
		RIGHT,
	};

public:
	struct stNode
	{
		// _pNext가 stNode의 this의 위치에 있어야 ++iter가 성능이 4배 빠르게 나옴.
		// 그리고 stl::map의 iter보다 빠른 성능이 나올 수 있음.
		stNode* _pNext = nullptr;
		F _val;
		T _second;
		COrderedmap::NODE_COLOR _color;

		stNode* _parent = nullptr;
		stNode* _left = nullptr;
		stNode* _right = nullptr;
		stNode* _pPrev = nullptr;
	};

private:
	class iterator
	{
		template <typename F, typename T> friend class COrderedmap;
	private:
		stNode* _node;
	public:
		iterator() = default;

		__forceinline iterator operator ++(int)
		{
			//현재 노드를 다음 노드로 이동
			iterator ret;
			ret._node = _node;

			_node = _node->_pNext;
			return ret;
		}

		__forceinline iterator& operator++()
		{
			_node = _node->_pNext;
			return *this;
		}

		__forceinline F first()
		{
			return _node->_val;
		}

		__forceinline T second()
		{
			return _node->_second;
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
	COrderedmap();
	~COrderedmap();

	__forceinline iterator begin()
	{
		//첫번째 노드를 가리키는 이터레이터 리턴
		iterator ret;
		ret._node = m_headNode._pNext;

		return ret;
	}

	__forceinline iterator end()
	{
		//Tail 노드를 가리키는(데이터가 없는 진짜 더미 끝 노드) 이터레이터를 리턴
		//또는 끝으로 인지할 수 있는 이터레이터를 리턴
		iterator ret;
		ret._node = &m_tailNode;
		return ret;
	}

	__forceinline iterator erase(iterator& iter)
	{
		stNode* pNext = nullptr;
		removeNode(iter._node, &pNext);
		if (pNext == nullptr)
		{
			int* crash = nullptr;
			*crash = 0;
		}
		iterator ret;
		ret._node = pNext;
		return ret;
	}

	__forceinline iterator find(F key)
	{
		stNode* pNode = SearchNode(key);
		if (pNode == nullptr)
		{
			iterator ret;
			ret._node = &m_tailNode;
			return ret;
		}
		iterator ret;
		ret._node = pNode;
		return ret;
	}

	__forceinline bool insert(F first, T second);
public:
	__forceinline void erase(F key);
	__forceinline int size() { return m_size; };
#ifdef MY_DEBUG
	__forceinline int GetFreeSize() { return _freeSize; };
#endif
	__forceinline bool at(F key, T* pOut)
	{
		stNode* pRet = SearchNode(key);
		if (pRet == nullptr)
		{
			return false;
		}
		*pOut = pRet->_second;
		return true;
	}
private:
	//__forceinline bool AddNode(stNode* pNode, stNode* pParent, F first, T second, int right);
	__forceinline void removeNode(stNode* delNode, stNode** pOut = nullptr);
	//__forceinline void AddBalancing(stNode* pNode);
	__forceinline void TurnRight(stNode* pNode);
	__forceinline void TurnLeft(stNode* pNode);
	__forceinline stNode* SearchNode(F val)
	{
		stNode* pFindNode = m_pRoot;
		while (true)
		{
			if (pFindNode == nullptr || pFindNode->_val == val)
				break;
			else if (pFindNode->_val > val)
				pFindNode = pFindNode->_left;
			else
				pFindNode = pFindNode->_right;
		}

		return pFindNode;
	}

	__forceinline void ReleaseNode(stNode* _node);

private:
	__forceinline void free_push_front(stNode* pNode)
	{
		pNode->_right = _freeHead._right;
		pNode->_left = &_freeHead;

		_freeHead._right->_left = pNode;
		_freeHead._right = pNode;
#ifdef MY_DEBUG
		++_freeSize;
#endif
	}
	__forceinline stNode* free_pop_front()
	{
		//if (_freeSize == 0)
		//	return nullptr;
		if (&_freeHead == _freeTail._left)
			return nullptr;

		stNode* node = _freeHead._right;

		node->_right->_left = &_freeHead;
		_freeHead._right = node->_right;
#ifdef MY_DEBUG
		--_freeSize;
#endif
		return node;
	}

public:
	__forceinline void allocFreeNode(int size)
	{
		for (int i = 0; i < size; ++i)
		{
			free_push_front(new stNode);
		}
	}
private:
	stNode* m_pNil = nullptr;

	// 쓰기가 되는 애들
	alignas(64) int m_size = 0;
	stNode* m_pRoot = nullptr;
#ifdef MY_DEBUG
	int _freeSize = 0;
#endif
	stNode m_headNode;
	stNode m_tailNode;
	// 이터레이터 검색용...
	stNode _freeHead;
	stNode _freeTail;
};

template <typename F, typename T>
COrderedmap<F, T>::COrderedmap()
{
	m_pNil = new stNode;
	m_pNil->_val = -1;
	m_pNil->_color = BLACK;

	m_headNode._pNext = &m_tailNode;
	m_tailNode._pPrev = &m_headNode;

	_freeHead._right = &_freeTail;
	_freeTail._left = &_freeHead;
	_freeHead._left = _freeTail._right = 0;
}

template <typename F, typename T>
COrderedmap<F, T>::~COrderedmap()
{
	ReleaseNode(m_pRoot);

	for (;;)
	{
		stNode* delNode = free_pop_front();
		if (delNode == nullptr)
			break;

		delete delNode;
	}

	delete m_pNil;
}

template <typename F, typename T>
bool COrderedmap<F, T>::insert(F first, T second)
{
	if (m_pRoot == nullptr)
	{
		m_pRoot = free_pop_front();
		if (m_pRoot == nullptr)
			m_pRoot = new stNode();

		m_pRoot->_val = first;
		m_pRoot->_second = second;
		m_pRoot->_left = m_pRoot->_right = m_pNil;
		m_pRoot->_parent = nullptr;
		m_pRoot->_color = BLACK;

		// 이터레이터
		m_pRoot->_pPrev = &m_headNode;
		m_pRoot->_pNext = &m_tailNode;
		m_headNode._pNext = m_tailNode._pPrev = m_pRoot;
	}
	else
	{
		stNode* pNode = m_pRoot;
		stNode* pParent = nullptr;
		stNode* newNode = nullptr;
		int right = RIGHT;

		if (pNode->_val == first)
		{
			// 중복된 값이 이미 있는 경우 넣지 않습니다.
			//pNode->_second = second;
			return false;
		}
		else if (pNode->_val > first)
		{
			right = LEFT;
		}

		for (;;)
		{
			newNode = nullptr;
			if (pNode == m_pNil)
			{
				newNode = free_pop_front();
				if (newNode == nullptr)
					newNode = new stNode();

				newNode->_color = RED;
				newNode->_parent = pParent;
				newNode->_left = newNode->_right = m_pNil;
				newNode->_val = first;
				newNode->_second = second;

				if (right == RIGHT)
				{
					pParent->_right = newNode;

					// 이터레이터
					newNode->_pPrev = pParent;
					newNode->_pNext = pParent->_pNext;
					pParent->_pNext->_pPrev = newNode;
					pParent->_pNext = newNode;
				}
				else
				{
					pParent->_left = newNode;

					// 이터레이터
					newNode->_pNext = pParent;
					newNode->_pPrev = pParent->_pPrev;
					pParent->_pPrev->_pNext = newNode;
					pParent->_pPrev = newNode;
				}

				//AddBalancing(newNode);

				stNode* pBlancNode = newNode;
				stNode* pBlancParent;
				stNode* pBlancUncle;
				stNode* pBlancGrandParent;
				int iBlancRight;
				for (;;)
				{
					pBlancParent = pBlancNode->_parent;
					if (pBlancParent == nullptr)
						break;

					// 부모 블랙인 경우
					if (pBlancParent->_color == BLACK)
						break;

					pBlancGrandParent = pBlancParent->_parent;

					pBlancUncle = nullptr;
					if (pBlancParent->_val < pBlancGrandParent->_val)
					{
						pBlancUncle = pBlancGrandParent->_right;
						iBlancRight = RIGHT;
					}
					else
					{
						pBlancUncle = pBlancGrandParent->_left;
						iBlancRight = LEFT;
					}

					if (pBlancUncle->_color == RED)
					{
						pBlancParent->_color = BLACK;
						pBlancGrandParent->_color = RED;
						pBlancUncle->_color = BLACK;
						m_pRoot->_color = BLACK;

						pBlancNode = pBlancGrandParent;
						continue;
					}
					else
					{
						// Node가 왼쪽에 있는 경우 
						if (iBlancRight == LEFT)
						{
							if (pBlancParent->_val > pBlancNode->_val)
							{
								TurnRight(pBlancParent);

								TurnLeft(pBlancGrandParent);

								pBlancNode->_color = BLACK;
								pBlancGrandParent->_color = RED;
								m_pRoot->_color = BLACK;

								pBlancNode = pBlancParent;
								continue;
							}
							else
							{
								TurnLeft(pBlancGrandParent);

								pBlancParent->_color = BLACK;
								pBlancGrandParent->_color = RED;
								m_pRoot->_color = BLACK;

								//AddBalancing(pNode);
								continue;
							}
						}
						else if (iBlancRight == RIGHT)
						{
							if (pBlancParent->_val < pBlancNode->_val)
							{
								TurnLeft(pBlancParent);

								TurnRight(pBlancGrandParent);

								pBlancNode->_color = BLACK;
								pBlancGrandParent->_color = RED;
								m_pRoot->_color = BLACK;

								//AddBalancing(pParent);
								pBlancNode = pBlancParent;
								continue;
							}
							else
							{
								TurnRight(pBlancGrandParent);

								pBlancParent->_color = BLACK;
								pBlancGrandParent->_color = RED;
								m_pRoot->_color = BLACK;

								//AddBalancing(pNode);
								continue;
							}
						}
					}

					break;
				}
			}
			else
			{
				if (pNode->_val == first)
				{
					// 중복된 값이 이미 있는 경우 넣지 않습니다.
					//pNode->_second = second;
					return false;
				}
				else if (pNode->_val > first)
				{
					pParent = pNode;
					pNode = pNode->_left;
					right = LEFT;
				}
				else
				{
					pParent = pNode;
					pNode = pNode->_right;
					right = RIGHT;
				}
				continue;
			}

			break;
		}
	}
	m_size++;

	return true;
}

template <typename F, typename T>
void COrderedmap<F, T>::erase(F key)
{
	if (m_pRoot == nullptr)
		return;

	stNode* delNode = m_pRoot;

	// 1. 삭제할 노드를 찾는다.
	while (true)
	{
		if (delNode == m_pNil || delNode->_val == key)
		{
			break;
		}
		else if (delNode->_val > key)
		{
			delNode = delNode->_left;
		}
		else
		{
			delNode = delNode->_right;
		}
	}

	removeNode(delNode);
}

//template <typename F, typename T>
//bool COrderedmap<F, T>::AddNode(stNode* pNode, stNode* pParent, F first, T second, int right)
//{
//	stNode* newNode = nullptr;
//	if (pNode == m_pNil)
//	{
//		newNode = free_pop_front();
//		if (newNode == nullptr)
//			newNode = new stNode();
//
//		newNode->_color = RED;
//		newNode->_parent = pParent;
//		newNode->_left = newNode->_right = m_pNil;
//		newNode->_val = first;
//		newNode->_second = second;
//
//		if (right == RIGHT)
//		{
//			pParent->_right = newNode;
//
//			// 이터레이터
//			newNode->_pPrev = pParent;
//			newNode->_pNext = pParent->_pNext;
//			pParent->_pNext->_pPrev = newNode;
//			pParent->_pNext = newNode;
//		}
//		else
//		{
//			pParent->_left = newNode;
//
//			// 이터레이터
//			newNode->_pNext = pParent;
//			newNode->_pPrev = pParent->_pPrev;
//			pParent->_pPrev->_pNext = newNode;
//			pParent->_pPrev = newNode;
//		}
//
//		AddBalancing(newNode);
//	}
//	else
//	{
//		if (pNode->_val == first)
//		{
//			// 중복된 값이 이미 있는 경우 넣지 않습니다.
//			//pNode->_second = second;
//			return false;
//		}
//		else if (pNode->_val > first)
//		{
//			return AddNode(pNode->_left, pNode, first, second, LEFT);
//		}
//		else
//		{
//			return AddNode(pNode->_right, pNode, first, second, RIGHT);
//		}
//	}
//
//	return true;
//}

template <typename F, typename T>
void COrderedmap<F, T>::removeNode(stNode* delNode, stNode** pOut)
{
	stNode* preNode = nullptr;
	stNode* replaceNode = m_pNil;
	NODE_COLOR deleteColor = BLACK;
	int replaceRight = RIGHT;

	if (delNode == nullptr || delNode == m_pNil)
	{
		return;
	}

	preNode = delNode->_parent;
	--m_size;

	// 이터레이터 작업.
	stNode* pNextIter = delNode->_pNext;
	stNode* pPrevIter = delNode->_pPrev;

	if (pOut != nullptr)
		*pOut = pNextIter;

	// 3. 자식이 하나만 있거나 없는 경우...
	if (delNode->_left == m_pNil && delNode->_right == m_pNil)
	{
		if (preNode != nullptr)
		{
			if (preNode->_left == delNode)
			{
				preNode->_left = m_pNil;
				replaceRight = LEFT;
			}
			else
				preNode->_right = m_pNil;
		}

		m_pNil->_parent = preNode;
		deleteColor = delNode->_color;

		free_push_front(delNode);

		pNextIter->_pPrev = pPrevIter;
		pPrevIter->_pNext = pNextIter;

		// 이터레이터
		pNextIter->_pPrev = pPrevIter;
		pPrevIter->_pNext = pNextIter;
	}
	else if (delNode->_left == m_pNil)
	{
		if (preNode != nullptr)
		{
			if (preNode->_left == delNode)
			{
				preNode->_left = delNode->_right;
				replaceRight = LEFT;
			}
			else
				preNode->_right = delNode->_right;
		}

		replaceNode = delNode->_right;
		replaceNode->_parent = preNode;
		deleteColor = delNode->_color;
		free_push_front(delNode);

		pNextIter->_pPrev = pPrevIter;
		pPrevIter->_pNext = pNextIter;

		// 이터레이터
		pNextIter->_pPrev = pPrevIter;
		pPrevIter->_pNext = pNextIter;
	}
	else if (delNode->_right == m_pNil)
	{
		if (preNode != nullptr)
		{
			if (preNode->_left == delNode)
			{
				preNode->_left = delNode->_left;
				replaceRight = LEFT;
			}
			else
				preNode->_right = delNode->_left;
		}

		replaceNode = delNode->_left;
		replaceNode->_parent = preNode;
		deleteColor = delNode->_color;
		free_push_front(delNode);

		// 이터레이터
		pNextIter->_pPrev = pPrevIter;
		pPrevIter->_pNext = pNextIter;
	}
	else
	{
		// 4. 양쪽 노드 존재하는 경우...
		// 오른쪽 노드에서 왼쪽 제일 아래 노드와 바꾼다.
		stNode* exchangeNode = delNode->_right;
		stNode* preExchangeNode = delNode;
		while (true)
		{
			if (exchangeNode->_left == m_pNil)
			{
				if (preExchangeNode->_left == exchangeNode)
				{
					preExchangeNode->_left = exchangeNode->_right;
					replaceRight = LEFT;
				}
				else
					preExchangeNode->_right = exchangeNode->_right;

				delNode->_val = exchangeNode->_val;
				delNode->_second = exchangeNode->_second;
				replaceNode = exchangeNode->_right;
				replaceNode->_parent = preExchangeNode;
				deleteColor = exchangeNode->_color;
				free_push_front(exchangeNode);

				// 이터레이터
				pNextIter = exchangeNode->_pNext;
				pPrevIter = exchangeNode->_pPrev;

				pNextIter->_pPrev = pPrevIter;
				pPrevIter->_pNext = pNextIter;

				if (pOut != nullptr)
					*pOut = pPrevIter;

				break;
			}

			preExchangeNode = exchangeNode;
			exchangeNode = exchangeNode->_left;
		}
	}

	if (replaceNode->_parent == nullptr)
	{
		if (replaceNode == m_pNil)
		{
			m_pRoot = nullptr;
		}
		else
		{
			m_pRoot = replaceNode;
			m_pRoot->_color = BLACK;
		}
		return;
	}

	if (deleteColor == RED)
	{
		return;
	}

	while (true)
	{
		// 조건 1, 조건 2
		if (replaceNode->_color == RED || replaceNode == m_pRoot)
		{
			replaceNode->_color = BLACK;
			return;
		}

		stNode* pParent = replaceNode->_parent;
		if ((replaceNode == m_pNil && replaceRight == LEFT) || (replaceNode != m_pNil && replaceNode->_val < pParent->_val))
		{
			stNode* pSiblings = pParent->_right;

			if (pSiblings->_color == RED)
			{
				pParent->_color = RED;
				pSiblings->_color = BLACK;
				TurnLeft(pParent);
				continue;
			}

			// 형제의 색이 검정인 경우
			if (pSiblings->_left->_color == BLACK && pSiblings->_right->_color == BLACK)
			{
				pSiblings->_color = RED;
				replaceNode = pParent;
				continue;
			}

			if (pSiblings->_right->_color == RED)
			{
				pSiblings->_color = pParent->_color;
				pSiblings->_right->_color = BLACK;
				pParent->_color = BLACK;
				TurnLeft(pParent);
				return;
			}

			stNode* pSiblingLeft = pSiblings->_left;
			pSiblingLeft->_color = pParent->_color;
			pSiblings->_color = BLACK;
			pParent->_color = BLACK;
			TurnRight(pSiblings);
			TurnLeft(pParent);
			return;
		}
		else
		{
			stNode* pSiblings = pParent->_left;

			if (pSiblings->_color == RED)
			{
				pParent->_color = RED;
				pSiblings->_color = BLACK;
				TurnRight(pParent);
				continue;
			}

			// 형제의 색이 검정인 경우
			if (pSiblings->_left->_color == BLACK && pSiblings->_right->_color == BLACK)
			{
				pSiblings->_color = RED;
				replaceNode = pParent;
				continue;
			}

			if (pSiblings->_left->_color == RED)
			{
				pSiblings->_color = pParent->_color;
				pSiblings->_left->_color = BLACK;
				pParent->_color = BLACK;
				TurnRight(pParent);
				return;
			}

			stNode* pSiblingRight = pSiblings->_right;
			pSiblingRight->_color = pParent->_color;
			pSiblings->_color = BLACK;
			pParent->_color = BLACK;
			TurnLeft(pSiblings);
			TurnRight(pParent);
			return;
		}
	}
}

//template <typename F, typename T>
//void COrderedmap<F, T>::AddBalancing(stNode* pNode)
//{
//	stNode* pParent = pNode->_parent;
//	if (pParent == nullptr)
//		return;
//
//	stNode* pGrandParent = pParent->_parent;
//
//	// 부모 블랙인 경우
//	if (pParent->_color == BLACK)
//		return;
//
//	// 삼촌이 블랙인 경우
//	stNode* pUncle = nullptr;
//	int iRight = RIGHT;
//	if (pParent->_val < pGrandParent->_val)
//	{
//		pUncle = pGrandParent->_right;
//		iRight = RIGHT;
//	}
//	else
//	{
//		pUncle = pGrandParent->_left;
//		iRight = LEFT;
//	}
//
//	if (pUncle->_color == RED)
//	{
//		pParent->_color = BLACK;
//		pGrandParent->_color = RED;
//		pUncle->_color = BLACK;
//		m_pRoot->_color = BLACK;
//		AddBalancing(pGrandParent);
//	}
//	else
//	{
//		// Node가 왼쪽에 있는 경우 
//		if (iRight == LEFT)
//		{
//			if (pParent->_val > pNode->_val)
//			{
//				TurnRight(pParent);
//
//				TurnLeft(pGrandParent);
//
//				pNode->_color = BLACK;
//				pGrandParent->_color = RED;
//				m_pRoot->_color = BLACK;
//				AddBalancing(pParent);
//			}
//			else
//			{
//				TurnLeft(pGrandParent);
//
//				pParent->_color = BLACK;
//				pGrandParent->_color = RED;
//				m_pRoot->_color = BLACK;
//
//				AddBalancing(pNode);
//			}
//		}
//		else if (iRight == RIGHT)
//		{
//			if (pParent->_val < pNode->_val)
//			{
//				TurnLeft(pParent);
//
//				TurnRight(pGrandParent);
//
//				pNode->_color = BLACK;
//				pGrandParent->_color = RED;
//				m_pRoot->_color = BLACK;
//
//				AddBalancing(pParent);
//			}
//			else
//			{
//				TurnRight(pGrandParent);
//
//				pParent->_color = BLACK;
//				pGrandParent->_color = RED;
//				m_pRoot->_color = BLACK;
//
//				AddBalancing(pNode);
//			}
//		}
//	}
//
//}

template <typename F, typename T>
void COrderedmap<F, T>::TurnRight(stNode* pNode)
{
	stNode* pLeftNode = pNode->_left;
	stNode* pParentNode = pNode->_parent;
	stNode* pLeftSon = pLeftNode->_right;

	// 중심 노드의 변경점
	pNode->_parent = pLeftNode;
	pNode->_left = pLeftSon;

	// 왼쪽 노드의 변경점
	pLeftNode->_right = pNode;
	pLeftNode->_parent = pParentNode;

	// 왼쪽 자식 노드의 변경점
	if (pLeftSon != nullptr)
	{
		pLeftSon->_parent = pNode;
	}

	// 부모 노드의 변경점
	if (pParentNode != nullptr)
	{
		if (pParentNode->_val > pLeftNode->_val)
		{
			pParentNode->_left = pLeftNode;
		}
		else
		{
			pParentNode->_right = pLeftNode;
		}
	}
	else
	{
		m_pRoot = pLeftNode;
		m_pRoot->_color = BLACK;
	}
}

template <typename F, typename T>
void COrderedmap<F, T>::TurnLeft(stNode* pNode)
{
	stNode* pRightNode = pNode->_right;
	stNode* pParentNode = pNode->_parent;
	stNode* pRightSon = pRightNode->_left;

	// 중심 노드의 변경점
	pNode->_parent = pRightNode;
	pNode->_right = pRightSon;

	// 오른쪽 노드의 변경점
	pRightNode->_left = pNode;
	pRightNode->_parent = pParentNode;

	// 오른쪽 자식 노드의 변경점
	if (pRightSon != nullptr)
	{
		pRightSon->_parent = pNode;
	}

	// 부모 노드의 변경점
	if (pParentNode != nullptr)
	{
		if (pParentNode->_val > pRightNode->_val)
		{
			pParentNode->_left = pRightNode;
		}
		else
		{
			pParentNode->_right = pRightNode;
		}
	}
	else
	{
		m_pRoot = pRightNode;
	}
}

template <typename F, typename T>
void COrderedmap<F, T>::ReleaseNode(stNode* _node)
{
	if (_node == m_pNil || _node == nullptr)
		return;

	ReleaseNode(_node->_left);
	ReleaseNode(_node->_right);
	free_push_front(_node);
}