#pragma once

// 16 size 
struct ObjectPool_int128 {
	long long LowPart;
	long long HighPart;

	bool operator!=(ObjectPool_int128& rval) {
		if (LowPart != rval.LowPart || HighPart != rval.HighPart)
			return true;

		return false;
	}

	bool operator==(ObjectPool_int128& rval) {
		if (LowPart == rval.LowPart && HighPart == rval.HighPart)
			return true;

		return false;
	}
};

class CPoolInterface
{
public:
	CPoolInterface() = default;
	virtual ~CPoolInterface() {}
};

template <typename T>
class CObjectPool : public CPoolInterface
{
	friend class CObjectPoolMgr;

	__declspec(align(16)) struct stNode
	{
		ObjectPool_int128 hashBlock1;
		T data;
		ObjectPool_int128 hashBlock2;
		stNode* pNext;
		stNode* pDelNext;
		// 안전 변수 - 제거한 포인터를 다시 제거하려는 경우.
		bool	bDealloced;
	};

public:
	CObjectPool() 
	{

	}

	virtual ~CObjectPool()
	{
		// 메모리 누수 방지 리스트 순회

		while (m_pDelHead != nullptr)
		{
			stNode* erase = m_pDelHead;
			m_pDelHead = m_pDelHead->pDelNext;
			free(erase);
		}

		// virtual alloc 제거
		if (m_pPoolPtr != nullptr)
		{
			VirtualFree(m_pPoolPtr, 0, MEM_RELEASE);
		}
	}

	T* Alloc()
	{
		stNode* setValue = nullptr;

		if (m_pHead == nullptr)
		{
			setValue = (stNode*)malloc(sizeof(stNode));
			if (m_bPlacementNew == false)
			{
				new (setValue) stNode;
			}

			// 다음 노드
			setValue->pNext = nullptr;

			// 언더플로우, 오버플로우 방지
			setValue->hashBlock1.LowPart = setValue->hashBlock2.LowPart = m_HashCode.LowPart;
			setValue->hashBlock1.HighPart = setValue->hashBlock2.HighPart = m_HashCode.HighPart;
			
			// 누수 방지 리스트
			setValue->pDelNext = m_pDelHead;
			m_pDelHead = setValue;
			
			// 할당 카운트
			++m_allocCnt;
		}
		else
		{
			setValue = m_pHead;
			m_pHead = m_pHead->pNext;
		}

		// 중복 삭제 방지
		setValue->bDealloced = false;

		T* ret = (T*)((char*)setValue + sizeof(setValue->hashBlock1));
		if (m_bPlacementNew == true)
		{
			new (ret) T;
		}

		// 사용 카운트
		++m_usedCnt;
		return ret;
	}

	// Free
	void DeAlloc(T* pDealloc)
	{
		if (pDealloc == nullptr)
		{
			// 중복 반환 시
			int* _crash = nullptr;
			*_crash = 1;
			return;
		}

		// stNode로 변환
		stNode* pNode = (stNode*)((char*)pDealloc - sizeof(ObjectPool_int128));
		if (pNode->bDealloced == true)
		{
			// 중복 반환 시
			int* _crash = nullptr;
			*_crash = 1;
			return;
		}

		// 소멸자 호출
		if (m_bPlacementNew == true)
		{
			pDealloc->~T();
		}

		// hash Block 확인
		if (pNode->hashBlock1 != m_HashCode || pNode->hashBlock2 != m_HashCode)
		{
			int* _crash = nullptr;
			*_crash = 1;
			return;
		}
		
		// 노드 연결
		stNode* _next = m_pHead;
		m_pHead = pNode;
		pNode->pNext = _next;

		// 반환 완료 상태
		pNode->bDealloced = true;

		// 사용 카운트
		--m_usedCnt;
	}

	bool SetPoolSize(int poolSize, bool _bVal = true)
	{
		// 이미 할당이 된 경우에는 pool을 변경할 수 없게 변경
		if (m_pPoolPtr != nullptr || m_allocCnt != 0 || m_usedCnt != 0)
			return false;

		m_pPoolPtr = (char*)VirtualAlloc(NULL, sizeof(stNode) * poolSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		if (m_pPoolPtr == nullptr)
			return false;

		if (VirtualLock(m_pPoolPtr, sizeof(stNode) * poolSize) == false)
		{
			VirtualFree(m_pPoolPtr, 0, MEM_RELEASE);
			return false;
		}

		m_bPlacementNew = _bVal;
		m_allocCnt = poolSize;		// 카운트로 쳐주자.
		m_usedCnt = 0;
		m_iPoolSize = sizeof(stNode) * poolSize;
		stNode* pInitNext = m_pHead = (stNode*)m_pPoolPtr;
		
		for (int i = 0; i < poolSize; ++i)
		{
			if (m_bPlacementNew == false) 
			{
				new (pInitNext) stNode;
			}
			// 언더플로우, 오버플로우 방지
			pInitNext->hashBlock1.LowPart = pInitNext->hashBlock2.LowPart = m_HashCode.LowPart;
			pInitNext->hashBlock1.HighPart = pInitNext->hashBlock2.HighPart = m_HashCode.HighPart;

			// 삭제시 삭제노드에 포함되지 않음.
			pInitNext->pDelNext = nullptr;
			
			// 반환 확인용 변수 초기화 - 반환되어 있는 상태
			pInitNext->bDealloced = true;

			// 다음 노드 설정, next 설정
			if (i != poolSize - 1) {
				pInitNext = pInitNext->pNext = pInitNext + 1;
			}
			else
			{
				pInitNext->pNext = nullptr;
			}
		}

		return true;
	}

private:

	stNode*				m_pHead = nullptr;
	stNode*				m_pDelHead = nullptr;
	int					m_allocCnt = 0;
	int					m_usedCnt = 0;

	// 해시 코드 - 잘못 사용하는 사람을 대비
	ObjectPool_int128			m_HashCode;

	// placement new 변수
	bool				m_bPlacementNew = true;

	// 풀링 관련 변수
	char*				m_pPoolPtr = nullptr;
	int					m_iPoolSize = 0;
};



class CObjectPoolMgr
{
private:
	CObjectPoolMgr() = default;
	~CObjectPoolMgr() = default;
	DECLARE_SINGLETON_IN_HEADER(CObjectPoolMgr)

public:
	bool Init();
	void Release();

	template<typename T>
	T* Alloc()
	{
		CObjectPool<T>* objPool = (CObjectPool<T>*)m_objectPool_map[typeid(T).hash_code()];
		if (objPool == nullptr)
		{
			objPool = new CObjectPool<T>();
			objPool->m_HashCode = m_HashCnt;
			m_HashCnt.HighPart++;
			m_objectPool_map[typeid(T).hash_code()] = objPool;
		}
		return objPool->Alloc();
	}

	template<typename T>
	void DeAlloc(T* pNode)
	{
		CObjectPool<T>* objPool = (CObjectPool<T>*)m_objectPool_map[typeid(T).hash_code()];
		if (objPool == nullptr)
		{
			int* _crash = nullptr;
			*_crash = 1;
			return;
		}

		objPool->DeAlloc(pNode);
	}

	template<typename T>
	bool InitObjectPool(int poolSize, bool _bPlacemenNew = true)
	{
		CObjectPool<T>* objPool = (CObjectPool<T>*)m_objectPool_map[typeid(T).hash_code()];
		if (objPool != nullptr)
		{
			// 이미 할당이 되었다면 초기화는 불가능하다.
			return false;
		}

		objPool = new CObjectPool<T>();

		objPool->m_HashCode = m_HashCnt;
		m_HashCnt.HighPart++;
		if (objPool->SetPoolSize(poolSize, _bPlacemenNew) == false)
		{
			delete objPool;
			return false;
		}
		m_objectPool_map[typeid(T).hash_code()] = objPool;

		return true;
	}

private:
	ObjectPool_int128						m_HashCnt;
	unordered_map<size_t, CPoolInterface*>	m_objectPool_map;
};