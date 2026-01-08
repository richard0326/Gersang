#include "stdafx.h"
#include "SerializeBufferPacking.h"
#include "UserMgr.h"
#include "RoomMgr.h"
#include "Packet.h"

//------------------------------------------------------------
// 2 Res 로그인                              
// 
// 1Byte	: 결과 (1:OK / 2:중복닉네임 / 3:사용자초과 / 4:기타오류)
// 4Byte	: 사용자 NO
//------------------------------------------------------------
void SPackingLogin(CPacket* pPacket, void* infoPtr)
{
    stResult_ID* pResultID = (stResult_ID*)infoPtr;
    pPacket->SetMsgType(df_RES_LOGIN);

    (*pPacket) << pResultID->_result << pResultID->_ID;

    pPacket->FullPacked();
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
void SPackingRoomList(CPacket* pPacket, void* infoPtr)
{
    pPacket->SetMsgType(df_RES_ROOM_LIST);

    (*pPacket) << SINGLETON(CRoomManager)->GetRoomCount();

    while (true)
    {
        CRoom* pRoom = SINGLETON(CRoomManager)->GetNextRoom();
        if (pRoom == nullptr)
            break;

        (*pPacket) << pRoom->GetRoomID() << (short)(pRoom->GetRoomNameSize() * sizeof(wchar_t));
        pPacket->PutData((char*)pRoom->GetRoomName(), pRoom->GetRoomNameSize() * sizeof(wchar_t));

        while (true)
        {
            CUser* pUser = pRoom->GetNextUser();
            if (pUser == nullptr)
                break;

            pPacket->PutData((char*)pUser->GetUserNickname(), dfNICK_MAX_LEN * sizeof(wchar_t));
            (*pPacket) << pUser->GetUserID();
        }
    }

    pPacket->FullPacked();
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
void SPackingRoomCreate(CPacket* pPacket, void* infoPtr)
{
    stResult_Room* pResultRoom = (stResult_Room*)infoPtr;
    pPacket->SetMsgType(df_RES_ROOM_CREATE);

    CRoom* pRoom = pResultRoom->_pRoom;
    (*pPacket) << pResultRoom->_result << pRoom->GetRoomID() << (short)(pRoom->GetRoomNameSize() * sizeof(wchar_t));
    pPacket->PutData((char*)pRoom->GetRoomName(), pRoom->GetRoomNameSize() * sizeof(wchar_t));

    pPacket->FullPacked();
}

//------------------------------------------------------------
// 8 Res 대화방 입장
//
// 1Byte : 결과 (1:OK / 2:방No 오류 / 3:인원초과 / 4:기타오류)
//
// OK 의 경우에만 다음 전송
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
void SPackingRoomEnter(CPacket* pPacket, void* infoPtr)
{
    stResult_Room* pResultRoom = (stResult_Room*)infoPtr;
    pPacket->SetMsgType(df_RES_ROOM_ENTER);

    (*pPacket) << pResultRoom->_result;
    
    if (pResultRoom->_result == df_RESULT_ROOM_CREATE_OK)
    {
        CRoom* pRoom = pResultRoom->_pRoom;
        (*pPacket) << pRoom->GetRoomID() << (short)(pRoom->GetRoomNameSize() * sizeof(wchar_t));
        pPacket->PutData((char*)pRoom->GetRoomName(), pRoom->GetRoomNameSize() * sizeof(wchar_t));

        (*pPacket) << pRoom->GetUserCount();

        while (true)
        {
            CUser* pUser = pRoom->GetNextUser();
            if (pUser == nullptr)
                break;

            pPacket->PutData((char*)pUser->GetUserNickname(), dfNICK_MAX_LEN * sizeof(wchar_t));
            (*pPacket) << pUser->GetUserID();
        }
    }

    pPacket->FullPacked();
}

//------------------------------------------------------------
// 10 Res 채팅수신 (아무때나 올 수 있음)  (나에겐 오지 않음)
//
// 4Byte : 송신자 No
//
// 2Byte : 메시지 Size
// Size  : 대화내용(유니코드)
//------------------------------------------------------------
void SPackingChat(CPacket* pPacket, void* infoPtr)
{
    stUser_Chat* pUserChat = (stUser_Chat*)infoPtr;
    pPacket->SetMsgType(df_RES_CHAT);

    (*pPacket) << pUserChat->_pUser->GetUserID() << pUserChat->_chatLen;
    pPacket->PutData((char*)pUserChat->_wchpChat, pUserChat->_chatLen);

    pPacket->FullPacked();
}

//------------------------------------------------------------
// 12 Res 방퇴장 (수시)
//
// 4Byte : 사용자 No
//------------------------------------------------------------
void SPackingRoomLeave(CPacket* pPacket, void* infoPtr)
{
    CUser* pUser = (CUser*)infoPtr;
    pPacket->SetMsgType(df_RES_ROOM_LEAVE);

    (*pPacket) << pUser->GetUserID();

    pPacket->FullPacked();
}

//------------------------------------------------------------
// 13 Res 방삭제 (수시)
//
// 4Byte : 방 No
//------------------------------------------------------------
void SPackingRoomDelete(CPacket* pPacket, void* infoPtr)
{
    CRoom* pRoom = (CRoom*)infoPtr;
    pPacket->SetMsgType(df_RES_ROOM_DELETE);

    (*pPacket) << pRoom->GetRoomID();

    pPacket->FullPacked();
}

//------------------------------------------------------------
// 14 Res 타 사용자 입장 (수시)
//
// WCHAR[15] : 닉네임(유니코드)
// 4Byte : 사용자 No
//------------------------------------------------------------
void SPackingUserEnter(CPacket* pPacket, void* infoPtr)
{
    CUser* pUser = (CUser*)infoPtr;
    pPacket->SetMsgType(df_RES_USER_ENTER);

    pPacket->PutData((char*)pUser->GetUserNickname(), dfNICK_MAX_LEN * sizeof(wchar_t));
    (*pPacket) << pUser->GetUserID();

    pPacket->FullPacked();
}