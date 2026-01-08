#pragma once

/**
 * 서버에서 보낼 데이터를 포장하는 곳. 메시지를 패킹하는 함수
 */

class CPacket;
void SPackingLogin(CPacket* pPacket, void* infoPtr);

void SPackingRoomList(CPacket* pPacket, void* infoPtr);

void SPackingRoomCreate(CPacket* pPacket, void* infoPtr);

void SPackingRoomEnter(CPacket* pPacket, void* infoPtr);

void SPackingChat(CPacket* pPacket, void* infoPtr);

void SPackingRoomLeave(CPacket* pPacket, void* infoPtr);

void SPackingRoomDelete(CPacket* pPacket, void* infoPtr);

void SPackingUserEnter(CPacket* pPacket, void* infoPtr);