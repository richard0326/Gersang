#include "stdafx.h"
#include "RoomMgr.h"
#include "UserMgr.h"

CRoom::CRoom(int ID, const wchar_t* roomName, short nameSize)
{
	m_roomID = ID;
	m_roomName = new wchar_t[nameSize / 2 + 1];
	m_roomNameSize = nameSize / 2;
	int i = 0;
	for (; i < m_roomNameSize; ++i)
	{
		m_roomName[i] = roomName[i];
	}
	m_roomName[i] = '\0';
}

CRoom::~CRoom()
{
	delete m_roomName;
}

bool CRoom::EnterRoom(CUser* pUser)
{
	for (auto iter = m_userList.begin(); iter != m_userList.end(); ++iter)
	{
		if ((*iter)->m_userID == pUser->m_userID)
			return false;
	}

	m_userList.push_back(pUser);
	return true;
}

bool CRoom::ExitRoom(CUser* pUser)
{
	bool eraseTrue = false;
	for (auto iter = m_userList.begin(); iter != m_userList.end(); ++iter)
	{
		if ((*iter)->m_userID == pUser->m_userID)
		{
			m_userList.erase(iter);

			eraseTrue = true;
			break;
		}
	}

	if (eraseTrue == false)
		return false;

	return true;
}

bool CRoom::DestroyRoom(stSession* pSession)
{
	if (m_userList.empty())
	{
		return true;
	}

	return false;
}

wchar_t* CRoom::GetRoomName()
{
	return m_roomName;
}

short CRoom::GetRoomNameSize()
{
	return m_roomNameSize;
}

int CRoom::GetRoomID()
{
	return m_roomID;
}

CUser* CRoom::GetNextUser()
{
	if (m_userList.empty())
	{
		return nullptr;
	}

	if (m_isNextUser == false)
	{
		m_nextUser = m_userList.begin();
		m_isNextUser = true;
	}
	else
	{
		++m_nextUser;
		if (m_nextUser == m_userList.end())
		{
			m_isNextUser = false;
			return nullptr;
		}
	}
	return *m_nextUser;
}

char CRoom::GetUserCount()
{
	return (char)m_userList.size();
}

DECLARE_SINGLETON_IN_CPP(CRoomManager);

CRoomManager::CRoomManager()
	: m_isNextUser(false)
	, m_isNextRoom(false)
{

}

CRoomManager::~CRoomManager()
{
	for (auto iter = m_roomList.begin(); iter != m_roomList.end();)
	{
		delete (*iter);
		iter = m_roomList.erase(iter);
	}
}

CRoom* CRoomManager::CreateRoom(int roomID, wchar_t* roomName, int nameSize)
{
	CRoom* existRoom = nullptr;
	for (auto iter = m_roomList.begin(); iter != m_roomList.end(); ++iter)
	{
		if (wcscmp((*iter)->GetRoomName(), roomName) == 0)
		{
			existRoom = *iter;
			break;
		}
	}

	// 중복된 이름의 방이 있는 경우
	if (existRoom)
	{
		return existRoom;
	}

	CRoom* pRoom = new CRoom(roomID++, roomName, nameSize);
	m_roomList.push_back(pRoom);

	return pRoom;
}

bool CRoomManager::EnterLobby(CUser* pUser)
{
	if (IsInLobby(pUser))
		return false;

	pUser->EnterLobby();
	m_userList.push_back(pUser);

	return true;
}

void CRoomManager::ExitLobby(CUser* pUser)
{
	for (auto iter = m_userList.begin(); iter != m_userList.end(); ++iter)
	{
		if ((*iter) == pUser)
		{
			m_userList.erase(iter);
			return;
		}
	}
}

bool CRoomManager::IsInLobby(CUser* pUser)
{
	for (auto iter = m_userList.begin(); iter != m_userList.end(); ++iter)
	{
		if ((*iter) == pUser)
		{
			return true;
		}
	}

	return false;
}

CRoom* CRoomManager::EnterRoom(stSession* pSession, int roomID)
{
	CUser* pUser = pSession->_pUser;

	CRoom* pRoom = nullptr;
	for (auto iter = m_roomList.begin(); iter != m_roomList.end(); ++iter)
	{
		if ((*iter)->GetRoomID() == roomID)
		{
			(*iter)->EnterRoom(pUser);
			pUser->SetRoom(*iter);
			ExitLobby(pUser);

			return *iter;
		}
	}

	return pRoom;
}

bool CRoomManager::LeaveRoom(CUser* pUser)
{
	// 방이 없는 경우
	if (pUser->GetRoom() == nullptr)
	{
		return false;
	}

	int roomID = pUser->GetRoom()->GetRoomID();
	for (auto iter = m_roomList.begin(); iter != m_roomList.end(); ++iter)
	{
		if ((*iter)->GetRoomID() == roomID)
		{
			CRoom* pRoom = *iter;
			// 방을 떠나고
			if (pRoom->ExitRoom(pUser) == false)
				return false;

			stSession* pSession = pUser->GetSession();

			// 로비로 유저를 옮김.
			EnterLobby(pUser);

			if (pRoom->GetUserCount() == 0)
			{
				delete* iter;
				m_roomList.erase(iter);
			}
			break;
		}
	}

	return true;
}

CUser* CRoomManager::GetNextUser()
{
	if (m_userList.empty())
	{
		return nullptr;
	}

	if (m_isNextUser == false)
	{
		m_nextUser = m_userList.begin();
		m_isNextUser = true;
	}
	else
	{
		++m_nextUser;
		if (m_nextUser == m_userList.end())
		{
			m_isNextUser = false;
			return nullptr;
		}
	}
	return *m_nextUser;
}

char CRoomManager::GetUserCount()
{
	return (char)m_userList.size();
}

CRoom* CRoomManager::GetNextRoom()
{
	if (m_roomList.empty())
	{
		return nullptr;
	}

	if (m_isNextRoom == false)
	{
		m_nextRoom = m_roomList.begin();
		m_isNextRoom = true;
	}
	else
	{
		++m_nextRoom;
		if (m_nextRoom == m_roomList.end())
		{
			m_isNextRoom = false;
			return nullptr;
		}
	}
	return *m_nextRoom;
}

short	CRoomManager::GetRoomCount()
{
	return (short)m_roomList.size();
}