#pragma once

//-------------------------------------------
// # define
//-------------------------------------------

/*
* player 이동 범위
*/
#define dfRANGE_MOVE_LEFT		0
#define dfRANGE_MOVE_TOP		0
#define dfRANGE_MOVE_RIGHT		6400
#define dfRANGE_MOVE_BOTTOM		6400

#define dfRANGE_WORLD_WIDTH		6400
#define dfRANGE_WORLD_HEIGHT	6400

/*
 player 이동 속도
*/
#define dfSPEED_PLAYER_X		6
#define dfSPEED_PLAYER_Y		4

/*
 tilemap 가로 세로 크기
*/
#define dfSECTOR_MAX_X		16
#define dfSECTOR_MAX_Y		20

#define dfSECTOR_WIDTH		(dfRANGE_WORLD_WIDTH / dfSECTOR_MAX_X)
#define dfSECTOR_HEIGHT		(dfRANGE_WORLD_HEIGHT / dfSECTOR_MAX_Y)

// Timeout 시간
#define dfNETWORK_PACKET_RECV_TIMEOUT	60000

// FPS time
#define dfNETWORK_FPS		25

// 에러 범위
#define dfERROR_RANGE		50

// 공격 데미지
#define dfDAMAGE_ATTACK1		3
#define dfDAMAGE_ATTACK2		6
#define dfDAMAGE_ATTACK3		9

/*
 player 8방향 이동
*/
#define dfACTION_MOVE_LL		0
#define dfACTION_MOVE_LU		1
#define dfACTION_MOVE_UU		2
#define dfACTION_MOVE_RU		3
#define dfACTION_MOVE_RR		4
#define dfACTION_MOVE_RD		5
#define dfACTION_MOVE_DD		6
#define dfACTION_MOVE_LD		7

/*
 player 공격
*/
#define dfACTION_ATTACK1		8
#define dfACTION_ATTACK2		9
#define dfACTION_ATTACK3		10

/*
 player 서 있기
*/
#define dfACTION_STAND			11

