#include "stdafx.h"
#include "SerializeBufferPacking.h"
#include "UserMgr.h"
#include "RoomMgr.h"
#include "Packet.h"

//------------------------------------------------------------
// 1 Req 로그인
//
//
// WCHAR[15]	: 닉네임 (유니코드)
//------------------------------------------------------------
void SPackingLogin(CPacket* pPacket, void* infoPtr)
{
    pPacket->SetMsgType(df_REQ_LOGIN);
    pPacket->PutData((char*)infoPtr, dfNICK_MAX_LEN * sizeof(wchar_t));

    pPacket->FullPacked();
}

//------------------------------------------------------------
// 3 Req 대화방 리스트
//
//	None
//------------------------------------------------------------
void SPackingRoomList(CPacket* pPacket, void* infoPtr)
{
    pPacket->SetMsgType(df_REQ_ROOM_LIST);
    pPacket->FullPacked();
}

//------------------------------------------------------------
// 5 Req 대화방 생성
//
// 2Byte : 방제목 Size			유니코드 문자 바이트 길이 (널 제외)
// Size  : 방제목 (유니코드)
//------------------------------------------------------------
void SPackingRoomCreate(CPacket* pPacket, void* infoPtr)
{
    CRoom* pRoom = (CRoom*)infoPtr;
    pPacket->SetMsgType(df_REQ_ROOM_CREATE);

    (*pPacket) << (short)(pRoom->GetRoomNameSize() * sizeof(wchar_t));
    pPacket->PutData((char*)pRoom->GetRoomName(), pRoom->GetRoomNameSize() * sizeof(wchar_t));

    pPacket->FullPacked();
}

//------------------------------------------------------------
// 7 Req 대화방 입장
//
//	4Byte : 방 No
//------------------------------------------------------------
void SPackingRoomEnter(CPacket* pPacket, void* infoPtr)
{
    CRoom* pRoom = (CRoom*)infoPtr;
    pPacket->SetMsgType(df_REQ_ROOM_ENTER);

    (*pPacket) << pRoom->GetRoomID();

    pPacket->FullPacked();
}

//------------------------------------------------------------
// 9 Req 채팅송신
//
// 2Byte : 메시지 Size
// Size  : 대화내용(유니코드)
//------------------------------------------------------------
void SPackingChat(CPacket* pPacket, void* infoPtr)
{
    stUser_Chat* pUserChat = (stUser_Chat*)infoPtr;
    pPacket->SetMsgType(df_REQ_CHAT);

    (*pPacket) << pUserChat->_chatLen;
    pPacket->PutData((char*)pUserChat->_wchpChat, pUserChat->_chatLen);

    pPacket->FullPacked();
}

//------------------------------------------------------------
// 11 Req 방퇴장 
//
// None
//------------------------------------------------------------
void SPackingRoomLeave(CPacket* pPacket, void* infoPtr)
{
    pPacket->SetMsgType(df_REQ_ROOM_LEAVE);
    pPacket->FullPacked();
}