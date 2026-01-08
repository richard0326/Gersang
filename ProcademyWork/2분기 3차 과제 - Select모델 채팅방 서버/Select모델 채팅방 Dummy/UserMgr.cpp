#include "stdafx.h"
#include "UserMgr.h"
#include "RoomMgr.h"

CUser::CUser(stSession* pSession)
	: m_pRoom(nullptr)
{
	m_pSession = pSession;

	ZeroMemory(m_nickname, dfNICK_MAX_LEN);

	m_userID = 0;
	m_userLevel = LEVEL_START;
}

CUser::~CUser()
{

}

void CUser::SetUserID(int ID)
{
	m_userID = ID;
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

bool CUser::SetNickname(const wchar_t* nickname)
{
	if (nickname != nullptr)
	{
		int i = 0;
		for (; nickname[i] != '\0'; ++i)
		{
			m_nickname[i] = nickname[i];
		}
		m_nickname[i] = '\0';

		return true;
	}
	return false;
}

stSession* CUser::GetSession()
{
	return m_pSession;
}

void CUser::SetUserLevel(int level)
{
	m_userLevel = level;
}

int CUser::GetUserLevel()
{
	return m_userLevel;
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