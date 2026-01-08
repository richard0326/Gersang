#pragma once

class CEffectObject : public CBaseObject
{
public:
	CEffectObject();
	virtual ~CEffectObject();

	virtual void Render() override;
	virtual bool Run() override;

private:
	bool	m_bEffectStart;
	DWORD	m_dwAttackID;
};