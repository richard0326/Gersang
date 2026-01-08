#pragma once

/**
 * 클라에서 보낼 데이터를 포장하는 곳. 메시지를 패킹하는 함수
 */

class CSerializeBuffer;
void mpMoveStart(CSerializeBuffer* pSerializeBuffer, char dir, unsigned short x, unsigned short y);

void mpMoveStop(CSerializeBuffer* pSerializeBuffer, char dir, unsigned short x, unsigned short y);

void mpAttack1(CSerializeBuffer* pSerializeBuffer, char dir, unsigned short x, unsigned short y);

void mpAttack2(CSerializeBuffer* pSerializeBuffer, char dir, unsigned short x, unsigned short y);

void mpAttack3(CSerializeBuffer* pSerializeBuffer, char dir, unsigned short x, unsigned short y);

void mpEcho(CSerializeBuffer* pSerializeBuffer, char dir, unsigned short x, unsigned short y);