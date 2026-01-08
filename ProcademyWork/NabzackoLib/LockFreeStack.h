#pragma once

#include <atomic>
#include "LockFreePool.h"

template <typename T>
class CLockFreeStack
{
	struct st_Node
	{
		T data;
		st_Node* pNext;
	};

	struct st_Top
	{
		st_Node* node;
		long long nodeID;
	};

public:
	CLockFreeStack(int size)
	{
		m_MaxSize = size;
		m_pPool = new CLockFreePool<st_Node>(m_MaxSize);

		m_TopNode = (st_Top*)_aligned_malloc(sizeof(st_Top), 16);
		m_TopNode->nodeID = 1;
		m_TopNode->node = nullptr;
	}

	~CLockFreeStack()
	{
		_aligned_free(m_TopNode);
		delete m_pPool;
	}

	__forceinline bool Push(T data)
	{
		st_Node* inputNode = m_pPool->Alloc();
		if (inputNode == nullptr)
			return false;

		inputNode->data = data;
		inputNode->pNext = nullptr;
		st_Top top;
		while (true)
		{
			top.node = m_TopNode->node;
			top.nodeID = m_TopNode->nodeID;
			inputNode->pNext = top.node;
			if (top.nodeID == m_TopNode->nodeID)
			{
				if (1 == InterlockedCompareExchange128((long long*)m_TopNode, top.nodeID + 1, (long long)inputNode, (long long*)&top))
				{
					InterlockedIncrement(&m_stackCount);
					break;
				}
			}
			YieldProcessor();
		}

		return true;
	}

	__forceinline bool Pop(T* outData)
	{
		if (InterlockedDecrement(&m_stackCount) < 0)
		{
			InterlockedIncrement(&m_stackCount);
			return false;
		}

		st_Top top;
		while (true)
		{
			top.node = m_TopNode->node;
			top.nodeID = m_TopNode->nodeID;
			if (top.node == nullptr) {
				YieldProcessor(); continue;
			}
			st_Node* pNext = top.node->pNext;
			*outData = top.node->data;
			if (top.nodeID == m_TopNode->nodeID)
			{
				if (1 == InterlockedCompareExchange128((long long*)m_TopNode, top.nodeID + 1, (long long)pNext, (long long*)&top))
				{
					m_pPool->Free(top.node);
					break;
				}
			}
			YieldProcessor();
		}
		return true;
	}

	__forceinline bool isEmpty()
	{
		return m_TopNode->node == nullptr;
	}

	__forceinline int GetSize()
	{
		return m_stackCount;
	}

private:
	__declspec(align(64)) long			m_stackCount = 0;
	st_Top*								m_TopNode;
	CLockFreePool<st_Node>*				m_pPool;
	long								m_MaxSize = 0;
};
