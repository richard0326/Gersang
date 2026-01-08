#pragma once

class CAnimation;
class CBaseObject
{
public:
	CBaseObject() 
		: m_iCurX(0)
		, m_iCurY(0)
		, m_Animation(nullptr)
		, m_iCurAnimation(0)
	{
		// 생성자
	}
	virtual ~CBaseObject() { }

	virtual void Render() = 0;
	virtual bool Run() = 0;
	
	// 위치 관련
	void SetPosition(int iX, int iY) {
		m_iCurX = iX;
		m_iCurY = iY;
	}
	int GetCurX() { return m_iCurX; }
	int GetCurY() { return m_iCurY; }
	
	// 오브젝트 관련
	void SetObjectID(int ObjID) { m_iObjectID = ObjID; }
	void SetObjectType(int ObjType) { m_iObjectType = ObjType; }
	int GetObjectID() { return m_iObjectID; }
	int GetObjectType() { return m_iObjectType; }

	bool IsAlive() {
		return !m_bDelObject;
	}

protected:
	// 위치 관련
	int		m_iCurX;
	int		m_iCurY;

	// 오브젝트 구분
	int		m_iObjectID;
	int		m_iObjectType;

	// 애니메이션
	CAnimation** m_Animation;
	int		m_iCurAnimation;

	// 삭제 여부
	bool	m_bDelObject = false;
};