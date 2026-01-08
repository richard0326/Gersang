#pragma once

#define df_PDH_ETHERNET_MAX 8

class CCpuUsage
{
public:
	// 생성자, 확인 대상 프로세스 핸들. 미입력시 자기 자신
	CCpuUsage(int ethernetCnt = 0, wchar_t** wstrArr = nullptr);
	~CCpuUsage();

	void	UpdateCpuTime(void);
	static void	FindEthernetArr(int* out, wstring* szArr);
	float	ProcessorTotal(void) { return m_fProcessorTotal; }
	float	ProcessorUser(void) { return m_fProcessorUser; }
	float	ProcessorKernel(void) { return m_fProcessorKernel; }

	float	ProcessTotal(void) { return m_fProcessTotal; }
	float	ProcessUser(void) { return m_fProcessUser; }
	float	ProcessKernel(void) { return m_fProcessKernel; }

	float ProcessorPagingFileUsage(void) { return m_fPagingFileUsage; }
	long long ProcessorAvailableMBytes(void) { return m_llAvailableMBytes; }
	long long ProcessorPoolNonpagedBytes(void) { return m_llPoolNonpagedBytes; }
	long long ProcessorRetransmittedPerSec(void) { return m_llRetransmittedPerSec; }
	long long ProcessorInterruptsPerSec(void) { return m_llInterruptsPerSec; }
	void ProcessEthernetInfo(int* outLen, long long** outArrSend, long long** outArrRecv);

private:
	int		m_iNumberOfProcessors;

	float	m_fProcessorTotal;
	float	m_fProcessorUser;
	float	m_fProcessorKernel;
	
	float	m_fProcessTotal;
	float	m_fProcessUser;
	float	m_fProcessKernel;

	ULARGE_INTEGER	m_ftProcessor_LastKernel;
	ULARGE_INTEGER	m_ftProcessor_LastUser;
	ULARGE_INTEGER	m_ftProcessor_LastIdle;

	ULARGE_INTEGER	m_ftProcess_LastKernel;
	ULARGE_INTEGER	m_ftProcess_LastUser;
	ULARGE_INTEGER	m_ftProcess_LastTime;

	HANDLE m_pdhQuery;
	HANDLE m_pdhPagingFileUsage;
	HANDLE m_pdhAvailableMBytes;
	HANDLE m_pdhProcessorPoolNonpagedBytes;
	HANDLE m_pdhProcessorRetransmittedPerSec;
	HANDLE m_pdhProcessorInterruptsPerSec;
	HANDLE* m_pdhProcessorNetworkSendBytes = nullptr;
	HANDLE* m_pdhProcessorNetworkRecvBytes = nullptr;
	
	float	  m_fPagingFileUsage;
	long long m_llAvailableMBytes;
	long long m_llPoolNonpagedBytes;
	long long m_llRetransmittedPerSec;
	long long m_llInterruptsPerSec;

	int m_ethernetCount;
	long long* m_llNetworkSendBytes = nullptr;
	long long* m_llNetworkRecvBytes = nullptr;
};

class CProcessCpuUsage
{
public:
	CProcessCpuUsage(wstring& pwchProcessName);

	void	UpdateCpuTime(void);

	float	ProcessTotal(void) { return m_fProcessTotal; }
	float	ProcessUser(void) { return m_fProcessUser; }
	float	ProcessKernel(void) { return m_fProcessKernel; }

	long long ProcessPrivateBytes(void) { return m_llPrivateBytes; }
	long long ProcessPoolNonpagedBytes(void) { return m_llPoolNonpagedBytes; }

private:
	HANDLE	m_hProcess;
	int		m_iNumberOfProcessors;
	float	m_fProcessTotal;
	float	m_fProcessUser;
	float	m_fProcessKernel;

	ULARGE_INTEGER	m_ftProcess_LastKernel;
	ULARGE_INTEGER	m_ftProcess_LastUser;
	ULARGE_INTEGER	m_ftProcess_LastTime;

	HANDLE m_pdhQuery;

	HANDLE m_pdhPrivateBytes;
	HANDLE m_pdhPoolNonpagedBytes;

	long long m_llPrivateBytes;
	long long m_llPoolNonpagedBytes;
};