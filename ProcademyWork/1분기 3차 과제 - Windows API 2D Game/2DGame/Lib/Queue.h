#pragma once

// 이름이 Queue이지만 
// 사실은 원형 큐이다.

template <typename _Ty>
class CQueue {    
private:
    enum {
        QUEUEMAXSIZE = 10,
    };

    __int32 REAL_SIZE = QUEUEMAXSIZE + 1;
    _Ty* m_arr;
    size_t m_frontIdx;
    size_t m_backIdx;

public:
    CQueue()
        : m_frontIdx(0)
        , m_backIdx(0)
    {
        m_arr = new _Ty[REAL_SIZE];

        for (__int32 i = 0; i < REAL_SIZE; i++)
        {
            m_arr[i] = 0;
        }
    }

    CQueue(int size)
        : m_frontIdx(0)
        , m_backIdx(0)
        , REAL_SIZE(size+1)
    {
        m_arr = new _Ty[REAL_SIZE];
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
        m_arr = new _Ty[REAL_SIZE];
    }

    bool empty() {
        return (m_backIdx - m_frontIdx) == 0;
    }

    bool isFull() {
        return (m_backIdx + 1) % REAL_SIZE == m_frontIdx;
    }

    __int32 maxSize() {
        return REAL_SIZE - 1;
    }

    size_t size() {
        return (m_backIdx - m_frontIdx + REAL_SIZE) % REAL_SIZE;
    }

    bool front(_Ty* out) {
        if (m_frontIdx == m_backIdx)
        {
            return false;
        }
        *out = m_arr[m_frontIdx];
        return true;
    }

    bool back(_Ty* out) {
        if (m_frontIdx == m_backIdx)
        {
            return false;
        }
        *out = m_arr[(m_backIdx + REAL_SIZE -1) % REAL_SIZE];
        return true;
    }

    bool push(const _Ty& _Val) {
        if ((m_backIdx + 1) % REAL_SIZE == m_frontIdx)
        {
            return false;
        }
        m_arr[m_backIdx] = _Val;
        m_backIdx = (m_backIdx + 1) % REAL_SIZE;
        return true;
    }

    bool pop() {
        if (m_frontIdx == m_backIdx)
        {
            return false;
        }
        m_frontIdx = (m_frontIdx + 1) % REAL_SIZE;
        return true;
    }

    void clear() {
        m_frontIdx = 0;
        m_backIdx = 0;
    }

    bool peek(int _idx,_Ty* _out) {
        if (m_frontIdx == m_backIdx || _idx >= size())
        {
            return false;
        }

        *_out = m_arr[(m_frontIdx + _idx) % REAL_SIZE];

        return true;
    }
};