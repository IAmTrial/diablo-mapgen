
#pragma once

/**
 * State of the universe. (aka all global variables)
 */

#include "defs.h"
#include "enums.h"
#include "structs.h"
#include "types.h"

struct Universe {
	// drlg_l1.h
#ifdef HELLFIRE
	int UberRow;
	int UberCol;
	int dword_577368;
	int IsUberRoomOpened;
	int UberLeverRow;
	int UberLeverCol;
	int IsUberLeverActivated;
	int UberDiabloMonsterIndex;
#endif

	// drlg_l1.cpp
	/** Represents a tile ID map of twice the size, repeating each tile of the original map in blocks of 4. */
	BYTE L5dungeon[80][80];
	BYTE L5dflags[DMAXX][DMAXY];
	/** Specifies whether a single player quest DUN has been loaded. */
	BOOL L5setloadflag;
	/** Specifies whether to generate a horizontal room at position 1 in the Cathedral. */
	int HR1;
	/** Specifies whether to generate a horizontal room at position 2 in the Cathedral. */
	int HR2;
	/** Specifies whether to generate a horizontal room at position 3 in the Cathedral. */
	int HR3;
	/** Specifies whether to generate a vertical room at position 1 in the Cathedral. */
	BOOL VR1;
	/** Specifies whether to generate a vertical room at position 2 in the Cathedral. */
	BOOL VR2;
	/** Specifies whether to generate a vertical room at position 3 in the Cathedral. */
	BOOL VR3;
	/** Contains the contents of the single player quest DUN file. */
	BYTE *L5pSetPiece;

	// drlg_l2.cpp
#ifndef SPAWN
	int nSx1;
	int nSy1;
	int nSx2;
	int nSy2;
	int nRoomCnt;
	BYTE predungeon[DMAXX][DMAXY];
	ROOMNODE RoomList[81];
	HALLNODE *pHallList;
#endif  // SPAWN

	// drlg_l4.h
	int diabquad1x;
	int diabquad1y;
	int diabquad2x;
	int diabquad2y;
	int diabquad3x;
	int diabquad3y;
	int diabquad4x;
	int diabquad4y;

	// drlg_l4.cpp
#ifndef SPAWN
	BOOL hallok[20];
	int l4holdx;
	int l4holdy;
	int SP4x1;
	int SP4y1;
	int SP4x2;
	int SP4y2;
	BYTE L4dungeon[80][80];
	BYTE dung[20][20];
	//int dword_52A4DC;
#endif  // SPAWN
};
