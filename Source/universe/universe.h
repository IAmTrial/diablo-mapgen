
#pragma once

/**
 * State of the universe. (aka all global variables)
 */

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
