#include "stdafx.h"
#include "RoomMgr.h"
#include "UserMgr.h"

CRoom::CRoom(int ID, const wchar_t* roomName, short nameSize)
{
	m_roomID = ID;
	m_roomName = new wchar_t[nameSize/2 + 1];
	m_roomNameSize = nameSize/2;
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

	stSession* pSession = pUser->GetSession();

	// 방을 떠나는 것에 대한 처리
	if (SendPacket(pSession, df_RES_ROOM_LEAVE, pUser) == false)
		return false;

	if (BroadcastRoom(this, pSession, df_RES_ROOM_LEAVE, pUser) == false)
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

bool CRoom::ChatRoom(stSession* pSession, wchar_t* msgWstr, short msgSize)
{
	CRoom* pRoom = pSession->_pUser->GetRoom();
	if (pRoom == nullptr)
		return false;

	stUser_Chat sendInfo;
	sendInfo._pUser = pSession->_pUser;
	sendInfo._wchpChat = msgWstr;
	sendInfo._chatLen = msgSize;

	if (BroadcastRoom(pRoom, pSession, df_RES_CHAT, &sendInfo) == false)
		return false;

	return true;
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

CRoom* CRoomManager::CreateRoom(stSession* pSession, wchar_t* roomName, int nameSize)
{
	static int roomID = 1;
	CRoom* existRoom = nullptr;
	for (auto iter = m_roomList.begin(); iter != m_roomList.end(); ++iter)
	{
		if (wcscmp((*iter)->GetRoomName(), roomName) == 0)
		{
			existRoom = *iter;
			break;
		}
	}

	stResult_Room sendInfo;
	sendInfo._pRoom = nullptr;

	// 중복된 이름의 방이 있는 경우
	if(existRoom)
	{
		sendInfo._result = df_RESULT_ROOM_CREATE_DNICK;
		sendInfo._pRoom = existRoom;
		if (SendPacket(pSession, df_RES_ROOM_CREATE, &sendInfo) == false)
			return nullptr;

		return existRoom;
	}

	CRoom* pRoom = new CRoom(roomID++, roomName, nameSize);
	m_roomList.push_back(pRoom);

	// 방 생성 성공
	sendInfo._result = df_RESULT_ROOM_CREATE_OK;
	sendInfo._pRoom = pRoom;
	if (SendPacket(pSession, df_RES_ROOM_CREATE, &sendInfo) == false)
		return nullptr;

	if (BroadcastAll(pSession, df_RES_ROOM_CREATE, &sendInfo) == false)
		return nullptr;

	wprintf_s(L"방 생성 [UserNO:%d][Room:%s][TotalRoom:%d]\n", pSession->_pUser->GetUserID(), pRoom->GetRoomName(), SINGLETON(CRoomManager)->GetRoomCount());

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

bool CRoomManager::RequestLobbyInfo(stSession* pSession)
{
	if (SendPacket(pSession, df_RES_ROOM_LIST) == false)
		return false;

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
	stResult_Room sendInfo;
	sendInfo._pRoom = nullptr;

	CUser* pUser = pSession->_pUser;

	// 이미 방에 들어가 있는 경우
	if (pUser->GetRoom() != nullptr)
	{
		sendInfo._result = df_RESULT_ROOM_ENTER_ETC;
		SendPacket(pSession, df_RES_ROOM_ENTER);
		return nullptr;
	}

	CRoom* pRoom = nullptr;
	for (auto iter = m_roomList.begin(); iter != m_roomList.end(); ++iter)
	{
		if ((*iter)->GetRoomID() == roomID)
		{
			(*iter)->EnterRoom(pUser);
			pUser->SetRoom(*iter);
			ExitLobby(pUser);

			pRoom = *iter;
			break;
		}
	}
	
	if (pRoom == nullptr)
	{
		sendInfo._result = df_RESULT_ROOM_ENTER_NOT;
		SendPacket(pSession, df_RES_ROOM_ENTER, &sendInfo);
		return nullptr;
	}

	sendInfo._pRoom = pRoom;
	sendInfo._result = df_RESULT_ROOM_ENTER_OK;
	if (SendPacket(pSession, df_RES_ROOM_ENTER, &sendInfo) == false)
		return nullptr;

	if (BroadcastRoom(pRoom, pSession, df_RES_USER_ENTER, pUser) == false)
		return nullptr;

	wprintf_s(L"방 입장 성공 [Room:%s][UserNO:%d]\n", pRoom->GetRoomName(), pUser->GetUserID());

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
				// 사람이 없는 경우 방을 파괴까지 진행
				if (SendPacket(pSession, df_RES_ROOM_DELETE, pRoom) == false)
					return false;

				if (BroadcastAll(pSession, df_RES_ROOM_DELETE, pRoom) == false)
					return false;

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