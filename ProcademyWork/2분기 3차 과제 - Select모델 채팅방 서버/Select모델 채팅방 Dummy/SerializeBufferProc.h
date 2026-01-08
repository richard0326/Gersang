#pragma once

/**
 * 클라에서 받은 데이터에 대한 컨텐츠 처리를 진행하는 곳.
 */
class CPacket;
struct stSession;
bool netPacketProc_Login(stSession* pSession, CPacket* pPacket);

bool netPacketProc_RoomList(stSession* pSession, CPacket* pPacket);

bool netPacketProc_RoomCreate(stSession* pSession, CPacket* pPacket);

bool netPacketProc_RoomEnter(stSession* pSession, CPacket* pPacket);

bool netPacketProc_Chat(stSession* pSession, CPacket* pPacket);

bool netPacketProc_RoomLeave(stSession* pSession, CPacket* pPacket);

bool netPacketProc_RoomDelete(stSession* pSession, CPacket* pPacket);

bool netPacketProc_UserEnter(stSession* pSession, CPacket* pPacket);