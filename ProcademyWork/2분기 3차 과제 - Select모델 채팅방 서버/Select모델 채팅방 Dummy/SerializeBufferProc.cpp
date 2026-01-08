#include "stdafx.h"
#include "SerializeBufferProc.h"
#include "UserMgr.h"
#include "RoomMgr.h"
#include "Packet.h"

//------------------------------------------------------------
// 2 Res 로그인                              
// 
// 1Byte	: 결과 (1:OK / 2:중복닉네임 / 3:사용자초과 / 4:기타오류)
// 4Byte	: 사용자 NO
//------------------------------------------------------------
bool netPacketProc_Login(stSession* pSession, CPacket* pPacket)
{
	if (!pPacket->CheckPacking() || pPacket->GetMsgType() != df_RES_LOGIN)
		return false;

	CUser* pUser = pSession->_pUser;
	if (pUser == nullptr)
	{
		return false;
	}

	char result;
	int UserID;
	(*pPacket) >> result >> UserID;
	if (result != df_RESULT_LOGIN_OK)
		return false;

	pUser->SetUserID(UserID);

	return true;
}

//------------------------------------------------------------
// 4 Res 대화방 리스트
//
//  2Byte	: 개수
//  {
//		4Byte : 방 No
//		2Byte : 방이름 byte size
//		Size  : 방이름 (유니코드)
//
//		1Byte : 참여인원		
//		{
//			WHCAR[15] : 닉네임
//		}
//	
//	}
//------------------------------------------------------------
bool netPacketProc_RoomList(stSession* pSession, CPacket* pPacket)
{
	if (!pPacket->CheckPacking() || pPacket->GetMsgType() != df_RES_ROOM_LIST)
		return false;

	short roomCount;
	(*pPacket) >> roomCount;
	for (int i = 0; i < roomCount; ++i)
	{
		int roomID;
		short roomNameSize;
		wchar_t roomName[100] = { 0, };
		(*pPacket) >> roomID >> roomNameSize;
		pPacket->GetData((char*)roomName, roomNameSize);

		char userCount;
		(*pPacket) >> userCount;
		for (int j = 0; j < userCount; ++j)
		{
			wchar_t nickname[dfNICK_MAX_LEN] = { 0, };
			pPacket->GetData((char*)nickname, dfNICK_MAX_LEN * sizeof(wchar_t));
		}
	}

	return true;
}

//------------------------------------------------------------
// 6 Res 대화방 생성 (수시로)
//
// 1Byte : 결과 (1:OK / 2:방이름 중복 / 3:개수초과 / 4:기타오류)
//
//
// 4Byte : 방 No
// 2Byte : 방제목 바이트 Size
// Size  : 방제목 (유니코드)
//------------------------------------------------------------
bool netPacketProc_RoomCreate(stSession* pSession, CPacket* pPacket)
{
	if (!pPacket->CheckPacking() || pPacket->GetMsgType() != df_RES_ROOM_CREATE)
		return false;

	char roomResult;
	int roomID;
	short roomNameSize;
	wchar_t roomName[100] = { 0, };
	(*pPacket) >> roomResult >> roomID >> roomNameSize;
	pPacket->GetData((char*)roomName, roomNameSize);

	if (roomResult != df_RESULT_ROOM_CREATE_OK)
		return false;

	return true;
}

//------------------------------------------------------------
//  8 Res 대화방 입장
//
//  1Byte : 결과 (1:OK / 2:방No 오류 / 3:인원초과 / 4:기타오류)
//
//  OK 의 경우에만 다음 전송
//	{
//		4Byte : 방 No
//		2Byte : 방제목 Size
//		Size  : 방제목 (유니코드)
//
//		1Byte : 참가인원
//		{
//			WCHAR[15] : 닉네임(유니코드)
//			4Byte     : 사용자No
//		}
//	}
//------------------------------------------------------------
bool netPacketProc_RoomEnter(stSession* pSession, CPacket* pPacket)
{
	if (!pPacket->CheckPacking() || pPacket->GetMsgType() != df_RES_ROOM_ENTER)
		return false;

	char roomResult;
	(*pPacket) >> roomResult;

	if (roomResult != df_RESULT_ROOM_ENTER_OK)
	{
		return false;
	}

	int roomID;
	short roomNameSize;
	wchar_t roomName[100] = { 0, };
	(*pPacket) >> roomID >> roomNameSize;
	pPacket->GetData((char*)roomName, roomNameSize);

	char userCount;
	(*pPacket) >> userCount;

	for (int j = 0; j < userCount; ++j)
	{
		wchar_t nickname[dfNICK_MAX_LEN] = { 0, };
		pPacket->GetData((char*)nickname, dfNICK_MAX_LEN * sizeof(wchar_t));
	}

	return true;
}

//------------------------------------------------------------
// 10 Res 채팅수신 (아무때나 올 수 있음)  (나에겐 오지 않음)
//
// 4Byte : 송신자 No
//
// 2Byte : 메시지 Size
// Size  : 대화내용(유니코드)
//------------------------------------------------------------
bool netPacketProc_Chat(stSession* pSession, CPacket* pPacket)
{
	if (!pPacket->CheckPacking() || pPacket->GetMsgType() != df_RES_CHAT)
		return false;

	short msgSize;
	wchar_t msgWstr[200];
	(*pPacket) >> msgSize;
	pPacket->GetData((char*)msgWstr, msgSize);

	return true;
}

//------------------------------------------------------------
// 12 Res 방퇴장 (수시)
//
// 4Byte : 사용자 No
//------------------------------------------------------------
bool netPacketProc_RoomLeave(stSession* pSession, CPacket* pPacket)
{
	if (!pPacket->CheckPacking() || pPacket->GetMsgType() != df_RES_ROOM_LEAVE)
		return false;

	int userID;
	(*pPacket) >> userID;

	return true;
}

//------------------------------------------------------------
// 13 Res 방삭제 (수시)
//
// 4Byte : 방 No
//------------------------------------------------------------
bool netPacketProc_RoomDelete(stSession* pSession, CPacket* pPacket)
{
	if (!pPacket->CheckPacking() || pPacket->GetMsgType() != df_RES_ROOM_DELETE)
		return false;

	int roomID;
	(*pPacket) >> roomID;

	return true;
}

//------------------------------------------------------------
// 14 Res 타 사용자 방 입장 (수시)
//
// WCHAR[15] : 닉네임(유니코드)
// 4Byte : 사용자 No
//------------------------------------------------------------
bool netPacketProc_UserEnter(stSession* pSession, CPacket* pPacket)
{
	if (!pPacket->CheckPacking() || pPacket->GetMsgType() != df_RES_USER_ENTER)
		return false;

	wchar_t nickname[dfNICK_MAX_LEN] = { 0, };
	pPacket->GetData((char*)nickname, dfNICK_MAX_LEN * sizeof(wchar_t));
	int userID;
	(*pPacket) >> userID;

	return true;
}