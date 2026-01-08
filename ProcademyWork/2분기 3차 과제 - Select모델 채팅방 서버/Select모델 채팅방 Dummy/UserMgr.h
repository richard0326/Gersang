#pragma once

struct stSession;
class CUser
{
	friend class CRoom;
	friend class CLobby;
	friend class CUserManager;
public:
	enum {
		LEVEL_START,
		LEVEL_LOGIN,
		LEVEL_LOBBY,
		LEVEL_ROOM,
	};

	CUser(stSession* pSession);
	~CUser();

	void SetUserID(int ID);
	int GetUserID();

	bool SetNickname(const wchar_t* nickname);
	const wchar_t* GetUserNickname();

	void SetRoom(CRoom* pRoom);
	CRoom* GetRoom();

	void EnterLobby();

	stSession* GetSession();

	void SetUserLevel(int level);
	int GetUserLevel();

private:
	int m_userID;
	wchar_t m_nickname[dfNICK_MAX_LEN];
	CRoom* m_pRoom;
	stSession* m_pSession;
	int m_userLevel;
};

class CUserManager
{
private:
	CUserManager();
	~CUserManager();

	DECLARE_SINGLETON_IN_HEADER(CUserManager)

public:
	int CreateUser(stSession* pSession);
	bool CheckNickname(const wchar_t* nickname);
	void DeleteUser(CUser* pUser);

private:
	list<CUser*> m_userList;
};