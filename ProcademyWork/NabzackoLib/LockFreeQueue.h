#pragma once

#include <Windows.h>
#include "LockFreePool.h"

template <typename T>
class CLockFreeQueue
{
	struct st_Node
	{
		T val;
		st_Node* pNext = nullptr;
	};

	struct st_Top
	{
		st_Node* node;
		long long nodeID;
	};

public:
	CLockFreeQueue(int size)
	{
		m_maxSize = size;
		m_queueCount = 0;

		m_pPool = new CLockFreePool<st_Node>(m_maxSize + 1, false, 8);
		m_pHead = (st_Top*)_aligned_malloc(sizeof(st_Top), 16);
		m_pTail = (st_Top*)_aligned_malloc(sizeof(st_Top), 16);
		
		// dummy Node ¸¸µé±â
		m_pHead->node = m_pTail->node = m_pPool->Alloc();
		m_pHead->node->pNext = nullptr;
		m_pHead->nodeID = 1;
		m_pTail->nodeID = 1;
	}

	~CLockFreeQueue()
	{
		_aligned_free(m_pHead);
		_aligned_free(m_pTail);

		delete m_pPool;
	}

	__forceinline bool Enqueue(T val)
	{
		st_Node* inputNode = m_pPool->Alloc();
		if (inputNode == nullptr)
			return false;

		inputNode->val = val;
		inputNode->pNext = nullptr;
		st_Top tTop;
		while (true)
		{
			tTop.nodeID = m_pTail->nodeID;
			tTop.node = m_pTail->node;
			st_Node* pPrevNext = tTop.node->pNext;
			if (tTop.nodeID != m_pTail->nodeID)
				continue;

			if (pPrevNext == nullptr)
			{
				if (0 == InterlockedCompareExchange64((long long*)&tTop.node->pNext, (long long)inputNode, 0))
				{
					InterlockedCompareExchange128((long long*)m_pTail,
						tTop.nodeID + 1, (long long)inputNode,
						(long long*)&tTop);
					InterlockedIncrement(&m_queueCount);
					break;
				}
			}
			else
			{
				InterlockedCompareExchange128((long long*)m_pTail,
					tTop.nodeID + 1, (long long)pPrevNext,
					(long long*)&tTop);
			}
			YieldProcessor();
		}
		return true;
	}

	__forceinline bool Dequeue(T* outVal)
	{
		if (InterlockedDecrement(&m_queueCount) < 0)
		{
			InterlockedIncrement(&m_queueCount);
			return false;
		}
		st_Top hTop;
		st_Top tTop;
		st_Node* pHeadNext;
		st_Node* pTailNext;
		while (true)
		{
			hTop.nodeID = m_pHead->nodeID;
			hTop.node = m_pHead->node;
			pHeadNext = hTop.node->pNext;
			tTop.nodeID = m_pTail->nodeID;
			tTop.node = m_pTail->node;
			pTailNext = tTop.node->pNext;
			if (tTop.nodeID != m_pTail->nodeID)
				continue;

			if (pTailNext != nullptr)
			{
				InterlockedCompareExchange128((long long*)m_pTail, tTop.nodeID + 1, (long long)pTailNext, (long long*)&tTop);
			}
			else
			{
				if (hTop.nodeID != m_pHead->nodeID)
					continue;

				if (pHeadNext != nullptr)
				{
					*outVal = pHeadNext->val;
					if (1 == InterlockedCompareExchange128((long long*)m_pHead, hTop.nodeID + 1, (long long)pHeadNext, (long long*)&hTop))
					{
						m_pPool->Free(hTop.node);
						break;
					}
				}
			}
			YieldProcessor();
		}
		return true;
	}

	__forceinline int GetSize()
	{
		return m_queueCount;
	}

private:
	__declspec(align(64)) long m_queueCount = 0;
	
	st_Top* m_pHead = nullptr;
	st_Top* m_pTail = nullptr;
	CLockFreePool<st_Node>* m_pPool;
	int m_maxSize = 0;
};