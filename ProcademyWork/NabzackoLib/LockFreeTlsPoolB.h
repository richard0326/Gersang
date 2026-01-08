#pragma once

#include "LockFreePool.h"

template <typename T>
class CEmptyFullPool
{
	struct st_Node
	{
		T data;
		int iGuard;
		st_Node* pNext;
		bool	bDealloced;
	};

	struct st_Top
	{
		st_Node* node;
		long long nodeID;
	};

public:
	CEmptyFullPool(int poolSize, bool placementNew = false, bool placementOnce = false, int aligned = 0, bool pageLock = false)
	{
		m_pFullHead = (st_Top*)_aligned_malloc(sizeof(st_Top), 16);
		m_pFullHead->node = nullptr;
		m_pFullHead->nodeID = 1;

		m_pEmptyHead = (st_Top*)_aligned_malloc(sizeof(st_Top), 16);
		m_pEmptyHead->node = nullptr;
		m_pEmptyHead->nodeID = 0xffffffff00000000;

		m_bPlacementNew = placementNew;
		m_bPlacementOnce = placementOnce;
		m_aligned = aligned;

		static int st_iGuard = 0xffff0000;
		m_iGuard = InterlockedIncrement((LONG*)&st_iGuard);

		if (SetPoolSize(poolSize, pageLock) == false)
		{
			int* crash = nullptr;
			*crash = 0;
		}
	}

	~CEmptyFullPool()
	{
		if (m_bPlacementOnce)
		{
			int padding = 0;
			if (m_aligned > 0)
				padding = m_aligned - (sizeof(st_Node) % m_aligned);
			int paddingNodeSize = sizeof(st_Node) + padding;

			T* pInitNext = (T*)m_pPoolPtr;

			for (int i = 0; i < m_maxPoolSize; ++i)
			{
				pInitNext->~T();

				pInitNext = (T*)((char*)pInitNext + paddingNodeSize);
			}
		}

		_aligned_free(m_pFullHead);
		_aligned_free(m_pEmptyHead);

		VirtualFree(m_pPoolPtr, 0, MEM_RELEASE);
	}

	T* FullNodeAlloc()
	{
		st_Top top;
		st_Node* outputNode = nullptr;

		while (1)
		{
			top.node = m_pFullHead->node;
			top.nodeID = m_pFullHead->nodeID;

			if (top.node == nullptr)
				return nullptr;

			if (top.nodeID == m_pFullHead->nodeID)
			{
				if (1 == InterlockedCompareExchange128((long long*)m_pFullHead,
					top.nodeID + 1, (long long)top.node->pNext,
					(long long*)&top))
				{
					outputNode = top.node;
					outputNode->bDealloced = false;
					outputNode->pNext = nullptr;

					if (m_bPlacementNew)
						new (outputNode) T;

					InterlockedIncrement(&m_allocFullCnt);
					break;
				}
			}
			YieldProcessor();
		}

		return (T*)outputNode;
	}

	bool FullNodeFree(T* pDealloc)
	{
		if (pDealloc == nullptr)
		{
			// nullptr 반환시
			int* _crash = nullptr;
			*_crash = 0;
			return false;
		}

		// stNode로 변환
		st_Node* pNode = (st_Node*)pDealloc;

		if (pNode->iGuard != m_iGuard)
		{
			// 가드 침범시...
			int* _crash = nullptr;
			*_crash = 1;
			return false;
		}

		if (pNode->bDealloced == true)
		{
			// 중복 반환 시
			int* _crash = nullptr;
			*_crash = 1;
			return false;
		}

		st_Top top;

		while (true)
		{
			top.node = m_pFullHead->node;
			top.nodeID = m_pFullHead->nodeID;

			pNode->pNext = top.node;
			pNode->bDealloced = true;

			if (top.nodeID == m_pFullHead->nodeID)
			{
				if (1 == InterlockedCompareExchange128((long long*)m_pFullHead,
					top.nodeID + 1, (long long)pNode,
					(long long*)&top))
				{
					if (m_bPlacementNew)
						pDealloc->~T();

					InterlockedDecrement(&m_allocFullCnt);
					break;
				}
			}
			YieldProcessor();
		}

		return true;
	}

	T* EmptyNodeAlloc()
	{
		st_Top top;
		st_Node* outputNode = nullptr;

		while (1)
		{
			top.node = m_pEmptyHead->node;
			top.nodeID = m_pEmptyHead->nodeID;

			if (top.node == nullptr)
				return nullptr;

			if (top.nodeID == m_pEmptyHead->nodeID)
			{
				if (1 == InterlockedCompareExchange128((long long*)m_pEmptyHead,
					top.nodeID + 1, (long long)top.node->pNext,
					(long long*)&top))
				{
					outputNode = top.node;
					outputNode->bDealloced = false;
					outputNode->pNext = nullptr;

					if (m_bPlacementNew)
						new (outputNode) T;

					InterlockedIncrement(&m_allocEmptyCnt);
					break;
				}
			}
			YieldProcessor();
		}

		return (T*)outputNode;
	}

	bool EmptyNodeFree(T* pDealloc)
	{
		if (pDealloc == nullptr)
		{
			// nullptr 반환시
			int* _crash = nullptr;
			*_crash = 0;
			return false;
		}

		// stNode로 변환
		st_Node* pNode = (st_Node*)pDealloc;

		if (pNode->iGuard != m_iGuard)
		{
			// 가드 침범시...
			int* _crash = nullptr;
			*_crash = 1;
			return false;
		}

		if (pNode->bDealloced == true)
		{
			// 중복 반환 시
			int* _crash = nullptr;
			*_crash = 1;
			return false;
		}

		st_Top top;

		while (true)
		{
			top.node = m_pEmptyHead->node;
			top.nodeID = m_pEmptyHead->nodeID;

			pNode->pNext = top.node;
			pNode->bDealloced = true;

			if (top.nodeID == m_pEmptyHead->nodeID)
			{
				if (1 == InterlockedCompareExchange128((long long*)m_pEmptyHead,
					top.nodeID + 1, (long long)pNode,
					(long long*)&top))
				{
					if (m_bPlacementNew)
						pDealloc->~T();

					InterlockedDecrement(&m_allocEmptyCnt);
					break;
				}
			}
			YieldProcessor();
		}

		return true;
	}

	int GetAllocSize()
	{
		return m_allocFullCnt;
	}

	int GetCapacitySize()
	{
		return m_maxPoolSize;
	}

private:
	bool SetPoolSize(int poolSize, bool pageLock)
	{
		// 이미 할당이 된 경우에는 pool을 변경할 수 없게 변경
		if (m_pPoolPtr != nullptr)
		{
			return false;
		}

		int paddingNodeSize = sizeof(st_Node);
		if (m_aligned != 0)
		{
			// 줄 맞춰서 생성하기
			if (m_aligned % 2 == 0)
			{
				int padding = m_aligned - (sizeof(st_Node) % m_aligned);
				paddingNodeSize = sizeof(st_Node) + padding;
			}
			else
				m_aligned = 0;
		}

		m_pPoolPtr = (char*)VirtualAlloc(NULL, paddingNodeSize * poolSize * 2, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		if (m_pPoolPtr == nullptr)
			return false;

		if (pageLock)
		{
			// page lock - 비효율적인 것 같아서 주석으로 처리함.
			if (VirtualLock(m_pPoolPtr, paddingNodeSize * poolSize * 2) == 0)
			{
				printf("Virtual Lock Error : %d\n", GetLastError());
				VirtualFree(m_pPoolPtr, 0, MEM_RELEASE);
				return false;
			}
		}

		st_Node* pInitNext = (st_Node*)m_pPoolPtr;
		m_pFullHead->node = pInitNext;
		m_allocFullCnt = 0;
		m_maxPoolSize = poolSize * 2;

		for (int i = 0; i < poolSize; ++i)
		{
			// 반환 확인용 변수 초기화 - 반환되어 있는 상태
			pInitNext->bDealloced = true;
			pInitNext->iGuard = m_iGuard;

			if (m_bPlacementOnce)
				new (pInitNext) T;

			// 다음 노드 설정, next 설정
			if (i != poolSize - 1) {
				pInitNext = pInitNext->pNext = (st_Node*)((char*)pInitNext + paddingNodeSize); // 포인터 연산
			}
			else
			{
				pInitNext->pNext = nullptr;
			}
		}

		pInitNext = (st_Node*)((char*)m_pPoolPtr + paddingNodeSize * poolSize);
		m_pEmptyHead->node = pInitNext;
		m_allocEmptyCnt = 0;

		for (int i = 0; i < poolSize; ++i)
		{
			pInitNext->bDealloced = true;
			pInitNext->iGuard = m_iGuard;

			if (m_bPlacementOnce)
				new (pInitNext) T;

			// 다음 노드 설정, next 설정
			if (i != poolSize - 1) {
				pInitNext = pInitNext->pNext = (st_Node*)((char*)pInitNext + paddingNodeSize); // 포인터 연산
			}
			else
			{
				pInitNext->pNext = nullptr;
			}
		}

		return true;
	}

private:
	__declspec(align(64)) long		m_allocFullCnt = 0;
	__declspec(align(64)) long		m_allocEmptyCnt = 0;

	st_Top* m_pFullHead = nullptr;
	st_Top* m_pEmptyHead = nullptr;
	char* m_pPoolPtr = nullptr;
	long			m_maxPoolSize = 0;
	int				m_aligned;
	int				m_iGuard;
	bool			m_bPlacementNew;
	bool			m_bPlacementOnce;
};

template <typename T>
class CLockFreeTlsPoolB
{
	struct st_ChunkNode
	{
		T data;
		int iGuard;
		alignas(64) st_ChunkNode* pNext;
	};

	struct st_Chunk
	{
		bool dummyAttr;
		alignas(64) st_ChunkNode* pLastNode;
		alignas(64) st_ChunkNode* pChunkNode;
		alignas(64) int idx;
	};

public:
	CLockFreeTlsPoolB(int poolSize, int chunkSize, bool placementNew = false, bool placementOnce = false, int aligned = 0, bool pageLock = false)
	{
		m_iTlsIndex = TlsAlloc();
		if (m_iTlsIndex == TLS_OUT_OF_INDEXES)
		{
			int* crash = nullptr;
			*crash = 0;
		}

		m_iChunkSize = chunkSize;
		m_iPoolSize = poolSize;
		m_bPlacementNew = placementNew;
		m_bPlacementOnce = placementOnce;

		m_pChunkNodePool = new CLockFreePool<st_ChunkNode>(m_iPoolSize * m_iChunkSize, false, false, aligned, pageLock);
		m_pChunkPool = new CEmptyFullPool<st_Chunk>(m_iPoolSize, false, false, 0, pageLock);

		st_ChunkNode** pChunkNodeArr = new st_ChunkNode * [m_iPoolSize * m_iChunkSize];
		st_Chunk** pChunkArr = new st_Chunk * [m_iPoolSize];

		for (int i = 0; i < m_iPoolSize; ++i)
		{
			pChunkArr[i] = m_pChunkPool->FullNodeAlloc();
			pChunkArr[i]->idx = m_iChunkSize;
			pChunkNodeArr[i * m_iChunkSize] = pChunkArr[i]->pLastNode = pChunkArr[i]->pChunkNode = m_pChunkNodePool->Alloc();

			if (m_bPlacementOnce)
				new (&pChunkNodeArr[i * m_iChunkSize]) T;

			st_ChunkNode** ppNext = &pChunkArr[i]->pChunkNode->pNext;
			for (int j = 0; j < m_iChunkSize - 1; ++j)
			{
				pChunkNodeArr[i * m_iChunkSize + j + 1] = pChunkArr[i]->pLastNode = *ppNext = m_pChunkNodePool->Alloc();
				if (m_bPlacementOnce)
					new (&pChunkNodeArr[i * m_iChunkSize + j + 1]) T;

				ppNext = &(*ppNext)->pNext;
			}
			*ppNext = nullptr;
		}

		for (int i = 0; i < m_iPoolSize; ++i)
		{
			m_pChunkPool->FullNodeFree(pChunkArr[i]);

			for (int j = 0; j < m_iChunkSize; ++j)
				m_pChunkNodePool->Free(pChunkNodeArr[i * m_iChunkSize + j]);
		}

		delete [] pChunkNodeArr;
		delete [] pChunkArr;
	}

	~CLockFreeTlsPoolB()
	{
		if (m_bPlacementOnce)
		{
			//
		}

		TlsFree(m_iTlsIndex);
	}

	T* Alloc()
	{
		st_Chunk* pChunk = (st_Chunk*)TlsGetValue(m_iTlsIndex);
		if (pChunk == nullptr)
		{
			pChunk = m_pChunkPool->FullNodeAlloc();
			if (pChunk == nullptr)
				return nullptr;

			pChunk->idx = m_iChunkSize;
			TlsSetValue(m_iTlsIndex, pChunk);
		}

		--pChunk->idx;
		T* pRet = (T*)pChunk->pChunkNode;
		pChunk->pChunkNode = pChunk->pChunkNode->pNext;

		if (pChunk->idx == 0)
		{
			m_pChunkPool->EmptyNodeFree(pChunk);
			pChunk = m_pChunkPool->FullNodeAlloc();
			if (pChunk != nullptr)
			{
				pChunk->idx = m_iChunkSize;
			}
			TlsSetValue(m_iTlsIndex, pChunk);			
		}

		return pRet;
	}

	void Free(T* pReturn)
	{
		if (pReturn == nullptr)
			return;

		st_Chunk* pChunk = (st_Chunk*)TlsGetValue(m_iTlsIndex);
		if (pChunk == nullptr)
		{
			pChunk = m_pChunkPool->FullNodeAlloc();
			if (pChunk == nullptr)
			{
				pChunk = m_pChunkPool->EmptyNodeAlloc();
				if (pChunk == nullptr)
				{
					int* crash = nullptr;
					*crash = 0;
				}

				pChunk->idx = 0;
				pChunk->pLastNode = pChunk->pChunkNode = (st_ChunkNode*)pReturn;
			}
			else
			{
				pChunk->idx = m_iChunkSize;
			}

			TlsSetValue(m_iTlsIndex, pChunk);
		}

		++pChunk->idx;
		pChunk->pLastNode = pChunk->pLastNode->pNext = (st_ChunkNode*)pReturn;
		
		if (pChunk->idx == m_iChunkSize * 2)
		{
			st_Chunk* pEmptyChunk = m_pChunkPool->EmptyNodeAlloc();
			st_ChunkNode* pChunkStart = pChunk->pChunkNode;
			st_ChunkNode* pChunkNode = pChunkStart;
			for (int i = 0; i < m_iChunkSize; ++i)
			{
				pChunkNode = pChunkNode->pNext;
			}
			pEmptyChunk->pChunkNode = pChunkStart;
			pEmptyChunk->pLastNode = pChunkNode;
			pChunk->pChunkNode = pChunkNode->pNext;
			pChunkNode->pNext = nullptr;

			pEmptyChunk->idx = m_iChunkSize;
			pChunk->idx = m_iChunkSize;

			m_pChunkPool->FullNodeFree(pEmptyChunk);
		}
	}

	int GetSize()
	{
		return m_pChunkPool->GetAllocSize();
	}

	__forceinline inline int GetChunkSize()
	{
		return m_pChunkPool->GetAllocSize();;
	}

private:
	CLockFreePool<st_ChunkNode>* m_pChunkNodePool;
	CEmptyFullPool<st_Chunk>* m_pChunkPool;
	int m_iTlsIndex;
	int m_iPoolSize;
	int m_iChunkSize;
	bool m_bPlacementNew;
	bool m_bPlacementOnce;
};