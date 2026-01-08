#pragma once

struct stSession;
class CUser
{
	friend class CRoom;
	friend class CLobby;
	friend class CUserManager;
public:
	CUser(stSession* pSession);
	~CUser();

	int GetUserID();
	const wchar_t* GetUserNickname();

	CRoom* GetRoom();
	void SetRoom(CRoom* pRoom);
	void EnterLobby();

	bool GetLoginState();
	bool SetLoginResult(char result, const wchar_t* nickname = nullptr);
	stSession* GetSession();

private:
	bool m_userLogin;
	int m_userID;
	wchar_t m_nickname[dfNICK_MAX_LEN];
	CRoom* m_pRoom;
	stSession* m_pSession;
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