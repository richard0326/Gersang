#include "stdafx.h"
#include "ObjectPool.h"

DECLARE_SINGLETON_IN_CPP(CObjectPoolMgr);

bool CObjectPoolMgr::Init()
{
	m_HashCnt.LowPart = 0x7fffffffffffffff;
	m_HashCnt.HighPart = 0x7fffffffffffffff;

	return true;
}

void CObjectPoolMgr::Release()
{
	for (auto iter = m_objectPool_map.begin(); iter != m_objectPool_map.end(); )
	{
		delete iter->second;
		iter = m_objectPool_map.erase(iter);
	}
}