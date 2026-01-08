#pragma once

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
		if (m_pHead == nullptr)
		{
			m_pHead = m_pTail = new LFNode(value);
		}
		else
		{
			LFNode* pNode = m_pTail;
			pNode->pNext = new LFNode(value);
			m_pTail = pNode->pNext;
		}
	}
	int Pop() {
		if (m_pHead == nullptr)
		{
			return 0;
		}
		else
		{
			LFNode* pRet = m_pHead;
			m_pHead = pRet->pNext;
			if (m_pHead == nullptr) {
				m_pTail = nullptr;
			}
			int ret = pRet->value;
			delete pRet;
			return ret;
		}
	}

	bool Empty() {
		while (m_pHead != nullptr)
		{
			Pop();
		}
	}
private:
	LFNode* m_pHead = nullptr;
	LFNode* m_pTail = nullptr;
};