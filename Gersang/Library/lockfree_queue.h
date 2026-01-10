#pragma once
#include <iostream>

class LockFreeQueue {
	struct LFNode {
		LFNode(int v) {
			value = v;
		}
		int value;
		LFNode* pNext = nullptr;
	};

public:    
	LockFreeQueue() = default;
	~LockFreeQueue() = default;

	void Push(int value) {
		while (true) {
			LFNode* pNode = m_pTail;
			if (pNode == nullptr)
			{
				auto p = new LFNode(value);
				if (_InterlockedCompareExchange64((long long*)&m_pTail->pNext, (long long)p, 0) == 0)
				{
					m_pHead = m_pTail;
					_InterlockedIncrement((long*)&m_count);
					break;
				}
			}
			else
			{
				auto p = new LFNode(value);
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
			LFNode* pRet = m_pHead;
			if (pRet == nullptr)
			{
				return 0;
			}
			else
			{
				if (_InterlockedCompareExchange64((long long*)&m_pHead, (long long)pRet->pNext, (long long)pRet) == (long long)pRet)
				{
					if (m_pHead == nullptr) {
						m_pTail = nullptr;
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