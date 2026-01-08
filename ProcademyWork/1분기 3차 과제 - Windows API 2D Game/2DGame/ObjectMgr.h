#pragma once

class CBaseObject;
class CObjectMgr
{
private:
	CObjectMgr();
	~CObjectMgr();

	DECLARE_SINGLETON_IN_HEADER(CObjectMgr)

public:
	bool Init();
	void Release();

	void Render();
	bool Run();

	CBaseObject* CreateObject(int ObjectType, int ObjectID = 0xFFFFFFFF);
	bool FindByObjectID(int ObjectID, CBaseObject** out);
	bool EraseByObjectID(int ObjectID);

	void YSort();

private:
	CList<CBaseObject*>		m_ObjList;
};