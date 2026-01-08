#include "stdafx.h"
#include "UserMgr.h"

CUser::CUser(stSession* pSession)
	: m_pRoom(nullptr)
	, m_userLogin(false)
{
	m_pSession = pSession;
	
	ZeroMemory(m_nickname, dfNICK_MAX_LEN);
	
	static int ID = 1;
	m_userID = ID++;
}

CUser::~CUser()
{

}

int CUser::GetUserID()
{
	return m_userID;
}

const wchar_t* CUser::GetUserNickname()
{
	return m_nickname;
}

CRoom* CUser::GetRoom()
{
	return m_pRoom;
}

void CUser::SetRoom(CRoom* pRoom)
{
	m_pRoom = pRoom;
}

void CUser::EnterLobby()
{
	m_pRoom = nullptr;
}

bool CUser::GetLoginState()
{
	return m_userLogin;
}

bool CUser::SetLoginResult(char result, const wchar_t* nickname)
{
	stResult_ID sendInfo;
	sendInfo._ID = m_userID;
	sendInfo._result = result;

	if (result == df_RESULT_LOGIN_OK)
	{
		m_userLogin = true;
		if (m_userLogin && nickname != nullptr)
		{
			int i = 0;
			for (; nickname[i] != '\0'; ++i)
			{
				m_nickname[i] = nickname[i];
			}
			m_nickname[i] = '\0';
		}
	}

	if (SendPacket(m_pSession, df_RES_LOGIN, &sendInfo) == false)
		return false;

	return true;
}

stSession* CUser::GetSession()
{
	return m_pSession;
}

DECLARE_SINGLETON_IN_CPP(CUserManager);

CUserManager::CUserManager()
{

}

CUserManager::~CUserManager()
{

}

int CUserManager::CreateUser(stSession* pSession)
{
	CUser* pUser = new CUser(pSession);
	pSession->_pUser = pUser;

	m_userList.push_back(pUser);

	return pUser->m_userID;
}

bool CUserManager::CheckNickname(const wchar_t* nickname)
{
	for (auto iter = m_userList.begin(); iter != m_userList.end(); ++iter)
	{
		if (wcscmp((*iter)->m_nickname, nickname) == 0)
			return false;
	}

	return true;
}

void CUserManager::DeleteUser(CUser* pUser)
{
	for (auto iter = m_userList.begin(); iter != m_userList.end(); ++iter)
	{
		if ((*iter) == pUser)
		{
			delete (*iter);
			m_userList.erase(iter);
			return;
		}
	}
}