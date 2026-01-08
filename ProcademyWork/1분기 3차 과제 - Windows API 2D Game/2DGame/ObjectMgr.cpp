#include "stdafx.h"
#include "ObjectMgr.h"
#include "BaseObject.h"
#include "PlayerObject.h"
#include "EffectObject.h"
#include "Lib/ObjectPool.h"

DECLARE_SINGLETON_IN_CPP(CObjectMgr);

CObjectMgr::CObjectMgr()
{

}

CObjectMgr::~CObjectMgr()
{

}

bool CObjectMgr::Init()
{

	return true;
}

void CObjectMgr::Release()
{
	for (auto eraseIter = m_ObjList.begin(); eraseIter != m_ObjList.end(); )
	{
		//delete (*eraseIter);
		if ((*eraseIter)->GetObjectType() == eTYPE_PLAYER) {
			CPlayerObject* player = (CPlayerObject*)(*eraseIter);
			SINGLETON(CObjectPoolMgr)->DeAlloc<CPlayerObject>(player);
		}
		else {
			CEffectObject* effect = (CEffectObject*)(*eraseIter);
			SINGLETON(CObjectPoolMgr)->DeAlloc<CEffectObject>(effect);
		}
		eraseIter = m_ObjList.erase(eraseIter);
	}
}

CBaseObject* CObjectMgr::CreateObject(int ObjectType, int ObjectID)
{
	CBaseObject* ret = nullptr;

	switch (ObjectType)
	{
	case eTYPE_PLAYER:
		//ret = new CPlayerObject();
		ret = SINGLETON(CObjectPoolMgr)->Alloc<CPlayerObject>();
		ret->SetObjectID(ObjectID);
		ret->SetObjectType(eTYPE_PLAYER);
		break;
	case eTYPE_EFFECT:
		//ret = new CEffectObject();
		ret = SINGLETON(CObjectPoolMgr)->Alloc<CEffectObject>();
		ret->SetObjectID(ObjectID);
		ret->SetObjectType(eTYPE_EFFECT);
		break;
	}

	if (ret != nullptr)
	{
		m_ObjList.push_back(ret);
	}

	return ret;
}

bool CObjectMgr::FindByObjectID(int ObjectID, CBaseObject** out)
{
	for (auto iter = m_ObjList.begin(); iter != m_ObjList.end(); ++iter)
	{
		if ((*iter)->GetObjectID() == ObjectID)
		{
			*out = *iter;
			return true;
		}
	}

	return false;
}

bool CObjectMgr::EraseByObjectID(int ObjectID)
{
	for (auto iter = m_ObjList.begin(); iter != m_ObjList.end(); ++iter)
	{
		if ((*iter)->GetObjectID() == ObjectID)
		{
			//delete (*iter);
			if ((*iter)->GetObjectType() == eTYPE_PLAYER) {
				CPlayerObject* player = (CPlayerObject*)(*iter);
				SINGLETON(CObjectPoolMgr)->DeAlloc<CPlayerObject>(player);
			}
			else {
				CEffectObject* effect = (CEffectObject*)(*iter);
				SINGLETON(CObjectPoolMgr)->DeAlloc<CEffectObject>(effect);
			}
			m_ObjList.erase(iter);
			return true;
		}
	}
	return false;
}

void CObjectMgr::Render()
{
	for (auto iter = m_ObjList.begin(); iter != m_ObjList.end(); ++iter)
	{
		(*iter)->Render();
	}
}

bool CObjectMgr::Run()
{
	for (auto iter = m_ObjList.begin(); iter != m_ObjList.end(); )
	{
		if ((*iter)->IsAlive()) {
			(*iter)->Run();
			++iter;
		}
		else {
			if ((*iter)->GetObjectType() == eTYPE_PLAYER) {
				CPlayerObject* player = (CPlayerObject*)(*iter);
				if (player->IsPlayer() == true)
				{
					//delete (*iter);
					SINGLETON(CObjectPoolMgr)->DeAlloc<CPlayerObject>(player);
					iter = m_ObjList.erase(iter);
					return false;
				}
			}

			//delete (*iter);
			if ((*iter)->GetObjectType() == eTYPE_PLAYER) {
				CPlayerObject* player = (CPlayerObject*)(*iter);
				SINGLETON(CObjectPoolMgr)->DeAlloc<CPlayerObject>(player);
			}
			else {
				CEffectObject* effect = (CEffectObject*)(*iter);
				SINGLETON(CObjectPoolMgr)->DeAlloc<CEffectObject>(effect);
			}
			iter = m_ObjList.erase(iter);
		}
	}
	return true;
}

// Ysort만을 위한 함수
bool YsortingFunc(CBaseObject* left, CBaseObject* right)
{
	if (left->GetObjectType() == eTYPE_EFFECT)
	{
		if (right->GetObjectType() == eTYPE_EFFECT && left->GetCurY() < right->GetCurY())
		{
			return false;
		}

		return true;
	}

	if (left->GetCurY() >= right->GetCurY())
	{
		return true;
	}	

	return false;
}

void CObjectMgr::YSort()
{
	m_ObjList.quick_sort(m_ObjList.begin(), m_ObjList.end(), YsortingFunc);
}