#pragma once

template <typename _Ty>
class CStack {    
private:
    __int32 STACK_SIZE = 10;
    _Ty* m_arr;
    size_t m_topIdx;

public:
    CStack()
        : m_topIdx(0)
    {
        m_arr = new _Ty[STACK_SIZE];

        for (__int32 i = 0; i < STACK_SIZE; i++)
        {
            m_arr[i] = 0;
        }
    }

	// 크기를 주면서 초기화
    CStack(int size)
        : m_topIdx(0)
        , STACK_SIZE(size)
    {
        m_arr = new _Ty[STACK_SIZE];
    }

    ~CStack() {
        if (STACK_SIZE == 1)
        {
            delete m_arr;
        }
        else
        {
            delete[] m_arr;
        }
    }

    // stack 크기를 다시 조정한다.
    void resize(int iSize) {
        
        if (STACK_SIZE == 1)
        {
            delete m_arr;
        }
        else
        {
            delete[] m_arr;
        }
        m_topIdx = 0;

        STACK_SIZE = iSize;
        m_arr = new _Ty[STACK_SIZE];
    }

    bool empty() {
        return m_topIdx == 0;
    }

    bool isFull() {
        return STACK_SIZE == m_topIdx;
    }

    __int32 maxSize() {
        return STACK_SIZE;
    }

    size_t size() {
        return m_topIdx;
    }

	// stack의 top 인자를 가져온다.
    bool top(_Ty* _outVal) {
        if (m_topIdx == 0)
        {
            return false;
        }
        *_outVal = m_arr[m_topIdx - 1];
        return true;
    }

    bool push(_Ty _Val) {
        if (STACK_SIZE == m_topIdx)
        {
            return false;
        }
        m_arr[m_topIdx] = _Val;
        m_topIdx++;
        return true;
    }

    bool pop() {
        if (0 == m_topIdx)
        {
            return false;
        }
        m_topIdx--;
        return true;
    }

    void clear() {
        m_topIdx = 0;
    }

	// stack 내부를 peeking한다.
    bool peek(int _idx, _Ty* _out) {
        if (0 == m_topIdx || _idx >= m_topIdx)
        {
            return false;
        }

        *_out = m_arr[_idx];
        return true;
    }
};