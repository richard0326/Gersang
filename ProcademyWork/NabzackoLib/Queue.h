#pragma once

template <typename T>
class CQueue {
private:
	enum {
		QUEUEMAXSIZE = 10,
	};

	int REAL_SIZE = QUEUEMAXSIZE + 1;
	T* m_arr;
	int m_frontIdx;
	int m_backIdx;

public:
	CQueue()
		: m_frontIdx(0)
		, m_backIdx(0)
	{
		m_arr = new T[REAL_SIZE];

		for (int i = 0; i < REAL_SIZE; i++)
		{
			m_arr[i] = 0;
		}
	}

	CQueue(int size)
		: m_frontIdx(0)
		, m_backIdx(0)
		, REAL_SIZE(size + 1)
	{
		m_arr = new T[REAL_SIZE];
	}

	~CQueue() {
		delete[] m_arr;
	}

	// 기존 값이 날리고 resize()
	void resize(int iSize) {

		delete[] m_arr;
		m_frontIdx = 0;
		m_backIdx = 0;

		REAL_SIZE = iSize + 1;
		m_arr = new T[REAL_SIZE];
	}

	bool empty() {
		return (m_backIdx - m_frontIdx) == 0;
	}

	bool isFull() {
		return (m_backIdx + 1) % REAL_SIZE == m_frontIdx;
	}

	int maxSize() {
		return REAL_SIZE - 1;
	}

	size_t size() {
		return (m_backIdx - m_frontIdx + REAL_SIZE) % REAL_SIZE;
	}

	bool push_back(T _Val) {
		int frontIdx = m_frontIdx;
		int backIdx = m_backIdx;

		if ((backIdx + 1) % REAL_SIZE == m_frontIdx)
		{
			return false;
		}
		m_arr[backIdx] = _Val;
		m_backIdx = (backIdx + 1) % REAL_SIZE;
		return true;
	}

	bool pop_front(T* out) {
		int frontIdx = m_frontIdx;
		int backIdx = m_backIdx;

		if (frontIdx == backIdx)
		{
			return false;
		}
		*out = m_arr[frontIdx];
		m_frontIdx = (frontIdx + 1) % REAL_SIZE;
		return true;
	}

	void clear() {
		m_frontIdx = 0;
		m_backIdx = 0;
	}

	bool peek(int _idx, T* _out) {
		if (m_frontIdx == m_backIdx || _idx >= size())
		{
			return false;
		}

		*_out = m_arr[(m_frontIdx + _idx) % REAL_SIZE];

		return true;
	}
};