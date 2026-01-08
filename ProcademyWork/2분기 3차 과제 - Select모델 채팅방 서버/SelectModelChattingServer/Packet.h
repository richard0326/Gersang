#pragma once

class CPacket : public CSerializeBuffer
{
public:
	CPacket();
	~CPacket();

	char*	GetHeaderPtr();
	int		GetHeaderSize();

	void	SetMsgType(short msgType);
	short	GetMsgType();
	short	GetPayloadSize();
	
	void	FullPacked();
	bool	CheckPacking();

private:
	st_PACKET_HEADER m_stHeader;
};