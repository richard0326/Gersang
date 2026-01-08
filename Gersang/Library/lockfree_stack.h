#pragma once
#include <iostream>

class LockFreeStack {
	struct LFNode {
		LFNode(int v, LFNode* prev) {
			value = v;
			pPrev = prev;
		}
		int value;
		LFNode* pNext = nullptr;
		LFNode* pPrev = nullptr;
	};

public:    
	LockFreeStack() = default;
	~LockFreeStack() = default;

	void Push(int value) {
		while (true) {
			if (m_pTail == nullptr)
			{
				auto p = new LFNode(value, nullptr);
				if (_InterlockedCompareExchange64((long long*)&m_pTail, (long long)p, 0) == 0)
				{
					m_pHead = m_pTail;
					_InterlockedIncrement((long*)&m_count);
					break;
				}
			}
			else
			{
				LFNode* pNode = m_pTail;
				auto p = new LFNode(value, pNode);
				if (_InterlockedCompareExchange64((long long*)&m_pTail, (long long)p, (long long)pNode) == (long long)pNode)
				{
					pNode->pNext = p;
					_InterlockedIncrement((long*)&m_count);
					break;
				}
				delete p;
			}
		}
		
	}
	int Pop() {
		while (true)
		{
			if (m_pTail == nullptr)
			{
				return 0;
			}
			else
			{
				LFNode* pRet = m_pTail;
				if (_InterlockedCompareExchange64((long long*)&m_pTail, (long long)pRet->pPrev, (long long)pRet) == (long long)pRet)
				{
					if (m_pTail == nullptr) {
						m_pHead = nullptr;
					}
					int ret = pRet->value;
					_InterlockedDecrement((long*) & m_count);
					delete pRet;
					return ret;
				}

				return 0;
			}
		}
	}

	int Count() { return m_count; }

	void Print() {
		LFNode* pNode = m_pHead;
		for (int i = 0; i < 10000; i++) {
			if (pNode == nullptr) {
				break;
			}
			std::cout << pNode->value << std::endl;
			pNode = pNode->pNext;
		}
	}

	void Empty() {
		while (m_pHead != nullptr)
		{
			Pop();
		}
	}
private:
	LFNode* m_pHead = nullptr;
	LFNode* m_pTail = nullptr;
	int m_count = 0;
};