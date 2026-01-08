#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <tchar.h>
#include "Parser.h"

CParser::CAreaInfo::CAreaInfo()
{

}

CParser::CAreaInfo::~CAreaInfo()
{

}

void CParser::CAreaInfo::SetName(const wchar_t* name)
{
	wcscpy_s(m_AreaName, name);
}

const wchar_t* CParser::CAreaInfo::GetName()
{
	return m_AreaName;
}

bool CParser::CAreaInfo::Insert(const wchar_t* val1, const wchar_t* val2)
{
	if (val2[0] == '\"')
	{
		size_t len = wcslen(val2);
		if (val2[len - 1] != '\"')
		{
			return false;
		}

		wchar_t temp[MAX_STRLEN];
		wcsncpy_s(temp, val2 + 1, len - 2);
		m_hMapStr[wstring(val1)] = wstring(temp);
	}
	else
	{
		m_hMapInt[wstring(val1)] = _wtoi(val2);
	}

	return true;
}

bool CParser::CAreaInfo::FindString(const wchar_t* hKey, wchar_t* out, int* inOut)
{
	if (m_hMapStr.size() == 0)
	{
		return false;
	}

	wcscpy_s(out, *inOut, m_hMapStr[hKey].c_str());
	*inOut = (int)wcslen(out);
	return true;
}

bool CParser::CAreaInfo::FindInt(const wchar_t* hKey, int* out)
{
	if (m_hMapInt.size() == 0)
	{
		return false;
	}
	*out = m_hMapInt[hKey];
	return true;
}

bool CParser::CAreaInfo::GetNextInt(wstring* outKey, int* outInt)
{
	if (m_IsIntIterSet == false)
	{
		if (m_hMapInt.empty())
		{
			return false;
		}
		m_hMapIntIterator = m_hMapInt.begin();
		m_IsIntIterSet = true;
	}
	else
	{
		++m_hMapIntIterator;
		if (m_hMapIntIterator == m_hMapInt.end())
		{
			m_IsIntIterSet = false;
			return false;
		}
	}

	*outKey = (*m_hMapIntIterator).first;
	*outInt = (*m_hMapIntIterator).second;

	return true;
}

bool CParser::CAreaInfo::GetNextWString(wstring* outKey, wstring* outWStr)
{
	if (m_IsWStrIterSet == false)
	{
		if (m_hMapStr.empty())
		{
			return false;
		}
		m_hMapWStrIterator = m_hMapStr.begin();
		m_IsWStrIterSet = true;
	}
	else
	{
		++m_hMapWStrIterator;
		if (m_hMapWStrIterator == m_hMapStr.end())
		{
			m_IsWStrIterSet = false;
			return false;
		}
	}

	*outKey = (*m_hMapWStrIterator).first;
	*outWStr = (*m_hMapWStrIterator).second;

	return true;
}

CParser::CParser()
	: m_iFilePointer(0)
	, m_FileSize(0)
{
}

CParser::~CParser()
{
	for (auto iter = m_AreaList.begin(); iter != m_AreaList.end();)
	{
		delete (*iter);
		iter = m_AreaList.erase(iter);
	}
}

// 전체 버퍼 읽어들이기
bool CParser::ReadBuffer(const wchar_t* filename, wchar_t** pError)
{
	FILE* fp;

	if (_wfopen_s(&fp, filename, L"rt,ccs=utf-8") != 0)
	{
		*pError = (wchar_t*)L"File Open Fail \n";
		return false;
	}

	wchar_t tmpBuf[255];
	m_FileSize = 0;

	while (fgetws(tmpBuf, 255, fp) != NULL)
	{
		wcsncpy_s(m_chFileBuffer + m_FileSize, 255, tmpBuf, wcslen(tmpBuf));
		m_FileSize += (int)wcslen(tmpBuf);
	}

	fclose(fp);

	if (ReadArea() == false)
	{
		*pError = m_ErrorStr;
		return false;
	}

	return true;
}

void CParser::Reset()
{
	m_iFilePointer = 0;
	m_IsAreaIterSet = false;

	for (auto iter = m_AreaList.begin(); iter != m_AreaList.end();)
	{
		delete (*iter);
		iter = m_AreaList.erase(iter);
	}
}

bool CParser::ReadArea()
{
	//
	// 무조건 영역이 있다고 가정한다.
	// 없으면 FAIL
	//
	// : XXXXXX
	// {
	//		...
	// }
	// 위의 형태를 찾고...
	// 영역을 만들어 준다.

	int AreaCnt = 0;

	while (SkipBlank())
	{
#pragma region ReadAreaClone	
		// 지역의 ":" 을 읽는다

		wchar_t word1[MAX_STRLEN];
		if (ReadWord(word1) == false)
		{
			swprintf_s(m_ErrorStr, MAX_STRLEN, L"ReadArea() ReadWord 1 오류 : %s \n", word1);
			return false;
		}

		if (wcscmp(word1, L":") != 0)
		{
			swprintf_s(m_ErrorStr, MAX_STRLEN, L"ReadArea() strcmp 1 오류 : %s \n", word1);
			return false;
		}

		if (SkipBlank() == false)
		{
			swprintf_s(m_ErrorStr, MAX_STRLEN, L"ReadArea() SkipBlank 1 오류 : %s \n", word1);
			return false;
		}
#pragma endregion

#pragma region ReadAreaName
		// 지역의 이름을 읽는다.

		wchar_t word2[MAX_STRLEN];
		if (ReadWord(word2) == false)
		{
			swprintf_s(m_ErrorStr, MAX_STRLEN, L"ReadArea() ReadWord 2 오류 : %s \n", word2);
			return false;
		}

		CAreaInfo* infoOut = new CAreaInfo();
		infoOut->SetName(word2);
		m_AreaList.push_back(infoOut);

		if (SkipBlank() == false)
		{
			swprintf_s(m_ErrorStr, MAX_STRLEN, L"ReadArea() SkipBlank 2 오류 : %s \n", word2);
			return false;
		}
#pragma endregion

#pragma region ReadBrackets
		// 첫번째 대괄호 "{"를 읽는다.

		wchar_t word3[MAX_STRLEN];
		if (ReadWord(word3) == false)
		{
			swprintf_s(m_ErrorStr, MAX_STRLEN, L"ReadArea() ReadWord 3 오류 : %s \n", word3);
			return false;
		}

		if (wcscmp(word3, L"{") != 0)
		{
			swprintf_s(m_ErrorStr, MAX_STRLEN, L"ReadArea() strcmp 3 오류 : %s \n", word3);
			return false;
		}
#pragma endregion

		// 영역의 내부 설정
		if (SetArea(infoOut, AreaCnt) == false)
		{
			return false;
		}

		++AreaCnt;
	}

	if (AreaCnt == 0)
	{
		swprintf_s(m_ErrorStr, MAX_STRLEN, L"ReadArea() No Area \n");
		return false;
	}

	return true;
}

bool CParser::SetArea(CAreaInfo* areaInfo, int AreaIdx)
{
	// AAAA : BBBB
	// 위의 형태를 찾고 있다
	// "}" 을 첫번째 순서에서 찾으면 정상 종료
	// "}" 을 첫번째가 아닌 순서에서 찾으면 비정상 종료

	while (SkipBlank())
	{
#pragma region ReadWord1
		// 첫번째 단어를 찾는다
		// 위의 주석에서는 AAAA
		// "}"를 찾으면 지역의 끝을 의미하기 때문에 종료

		wchar_t word1[MAX_STRLEN];
		if (ReadWord(word1) == false)
		{
			swprintf_s(m_ErrorStr, MAX_STRLEN, L"SetArea() ReadWord 1 오류 : %s \n", word1);
			return false;
		}

		if (wcscmp(word1, L"}") == 0)
		{
			// 정상 종료
			return true;
		}

		if (SkipBlank() == false)
		{
			swprintf_s(m_ErrorStr, MAX_STRLEN, L"SetArea() SkipBlank 1 오류 : %s \n", word1);
			return false;
		}
#pragma endregion

#pragma region ReadClone
		// ":" 을 읽는다.

		wchar_t word2[MAX_STRLEN];
		if (ReadWord(word2) == false)
		{
			swprintf_s(m_ErrorStr, MAX_STRLEN, L"SetArea() ReadWord 2 오류 : %s \n", word2);
			return false;
		}

		if (wcscmp(word2, L":") != 0)
		{
			swprintf_s(m_ErrorStr, MAX_STRLEN, L"SetArea() strcmp 2 오류 : %s \n", word2);
			return false;
		}

		if (SkipBlank() == false)
		{
			swprintf_s(m_ErrorStr, MAX_STRLEN, L"SetArea() SkipBlank 2 오류 : %s\n", word1);
			return false;
		}
#pragma endregion

#pragma region ReadWord2
		// 두번째 단어를 찾는다
		// 위의 주석에서는 BBBB
		// "}"를 찾으면 오류

		wchar_t word3[MAX_STRLEN];
		if (ReadWord(word3) == false)
		{
			swprintf_s(m_ErrorStr, MAX_STRLEN, L"SetArea() ReadWord 3 오류 : %s \n", word3);
			return false;
		}

		if (wcscmp(word3, L"}") == 0)
		{
			swprintf_s(m_ErrorStr, MAX_STRLEN, L"SetArea() strcmp 3 오류 : %s \n", word3);
			return false;
		}
#pragma endregion

		// 여기까지 왔다면 성공적으로 읽기를 수행한 것
		if (areaInfo->Insert(word1, word3) == false)
		{
			swprintf_s(m_ErrorStr, MAX_STRLEN, L"SetArea() Insert 오류 : %s : %s \n", word1, word3);
			return false;
		}
	}

	swprintf_s(m_ErrorStr, MAX_STRLEN, L"SetArea() ret 오류 \n");
	return false;
}

bool CParser::SkipBlank()
{
	// 주석 여부
	bool ignore = false;
	bool isStringVal = false;
	IGNORETYPE ignoreType;

	while (m_iFilePointer < m_FileSize)
	{
#pragma region OnRemark
		// 주석 중인 경우
		if (ignore == true)
		{
			if (ignoreType == LINE && m_chFileBuffer[m_iFilePointer] == '\n')
			{
				ignore = false;
			}
			else if (ignoreType == AREA && m_chFileBuffer[m_iFilePointer] == '*')
			{
				++m_iFilePointer;
				if (m_iFilePointer >= m_FileSize)
				{
					return false;
				}

				if (m_chFileBuffer[m_iFilePointer] == '/')
				{
					ignore = false;
				}
			}

			++m_iFilePointer;
			continue;
		}
#pragma endregion

#pragma region NotRemark
		// 주석이 아닌 경우

		// 주석이 아닌데 주석이 나온 경우
		if (ignore == false)
		{
			if (m_chFileBuffer[m_iFilePointer] == '/')
			{
				++m_iFilePointer;
				if (m_iFilePointer >= m_FileSize)
				{
					return false;
				}

				if (m_chFileBuffer[m_iFilePointer] == '/')
				{
					ignore = true;
					ignoreType = LINE;
				}
				else if (m_chFileBuffer[m_iFilePointer] == '*')
				{
					ignore = true;
					ignoreType = AREA;
				}

				++m_iFilePointer;
				continue;
			}

			// 주석 여부가 상관이 없는 경우
			if (m_chFileBuffer[m_iFilePointer] == ' ' || m_chFileBuffer[m_iFilePointer] == '\0' ||
				m_chFileBuffer[m_iFilePointer] == '\r' || m_chFileBuffer[m_iFilePointer] == '\n' ||
				m_chFileBuffer[m_iFilePointer] == '\t')
			{
				++m_iFilePointer;
				continue;
			}
		}
#pragma endregion

		break;
	}

	if (m_iFilePointer >= m_FileSize)
	{
		return false;
	}

	return true;
}

bool CParser::ReadWord(wchar_t* outStr)
{
	int i = 0;
	for (i = 0; m_iFilePointer < m_FileSize; ++i)
	{
		if (m_chFileBuffer[m_iFilePointer] == ' ' || m_chFileBuffer[m_iFilePointer] == '\0' ||
			m_chFileBuffer[m_iFilePointer] == '\r' || m_chFileBuffer[m_iFilePointer] == '\n' ||
			m_chFileBuffer[m_iFilePointer] == '\t')
		{
			break;
		}

		outStr[i] = m_chFileBuffer[m_iFilePointer];

		++m_iFilePointer;
	}
	outStr[i] = '\0';

	return true;
}

// inOut의 크기는 out문자열의 크기 : wcslen의 결과값, wchar_t 크기
bool CParser::GetValue(const wchar_t* AreaName, const wchar_t* valueName, wchar_t* out, int* inOut)
{
	// 문자열을 뽑는 GetValue 의 경우 마지막 인자는
	// inOut 인자 버퍼의 길이를 넘겨주며, 내부에서는
	// 그 길이 값 만큼만 안전하게 사용 합니다.

	for (auto iter = m_AreaList.begin(); iter != m_AreaList.end(); ++iter)
	{
		if (wcscmp((*iter)->GetName(), AreaName) == 0)
		{
			if ((*iter)->FindString(valueName, out, inOut) == false)
			{
				return false;
			}
			return true;
		}
	}

	return false;
}

bool CParser::GetValue(const wchar_t* AreaName, const wchar_t* valueName, int* out)
{
	for (auto iter = m_AreaList.begin(); iter != m_AreaList.end(); ++iter)
	{
		if (wcscmp((*iter)->GetName(), AreaName) == 0)
		{
			if ((*iter)->FindInt(valueName, out) == true)
			{
				return true;
			}
		}
	}

	return false;
}

void CParser::SetValue(const wchar_t* name, const wchar_t* val1, const wchar_t* val2)
{
	for (auto iter = m_AreaList.begin(); iter != m_AreaList.end(); ++iter)
	{
		if (wcscmp(name, (*iter)->GetName()) == 0)
		{
			wchar_t buf[MAX_STRLEN];
			wsprintf(buf, L"\"%s\"", val2);
			(*iter)->Insert(val1, buf);

			return;
		}
	}

	CAreaInfo* input = new CAreaInfo();
	input->SetName(name);
	wchar_t buf[MAX_STRLEN];
	wsprintf(buf, L"\"%s\"", val2);
	input->Insert(val1, buf);

	m_AreaList.push_back(input);
}

void CParser::SetValue(const wchar_t* name, const wchar_t* val1, int val2)
{
	for (auto iter = m_AreaList.begin(); iter != m_AreaList.end(); ++iter)
	{
		if (wcscmp(name, (*iter)->GetName()) == 0)
		{
			wchar_t buf[MAX_STRLEN];
			_itow_s(val2, buf, MAX_STRLEN, 10);
			(*iter)->Insert(val1, buf);

			return;
		}
	}

	CAreaInfo* input = new CAreaInfo();
	input->SetName(name);

	wchar_t buf[MAX_STRLEN];
	_itow_s(val2, buf, MAX_STRLEN, 10);
	input->Insert(val1, buf);

	m_AreaList.push_back(input);
}

void CParser::SaveDataToFile(const wchar_t* fileName)
{
	FILE* fp;
	if (_wfopen_s(&fp, fileName, L"wt,ccs=utf-8") != 0)
	{
		return;
	}

	wchar_t buf[MAX_BUFFER];
	for (auto outeriter = m_AreaList.begin(); outeriter != m_AreaList.end(); ++outeriter)
	{
		swprintf_s(buf, L": %s\n", (*outeriter)->GetName());
		fputws(buf, fp);
		fputws(L"{\n", fp);

		// 문자열로 되어 있는 데이터를 파일에 저장
		for (auto innerIter = (*outeriter)->m_hMapStr.begin(); innerIter != (*outeriter)->m_hMapStr.end(); ++innerIter)
		{
			swprintf_s(buf, L"\t%s : \"%s\"\n", innerIter->first.c_str(), innerIter->second.c_str());
			fputws(buf, fp);
		}

		// 정수형으로 되어 있는 데이터를 파일로 저장
		for (auto innerIter = (*outeriter)->m_hMapInt.begin(); innerIter != (*outeriter)->m_hMapInt.end(); ++innerIter)
		{
			swprintf_s(buf, L"\t%s : %d\n", innerIter->first.c_str(), innerIter->second);
			fputws(buf, fp);
		}

		fputws(L"}", fp);
		fputws(L"\n", fp);
	}

	fclose(fp);
}

bool CParser::GetNextInt(wstring* outKey, int* outInt)
{
	if (m_IsAreaIterSet == false)
		return false;

	return (*m_AreaIterator)->GetNextInt(outKey, outInt);
}

bool CParser::GetNextWString(wstring* outKey, wstring* outWStr)
{
	if (m_AreaIterator == m_AreaList.end())
		return false;

	return (*m_AreaIterator)->GetNextWString(outKey, outWStr);
}

bool CParser::SetNextArea()
{
	if (m_IsAreaIterSet == false)
	{
		if (m_AreaList.empty())
		{
			return false;
		}
		m_AreaIterator = m_AreaList.begin();
		m_IsAreaIterSet = true;
	}
	else
	{
		++m_AreaIterator;
		if (m_AreaIterator == m_AreaList.end())
		{
			m_IsAreaIterSet = false;
			return false;
		}
	}
	return true;
}

const wchar_t* CParser::GetAreaName()
{
	if (m_IsAreaIterSet == false)
	{
		return nullptr;
	}
	return (*m_AreaIterator)->GetName();
}

#if defined(_DEBUG) && defined(_CONSOLE)
// Console 환경에서 Parser.txt 만들기
void CParser::PrintConsole()
{
	printf("Size : %d\n", m_AreaList.size());

	for (auto outeriter = m_AreaList.begin(); outeriter != m_AreaList.end(); ++outeriter)
	{
		printf(": ");
		wprintf(L"%s\n", (*outeriter)->GetName());
		printf("{\n");

		for (auto innerIter = (*outeriter)->m_hMapStr.begin(); innerIter != (*outeriter)->m_hMapStr.end(); ++innerIter)
		{
			wprintf(L"\t%s : \"%s\"\n", innerIter->first.c_str(), innerIter->second.c_str());
		}

		for (auto innerIter = (*outeriter)->m_hMapInt.begin(); innerIter != (*outeriter)->m_hMapInt.end(); ++innerIter)
		{
			wprintf(L"\t%s : %d\n", innerIter->first.c_str(), innerIter->second);
		}
		wprintf(L"}\n\n");
	}
}
#endif // _DEBUG && _CONSOLE