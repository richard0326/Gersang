#pragma once

template <typename T>
class CSmartPtr
{
public:
	CSmartPtr()
	{
	}

	CSmartPtr(T* pNew)
	{
		m_ptr = pNew;
		m_refCount = new long(1);
	}

	~CSmartPtr()
	{
		long refCount = _InterlockedDecrement(m_refCount);
		if (refCount == 0)
		{
			delete m_refCount;
			delete m_ptr;
		}
	}

	CSmartPtr(CSmartPtr& ref)
	{
		this->m_ptr = ref.m_ptr;
		this->m_refCount = ref.m_refCount;
		_InterlockedIncrement(m_refCount);
	}

	CSmartPtr& operator = (CSmartPtr& ref)
	{
		this->m_ptr = ref.m_ptr;
		this->m_refCount = ref.m_refCount;
		_InterlockedIncrement(m_refCount);
		return *this;
	}

private:
	T* m_ptr = nullptr;
	long* m_refCount = nullptr;
};