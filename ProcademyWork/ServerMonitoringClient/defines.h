#pragma once

#define WCHAR_LENGTH		50
#define UM_ALERT			WM_USER + 1

enum TYPE
{
	LINE,
	NUMBER,
	ONOFF,
	PIE,
};

enum ALERT_TYPE
{
	OFF,
	OVER,
	SAME,
	UNDER,
};

struct StAddGraphInfo {
	TYPE infoType;
	WCHAR graphName[WCHAR_LENGTH];
	COLORREF graphColor;
	__int32 penWidth;
	__int32 queueSize;

	StAddGraphInfo(TYPE _infoType, CONST WCHAR* _graphName, COLORREF _graphColor, __int32 _penWidth = 2, __int32 _queueSize = 20) 
		: infoType(_infoType)
		, graphColor(_graphColor)
		, penWidth(_penWidth)
		, queueSize(_queueSize)
	{
		wcscpy_s(graphName, _graphName);
	}
};