#pragma once

class CUser;
class CRoom
{
public:
	CRoom(int ID, const wchar_t* roomName, short nameSize);
	~CRoom();

	bool EnterRoom(CUser* pUser);
	bool ExitRoom(CUser* pUser);
	bool DestroyRoom(stSession* pSession);
	wchar_t* GetRoomName();
	short GetRoomNameSize();
	int GetRoomID();
	CUser* GetNextUser();
	char GetUserCount();
	bool ChatRoom(stSession* pSession, wchar_t* msgWstr, short msgSize);	

private:
	int					m_roomID;
	wchar_t*			m_roomName;
	short				m_roomNameSize;
	list<CUser*>		m_userList;

	// 반복자용 변수
	bool				m_isNextUser;
	list<CUser*>::iterator m_nextUser;
};

class CRoomManager
{
private:
	CRoomManager();
	~CRoomManager();

	DECLARE_SINGLETON_IN_HEADER(CRoomManager)
public:
	CRoom*		CreateRoom(stSession* pSession, wchar_t* roomName, int nameSize);

	bool		EnterLobby(CUser* pUser);
	bool		RequestLobbyInfo(stSession* pSession);
	void		ExitLobby(CUser* pUser);
	bool		IsInLobby(CUser* pUser);
	
	CRoom*		EnterRoom(stSession* pSession, int roomID);
	bool		LeaveRoom(CUser* pUser);

	CUser*		GetNextUser();
	char		GetUserCount();

	CRoom*		GetNextRoom();
	short		GetRoomCount();
private:
	// 유저 관리용
	list<CUser*>		m_userList;

	// 반복자용 변수
	bool				m_isNextUser;
	list<CUser*>::iterator m_nextUser;

	// 방 관리용
	list<CRoom*> m_roomList;

	// 반복자용 변수
	bool					m_isNextRoom;
	list<CRoom*>::iterator	m_nextRoom;
};