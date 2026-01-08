#pragma once

class CSprite;
class CPlayerObject : public CBaseObject
{
public:
	CPlayerObject();

	virtual ~CPlayerObject();

public:
	virtual void Render() override;
	virtual bool Run() override;

	bool IsPlayer();
	void SetPlayerCharacterTrue();

	bool GetDirection();
	void SetDirection(bool bRight);

	char GetHP();
	void SetHP(char chHP);

	void InputActionProc(DWORD actionInput);
private:
	// Run() 함수 내부에서 호출해주는 실제 객체 처리 부
	void ActionProc();

	// ActionInput을 통해서 들어온 액션 값을 실제로 처리하는 부분. 
	// ActionProc 내부에서 호출
	bool SendAction(DWORD byActionType);

	// 액션 변경 함수들
	void SetActionAttack1();
	void SetActionAttack2();
	void SetActionAttack3();
	void SetActionMove(DWORD move);
	void SetActionStand();

	// 애니메이션 변경 함수
	void SetAnimation(int ActionID, bool bRight);
	
public:
	void SetHitEffect(int effectX, int effectY);
	void ResetAnimation();
private:
	bool		m_bPlayerCharacter;
	char		m_chHP;

	// 액션 정보
	DWORD		m_dwActionCur;
	DWORD		m_dwActionOld;

	// 방향 정보
	bool		m_bDirCur;

	// 스프라이트 정보
	CSprite*	m_sprite_hp;
	CSprite*	m_sprite_shadow;
};