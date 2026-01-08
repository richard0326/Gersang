#include "stdafx.h"
#include <Pdh.h>
#pragma comment(lib,"Pdh.lib")
#include <tlhelp32.h>
#include <string>
#include <strsafe.h>
using namespace std;

#include "CpuUsage.h"

CCpuUsage::CCpuUsage(int ethernetCnt, wchar_t** wstrArr)
{
	if (ethernetCnt == 0 || wstrArr == nullptr)
	{
		return;
	}

	// 프로세서 개수를 확인한다.
	// 프로세스(exe) 실행률 계산 시 cpu 개수로 나누기를 하여 실제 사용률을 구함.
	SYSTEM_INFO SystemInfo;

	GetSystemInfo(&SystemInfo);
	m_iNumberOfProcessors = SystemInfo.dwNumberOfProcessors;

	m_fProcessorTotal = 0;
	m_fProcessorUser = 0;
	m_fProcessorKernel = 0;

	m_ftProcessor_LastKernel.QuadPart = 0;
	m_ftProcessor_LastUser.QuadPart = 0;
	m_ftProcessor_LastIdle.QuadPart = 0;

	// PDH 쿼리 핸들 생성
	PdhOpenQuery(NULL, NULL, &m_pdhQuery);

	PdhAddCounter(m_pdhQuery, L"\\Paging File(_Total)\\% Usage", NULL, &m_pdhPagingFileUsage);
	PdhAddCounter(m_pdhQuery, L"\\Memory\\Available MBytes", NULL, &m_pdhAvailableMBytes);
	PdhAddCounter(m_pdhQuery, L"\\Memory\\Pool Nonpaged Bytes", NULL, &m_pdhProcessorPoolNonpagedBytes);
	PdhAddCounter(m_pdhQuery, L"\\TCPv4\\Segments Retransmitted/sec", NULL, &m_pdhProcessorRetransmittedPerSec);
	PdhAddCounter(m_pdhQuery, L"\\Processor(_Total)\\Interrupts/sec", NULL, &m_pdhProcessorInterruptsPerSec);

	m_ethernetCount = ethernetCnt;
	m_pdhProcessorNetworkSendBytes = new HANDLE[m_ethernetCount];
	m_pdhProcessorNetworkRecvBytes = new HANDLE[m_ethernetCount];
	m_llNetworkSendBytes = new long long[m_ethernetCount];
	m_llNetworkRecvBytes = new long long[m_ethernetCount];

	wchar_t szQuery[1024] = { 0, };
	for (int i = 0; i < m_ethernetCount; ++i)
	{
		int strLen = wcslen(wstrArr[i]);
		for (int j = strLen - 1; ; j--)
		{
			if (wstrArr[i][j] == L' ')
			{
				wstrArr[i][j] = L'\0';
				continue;
			}
			break;
		}

		szQuery[0] = L'\0';
		StringCbPrintf(szQuery, sizeof(wchar_t) * 1024, L"\\Network Interface(%s)\\Bytes Received/sec", wstrArr[i]);
		PdhAddCounter(m_pdhQuery, szQuery, NULL, &m_pdhProcessorNetworkRecvBytes[i]);

		printf("%ws\n", szQuery);

		szQuery[0] = L'\0';
		StringCbPrintf(szQuery, sizeof(wchar_t) * 1024, L"\\Network Interface(%s)\\Bytes Sent/sec", wstrArr[i]);
		PdhAddCounter(m_pdhQuery, szQuery, NULL, &m_pdhProcessorNetworkSendBytes[i]);

		printf("%ws\n", szQuery);
	}
	Sleep(10000);

	UpdateCpuTime();
}

CCpuUsage::~CCpuUsage()
{
	if(m_llNetworkRecvBytes != nullptr)
		delete[] m_llNetworkRecvBytes;
	if(m_llNetworkSendBytes != nullptr)
		delete[] m_llNetworkSendBytes;
	if(m_pdhProcessorNetworkRecvBytes != nullptr)
		delete[] m_pdhProcessorNetworkRecvBytes;
	if(m_pdhProcessorNetworkSendBytes != nullptr)
		delete[] m_pdhProcessorNetworkSendBytes;
}

// cpu 사용률을 갱신한다. 500ms, 1000ms 단위의 호출이 적절한 듯.
void	CCpuUsage::UpdateCpuTime(void)
{
	// 프로세서 사용률을 갱신한다.
	// 본래의 사용 구조체는 FILETIME이지만, ULARGE_INTEGER와 구조가 같으므로 이를 사용함.
	// FILETIME 구조체는 100ns 단위의 시간 단위를 표현하는 구조체임.
	ULARGE_INTEGER Idle;
	ULARGE_INTEGER Kernel;
	ULARGE_INTEGER User;

	// 시스템 사용 시간을 구한다.
	// 아이들 타임 / 커널 사용 타임(아이들 포함) / 유저 사용 타임
	if (GetSystemTimes((PFILETIME)&Idle, (PFILETIME)&Kernel, (PFILETIME)&User) == false)
	{
		return;
	}

	// 커널 타임에는 아이틀 타임이 포함됨.
	ULONGLONG KernelDiff = Kernel.QuadPart - m_ftProcessor_LastKernel.QuadPart;
	ULONGLONG UserDiff = User.QuadPart - m_ftProcessor_LastUser.QuadPart;
	ULONGLONG IdleDiff = Idle.QuadPart - m_ftProcessor_LastIdle.QuadPart;

	ULONGLONG Total = KernelDiff + UserDiff;
	ULONGLONG TimeDiff;

	if (Total == 0)
	{
		m_fProcessorUser = 0.0f;
		m_fProcessorKernel = 0.0f;
		m_fProcessorTotal = 0.0f;
	}
	else
	{
		// 커널 타임에 아이들 타임이 있으므로 빼서 계산
		m_fProcessorTotal = (float)((double)(Total - IdleDiff) / Total * 100.0f);
		m_fProcessorUser = (float)((double)(UserDiff) / Total * 100.0f);
		m_fProcessorKernel = (float)((double)(KernelDiff - IdleDiff) / Total * 100.0f);
	}

	m_ftProcessor_LastKernel = Kernel;
	m_ftProcessor_LastUser = User;
	m_ftProcessor_LastIdle = Idle;

	PdhCollectQueryData(m_pdhQuery);

	PDH_FMT_COUNTERVALUE PagingFileUsage;
	PdhGetFormattedCounterValue(m_pdhPagingFileUsage, PDH_FMT_DOUBLE, NULL, &PagingFileUsage);

	PDH_FMT_COUNTERVALUE AvailableMBytes;
	PdhGetFormattedCounterValue(m_pdhAvailableMBytes, PDH_FMT_LARGE, NULL, &AvailableMBytes);

	PDH_FMT_COUNTERVALUE ProcessorPoolNonpagedBytes;
	PdhGetFormattedCounterValue(m_pdhProcessorPoolNonpagedBytes, PDH_FMT_LARGE, NULL, &ProcessorPoolNonpagedBytes);

	PDH_FMT_COUNTERVALUE ProcessorRetransmittedPerSec;
	PdhGetFormattedCounterValue(m_pdhProcessorRetransmittedPerSec, PDH_FMT_LARGE, NULL, &ProcessorRetransmittedPerSec);

	PDH_FMT_COUNTERVALUE ProcessorInterruptsPerSec;
	PdhGetFormattedCounterValue(m_pdhProcessorInterruptsPerSec, PDH_FMT_LARGE, NULL, &ProcessorInterruptsPerSec);
	
	for (int i = 0; i < m_ethernetCount; ++i)
	{
		PDH_FMT_COUNTERVALUE ProcessorNetworkRecvBytes;
		PdhGetFormattedCounterValue(m_pdhProcessorNetworkRecvBytes[i], PDH_FMT_LARGE, NULL, &ProcessorNetworkRecvBytes);
		PDH_FMT_COUNTERVALUE ProcessorNetworkSendBytes;
		PdhGetFormattedCounterValue(m_pdhProcessorNetworkSendBytes[i], PDH_FMT_LARGE, NULL, &ProcessorNetworkSendBytes);

		m_llNetworkRecvBytes[i] = ProcessorNetworkRecvBytes.largeValue;
		m_llNetworkSendBytes[i] = ProcessorNetworkSendBytes.largeValue;
	}

	m_fPagingFileUsage = PagingFileUsage.doubleValue;
	m_llAvailableMBytes = AvailableMBytes.largeValue;
	m_llPoolNonpagedBytes = ProcessorPoolNonpagedBytes.largeValue;
	m_llRetransmittedPerSec = ProcessorRetransmittedPerSec.largeValue;
	m_llInterruptsPerSec = ProcessorInterruptsPerSec.largeValue;
}

void	CCpuUsage::FindEthernetArr(int* outLen, wstring* wstrArr)
{
	wchar_t* szCur = NULL;
	wchar_t* szCounters = NULL;
	wchar_t* szInterfaces = NULL;
	DWORD dwCounterSize = 0, dwInterfaceSize = 0;

	// PDH enum Object를 사용하는 방법
	// 모든 이더넷 이름이 나오지만 실제 사용중인 이더넷, 가상이더넷 등등을 확인불가함.

	// PdhEnumObjectItems을 통해서 "NetworkInterface" 항목에서 얻을 수 있는
	// 측성항목(Counters) / 인터페이스 항목(Interfaces) 를 얻음. 그런데 그 개수나 길이를 모르기 때문에
	// 먼저 버퍼의 길이를 알기 위해서 Out Buffer 인자들을 NULL 포인터로 넣어서 사이즈만 확인.
	PdhEnumObjectItems(NULL, NULL, L"Network Interface", szCounters, &dwCounterSize, szInterfaces, &dwInterfaceSize, PERF_DETAIL_WIZARD, 0);

	szCounters = new WCHAR[dwCounterSize];
	szInterfaces = new WCHAR[dwInterfaceSize];

	// 버퍼의 동적 할당 후 다시 호출!
	// szCounters와 szInterfaces 버퍼에는 문자열이 쭉쭉쭉 들어온다. 2차원 배열도 아니고,
	// 그냥 NULL 포인터로 끝나는 문자열들이 dwCounterSize, dwInterfaceSize 길이 만큼 줄줄이 들어있음.
	// 이를 문자열 단위로 끊어서 개수를 확인해야 함. aaa\0bbb\0ccc\0ddd

	if (PdhEnumObjectItems(NULL, NULL, L"Network Interface", szCounters, &dwCounterSize, szInterfaces, &dwInterfaceSize, PERF_DETAIL_WIZARD, 0) != ERROR_SUCCESS)
	{
		delete[] szCounters;
		delete[] szInterfaces;
		return ;
	}

	szCur = szInterfaces;
	wchar_t temp[1000] = { 0, };
	for (int i = 0; *szCur != L'\0' && i < df_PDH_ETHERNET_MAX; i++)
	{
		wcscpy_s(temp, 1000, szCur);
		int len = wcslen(temp);
		temp[len] = '\n';
		temp[len + 1] = '\0';
		wstrArr[i] = temp;

		szCur += wcslen(szCur) + 1;
		*outLen = i + 1;
	}
	
	delete[] szCounters;
	delete[] szInterfaces;
}

void CCpuUsage::ProcessEthernetInfo(int* outLen, long long** outArrSend, long long** outArrRecv)
{
	*outLen = m_ethernetCount;
	*outArrSend = m_llNetworkSendBytes;
	*outArrRecv = m_llNetworkRecvBytes;
}

DWORD FindProcessId(const std::wstring& processName)
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE) {
		return 0;
	}

	Process32First(processesSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile))
	{
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processesSnapshot, &processInfo))
	{
		if (!processName.compare(processInfo.szExeFile))
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	CloseHandle(processesSnapshot);
	return -1;
}

CProcessCpuUsage::CProcessCpuUsage(wstring& wstrProcessName)
{
	wstring addExe = wstrProcessName + L".exe";

	// 임의로 만든 프로세스 이름으로 프로세스 ID를 검색해오는 함수
	// 이 함수는 중복된 이름의 프로세스가 존재하는 경우에 ID를 찾는데 문제가 있다.
	DWORD id = FindProcessId(addExe);
	if (id == -1)
	{
		int* crash = nullptr;
		*crash = 0;
	}
	m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, id);
	if (m_hProcess == nullptr)
	{
		int* crash = nullptr;
		*crash = 0;
	}

	SYSTEM_INFO SystemInfo;

	GetSystemInfo(&SystemInfo);
	m_iNumberOfProcessors = SystemInfo.dwNumberOfProcessors;

	// PDH 쿼리 핸들 생성
	PdhOpenQuery(NULL, NULL, &m_pdhQuery);
		
	// PDH 리소스 카운터 생성 (여러개 수집시 이를 여러개 생성)
	wchar_t buf[100];
	wsprintf(buf, L"\\Process(%s)\\Private Bytes", wstrProcessName.c_str());
	PdhAddCounter(m_pdhQuery, buf, NULL, &m_pdhPrivateBytes);
	wsprintf(buf, L"\\Process(%s)\\Pool Nonpaged Bytes", wstrProcessName.c_str());
	PdhAddCounter(m_pdhQuery, buf, NULL, &m_pdhPoolNonpagedBytes);

	m_fProcessTotal = 0;
	m_fProcessUser = 0;
	m_fProcessKernel = 0;

	m_ftProcess_LastKernel.QuadPart = 0;
	m_ftProcess_LastUser.QuadPart = 0;
	m_ftProcess_LastTime.QuadPart = 0;

	UpdateCpuTime();
}

void	CProcessCpuUsage::UpdateCpuTime(void)
{
	// 지정된 프로세스 사용률을 갱신한다.
	ULARGE_INTEGER None;
	ULARGE_INTEGER NowTime;
	ULARGE_INTEGER Kernel;
	ULARGE_INTEGER User;
	// 현재의 100 ns 단위 시간을 구한다. UTC 시간
	// 프로세스 사용률 판단의 공식
	// a = 샘플간격의 시스템 시간을 구함. (그냥 실제로 지나간 시간)
	// b = 프로세스의 CPU 사용 시간을 구함.
	// a : 100 = b : 사용률  공식으로 사용률을 구함.

	// 얼마의 시간이 지났는지 100 ns 시간을 구함.
	GetSystemTimeAsFileTime((LPFILETIME)&NowTime);

	// 해당 프로세스가 사용한 시간을 구함.
	// 두번째, 세번쨰는 실행, 종료 시간으로 미사용
	GetProcessTimes(m_hProcess, (LPFILETIME)&None, (LPFILETIME)&None, (LPFILETIME)&Kernel, (LPFILETIME)&User);
	if (m_hProcess == nullptr)
	{
		int* crash = nullptr;
		*crash = 0;
	}
	// 이전에 저장된 프로세스 시간과의 차를 구해서 실제로 얼마의 시간이 지났는지 확인.
	// 그리고 실제 지나온 시간으로 나누면 사용률이 나옴.
	ULONGLONG TimeDiff = NowTime.QuadPart - m_ftProcess_LastTime.QuadPart;
	ULONGLONG KernelDiff = Kernel.QuadPart - m_ftProcess_LastKernel.QuadPart;
	ULONGLONG UserDiff = User.QuadPart - m_ftProcess_LastUser.QuadPart;

	ULONGLONG Total = KernelDiff + UserDiff;

	m_fProcessTotal = (float)(Total / (double)m_iNumberOfProcessors / (double)TimeDiff * 100.0f);
	m_fProcessKernel = (float)(KernelDiff / (double)m_iNumberOfProcessors / (double)TimeDiff * 100.0f);
	m_fProcessUser = (float)(UserDiff / (double)m_iNumberOfProcessors / (double)TimeDiff * 100.0f);

	m_ftProcess_LastTime = NowTime;
	m_ftProcess_LastKernel = Kernel;
	m_ftProcess_LastUser = User;

	PdhCollectQueryData(m_pdhQuery);

	PDH_FMT_COUNTERVALUE PrivateBytes;
	if (PdhGetFormattedCounterValue(m_pdhPrivateBytes, PDH_FMT_LARGE, NULL, &PrivateBytes) == ERROR_SUCCESS)
	{
		m_llPrivateBytes = PrivateBytes.largeValue;
	}
	else
		m_llPrivateBytes = 0;

	PDH_FMT_COUNTERVALUE ProcessPoolNonpagedBytes;
	PdhGetFormattedCounterValue(m_pdhPoolNonpagedBytes, PDH_FMT_LARGE, NULL, &ProcessPoolNonpagedBytes);

	m_llPoolNonpagedBytes = ProcessPoolNonpagedBytes.largeValue;
}