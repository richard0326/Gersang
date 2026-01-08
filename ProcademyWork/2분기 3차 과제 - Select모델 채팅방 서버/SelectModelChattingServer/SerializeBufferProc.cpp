#include "stdafx.h"
#include "SerializeBufferProc.h"
#include "UserMgr.h"
#include "RoomMgr.h"
#include "Packet.h"

//------------------------------------------------------------
// 1 Req 로그인
//
//
// WCHAR[15]	: 닉네임 (유니코드)
//------------------------------------------------------------
bool netPacketProc_Login(stSession* pSession, CPacket* pPacket)
{
	if (!pPacket->CheckPacking() || pPacket->GetMsgType() != df_REQ_LOGIN)
		return false;
	
	CUser* pUser = pSession->_pUser;
	if (pUser == nullptr)
	{
		return false;
	}

	// 세션이 이미 로그인에 성공한 경우
	if(pUser->GetLoginState())
	{
		pUser->SetLoginResult(df_RESULT_LOGIN_ETC);
		return false;
	}

	// 닉네임의 크기가 생각보다 크게 온 경우
	if (pPacket->GetDataSize() > dfNICK_MAX_LEN * sizeof(wchar_t))
	{
		pUser->SetLoginResult(df_RESULT_LOGIN_MAX);
		return false;
	}

	wchar_t nickname[dfNICK_MAX_LEN];
	pPacket->GetData((char*)&nickname, dfNICK_MAX_LEN * sizeof(wchar_t));

	// 닉네임이 중복된 경우
	if (SINGLETON(CUserManager)->CheckNickname(nickname) == false)
	{
		pUser->SetLoginResult(df_RESULT_LOGIN_DNICK);
		return false;
	}

	// 성공적으로 로그인한 경우
	if (pUser->SetLoginResult(df_RESULT_LOGIN_OK, nickname) == false)
		return false;

	if (SINGLETON(CRoomManager)->EnterLobby(pUser) == false)
		return false;

	return true;
}

//------------------------------------------------------------
// 3 Req 대화방 리스트
//
//	None
//------------------------------------------------------------
bool netPacketProc_RoomList(stSession* pSession, CPacket* pPacket)
{
	if (!pPacket->CheckPacking() || pPacket->GetMsgType() != df_REQ_ROOM_LIST)
		return false;

	if (SINGLETON(CRoomManager)->RequestLobbyInfo(pSession) == false)
		return false;

	return true;
}

//------------------------------------------------------------
// 5 Req 대화방 생성
//
// 2Byte : 방제목 Size			유니코드 문자 바이트 길이 (널 제외)
// Size  : 방제목 (유니코드)
//------------------------------------------------------------
bool netPacketProc_RoomCreate(stSession* pSession, CPacket* pPacket)
{
	if (!pPacket->CheckPacking() || pPacket->GetMsgType() != df_REQ_ROOM_CREATE)
		return false;

	CUser* pUser = pSession->_pUser;
	short roomNameSize;
	wchar_t roomName[100] = { 0, };
	(*pPacket) >> roomNameSize;

	pPacket->GetData((char*)roomName, roomNameSize);

	CRoom* pRoom = SINGLETON(CRoomManager)->CreateRoom(pSession, roomName, roomNameSize);
	if(pRoom == nullptr)
		return false;

	return true;
}

//------------------------------------------------------------
// 7 Req 대화방 입장
//
//	4Byte : 방 No
//------------------------------------------------------------
bool netPacketProc_RoomEnter(stSession* pSession, CPacket* pPacket)
{
	if (!pPacket->CheckPacking() || pPacket->GetMsgType() != df_REQ_ROOM_ENTER)
		return false;

	CUser* pUser = pSession->_pUser;
	int roomID;
	(*pPacket) >> roomID;

	CRoom* pRoom = SINGLETON(CRoomManager)->EnterRoom(pSession, roomID);
	
	// 방이 없는 경우
	if (pRoom == nullptr)
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------
// 9 Req 채팅송신
//
// 2Byte : 메시지 Size
// Size  : 대화내용(유니코드)
//------------------------------------------------------------
bool netPacketProc_Chat(stSession* pSession, CPacket* pPacket)
{
	if (!pPacket->CheckPacking() || pPacket->GetMsgType() != df_REQ_CHAT)
		return false;

	short msgSize;
	wchar_t msgWstr[200];
	(*pPacket) >> msgSize;
	pPacket->GetData((char*)msgWstr, msgSize);

	CRoom* pRoom = pSession->_pUser->GetRoom();
	if (pRoom == nullptr)
		return false;

	if (pRoom->ChatRoom(pSession, msgWstr, msgSize) == false)
		return false;

	return true;
}

//------------------------------------------------------------
// 11 Req 방퇴장 
//
// None
//------------------------------------------------------------
bool netPacketProc_RoomLeave(stSession* pSession, CPacket* pPacket)
{
	if (!pPacket->CheckPacking() || pPacket->GetMsgType() != df_REQ_ROOM_LEAVE)
		return false;

	return SINGLETON(CRoomManager)->LeaveRoom(pSession->_pUser);
}