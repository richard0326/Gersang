#pragma once

class CParser
{
private:
	enum {
		MAX_BUFFER = 5000,
		MAX_VAL_CNT = 30,
		MAX_STRLEN = 50,
		MAX_AREA = 10,
	};

	enum IGNORETYPE {
		LINE,
		AREA,
	};

	class CAreaInfo
	{
	private:
		friend CParser;

		unordered_map<wstring, wstring> m_hMapStr;
		unordered_map<wstring, int> m_hMapInt;

		// 지역의 이름
		wchar_t	m_AreaName[MAX_STRLEN + 1];

	public:
		CAreaInfo();
		~CAreaInfo();

		void SetName(const wchar_t* name);
		const wchar_t* GetName();

		bool Insert(const wchar_t* val1, const wchar_t* val2);
		bool FindString(const wchar_t* hKey, wchar_t* out, int* inOut);
		bool FindInt(const wchar_t* hKey, int* out);
	};

public:
	CParser();
	~CParser();

	// 전체 버퍼 읽어들이기
	bool ReadBuffer(const wchar_t* filename, wchar_t** pError);

private:
	// 영역 별로 분류
	bool ReadArea();

	// 영역 안의 값을 분류
	bool SetArea(CAreaInfo* areaInfo, int AreaIdx);

	// 주석과 빈칸을 건너 뛴다
	bool SkipBlank();

	// 단어 시작 위치에서 호출
	// 단어를 가지고 온다.
	bool ReadWord(wchar_t* outStr);

public:
	bool GetValue(const wchar_t* AreaName, const wchar_t* valueName, wchar_t* out, int* inOut);
	bool GetValue(const wchar_t* AreaName, const wchar_t* valueName, int* out);

	void SetValue(const wchar_t* AreaName, const wchar_t* val1, const wchar_t* val2);
	void SetValue(const wchar_t* AreaName, const wchar_t* val1, int val2);
	void SaveDataToFile(const wchar_t* fileName);

#if defined(_DEBUG) && defined(_CONSOLE)
	// Console 환경에서 Parser.txt 만들기
	void PrintConsole();
#endif // _DEBUG && _CONSOLE

private:
	wchar_t	m_chFileBuffer[MAX_BUFFER + 1];
	int		m_iFilePointer;
	int		m_FileSize;
	wchar_t	m_ErrorStr[MAX_STRLEN];

	CList<CAreaInfo*> m_AreaList;
};
