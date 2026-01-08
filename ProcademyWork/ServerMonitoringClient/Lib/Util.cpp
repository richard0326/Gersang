#include <iostream>
#include <Windows.h>
#include "Util.h"
#include <time.h>
#include <math.h>

namespace Util {
	bool Init_Util()
	{
		// 랜덤 시드 초기화
		srand((unsigned int)time(NULL));

		return TRUE;
	}

	__int32 GetDegreeFromPt1_Pt2(__int32 FromX, __int32 FromY, __int32 ToX, __int32 ToY)
	{
		// 높이 a, 밑변 b
		__int32 a = ToY - FromY;
		__int32 b = ToX - FromX;

		// 아크 탄젠트 함수 atan2
		// 출처 : https://m.blog.naver.com/PostView.nhn?blogId=paulj2000&logNo=220847709234&proxyReferer=https:%2F%2Fwww.google.com%2F
		double theta = atan2(a, b);

		__int32 _degree = __int32(theta * 180 / 3.14);
		if (_degree < 0)
		{
			return 360 + _degree;
		}
		return _degree;
	}
}