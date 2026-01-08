#pragma once

class CFPSTimer
{
private:
	DECLARE_SINGLETON_IN_HEADER(CFPSTimer)
	
	CFPSTimer();
	~CFPSTimer();

public:
	bool Init(__int64 FramePerSecond);
	void Release();

	bool CheckFrame();

	/** 
	 * DontSkipRendering()
	 * 
	 * 이전 DontSkipRendering() 부터
	 * 현재 DontSkipRendering() 까지의 시간이
	 * 1 frame을 초과한지 확인하는 함수
	 * 
	 * 파라미터:
	 *		없음
	 * 반환 값:
	 *		false : 1 frame을 초과한 경우
	 *		true : 1 frame을 초과하지 않은 경우
	 *	 		   내부에서 Sleep을 사용하여 1 frame 만큼 쉬어준다.
	 */
	bool DontSkipRendering();

	// 현재 FPS를 가지고 옵니다.
	__int64 GetCurrentLogicFPS();
	__int64 GetCurrentRenderFPS();
	__int64 GetCurrentLoopFPS();
	bool CheckSec();

private:
	/**
	 * RenderSkip 여부를 판단하기 위한 변수들
	 */
	__int64 m_freq;				// 1 초의 기준이 되는 frequency
	__int64 m_Frame;			// 1 Frame에 걸리는 시간
	__int64 m_Prev;				// 이전 Frame
	__int64 m_accumulateTime;		// 누적 시간

	/**
	 * FPS 출력용 변수
	 */
	__int64 m_PrevSec;				// 1초의 기준이 되는 이전 프레임
	__int64 m_countLogicFPS;		// logic의 Frame의 개수를 세는 변수
	__int64 m_countRenderFPS;		// render의 Frame의 개수를 세는 변수
	__int64 m_countLoopFPS;		// render의 Frame의 개수를 세는 변수
	__int64 m_FramePerSecond;		// User가 넣어준 FPS 시간 (출력 용)
	__int64 m_PreLogicCountFPS;		// 1초 마다 logicFPS를 담아두는 변수 (출력 용)
	__int64 m_PreRenderCountFPS;	// 1초 마다 renderFPS를 담아두는 변수 (출력 용)
	__int64 m_PreLoopCountFPS;	// 1초 마다 renderFPS를 담아두는 변수 (출력 용)
	bool	m_SecCheck;			// 1초가 지났는지 확인하는 변수
};
