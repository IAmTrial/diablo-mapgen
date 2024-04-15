/**
 * @file drlg_l3.cpp
 *
 * Implementation of the caves level generation algorithms.
 */

#include "Source/drlg_l3.h"

#ifndef SPAWN

#include <optional>

#include "structs.h"
#include "types.h"
#include "Source/drlg_l1.h"
#include "Source/engine.h"
#include "Source/lighting.h"
#include "Source/quests.h"
#include "Source/universe/universe.h"

/**
 * A lookup table for the 16 possible patterns of a 2x2 area,
 * where each cell either contains a SW wall or it doesn't.
 */
const BYTE L3ConvTbl[16] = { 8, 11, 3, 10, 1, 9, 12, 12, 6, 13, 4, 13, 2, 14, 5, 7 };
/** Miniset: Stairs up. */
const BYTE L3UP[] = {
	// clang-format off
	3, 3, // width, height

	 8,  8, 0, // search
	10, 10, 0,
	 7,  7, 0,

	51, 50, 0, // replace
	48, 49, 0,
	 0,  0, 0,
	// clang-format on
};
#ifdef HELLFIRE
const BYTE L6UP[] = {
	// clang-format off
	3, 3, // width, height

	 8,  8, 0, // search
	10, 10, 0,
	 7,  7, 0,

	20, 19, 0, // replace
	17, 18, 0,
	 0,  0, 0,
	// clang-format on
};
#endif
/** Miniset: Stairs down. */
const BYTE L3DOWN[] = {
	// clang-format off
	3, 3, // width, height

	8, 9, 7, // search
	8, 9, 7,
	0, 0, 0,

	0, 47, 0, // replace
	0, 46, 0,
	0,  0, 0,
	// clang-format on
};
#ifdef HELLFIRE
const BYTE L6DOWN[] = {
	// clang-format off
	3, 3, // width, height

	8, 9, 7, // search
	8, 9, 7,
	0, 0, 0,

	0, 16, 0, // replace
	0, 15, 0,
	0,  0, 0,
	// clang-format on
};
#endif
/** Miniset: Stairs up to town. */
const BYTE L3HOLDWARP[] = {
	// clang-format off
	3, 3, // width, height

	 8,  8, 0, // search
	10, 10, 0,
	 7,  7, 0,

	125, 125, 0, // replace
	125, 125, 0,
	  0,   0, 0,
	// clang-format on
};
#ifdef HELLFIRE
const BYTE L6HOLDWARP[] = {
	// clang-format off
	3, 3, // width, height

	 8,  8, 0, // search
	10, 10, 0,
	 7,  7, 0,

	24, 23, 0, // replace
	21, 22, 0,
	 0,  0, 0,
	// clang-format on
};
#endif
/** Miniset: Stalagmite white stalactite 1. */
const BYTE L3TITE1[] = {
	// clang-format off
	4, 4, // width, height

	7, 7, 7, 7, // search
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,

	0,  0,  0, 0, // replace
	0, 57, 58, 0,
	0, 56, 55, 0,
	0,  0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite white stalactite 2. */
const BYTE L3TITE2[] = {
	// clang-format off
	4, 4, // width, height

	7, 7, 7, 7, // search
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,

	0,  0,  0, 0, // replace
	0, 61, 62, 0,
	0, 60, 59, 0,
	0,  0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite white stalactite 3. */
const BYTE L3TITE3[] = {
	// clang-format off
	4, 4, // width, height

	7, 7, 7, 7, // search
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,

	0,  0,  0, 0, // replace
	0, 65, 66, 0,
	0, 64, 63, 0,
	0,  0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite white stalactite horizontal. */
const BYTE L3TITE6[] = {
	// clang-format off
	5, 4, // width, height

	7, 7, 7, 7, 7, // search
	7, 7, 7, 0, 7,
	7, 7, 7, 0, 7,
	7, 7, 7, 7, 7,

	0,  0,  0,  0, 0, // replace
	0, 77, 78,  0, 0,
	0, 76, 74, 75, 0,
	0,  0,  0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite white stalactite vertical. */
const BYTE L3TITE7[] = {
	// clang-format off
	4, 5, // width, height

	7, 7, 7, 7, // search
	7, 7, 0, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,

	0,  0,  0, 0, // replace
	0, 83,  0, 0,
	0, 82, 80, 0,
	0, 81, 79, 0,
	0,  0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite 1. */
const BYTE L3TITE8[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	0,  0, 0, // replace
	0, 52, 0,
	0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite 2. */
const BYTE L3TITE9[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	0,  0, 0, // replace
	0, 53, 0,
	0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite 3. */
const BYTE L3TITE10[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	0,  0, 0, // replace
	0, 54, 0,
	0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite 4. */
const BYTE L3TITE11[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	0,  0, 0, // replace
	0, 67, 0,
	0,  0, 0,
	// clang-format on
};
/** Miniset: Stalagmite on vertical wall. */
const BYTE L3TITE12[] = {
	// clang-format off
	2, 1, // width, height

	9, 7, // search

	68, 0, // replace
	// clang-format on
};
/** Miniset: Stalagmite on horizontal wall. */
const BYTE L3TITE13[] = {
	// clang-format off
	1, 2, // width, height

	10, // search
	 7,

	69, // replace
	 0,
	// clang-format on
};
/** Miniset: Cracked vertical wall 1. */
const BYTE L3CREV1[] = {
	// clang-format off
	2, 1, // width, height

	8, 7, // search

	84, 85, // replace
	// clang-format on
};
/** Miniset: Cracked vertical wall - north corner. */
const BYTE L3CREV2[] = {
	// clang-format off
	2, 1, // width, height

	8, 11, // search

	86, 87, // replace
	// clang-format on
};
/** Miniset: Cracked horizontal wall 1. */
const BYTE L3CREV3[] = {
	// clang-format off
	1, 2, // width, height

	 8, // search
	10,

	89, // replace
	88,
	// clang-format on
};
/** Miniset: Cracked vertical wall 2. */
const BYTE L3CREV4[] = {
	// clang-format off
	2, 1, // width, height

	8, 7, // search

	90, 91, // replace
	// clang-format on
};
/** Miniset: Cracked horizontal wall - north corner. */
const BYTE L3CREV5[] = {
	// clang-format off
	1, 2, // width, height

	 8, // search
	11,

	92, // replace
	93,
	// clang-format on
};
/** Miniset: Cracked horizontal wall 2. */
const BYTE L3CREV6[] = {
	// clang-format off
	1, 2, // width, height

	 8, // search
	10,

	95, // replace
	94,
	// clang-format on
};
/** Miniset: Cracked vertical wall - west corner. */
const BYTE L3CREV7[] = {
	// clang-format off
	2, 1, // width, height

	8, 7, // search

	96, 101, // replace
	// clang-format on
};
/** Miniset: Cracked horizontal wall - north. */
const BYTE L3CREV8[] = {
	// clang-format off
	1, 2, // width, height

	2, // search
	8,

	102, // replace
	 97,
	// clang-format on
};
/** Miniset: Cracked vertical wall - east corner. */
const BYTE L3CREV9[] = {
	// clang-format off
	2, 1, // width, height

	3, 8, // search

	103, 98, // replace
	// clang-format on
};
/** Miniset: Cracked vertical wall - west. */
const BYTE L3CREV10[] = {
	// clang-format off
	2, 1, // width, height

	4, 8, // search

	104, 99, // replace
	// clang-format on
};
/** Miniset: Cracked horizontal wall - south corner. */
const BYTE L3CREV11[] = {
	// clang-format off
	1, 2, // width, height

	6, // search
	8,

	105, // replace
	100,
	// clang-format on
};
/** Miniset: Replace broken wall with floor 1. */
const BYTE L3ISLE1[] = {
	// clang-format off
	2, 3, // width, height

	5, 14, // search
	4,  9,
	13, 12,

	7, 7, // replace
	7, 7,
	7, 7,
	// clang-format on
};
/** Miniset: Replace small wall with floor 2. */
const BYTE L3ISLE2[] = {
	// clang-format off
	3, 2, // width, height

	 5,  2, 14, // search
	13, 10, 12,

	7, 7, 7, // replace
	7, 7, 7,
	// clang-format on
};
/** Miniset: Replace small wall with lava 1. */
const BYTE L3ISLE3[] = {
	// clang-format off
	2, 3, // width, height

	 5, 14, // search
	 4,  9,
	13, 12,

	29, 30, // replace
	25, 28,
	31, 32,
	// clang-format on
};
/** Miniset: Replace small wall with lava 2. */
const BYTE L3ISLE4[] = {
	// clang-format off
	3, 2, // width, height

	 5,  2, 14, // search
	13, 10, 12,

	29, 26, 30, // replace
	31, 27, 32,
	// clang-format on
};
/** Miniset: Replace small wall with floor 3. */
const BYTE L3ISLE5[] = {
	// clang-format off
	2, 2, // width, height

	 5, 14, // search
	13, 12,

	7, 7, // replace
	7, 7,
	// clang-format on
};
/** Miniset: Use random floor tile 1. */
const BYTE L3XTRA1[] = {
	// clang-format off
	1, 1, // width, height

	7, // search

	106, // replace
	// clang-format on
};
/** Miniset: Use random floor tile 2. */
const BYTE L3XTRA2[] = {
	// clang-format off
	1, 1, // width, height

	7, // search

	107, // replace
	// clang-format on
};
/** Miniset: Use random floor tile 3. */
const BYTE L3XTRA3[] = {
	// clang-format off
	1, 1, // width, height

	7, // search

	108, // replace
	// clang-format on
};
/** Miniset: Use random horizontal wall tile. */
const BYTE L3XTRA4[] = {
	// clang-format off
	1, 1, // width, height

	9, // search

	109, // replace
	// clang-format on
};
/** Miniset: Use random vertical wall tile. */
const BYTE L3XTRA5[] = {
	// clang-format off
	1, 1, // width, height

	10, // search

	110, // replace
	// clang-format on
};

/** Miniset: Anvil of Fury island. */
const BYTE L3ANVIL[] = {
	// clang-format off
	11, 11, // width, height

	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, // search
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,

	0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, // replace
	0,  0, 29, 26, 26, 26, 26, 26, 30,  0, 0,
	0, 29, 34, 33, 33, 37, 36, 33, 35, 30, 0,
	0, 25, 33, 37, 27, 32, 31, 36, 33, 28, 0,
	0, 25, 37, 32,  7,  7,  7, 31, 27, 32, 0,
	0, 25, 28,  7,  7,  7,  7,  2,  2,  2, 0,
	0, 25, 35, 30,  7,  7,  7, 29, 26, 30, 0,
	0, 25, 33, 35, 26, 30, 29, 34, 33, 28, 0,
	0, 31, 36, 33, 33, 35, 34, 33, 37, 32, 0,
	0,  0, 31, 27, 27, 27, 27, 27, 32,  0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,
	// clang-format on
};
#ifdef HELLFIRE
const BYTE byte_48A76C[] = { 1, 1, 8, 25 };
const BYTE byte_48A770[] = { 1, 1, 8, 26 };
const BYTE byte_48A774[] = { 1, 1, 8, 27 };
const BYTE byte_48A778[] = { 1, 1, 8, 28 };
const BYTE byte_48A77C[] = { 1, 1, 7, 29 };
const BYTE byte_48A780[] = { 1, 1, 7, 30 };
const BYTE byte_48A784[] = { 1, 1, 7, 31 };
const BYTE byte_48A788[] = { 1, 1, 7, 32 };
const BYTE byte_48A790[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	0,   0, 0, // replace
	0, 126, 0,
	0,   0, 0,
	// clang-format on
};
const BYTE byte_48A7A8[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	0,   0, 0, // replace
	0, 124, 0,
	0,   0, 0,
	// clang-format on
};
const BYTE byte_48A7BC[] = { 1, 1, 9, 33 };
const BYTE byte_48A7C0[] = { 1, 1, 9, 34 };
const BYTE byte_48A7C4[] = { 1, 1, 9, 35 };
const BYTE byte_48A7C8[] = { 1, 1, 9, 36 };
const BYTE byte_48A7CC[] = { 1, 1, 9, 37 };
const BYTE byte_48A7D0[] = { 1, 1, 11, 38 };
const BYTE byte_48A7D4[] = { 1, 1, 10, 39 };
const BYTE byte_48A7D8[] = { 1, 1, 10, 40 };
const BYTE byte_48A7DC[] = { 1, 1, 10, 41 };
const BYTE byte_48A7E0[] = { 1, 1, 10, 42 };
const BYTE byte_48A7E4[] = { 1, 1, 10, 43 };
const BYTE byte_48A7E8[] = { 1, 1, 11, 44 };
const BYTE byte_48A7EC[] = { 1, 1, 9, 45 };
const BYTE byte_48A7F0[] = { 1, 1, 9, 46 };
const BYTE byte_48A7F4[] = { 1, 1, 10, 47 };
const BYTE byte_48A7F8[] = { 1, 1, 10, 48 };
const BYTE byte_48A7FC[] = { 1, 1, 11, 49 };
const BYTE byte_48A800[] = { 1, 1, 11, 50 };
const BYTE byte_48A808[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	67,  0, 0, // replace
	66, 51, 0,
	 0,  0, 0,
	// clang-format on
};
const BYTE byte_48A820[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	69,  0, 0, // replace
	68, 52, 0,
	 0,  0, 0,
	// clang-format on
};
const BYTE byte_48A838[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	70,  0, 0, // replace
	71, 53, 0,
	 0,  0, 0,
	// clang-format on
};
const BYTE byte_48A850[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	73,  0, 0, // replace
	72, 54, 0,
	 0,  0, 0,
	// clang-format on
};
const BYTE byte_48A868[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	75,  0, 0, // replace
	74, 55, 0,
	 0,  0, 0,
	// clang-format on
};
const BYTE byte_48A880[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	77,  0, 0, // replace
	76, 56, 0,
	 0,  0, 0,
	// clang-format on
};
const BYTE byte_48A898[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	79,  0, 0, // replace
	78, 57, 0,
	 0,  0, 0,
	// clang-format on
};
const BYTE byte_48A8B0[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	81,  0, 0, // replace
	80, 58, 0,
	 0,  0, 0,
	// clang-format on
};
const BYTE byte_48A8C8[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	83,  0, 0, // replace
	82, 59, 0,
	 0,  0, 0,
	// clang-format on
};
const BYTE byte_48A8E0[] = {
	// clang-format off
	3, 3, // width, height

	7, 7, 7, // search
	7, 7, 7,
	7, 7, 7,

	84,  0, 0, // replace
	85, 60, 0,
	 0,  0, 0,
	// clang-format on
};
const BYTE L6ISLE1[] = {
	// clang-format off
	2, 3, // width, height

	 5, 14, // search
	 4,  9,
	13, 12,

	7, 7, // replace
	7, 7,
	7, 7,
	// clang-format on
};
const BYTE L6ISLE2[] = {
	// clang-format off
	3, 2, // width, height

	 5,  2, 14, // search
	13, 10, 12,

	7, 7, 7, // replace
	7, 7, 7,
	// clang-format on
};
const BYTE L6ISLE3[] = {
	// clang-format off
	2, 3, // width, height

	 5, 14, // search
	 4,  9,
	13, 12,

	107, 115, // replace
	119, 122,
	131, 123,
	// clang-format on
};
const BYTE L6ISLE4[] = {
	// clang-format off
	3, 2, // width, height

	 5,  2, 14, // search
	13, 10, 12,

	107, 120, 115, // replace
	131, 121, 123,
	// clang-format on
};
const BYTE L6ISLE5[] = {
	// clang-format off
	2, 2, // width, height

	 5, 14, // search
	13, 12,

	7, 7, // replace
	7, 7,
	// clang-format on
};
const BYTE byte_48A948[] = {
	// clang-format off
	4, 4, // width, height

	7, 7, 7, 7, // search
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,

	7,   7,   7, 7, // replace
	7, 107, 115, 7,
	7, 131, 123, 7,
	7,   7,   7, 7,
	// clang-format on
};
const BYTE byte_48A970[] = {
	// clang-format off
	4, 4, // width, height

	7, 7, 7, 7, // search
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,

	7,   7,   7, 7, // replace
	7,   7, 108, 7,
	7, 109, 112, 7,
	7,   7,   7, 7,
	// clang-format on
};
const BYTE byte_48A998[] = {
	// clang-format off
	4, 5, // width, height

	7, 7, 7, 7, // search
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,

	7,   7,   7, 7, // replace
	7, 107, 115, 7,
	7, 119, 122, 7,
	7, 131, 123, 7,
	7,   7,   7, 7,
	// clang-format on
};
const BYTE byte_48A9C8[] = {
	// clang-format off
	4, 5, // width, height

	7, 7, 7, 7, // search
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,
	7, 7, 7, 7,

	7,   7,   7, 7, // replace
	7, 126, 108, 7,
	7,   7, 117, 7,
	7, 109, 112, 7,
	7,   7,   7, 7,
	// clang-format on
};
#endif

static void InitL3Dungeon(Universe& universe)
{
	int i, j;

	memset(universe.dungeon, 0, sizeof(universe.dungeon));

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			SetDungeon(universe, i, j, 0);
			universe.dflags[i][j] = 0;
		}
	}
}

static BOOL DRLG_L3FillRoom(Universe& universe, int x1, int y1, int x2, int y2)
{
	int i, j, v;

	if (x1 <= 1 || x2 >= 34 || y1 <= 1 || y2 >= 38) {
		return FALSE;
	}

	v = 0;
	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			v += GetDungeon(universe, i, j);
		}
	}

	if (v != 0) {
		return FALSE;
	}

	for (j = y1 + 1; j < y2; j++) {
		for (i = x1 + 1; i < x2; i++) {
			SetDungeon(universe, i, j, 1);
		}
	}
	for (j = y1; j <= y2; j++) {
		if (random_(universe, 0, 2) != 0) {
			SetDungeon(universe, x1, j, 1);
		}
		if (random_(universe, 0, 2) != 0) {
			SetDungeon(universe, x2, j, 1);
		}
	}
	for (i = x1; i <= x2; i++) {
		if (random_(universe, 0, 2) != 0) {
			SetDungeon(universe, i, y1, 1);
		}
		if (random_(universe, 0, 2) != 0) {
			SetDungeon(universe, i, y2, 1);
		}
	}

	return TRUE;
}

static void DRLG_L3CreateBlock(Universe& universe, int x, int y, int obs, int dir)
{
	int blksizex, blksizey, x1, y1, x2, y2;
	int contflag;

	blksizex = random_(universe, 0, 2) + 3;
	blksizey = random_(universe, 0, 2) + 3;

	if (dir == 0) {
		y2 = y - 1;
		y1 = y2 - blksizey;
		if (blksizex < obs) {
			x1 = random_(universe, 0, blksizex) + x;
		}
		if (blksizex == obs) {
			x1 = x;
		}
		if (blksizex > obs) {
			x1 = x - random_(universe, 0, blksizex);
		}
		x2 = blksizex + x1;
	}
	if (dir == 3) {
		x2 = x - 1;
		x1 = x2 - blksizex;
		if (blksizey < obs) {
			y1 = random_(universe, 0, blksizey) + y;
		}
		if (blksizey == obs) {
			y1 = y;
		}
		if (blksizey > obs) {
			y1 = y - random_(universe, 0, blksizey);
		}
		y2 = y1 + blksizey;
	}
	if (dir == 2) {
		y1 = y + 1;
		y2 = y1 + blksizey;
		if (blksizex < obs) {
			x1 = random_(universe, 0, blksizex) + x;
		}
		if (blksizex == obs) {
			x1 = x;
		}
		if (blksizex > obs) {
			x1 = x - random_(universe, 0, blksizex);
		}
		x2 = blksizex + x1;
	}
	if (dir == 1) {
		x1 = x + 1;
		x2 = x1 + blksizex;
		if (blksizey < obs) {
			y1 = random_(universe, 0, blksizey) + y;
		}
		if (blksizey == obs) {
			y1 = y;
		}
		if (blksizey > obs) {
			y1 = y - random_(universe, 0, blksizey);
		}
		y2 = y1 + blksizey;
	}

	if (DRLG_L3FillRoom(universe, x1, y1, x2, y2) == TRUE) {
		contflag = random_(universe, 0, 4);
		if (contflag != 0 && dir != 2) {
			DRLG_L3CreateBlock(universe, x1, y1, blksizey, 0);
		}
		if (contflag != 0 && dir != 3) {
			DRLG_L3CreateBlock(universe, x2, y1, blksizex, 1);
		}
		if (contflag != 0 && dir != 0) {
			DRLG_L3CreateBlock(universe, x1, y2, blksizey, 2);
		}
		if (contflag != 0 && dir != 1) {
			DRLG_L3CreateBlock(universe, x1, y1, blksizex, 3);
		}
	}
}

static void DRLG_L3FloorArea(Universe& universe, int x1, int y1, int x2, int y2)
{
	int i, j;

	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			universe.dungeon[i][j] = 1;
		}
	}
}

static void DRLG_L3FillDiags(Universe& universe)
{
	int i, j, v;

	for (j = 0; j < DMAXY - 1; j++) {
		for (i = 0; i < DMAXX - 1; i++) {
			v = universe.dungeon[i + 1][j + 1] + 2 * universe.dungeon[i][j + 1] + 4 * universe.dungeon[i + 1][j] + 8 * universe.dungeon[i][j];
			if (v == 6) {
				if (random_(universe, 0, 2) == 0) {
					universe.dungeon[i][j] = 1;
				} else {
					universe.dungeon[i + 1][j + 1] = 1;
				}
			}
			if (v == 9) {
				if (random_(universe, 0, 2) == 0) {
					universe.dungeon[i + 1][j] = 1;
				} else {
					universe.dungeon[i][j + 1] = 1;
				}
			}
		}
	}
}

static void DRLG_L3FillSingles(Universe& universe)
{
	int i, j;

	for (j = 1; j < DMAXY - 1; j++) {
		for (i = 1; i < DMAXX - 1; i++) {
			if (universe.dungeon[i][j] == 0
			    && universe.dungeon[i][j - 1] + universe.dungeon[i - 1][j - 1] + universe.dungeon[i + 1][j - 1] == 3
			    && universe.dungeon[i + 1][j] + universe.dungeon[i - 1][j] == 2
			    && universe.dungeon[i][j + 1] + universe.dungeon[i - 1][j + 1] + universe.dungeon[i + 1][j + 1] == 3) {
				universe.dungeon[i][j] = 1;
			}
		}
	}
}

static void DRLG_L3FillStraights(Universe& universe)
{
	int i, j, xc, xs, yc, ys, k, rv;

	for (j = 0; j < DMAXY - 1; j++) {
		xs = 0;
		for (i = 0; i < 37; i++) {
			if (universe.dungeon[i][j] == 0 && universe.dungeon[i][j + 1] == 1) {
				if (xs == 0) {
					xc = i;
				}
				xs++;
			} else {
				if (xs > 3 && random_(universe, 0, 2) != 0) {
					for (k = xc; k < i; k++) {
						rv = random_(universe, 0, 2);
						universe.dungeon[k][j] = rv;
					}
				}
				xs = 0;
			}
		}
	}
	for (j = 0; j < DMAXY - 1; j++) {
		xs = 0;
		for (i = 0; i < 37; i++) {
			if (universe.dungeon[i][j] == 1 && universe.dungeon[i][j + 1] == 0) {
				if (xs == 0) {
					xc = i;
				}
				xs++;
			} else {
				if (xs > 3 && random_(universe, 0, 2) != 0) {
					for (k = xc; k < i; k++) {
						rv = random_(universe, 0, 2);
						universe.dungeon[k][j + 1] = rv;
					}
				}
				xs = 0;
			}
		}
	}
	for (i = 0; i < DMAXX - 1; i++) {
		ys = 0;
		for (j = 0; j < 37; j++) {
			if (universe.dungeon[i][j] == 0 && universe.dungeon[i + 1][j] == 1) {
				if (ys == 0) {
					yc = j;
				}
				ys++;
			} else {
				if (ys > 3 && random_(universe, 0, 2) != 0) {
					for (k = yc; k < j; k++) {
						rv = random_(universe, 0, 2);
						universe.dungeon[i][k] = rv;
					}
				}
				ys = 0;
			}
		}
	}
	for (i = 0; i < DMAXX - 1; i++) {
		ys = 0;
		for (j = 0; j < 37; j++) {
			if (universe.dungeon[i][j] == 1 && universe.dungeon[i + 1][j] == 0) {
				if (ys == 0) {
					yc = j;
				}
				ys++;
			} else {
				if (ys > 3 && random_(universe, 0, 2) != 0) {
					for (k = yc; k < j; k++) {
						rv = random_(universe, 0, 2);
						universe.dungeon[i + 1][k] = rv;
					}
				}
				ys = 0;
			}
		}
	}
}

static void DRLG_L3Edges(Universe& universe)
{
	int i, j;

	for (j = 0; j < DMAXY; j++) {
		SetDungeon(universe, DMAXX - 1, j, 0);
	}
	for (i = 0; i < DMAXX; i++) {
		SetDungeon(universe, i, DMAXY - 1, 0);
	}
}

static int DRLG_L3GetFloorArea(Universe& universe)
{
	int i, j, gfa;

	gfa = 0;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			gfa += GetDungeon(universe, i, j);
		}
	}

	return gfa;
}

static void DRLG_L3MakeMegas(Universe& universe)
{
	int i, j, v, rv;

	for (j = 0; j < DMAXY - 1; j++) {
		for (i = 0; i < DMAXX - 1; i++) {
			v = GetDungeon(universe, i + 1, j + 1) + 2 * GetDungeon(universe, i, j + 1) + 4 * GetDungeon(universe, i + 1, j) + 8 * GetDungeon(universe, i, j);
			if (v == 6) {
				rv = random_(universe, 0, 2);
				if (rv == 0) {
					v = 12;
				} else {
					v = 5;
				}
			}
			if (v == 9) {
				rv = random_(universe, 0, 2);
				if (rv == 0) {
					v = 13;
				} else {
					v = 14;
				}
			}
			SetDungeon(universe, i, j, L3ConvTbl[v]);
		}
		SetDungeon(universe, DMAXX - 1, j, 8);
	}
	for (i = 0; i < DMAXX; i++) {
		SetDungeon(universe, i, DMAXY - 1, 8);
	}
}

static void DRLG_L3River(Universe& universe)
{
	int rx, ry, px, py, dir, pdir, nodir, nodir2, dircheck;
	int river[3][100];
	int rivercnt, riveramt;
	int i, trys, found, bridge, lpcnt;
	BOOL bail;

	rivercnt = 0;
	bail = FALSE;
	trys = 0;
	/// BUGFIX: pdir is uninitialized, add code `pdir = -1;`
	pdir = -1;

	while (trys < 200 && rivercnt < 4) {
		bail = FALSE;
		while (!bail && trys < 200) {
			trys++;
			rx = 0;
			ry = 0;
			i = 0;
			// BUGFIX: Replace with `(ry >= DMAXY || GetDungeon(universe, rx, ry) < 25 || GetDungeon(universe, rx, ry) > 28) && i < 100`
			while ((GetDungeon(universe, rx, ry) < 25 || GetDungeon(universe, rx, ry) > 28) && i < 100) {
				rx = random_(universe, 0, DMAXX);
				ry = random_(universe, 0, DMAXY);
				i++;
				// BUGFIX: Move `ry < DMAXY` check before universe.dungeon checks
				while ((GetDungeon(universe, rx, ry) < 25 || GetDungeon(universe, rx, ry) > 28) && ry < DMAXY) {
					rx++;
					if (rx >= DMAXX) {
						rx = 0;
						ry++;
					}
				}
			}
			// BUGFIX: Continue if `ry >= DMAXY`
			if (i >= 100) {
				return;
			}
			switch (GetDungeon(universe, rx, ry)) {
			case 25:
				dir = 3;
				nodir = 2;
				river[2][0] = 40;
				break;
			case 26:
				dir = 0;
				nodir = 1;
				river[2][0] = 38;
				break;
			case 27:
				dir = 1;
				nodir = 0;
				river[2][0] = 41;
				break;
			case 28:
				dir = 2;
				nodir = 3;
				river[2][0] = 39;
				break;
			}
			river[0][0] = rx;
			river[1][0] = ry;
			riveramt = 1;
			nodir2 = 4;
			dircheck = 0;
			while (dircheck < 4 && riveramt < 100) {
				px = rx;
				py = ry;
				if (dircheck == 0) {
					dir = random_(universe, 0, 4);
				} else {
					dir = (dir + 1) & 3;
				}
				dircheck++;
				while (dir == nodir || dir == nodir2) {
					dir = (dir + 1) & 3;
					dircheck++;
				}
				if (dir == 0 && ry > 0) {
					ry--;
				}
				if (dir == 1 && ry < DMAXY) {
					ry++;
				}
				if (dir == 2 && rx < DMAXX) {
					rx++;
				}
				if (dir == 3 && rx > 0) {
					rx--;
				}
				if (GetDungeon(universe, rx, ry) == 7) {
					dircheck = 0;
					if (dir < 2) {
						river[2][riveramt] = (BYTE)random_(universe, 0, 2) + 17;
					}
					if (dir > 1) {
						river[2][riveramt] = (BYTE)random_(universe, 0, 2) + 15;
					}
					river[0][riveramt] = rx;
					river[1][riveramt] = ry;
					riveramt++;
					if (dir == 0 && pdir == 2 || dir == 3 && pdir == 1) {
						if (riveramt > 2) {
							river[2][riveramt - 2] = 22;
						}
						if (dir == 0) {
							nodir2 = 1;
						} else {
							nodir2 = 2;
						}
					}
					if (dir == 0 && pdir == 3 || dir == 2 && pdir == 1) {
						if (riveramt > 2) {
							river[2][riveramt - 2] = 21;
						}
						if (dir == 0) {
							nodir2 = 1;
						} else {
							nodir2 = 3;
						}
					}
					if (dir == 1 && pdir == 2 || dir == 3 && pdir == 0) {
						if (riveramt > 2) {
							river[2][riveramt - 2] = 20;
						}
						if (dir == 1) {
							nodir2 = 0;
						} else {
							nodir2 = 2;
						}
					}
					if (dir == 1 && pdir == 3 || dir == 2 && pdir == 0) {
						if (riveramt > 2) {
							river[2][riveramt - 2] = 19;
						}
						if (dir == 1) {
							nodir2 = 0;
						} else {
							nodir2 = 3;
						}
					}
					pdir = dir;
				} else {
					rx = px;
					ry = py;
				}
			}
			// BUGFIX: Check `ry >= 2`
			if (dir == 0 && GetDungeon(universe, rx, ry - 1) == 10 && GetDungeon(universe, rx, ry - 2) == 8) {
				river[0][riveramt] = rx;
				river[1][riveramt] = ry - 1;
				river[2][riveramt] = 24;
				if (pdir == 2) {
					river[2][riveramt - 1] = 22;
				}
				if (pdir == 3) {
					river[2][riveramt - 1] = 21;
				}
				bail = TRUE;
			}
			// BUGFIX: Check `ry + 2 < DMAXY`
			if (dir == 1 && GetDungeon(universe, rx, ry + 1) == 2 && GetDungeon(universe, rx, ry + 2) == 8) {
				river[0][riveramt] = rx;
				river[1][riveramt] = ry + 1;
				river[2][riveramt] = 42;
				if (pdir == 2) {
					river[2][riveramt - 1] = 20;
				}
				if (pdir == 3) {
					river[2][riveramt - 1] = 19;
				}
				bail = TRUE;
			}
			// BUGFIX: Check `rx + 2 < DMAXX`
			if (dir == 2 && GetDungeon(universe, rx + 1, ry) == 4 && GetDungeon(universe, rx + 2, ry) == 8) {
				river[0][riveramt] = rx + 1;
				river[1][riveramt] = ry;
				river[2][riveramt] = 43;
				if (pdir == 0) {
					river[2][riveramt - 1] = 19;
				}
				if (pdir == 1) {
					river[2][riveramt - 1] = 21;
				}
				bail = TRUE;
			}
			// BUGFIX: Check `rx >= 2`
			if (dir == 3 && GetDungeon(universe, rx - 1, ry) == 9 && GetDungeon(universe, rx - 2, ry) == 8) {
				river[0][riveramt] = rx - 1;
				river[1][riveramt] = ry;
				river[2][riveramt] = 23;
				if (pdir == 0) {
					river[2][riveramt - 1] = 20;
				}
				if (pdir == 1) {
					river[2][riveramt - 1] = 22;
				}
				bail = TRUE;
			}
		}
		if (bail == TRUE && riveramt < 7) {
			bail = FALSE;
		}
		if (bail == TRUE) {
			found = 0;
			lpcnt = 0;
			while (found == 0 && lpcnt < 30) {
				lpcnt++;
				bridge = random_(universe, 0, riveramt);
				if ((river[2][bridge] == 15 || river[2][bridge] == 16)
				    && GetDungeon(universe, river[0][bridge], river[1][bridge] - 1) == 7
				    && GetDungeon(universe, river[0][bridge], river[1][bridge] + 1) == 7) {
					found = 1;
				}
				if ((river[2][bridge] == 17 || river[2][bridge] == 18)
				    && GetDungeon(universe, river[0][bridge] - 1, river[1][bridge]) == 7
				    && GetDungeon(universe, river[0][bridge] + 1, river[1][bridge]) == 7) {
					found = 2;
				}
				for (i = 0; i < riveramt && found != 0; i++) {
					if (found == 1
					    && (river[1][bridge] - 1 == river[1][i] || river[1][bridge] + 1 == river[1][i])
					    && river[0][bridge] == river[0][i]) {
						found = 0;
					}
					if (found == 2
					    && (river[0][bridge] - 1 == river[0][i] || river[0][bridge] + 1 == river[0][i])
					    && river[1][bridge] == river[1][i]) {
						found = 0;
					}
				}
			}
			if (found != 0) {
				if (found == 1) {
					river[2][bridge] = 44;
				} else {
					river[2][bridge] = 45;
				}
				rivercnt++;
				for (bridge = 0; bridge <= riveramt; bridge++) {
					SetDungeon(universe, river[0][bridge], river[1][bridge], river[2][bridge]);
				}
			} else {
				bail = FALSE;
			}
		}
	}
}

static BOOL DRLG_L3Spawn(Universe& universe, int x, int y, int *totarea);

static BOOL DRLG_L3SpawnEdge(Universe& universe, int x, int y, int *totarea)
{
	BYTE i;
	const static BYTE spawntable[15] = { 0x00, 0x0A, 0x43, 0x05, 0x2c, 0x06, 0x09, 0x00, 0x00, 0x1c, 0x83, 0x06, 0x09, 0x0A, 0x05 };

	if (*totarea > 40) {
		return TRUE;
	}
	if (x < 0 || y < 0 || x >= DMAXX || y >= DMAXY) {
		return TRUE;
	}
	if (GetDungeon(universe, x, y) & 0x80) {
		return FALSE;
	}
	if (GetDungeon(universe, x, y) > 15) {
		return TRUE;
	}

	i = GetDungeon(universe, x, y);
	SetDungeon(universe, x, y, GetDungeon(universe, x, y) | 0x80);
	*totarea += 1;

	if (spawntable[i] & 8 && DRLG_L3SpawnEdge(universe, x, y - 1, totarea) == TRUE) {
		return TRUE;
	}
	if (spawntable[i] & 4 && DRLG_L3SpawnEdge(universe, x, y + 1, totarea) == TRUE) {
		return TRUE;
	}
	if (spawntable[i] & 2 && DRLG_L3SpawnEdge(universe, x + 1, y, totarea) == TRUE) {
		return TRUE;
	}
	if (spawntable[i] & 1 && DRLG_L3SpawnEdge(universe, x - 1, y, totarea) == TRUE) {
		return TRUE;
	}
	if (spawntable[i] & 0x80 && DRLG_L3Spawn(universe, x, y - 1, totarea) == TRUE) {
		return TRUE;
	}
	if (spawntable[i] & 0x40 && DRLG_L3Spawn(universe, x, y + 1, totarea) == TRUE) {
		return TRUE;
	}
	if (spawntable[i] & 0x20 && DRLG_L3Spawn(universe, x + 1, y, totarea) == TRUE) {
		return TRUE;
	}
	if (spawntable[i] & 0x10 && DRLG_L3Spawn(universe, x - 1, y, totarea) == TRUE) {
		return TRUE;
	}

	return FALSE;
}

static BOOL DRLG_L3Spawn(Universe& universe, int x, int y, int *totarea)
{
	BYTE i;
	const static BYTE spawntable[15] = { 0x00, 0x0A, 0x03, 0x05, 0x0C, 0x06, 0x09, 0x00, 0x00, 0x0C, 0x03, 0x06, 0x09, 0x0A, 0x05 };

	if (*totarea > 40) {
		return TRUE;
	}
	if (x < 0 || y < 0 || x >= DMAXX || y >= DMAXY) {
		return TRUE;
	}
	if (GetDungeon(universe, x, y) & 0x80) {
		return FALSE;
	}
	if (GetDungeon(universe, x, y) > 15) {
		return TRUE;
	}

	i = GetDungeon(universe, x, y);
	SetDungeon(universe, x, y, GetDungeon(universe, x, y) | 0x80);
	*totarea += 1;

	if (i != 8) {
		if (spawntable[i] & 8 && DRLG_L3SpawnEdge(universe, x, y - 1, totarea) == TRUE) {
			return TRUE;
		}
		if (spawntable[i] & 4 && DRLG_L3SpawnEdge(universe, x, y + 1, totarea) == TRUE) {
			return TRUE;
		}
		if (spawntable[i] & 2 && DRLG_L3SpawnEdge(universe, x + 1, y, totarea) == TRUE) {
			return TRUE;
		}
		if (spawntable[i] & 1 && DRLG_L3SpawnEdge(universe, x - 1, y, totarea) == TRUE) {
			return TRUE;
		}
	} else {
		if (DRLG_L3Spawn(universe, x + 1, y, totarea) == TRUE) {
			return TRUE;
		}
		if (DRLG_L3Spawn(universe, x - 1, y, totarea) == TRUE) {
			return TRUE;
		}
		if (DRLG_L3Spawn(universe, x, y + 1, totarea) == TRUE) {
			return TRUE;
		}
		if (DRLG_L3Spawn(universe, x, y - 1, totarea) == TRUE) {
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Flood fills dirt and wall tiles looking for
 * an area of at most 40 tiles and disconnected from the map edge.
 * If it finds one, converts it to lava tiles and sets universe.lavapool to TRUE.
 */
static void DRLG_L3Pool(Universe& universe)
{
	int i, j, dunx, duny, totarea, poolchance;
	BOOL found;
	BYTE k;
	const static BYTE poolsub[15] = { 0, 35, 26, 36, 25, 29, 34, 7, 33, 28, 27, 37, 32, 31, 30 };

	for (duny = 0; duny < DMAXY; duny++) {
		for (dunx = 0; dunx < DMAXY; dunx++) {
			if (GetDungeon(universe, dunx, duny) != 8) {
				continue;
			}
			SetDungeon(universe, dunx, duny, GetDungeon(universe, dunx, duny) | 0x80);
			totarea = 1;
			if (dunx + 1 < DMAXX) {
				found = DRLG_L3Spawn(universe, dunx + 1, duny, &totarea);
			} else {
				found = TRUE;
			}
			if (dunx - 1 > 0 && !found) {
				found = DRLG_L3Spawn(universe, dunx - 1, duny, &totarea);
			} else {
				found = TRUE;
			}
			if (duny + 1 < DMAXY && !found) {
				found = DRLG_L3Spawn(universe, dunx, duny + 1, &totarea);
			} else {
				found = TRUE;
			}
			if (duny - 1 > 0 && !found) {
				found = DRLG_L3Spawn(universe, dunx, duny - 1, &totarea);
			} else {
				found = TRUE;
			}
			poolchance = random_(universe, 0, 100);
			for (j = duny - totarea; j < duny + totarea; j++) {
				for (i = dunx - totarea; i < dunx + totarea; i++) {
					// BUGFIX: In the following swap the order to first do the
					// index checks and only then access GetDungeon(universe, i, j)
					if (GetDungeon(universe, i, j) & 0x80 && j >= 0 && j < DMAXY && i >= 0 && i < DMAXX) {
						SetDungeon(universe, i, j, GetDungeon(universe, i, j) & ~0x80);
						if (totarea > 4 && poolchance < 25 && !found) {
							k = poolsub[GetDungeon(universe, i, j)];
							if (k != 0 && k <= 37) {
								SetDungeon(universe, i, j, k);
							}
							universe.lavapool = TRUE;
						}
					}
				}
			}
		}
	}
}

static void DRLG_L3PoolFix(Universe& universe)
{
	int dunx, duny;

	for (duny = 0; duny < DMAXY; duny++) {     // BUGFIX: Change '0' to '1' and 'DMAXY' to 'DMAXY - 1'
		for (dunx = 0; dunx < DMAXX; dunx++) { // BUGFIX: Change '0' to '1' and 'DMAXX' to 'DMAXX - 1'
			if (GetDungeon(universe, dunx, duny) == 8) {
				if (GetDungeon(universe, dunx - 1, duny - 1) >= 25 && GetDungeon(universe, dunx - 1, duny - 1) <= 41
				    && GetDungeon(universe, dunx - 1, duny) >= 25 && GetDungeon(universe, dunx - 1, duny) <= 41
				    && GetDungeon(universe, dunx - 1, duny + 1) >= 25 && GetDungeon(universe, dunx - 1, duny + 1) <= 41
				    && GetDungeon(universe, dunx, duny - 1) >= 25 && GetDungeon(universe, dunx, duny - 1) <= 41
				    && GetDungeon(universe, dunx, duny + 1) >= 25 && GetDungeon(universe, dunx, duny + 1) <= 41
				    && GetDungeon(universe, dunx + 1, duny - 1) >= 25 && GetDungeon(universe, dunx + 1, duny - 1) <= 41
				    && GetDungeon(universe, dunx + 1, duny) >= 25 && GetDungeon(universe, dunx + 1, duny) <= 41
				    && GetDungeon(universe, dunx + 1, duny + 1) >= 25 && GetDungeon(universe, dunx + 1, duny + 1) <= 41) {
					SetDungeon(universe, dunx, duny, 33);
				}
			}
		}
	}
}

static BOOL DRLG_L3PlaceMiniSet(Universe& universe, const BYTE *miniset, int tmin, int tmax, int cx, int cy, BOOL setview, int ldir)
{
	int sx, sy, sw, sh, xx, yy, i, ii, numt, trys;
	BOOL found;

	sw = miniset[0];
	sh = miniset[1];

	if (tmax - tmin == 0) {
		numt = 1;
	} else {
		numt = random_(universe, 0, tmax - tmin) + tmin;
	}

	for (i = 0; i < numt; i++) {
		sx = random_(universe, 0, DMAXX - sw);
		sy = random_(universe, 0, DMAXY - sh);
		found = FALSE;
		trys = 0;
		while (!found && trys < 200) {
			trys++;
			found = TRUE;
			if (cx != -1 && sx >= cx - sw && sx <= cx + 12) {
				sx = random_(universe, 0, DMAXX - sw);
				sy = random_(universe, 0, DMAXY - sh);
				found = FALSE;
			}
			if (cy != -1 && sy >= cy - sh && sy <= cy + 12) {
				sx = random_(universe, 0, DMAXX - sw);
				sy = random_(universe, 0, DMAXY - sh);
				found = FALSE;
			}
			ii = 2;
			for (yy = 0; yy < sh && found == TRUE; yy++) {
				for (xx = 0; xx < sw && found == TRUE; xx++) {
					if (miniset[ii] != 0 && GetDungeon(universe, xx + sx, yy + sy) != miniset[ii]) {
						found = FALSE;
					}
					if (universe.dflags[xx + sx][yy + sy] != 0) {
						found = FALSE;
					}
					ii++;
				}
			}
			if (!found) {
				sx++;
				if (sx == DMAXX - sw) {
					sx = 0;
					sy++;
					if (sy == DMAXY - sh) {
						sy = 0;
					}
				}
			}
		}
		if (trys >= 200) {
			return TRUE;
		}
		ii = sw * sh + 2;
		for (yy = 0; yy < sh; yy++) {
			for (xx = 0; xx < sw; xx++) {
				if (miniset[ii] != 0) {
					SetDungeon(universe, xx + sx, yy + sy, miniset[ii]);
				}
				ii++;
			}
		}
	}

	if (setview == TRUE) {
		universe.ViewX = 2 * sx + 17;
		universe.ViewY = 2 * sy + 19;
	}
	if (ldir == 0) {
		universe.LvlViewX = 2 * sx + 17;
		universe.LvlViewY = 2 * sy + 19;
	}

	return FALSE;
}

static void DRLG_L3PlaceRndSet(Universe& universe, const BYTE *miniset, int rndper)
{
	int sx, sy, sw, sh, xx, yy, ii, kk;
	BOOL found;

	sw = miniset[0];
	sh = miniset[1];

	for (sy = 0; sy < DMAXX - sh; sy++) {
		for (sx = 0; sx < DMAXY - sw; sx++) {
			found = TRUE;
			ii = 2;
			for (yy = 0; yy < sh && found == TRUE; yy++) {
				for (xx = 0; xx < sw && found == TRUE; xx++) {
					if (miniset[ii] != 0 && GetDungeon(universe, xx + sx, yy + sy) != miniset[ii]) {
						found = FALSE;
					}
					if (universe.dflags[xx + sx][yy + sy] != 0) {
						found = FALSE;
					}
					ii++;
				}
			}
			kk = sw * sh + 2;
			// BUGFIX: This should not be applied to Nest levels
			if (miniset[kk] >= 84 && miniset[kk] <= 100 && found == TRUE) {
				// BUGFIX: accesses to universe.dungeon can go out of bounds
				// BUGFIX: Comparisons vs 100 should use same tile as comparisons vs 84.
				if (GetDungeon(universe, sx - 1, sy) >= 84 && GetDungeon(universe, sx - 1, sy) <= 100) {
					found = FALSE;
				}
				if (GetDungeon(universe, sx + 1, sy) >= 84 && GetDungeon(universe, sx - 1, sy) <= 100) {
					found = FALSE;
				}
				if (GetDungeon(universe, sx, sy + 1) >= 84 && GetDungeon(universe, sx - 1, sy) <= 100) {
					found = FALSE;
				}
				if (GetDungeon(universe, sx, sy - 1) >= 84 && GetDungeon(universe, sx - 1, sy) <= 100) {
					found = FALSE;
				}
			}
			if (found == TRUE && random_(universe, 0, 100) < rndper) {
				for (yy = 0; yy < sh; yy++) {
					for (xx = 0; xx < sw; xx++) {
						if (miniset[kk] != 0) {
							SetDungeon(universe, xx + sx, yy + sy, miniset[kk]);
						}
						kk++;
					}
				}
			}
		}
	}
}

#ifdef HELLFIRE
BOOLEAN drlg_l3_hive_rnd_piece(Universe& universe, const BYTE *miniset, int rndper)
{
	int sx, sy, sw, sh, xx, yy, ii, kk;
	BOOL found;
	BOOLEAN placed;

	placed = FALSE;
	sw = miniset[0];
	sh = miniset[1];

	for (sy = 0; sy < DMAXX - sh; sy++) {
		for (sx = 0; sx < DMAXY - sw; sx++) {
			found = TRUE;
			ii = 2;
			for (yy = 0; yy < sh && found == TRUE; yy++) {
				for (xx = 0; xx < sw && found == TRUE; xx++) {
					if (miniset[ii] != 0 && GetDungeon(universe, xx + sx, yy + sy) != miniset[ii]) {
						found = FALSE;
					}
					if (universe.dflags[xx + sx][yy + sy] != 0) {
						found = FALSE;
					}
					ii++;
				}
			}
			kk = sw * sh + 2;
			if (miniset[kk] >= 84 && miniset[kk] <= 100 && found == TRUE) {
				// BUGFIX: accesses to universe.dungeon can go out of bounds
				// BUGFIX: Comparisons vs 100 should use same tile as comparisons vs 84.
				if (GetDungeon(universe, sx - 1, sy) >= 84 && GetDungeon(universe, sx - 1, sy) <= 100) {
					found = FALSE;
				}
				if (GetDungeon(universe, sx + 1, sy) >= 84 && GetDungeon(universe, sx - 1, sy) <= 100) {
					found = FALSE;
				}
				if (GetDungeon(universe, sx, sy + 1) >= 84 && GetDungeon(universe, sx - 1, sy) <= 100) {
					found = FALSE;
				}
				if (GetDungeon(universe, sx, sy - 1) >= 84 && GetDungeon(universe, sx - 1, sy) <= 100) {
					found = FALSE;
				}
			}
			if (found == TRUE && random_(universe, 0, 100) < rndper) {
				placed = TRUE;
				for (yy = 0; yy < sh; yy++) {
					for (xx = 0; xx < sw; xx++) {
						if (miniset[kk] != 0) {
							SetDungeon(universe, xx + sx, yy + sy, miniset[kk]);
						}
						kk++;
					}
				}
			}
		}
	}

	return placed;
}
#endif

static BOOL WoodVertU(Universe& universe, int i, int y)
{
	if ((GetDungeon(universe, i + 1, y) > 152 || GetDungeon(universe, i + 1, y) < 130)
	    && (GetDungeon(universe, i - 1, y) > 152 || GetDungeon(universe, i - 1, y) < 130)) {
		if (GetDungeon(universe, i, y) == 7) {
			return TRUE;
		}
		if (GetDungeon(universe, i, y) == 10) {
			return TRUE;
		}
		if (GetDungeon(universe, i, y) == 126) {
			return TRUE;
		}
		if (GetDungeon(universe, i, y) == 129) {
			return TRUE;
		}
		if (GetDungeon(universe, i, y) == 134) {
			return TRUE;
		}
		if (GetDungeon(universe, i, y) == 136) {
			return TRUE;
		}
	}

	return FALSE;
}

static BOOL WoodVertD(Universe& universe, int i, int y)
{
	if ((universe.dungeon[i + 1][y] > 152 || universe.dungeon[i + 1][y] < 130)
	    && (universe.dungeon[i - 1][y] > 152 || universe.dungeon[i - 1][y] < 130)) {
		if (universe.dungeon[i][y] == 7) {
			return TRUE;
		}
		if (universe.dungeon[i][y] == 2) {
			return TRUE;
		}
		if (universe.dungeon[i][y] == 134) {
			return TRUE;
		}
		if (universe.dungeon[i][y] == 136) {
			return TRUE;
		}
	}

	return FALSE;
}

static BOOL WoodHorizL(Universe& universe, int x, int j)
{
	if ((GetDungeon(universe, x, j + 1) > 152 || GetDungeon(universe, x, j + 1) < 130)
	    && (GetDungeon(universe, x, j - 1) > 152 || GetDungeon(universe, x, j - 1) < 130)) {
		if (GetDungeon(universe, x, j) == 7) {
			return TRUE;
		}
		if (GetDungeon(universe, x, j) == 9) {
			return TRUE;
		}
		if (GetDungeon(universe, x, j) == 121) {
			return TRUE;
		}
		if (GetDungeon(universe, x, j) == 124) {
			return TRUE;
		}
		if (GetDungeon(universe, x, j) == 135) {
			return TRUE;
		}
		if (GetDungeon(universe, x, j) == 137) {
			return TRUE;
		}
	}

	return FALSE;
}

static BOOL WoodHorizR(Universe& universe, int x, int j)
{
	if ((universe.dungeon[x][j + 1] > 152 || universe.dungeon[x][j + 1] < 130)
	    && (universe.dungeon[x][j - 1] > 152 || universe.dungeon[x][j - 1] < 130)) {
		if (universe.dungeon[x][j] == 7) {
			return TRUE;
		}
		if (universe.dungeon[x][j] == 4) {
			return TRUE;
		}
		if (universe.dungeon[x][j] == 135) {
			return TRUE;
		}
		if (universe.dungeon[x][j] == 137) {
			return TRUE;
		}
	}

	return FALSE;
}

void AddFenceDoors(Universe& universe)
{
	int i, j;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (universe.dungeon[i][j] == 7) {
				if (universe.dungeon[i - 1][j] <= 152 && universe.dungeon[i - 1][j] >= 130
				    && universe.dungeon[i + 1][j] <= 152 && universe.dungeon[i + 1][j] >= 130) {
					universe.dungeon[i][j] = 146;
					continue;
				}
			}
			if (universe.dungeon[i][j] == 7) {
				if (universe.dungeon[i][j - 1] <= 152 && universe.dungeon[i][j - 1] >= 130
				    && universe.dungeon[i][j + 1] <= 152 && universe.dungeon[i][j + 1] >= 130) {
					universe.dungeon[i][j] = 147;
					continue;
				}
			}
		}
	}
}

void FenceDoorFix(Universe& universe)
{
	int i, j;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (GetDungeon(universe, i, j) == 146) {
				if (GetDungeon(universe, i + 1, j) > 152 || GetDungeon(universe, i + 1, j) < 130
				    || GetDungeon(universe, i - 1, j) > 152 || GetDungeon(universe, i - 1, j) < 130) {
					SetDungeon(universe, i, j, 7);
					continue;
				}
			}
			if (GetDungeon(universe, i, j) == 146) {
				if (GetDungeon(universe, i + 1, j) != 130 && GetDungeon(universe, i - 1, j) != 130
				    && GetDungeon(universe, i + 1, j) != 132 && GetDungeon(universe, i - 1, j) != 132
				    && GetDungeon(universe, i + 1, j) != 133 && GetDungeon(universe, i - 1, j) != 133
				    && GetDungeon(universe, i + 1, j) != 134 && GetDungeon(universe, i - 1, j) != 134
				    && GetDungeon(universe, i + 1, j) != 136 && GetDungeon(universe, i - 1, j) != 136
				    && GetDungeon(universe, i + 1, j) != 138 && GetDungeon(universe, i - 1, j) != 138
				    && GetDungeon(universe, i + 1, j) != 140 && GetDungeon(universe, i - 1, j) != 140) {
					SetDungeon(universe, i, j, 7);
					continue;
				}
			}
			if (GetDungeon(universe, i, j) == 147) {
				if (GetDungeon(universe, i, j + 1) > 152 || GetDungeon(universe, i, j + 1) < 130
				    || GetDungeon(universe, i, j - 1) > 152 || GetDungeon(universe, i, j - 1) < 130) {
					SetDungeon(universe, i, j, 7);
					continue;
				}
			}
			if (GetDungeon(universe, i, j) == 147) {
				if (GetDungeon(universe, i, j + 1) != 131 && GetDungeon(universe, i, j - 1) != 131
				    && GetDungeon(universe, i, j + 1) != 132 && GetDungeon(universe, i, j - 1) != 132
				    && GetDungeon(universe, i, j + 1) != 133 && GetDungeon(universe, i, j - 1) != 133
				    && GetDungeon(universe, i, j + 1) != 135 && GetDungeon(universe, i, j - 1) != 135
				    && GetDungeon(universe, i, j + 1) != 137 && GetDungeon(universe, i, j - 1) != 137
				    && GetDungeon(universe, i, j + 1) != 138 && GetDungeon(universe, i, j - 1) != 138
				    && GetDungeon(universe, i, j + 1) != 139 && GetDungeon(universe, i, j - 1) != 139) {
					SetDungeon(universe, i, j, 7);
					continue;
				}
			}
		}
	}
}

static void DRLG_L3Wood(Universe& universe)
{
	int i, j, x, y, xx, yy, rt, rp, x1, y1, x2, y2;
	BOOL skip;

	for (j = 0; j < DMAXY - 1; j++) {     // BUGFIX: Change '0' to '1'
		for (i = 0; i < DMAXX - 1; i++) { // BUGFIX: Change '0' to '1'
			if (GetDungeon(universe, i, j) == 10 && random_(universe, 0, 2) != 0) {
				x = i;
				while (GetDungeon(universe, x, j) == 10) {
					x++;
				}
				x--;
				if (x - i > 0) {
					SetDungeon(universe, i, j, 127);
					for (xx = i + 1; xx < x; xx++) {
						if (random_(universe, 0, 2) != 0) {
							SetDungeon(universe, xx, j, 126);
						} else {
							SetDungeon(universe, xx, j, 129);
						}
					}
					SetDungeon(universe, x, j, 128);
				}
			}
			if (GetDungeon(universe, i, j) == 9 && random_(universe, 0, 2) != 0) {
				y = j;
				while (GetDungeon(universe, i, y) == 9) {
					y++;
				}
				y--;
				if (y - j > 0) {
					SetDungeon(universe, i, j, 123);
					for (yy = j + 1; yy < y; yy++) {
						if (random_(universe, 0, 2) != 0) {
							SetDungeon(universe, i, yy, 121);
						} else {
							SetDungeon(universe, i, yy, 124);
						}
					}
					SetDungeon(universe, i, y, 122);
				}
			}
			if (GetDungeon(universe, i, j) == 11 && GetDungeon(universe, i + 1, j) == 10 && GetDungeon(universe, i, j + 1) == 9 && random_(universe, 0, 2) != 0) {
				SetDungeon(universe, i, j, 125);
				x = i + 1;
				while (GetDungeon(universe, x, j) == 10) {
					x++;
				}
				x--;
				for (xx = i + 1; xx < x; xx++) {
					if (random_(universe, 0, 2) != 0) {
						SetDungeon(universe, xx, j, 126);
					} else {
						SetDungeon(universe, xx, j, 129);
					}
				}
				SetDungeon(universe, x, j, 128);
				y = j + 1;
				while (GetDungeon(universe, i, y) == 9) {
					y++;
				}
				y--;
				for (yy = j + 1; yy < y; yy++) {
					if (random_(universe, 0, 2) != 0) {
						SetDungeon(universe, i, yy, 121);
					} else {
						SetDungeon(universe, i, yy, 124);
					}
				}
				SetDungeon(universe, i, y, 122);
			}
		}
	}

	for (j = 0; j < DMAXY; j++) {     // BUGFIX: Change '0' to '1'
		for (i = 0; i < DMAXX; i++) { // BUGFIX: Change '0' to '1'
			if (GetDungeon(universe, i, j) == 7 && random_(universe, 0, 1) == 0 && SkipThemeRoom(universe, i, j)) {
				rt = random_(universe, 0, 2);
				if (rt == 0) {
					y1 = j;
					// BUGFIX: Check `y1 >= 0` first
					while (WoodVertU(universe, i, y1)) {
						y1--;
					}
					y1++;
					y2 = j;
					// BUGFIX: Check `y2 < DMAXY` first
					while (WoodVertD(universe, i, y2)) {
						y2++;
					}
					y2--;
					skip = TRUE;
					if (GetDungeon(universe, i, y1) == 7) {
						skip = FALSE;
					}
					if (GetDungeon(universe, i, y2) == 7) {
						skip = FALSE;
					}
					if (y2 - y1 > 1 && skip) {
						rp = random_(universe, 0, y2 - y1 - 1) + y1 + 1;
						for (y = y1; y <= y2; y++) {
							if (y == rp) {
								continue;
							}
							if (GetDungeon(universe, i, y) == 7) {
								if (random_(universe, 0, 2) != 0) {
									SetDungeon(universe, i, y, 135);
								} else {
									SetDungeon(universe, i, y, 137);
								}
							}
							if (GetDungeon(universe, i, y) == 10) {
								SetDungeon(universe, i, y, 131);
							}
							if (GetDungeon(universe, i, y) == 126) {
								SetDungeon(universe, i, y, 133);
							}
							if (GetDungeon(universe, i, y) == 129) {
								SetDungeon(universe, i, y, 133);
							}
							if (GetDungeon(universe, i, y) == 2) {
								SetDungeon(universe, i, y, 139);
							}
							if (GetDungeon(universe, i, y) == 134) {
								SetDungeon(universe, i, y, 138);
							}
							if (GetDungeon(universe, i, y) == 136) {
								SetDungeon(universe, i, y, 138);
							}
						}
					}
				}
				if (rt == 1) {
					x1 = i;
					// BUGFIX: Check `x1 >= 0` first (fixed)
					while (WoodHorizL(universe, x1, j)) {
						x1--;
					}
					x1++;
					x2 = i;
					// BUGFIX: Check `x2 < DMAXX` first (fixed)
					while (WoodHorizR(universe, x2, j)) {
						x2++;
					}
					x2--;
					skip = TRUE;
					if (GetDungeon(universe, x1, j) == 7) {
						skip = FALSE;
					}
					if (GetDungeon(universe, x2, j) == 7) {
						skip = FALSE;
					}
					if (x2 - x1 > 1 && skip) {
						rp = random_(universe, 0, x2 - x1 - 1) + x1 + 1;
						for (x = x1; x <= x2; x++) {
							if (x == rp) {
								continue;
							}
							if (GetDungeon(universe, x, j) == 7) {
								if (random_(universe, 0, 2) != 0) {
									SetDungeon(universe, x, j, 134);
								} else {
									SetDungeon(universe, x, j, 136);
								}
							}
							if (GetDungeon(universe, x, j) == 9) {
								SetDungeon(universe, x, j, 130);
							}
							if (GetDungeon(universe, x, j) == 121) {
								SetDungeon(universe, x, j, 132);
							}
							if (GetDungeon(universe, x, j) == 124) {
								SetDungeon(universe, x, j, 132);
							}
							if (GetDungeon(universe, x, j) == 4) {
								SetDungeon(universe, x, j, 140);
							}
							if (GetDungeon(universe, x, j) == 135) {
								SetDungeon(universe, x, j, 138);
							}
							if (GetDungeon(universe, x, j) == 137) {
								SetDungeon(universe, x, j, 138);
							}
						}
					}
				}
			}
		}
	}

	AddFenceDoors(universe);
	FenceDoorFix(universe);
}

BOOL DRLG_L3Anvil(Universe& universe)
{
	int sx, sy, sw, sh, xx, yy, ii, trys;
	BOOL found;

	sw = L3ANVIL[0];
	sh = L3ANVIL[1];
	sx = random_(universe, 0, DMAXX - sw);
	sy = random_(universe, 0, DMAXY - sh);

	found = FALSE;
	trys = 0;
	while (!found && trys < 200) {
		trys++;
		found = TRUE;
		ii = 2;
		for (yy = 0; yy < sh && found == TRUE; yy++) {
			for (xx = 0; xx < sw && found == TRUE; xx++) {
				if (L3ANVIL[ii] != 0 && GetDungeon(universe, xx + sx, yy + sy) != L3ANVIL[ii]) {
					found = FALSE;
				}
				if (universe.dflags[xx + sx][yy + sy] != 0) {
					found = FALSE;
				}
				ii++;
			}
		}
		if (!found) {
			sx++;
			if (sx == DMAXX - sw) {
				sx = 0;
				sy++;
				if (sy == DMAXY - sh) {
					sy = 0;
				}
			}
		}
	}
	if (trys >= 200) {
		return TRUE;
	}

	ii = sw * sh + 2;
	for (yy = 0; yy < sh; yy++) {
		for (xx = 0; xx < sw; xx++) {
			if (L3ANVIL[ii] != 0) {
				SetDungeon(universe, xx + sx, yy + sy, L3ANVIL[ii]);
			}
			universe.dflags[xx + sx][yy + sy] |= DLRG_PROTECTED;
			ii++;
		}
	}

	universe.setpc_x = sx;
	universe.setpc_y = sy;
	universe.setpc_w = sw;
	universe.setpc_h = sh;

	return FALSE;
}

void FixL3Warp(Universe& universe)
{
	int i, j;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (universe.dungeon[i][j] == 125 && universe.dungeon[i + 1][j] == 125 && universe.dungeon[i][j + 1] == 125 && universe.dungeon[i + 1][j + 1] == 125) {
				universe.dungeon[i][j] = 156;
				universe.dungeon[i + 1][j] = 155;
				universe.dungeon[i][j + 1] = 153;
				universe.dungeon[i + 1][j + 1] = 154;
				return;
			}
			if (universe.dungeon[i][j] == 5 && universe.dungeon[i + 1][j + 1] == 7) {
				universe.dungeon[i][j] = 7;
			}
		}
	}
}

void FixL3HallofHeroes(Universe& universe)
{
	int i, j;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (universe.dungeon[i][j] == 5 && universe.dungeon[i + 1][j + 1] == 7) {
				universe.dungeon[i][j] = 7;
			}
		}
	}
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (universe.dungeon[i][j] == 5 && universe.dungeon[i + 1][j + 1] == 12 && universe.dungeon[i + 1][j] == 7) {
				universe.dungeon[i][j] = 7;
				universe.dungeon[i][j + 1] = 7;
				universe.dungeon[i + 1][j + 1] = 7;
			}
			if (universe.dungeon[i][j] == 5 && universe.dungeon[i + 1][j + 1] == 12 && universe.dungeon[i][j + 1] == 7) {
				universe.dungeon[i][j] = 7;
				universe.dungeon[i + 1][j] = 7;
				universe.dungeon[i + 1][j + 1] = 7;
			}
		}
	}
}

void DRLG_L3LockRec(Universe& universe, int x, int y)
{
	if (!universe.lockout[x][y]) {
		return;
	}

	universe.lockout[x][y] = FALSE;
	universe.lockoutcnt++;
	DRLG_L3LockRec(universe, x, y - 1);
	DRLG_L3LockRec(universe, x, y + 1);
	DRLG_L3LockRec(universe, x - 1, y);
	DRLG_L3LockRec(universe, x + 1, y);
}

BOOL DRLG_L3Lockout(Universe& universe)
{
	int i, j, t, fx, fy;

	t = 0;
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (GetDungeon(universe, i, j) != 0) {
				universe.lockout[i][j] = TRUE;
				fx = i;
				fy = j;
				t++;
			} else {
				universe.lockout[i][j] = FALSE;
			}
		}
	}

	universe.lockoutcnt = 0;
	DRLG_L3LockRec(universe, fx, fy);

	return t == universe.lockoutcnt;
}

static std::optional<uint32_t> DRLG_L3(Universe& universe, int entry, DungeonMode mode)
{
	int x1, y1, x2, y2, i, j;
	BOOL found, genok;

	universe.lavapool = FALSE;

	std::optional<uint32_t> levelSeed = std::nullopt;
	do {
		do {
			do {
				levelSeed = GetRndState(universe);
				InitL3Dungeon(universe);
				x1 = random_(universe, 0, 20) + 10;
				y1 = random_(universe, 0, 20) + 10;
				x2 = x1 + 2;
				y2 = y1 + 2;
				DRLG_L3FillRoom(universe, x1, y1, x2, y2);
				DRLG_L3CreateBlock(universe, x1, y1, 2, 0);
				DRLG_L3CreateBlock(universe, x2, y1, 2, 1);
				DRLG_L3CreateBlock(universe, x1, y2, 2, 2);
				DRLG_L3CreateBlock(universe, x1, y1, 2, 3);
				if (QuestStatus(universe, Q_ANVIL)) {
					x1 = random_(universe, 0, 10) + 10;
					y1 = random_(universe, 0, 10) + 10;
					x2 = x1 + 12;
					y2 = y1 + 12;
					DRLG_L3FloorArea(universe, x1, y1, x2, y2);
				}
				DRLG_L3FillDiags(universe);
				DRLG_L3FillSingles(universe);
				DRLG_L3FillStraights(universe);
				DRLG_L3FillDiags(universe);
				DRLG_L3Edges(universe);
				if (DRLG_L3GetFloorArea(universe) >= 600) {
					found = DRLG_L3Lockout(universe);
				} else {
					found = FALSE;
				}
				if (mode == DungeonMode::BreakOnFailure && !found)
					return std::nullopt;
			} while (!found);
			DRLG_L3MakeMegas(universe);
			if (entry == ENTRY_MAIN) {
#ifdef HELLFIRE
				if (universe.currlevel < 17) {
#endif
					genok = DRLG_L3PlaceMiniSet(universe, L3UP, 1, 1, -1, -1, TRUE, 0);
#ifdef HELLFIRE
				} else {
					if (universe.currlevel != 17)
						genok = DRLG_L3PlaceMiniSet(universe, L6UP, 1, 1, -1, -1, TRUE, 0);
					else
						genok = DRLG_L3PlaceMiniSet(universe, L6HOLDWARP, 1, 1, -1, -1, TRUE, 6);
				}
#endif
				if (!genok) {
#ifdef HELLFIRE
					if (universe.currlevel < 17) {
#endif
						genok = DRLG_L3PlaceMiniSet(universe, L3DOWN, 1, 1, -1, -1, FALSE, 1);
#ifdef HELLFIRE
					} else {
						if (universe.currlevel != 20)
							genok = DRLG_L3PlaceMiniSet(universe, L6DOWN, 1, 1, -1, -1, FALSE, 1);
					}
#endif
					if (!genok && universe.currlevel == 9) {
						genok = DRLG_L3PlaceMiniSet(universe, L3HOLDWARP, 1, 1, -1, -1, FALSE, 6);
					}
				}
			} else if (entry == ENTRY_PREV) {
#ifdef HELLFIRE
				if (universe.currlevel < 17) {
#endif
					genok = DRLG_L3PlaceMiniSet(universe, L3UP, 1, 1, -1, -1, FALSE, 0);
#ifdef HELLFIRE
				} else {
					if (universe.currlevel != 17)
						genok = DRLG_L3PlaceMiniSet(universe, L6UP, 1, 1, -1, -1, FALSE, 0);
					else
						genok = DRLG_L3PlaceMiniSet(universe, L6HOLDWARP, 1, 1, -1, -1, FALSE, 6);
				}
#endif
				if (!genok) {
#ifdef HELLFIRE
					if (universe.currlevel < 17) {
#endif
						genok = DRLG_L3PlaceMiniSet(universe, L3DOWN, 1, 1, -1, -1, TRUE, 1);
						universe.ViewX += 2;
						universe.ViewY -= 2;
#ifdef HELLFIRE
					} else {
						if (universe.currlevel != 20) {
							genok = DRLG_L3PlaceMiniSet(universe, L6DOWN, 1, 1, -1, -1, TRUE, 1);
							universe.ViewX += 2;
							universe.ViewY -= 2;
						}
					}
#endif
					if (!genok && universe.currlevel == 9) {
						genok = DRLG_L3PlaceMiniSet(universe, L3HOLDWARP, 1, 1, -1, -1, FALSE, 6);
					}
				}
			} else {
#ifdef HELLFIRE
				if (universe.currlevel < 17) {
#endif
					genok = DRLG_L3PlaceMiniSet(universe, L3UP, 1, 1, -1, -1, FALSE, 0);
#ifdef HELLFIRE
				} else {
					if (universe.currlevel != 17)
						genok = DRLG_L3PlaceMiniSet(universe, L6UP, 1, 1, -1, -1, FALSE, 0);
					else
						genok = DRLG_L3PlaceMiniSet(universe, L6HOLDWARP, 1, 1, -1, -1, TRUE, 6);
				}
#endif
				if (!genok) {
#ifdef HELLFIRE
					if (universe.currlevel < 17) {
#endif
						genok = DRLG_L3PlaceMiniSet(universe, L3DOWN, 1, 1, -1, -1, FALSE, 1);
#ifdef HELLFIRE
					} else {
						if (universe.currlevel != 20)
							genok = DRLG_L3PlaceMiniSet(universe, L6DOWN, 1, 1, -1, -1, FALSE, 1);
					}
#endif
					if (!genok && universe.currlevel == 9) {
						genok = DRLG_L3PlaceMiniSet(universe, L3HOLDWARP, 1, 1, -1, -1, TRUE, 6);
					}
				}
			}
			if (!genok && QuestStatus(universe, Q_ANVIL)) {
				genok = DRLG_L3Anvil(universe);
			}
			if (mode == DungeonMode::BreakOnFailure && genok == TRUE)
				return std::nullopt;
		} while (genok == TRUE);
#ifdef HELLFIRE
		if (universe.currlevel < 17) {
#endif
			DRLG_L3Pool(universe);
#ifdef HELLFIRE
		} else {
			universe.lavapool += drlg_l3_hive_rnd_piece(universe, byte_48A998, 30);
			universe.lavapool += drlg_l3_hive_rnd_piece(universe, byte_48A9C8, 40);
			universe.lavapool += drlg_l3_hive_rnd_piece(universe, byte_48A948, 50);
			universe.lavapool += drlg_l3_hive_rnd_piece(universe, byte_48A970, 60);
			if (universe.lavapool < 3)
				universe.lavapool = FALSE;
		}
#endif
		if (mode == DungeonMode::BreakOnFailure && !universe.lavapool)
			return std::nullopt;
	} while (!universe.lavapool);

	if (mode == DungeonMode::BreakOnSuccess)
		return levelSeed;

#ifdef HELLFIRE
	if (universe.currlevel < 17)
#endif
		DRLG_L3PoolFix(universe);
#ifdef HELLFIRE
	if (universe.currlevel < 17)
#endif
		FixL3Warp(universe);

#ifdef HELLFIRE
	if (universe.currlevel < 17) {
#endif
		DRLG_L3PlaceRndSet(universe, L3ISLE1, 70);
		DRLG_L3PlaceRndSet(universe, L3ISLE2, 70);
		DRLG_L3PlaceRndSet(universe, L3ISLE3, 30);
		DRLG_L3PlaceRndSet(universe, L3ISLE4, 30);
		DRLG_L3PlaceRndSet(universe, L3ISLE1, 100);
		DRLG_L3PlaceRndSet(universe, L3ISLE2, 100);
		DRLG_L3PlaceRndSet(universe, L3ISLE5, 90);
#ifdef HELLFIRE
	} else {
		DRLG_L3PlaceRndSet(universe, L6ISLE1, 70);
		DRLG_L3PlaceRndSet(universe, L6ISLE2, 70);
		DRLG_L3PlaceRndSet(universe, L6ISLE3, 30);
		DRLG_L3PlaceRndSet(universe, L6ISLE4, 30);
		DRLG_L3PlaceRndSet(universe, L6ISLE1, 100);
		DRLG_L3PlaceRndSet(universe, L6ISLE2, 100);
		DRLG_L3PlaceRndSet(universe, L6ISLE5, 90);
	}
#endif

#ifdef HELLFIRE
	if (universe.currlevel < 17)
#endif
		FixL3HallofHeroes(universe);
#ifdef HELLFIRE
	if (universe.currlevel < 17)
#endif
		DRLG_L3River(universe);

	if (QuestStatus(universe, Q_ANVIL)) {
		SetDungeon(universe, universe.setpc_x + 7, universe.setpc_y + 5, 7);
		SetDungeon(universe, universe.setpc_x + 8, universe.setpc_y + 5, 7);
		SetDungeon(universe, universe.setpc_x + 9, universe.setpc_y + 5, 7);
		if (GetDungeon(universe, universe.setpc_x + 10, universe.setpc_y + 5) == 17 || GetDungeon(universe, universe.setpc_x + 10, universe.setpc_y + 5) == 18) {
			SetDungeon(universe, universe.setpc_x + 10, universe.setpc_y + 5, 45);
		}
	}

#ifdef HELLFIRE
	if (universe.currlevel < 17)
#endif
		DRLG_PlaceThemeRooms(universe, 5, 10, 7, 0, 0);

#ifdef HELLFIRE
	if (universe.currlevel < 17) {
#endif
		DRLG_L3Wood(universe);
		DRLG_L3PlaceRndSet(universe, L3TITE1, 10);
		DRLG_L3PlaceRndSet(universe, L3TITE2, 10);
		DRLG_L3PlaceRndSet(universe, L3TITE3, 10);
		DRLG_L3PlaceRndSet(universe, L3TITE6, 20);
		DRLG_L3PlaceRndSet(universe, L3TITE7, 20);
		DRLG_L3PlaceRndSet(universe, L3TITE8, 20);
		DRLG_L3PlaceRndSet(universe, L3TITE9, 20);
		DRLG_L3PlaceRndSet(universe, L3TITE10, 20);
		DRLG_L3PlaceRndSet(universe, L3TITE11, 30);
		DRLG_L3PlaceRndSet(universe, L3TITE12, 20);
		DRLG_L3PlaceRndSet(universe, L3TITE13, 20);
		DRLG_L3PlaceRndSet(universe, L3CREV1, 30);
		DRLG_L3PlaceRndSet(universe, L3CREV2, 30);
		DRLG_L3PlaceRndSet(universe, L3CREV3, 30);
		DRLG_L3PlaceRndSet(universe, L3CREV4, 30);
		DRLG_L3PlaceRndSet(universe, L3CREV5, 30);
		DRLG_L3PlaceRndSet(universe, L3CREV6, 30);
		DRLG_L3PlaceRndSet(universe, L3CREV7, 30);
		DRLG_L3PlaceRndSet(universe, L3CREV8, 30);
		DRLG_L3PlaceRndSet(universe, L3CREV9, 30);
		DRLG_L3PlaceRndSet(universe, L3CREV10, 30);
		DRLG_L3PlaceRndSet(universe, L3CREV11, 30);
		DRLG_L3PlaceRndSet(universe, L3XTRA1, 25);
		DRLG_L3PlaceRndSet(universe, L3XTRA2, 25);
		DRLG_L3PlaceRndSet(universe, L3XTRA3, 25);
		DRLG_L3PlaceRndSet(universe, L3XTRA4, 25);
		DRLG_L3PlaceRndSet(universe, L3XTRA5, 25);
#ifdef HELLFIRE
	} else {
		DRLG_L3PlaceRndSet(universe, byte_48A76C, 20);
		DRLG_L3PlaceRndSet(universe, byte_48A770, 20);
		DRLG_L3PlaceRndSet(universe, byte_48A774, 20);
		DRLG_L3PlaceRndSet(universe, byte_48A778, 20);
		DRLG_L3PlaceRndSet(universe, byte_48A808, 10);
		DRLG_L3PlaceRndSet(universe, byte_48A820, 15);
		DRLG_L3PlaceRndSet(universe, byte_48A838, 20);
		DRLG_L3PlaceRndSet(universe, byte_48A850, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A868, 30);
		DRLG_L3PlaceRndSet(universe, byte_48A880, 35);
		DRLG_L3PlaceRndSet(universe, byte_48A898, 40);
		DRLG_L3PlaceRndSet(universe, byte_48A8B0, 45);
		DRLG_L3PlaceRndSet(universe, byte_48A8C8, 50);
		DRLG_L3PlaceRndSet(universe, byte_48A8E0, 55);
		DRLG_L3PlaceRndSet(universe, byte_48A8E0, 10);
		DRLG_L3PlaceRndSet(universe, byte_48A8C8, 15);
		DRLG_L3PlaceRndSet(universe, byte_48A8B0, 20);
		DRLG_L3PlaceRndSet(universe, byte_48A898, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A880, 30);
		DRLG_L3PlaceRndSet(universe, byte_48A868, 35);
		DRLG_L3PlaceRndSet(universe, byte_48A850, 40);
		DRLG_L3PlaceRndSet(universe, byte_48A838, 45);
		DRLG_L3PlaceRndSet(universe, byte_48A820, 50);
		DRLG_L3PlaceRndSet(universe, byte_48A808, 55);
		DRLG_L3PlaceRndSet(universe, byte_48A790, 40);
		DRLG_L3PlaceRndSet(universe, byte_48A7A8, 45);
		DRLG_L3PlaceRndSet(universe, byte_48A77C, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A780, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A784, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A788, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A7BC, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A7C0, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A7C4, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A7C8, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A7CC, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A7D4, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A7D8, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A7DC, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A7E0, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A7E4, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A7EC, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A7F0, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A7F4, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A7F8, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A7D0, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A7E8, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A7FC, 25);
		DRLG_L3PlaceRndSet(universe, byte_48A800, 25);
	}
#endif

	memcpy(universe.pdungeon, universe.dungeon, sizeof(universe.pdungeon));

	DRLG_Init_Globals(universe);

	return levelSeed;
}

static void DRLG_L3Pass3(Universe& universe)
{
	int i, j, xx, yy;
	long v1, v2, v3, v4, lv;

	lv = 8 - 1;

#ifdef USE_ASM
	__asm {
		mov		esi, pMegaTiles
		mov		eax, lv
		shl		eax, 3
		add		esi, eax
		xor		eax, eax
		lodsw
		inc		eax
		mov		v1, eax
		lodsw
		inc		eax
		mov		v2, eax
		lodsw
		inc		eax
		mov		v3, eax
		lodsw
		inc		eax
		mov		v4, eax
	}
#else
	v1 = *((WORD *)&pMegaTiles[lv * 8] + 0) + 1;
	v2 = *((WORD *)&pMegaTiles[lv * 8] + 1) + 1;
	v3 = *((WORD *)&pMegaTiles[lv * 8] + 2) + 1;
	v4 = *((WORD *)&pMegaTiles[lv * 8] + 3) + 1;
#endif

	for (j = 0; j < MAXDUNY; j += 2)
	{
		for (i = 0; i < MAXDUNX; i += 2) {
			universe.dPiece[i][j] = v1;
			universe.dPiece[i + 1][j] = v2;
			universe.dPiece[i][j + 1] = v3;
			universe.dPiece[i + 1][j + 1] = v4;
		}
	}

	yy = 16;
	for (j = 0; j < DMAXY; j++) {
		xx = 16;
		for (i = 0; i < DMAXX; i++) {
			lv = GetDungeon(universe, i, j) - 1;
#ifdef USE_ASM
			if (lv >= 0) {
				__asm {
					mov		esi, pMegaTiles
					mov		eax, lv
					shl		eax, 3
					add		esi, eax
					xor		eax, eax
					lodsw
					inc		eax
					mov		v1, eax
					lodsw
					inc		eax
					mov		v2, eax
					lodsw
					inc		eax
					mov		v3, eax
					lodsw
					inc		eax
					mov		v4, eax
				}
			} else {
				v1 = 0;
				v2 = 0;
				v3 = 0;
				v4 = 0;
			}
#else
			if (lv >= 0) {
				v1 = *((WORD *)&pMegaTiles[lv * 8] + 0) + 1;
				v2 = *((WORD *)&pMegaTiles[lv * 8] + 1) + 1;
				v3 = *((WORD *)&pMegaTiles[lv * 8] + 2) + 1;
				v4 = *((WORD *)&pMegaTiles[lv * 8] + 3) + 1;
			} else {
				v1 = 0;
				v2 = 0;
				v3 = 0;
				v4 = 0;
			}
#endif
			universe.dPiece[xx][yy] = v1;
			universe.dPiece[xx + 1][yy] = v2;
			universe.dPiece[xx][yy + 1] = v3;
			universe.dPiece[xx + 1][yy + 1] = v4;
			xx += 2;
		}
		yy += 2;
	}
}

std::optional<uint32_t> CreateL3Dungeon(Universe& universe, DWORD rseed, int entry, DungeonMode mode)
{
	int i, j;

	SetRndSeed(universe, rseed);
	universe.dminx = 16;
	universe.dminy = 16;
	universe.dmaxx = 96;
	universe.dmaxy = 96;
	DRLG_InitTrans(universe);
	DRLG_InitSetPC(universe);
	std::optional<uint32_t> levelSeed = DRLG_L3(universe, entry, mode);
	if (mode == DungeonMode::BreakOnFailure || mode == DungeonMode::BreakOnSuccess)
		return levelSeed;
	DRLG_L3Pass3(universe);

#ifdef HELLFIRE
	if (universe.currlevel < 17) {
#endif
#ifdef HELLFIRE
	} else {
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++) {
				if (universe.dPiece[i][j] >= 382 && universe.dPiece[i][j] <= 457) {
					DoLighting(i, j, 9, -1);
				}
			}
		}
	}
#endif

	DRLG_SetPC(universe);

	return levelSeed;
}

void LoadL3Dungeon(Universe& universe, const char *sFileName, int vx, int vy)
{
	int i, j, rw, rh;
	BYTE *pLevelMap, *lm;

	InitL3Dungeon(universe);
	universe.dminx = 16;
	universe.dminy = 16;
	universe.dmaxx = 96;
	universe.dmaxy = 96;
	DRLG_InitTrans(universe);
	pLevelMap = LoadFileInMem(sFileName, NULL);

	lm = pLevelMap;
	rw = *lm;
	lm += 2;
	rh = *lm;
	lm += 2;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*lm != 0) {
				SetDungeon(universe, i, j, *lm);
			} else {
				SetDungeon(universe, i, j, 7);
			}
			lm += 2;
		}
	}
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (GetDungeon(universe, i, j) == 0) {
				SetDungeon(universe, i, j, 8);
			}
		}
	}

	universe.abyssx = MAXDUNX; // Unused
	DRLG_L3Pass3(universe);
	DRLG_Init_Globals(universe);
	universe.ViewX = 31;
	universe.ViewY = 83;
	SetMapMonsters(universe, pLevelMap, 0, 0);
	SetMapObjects(universe, pLevelMap, 0, 0);

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (universe.dPiece[i][j] >= 56 && universe.dPiece[i][j] <= 147) {
				DoLighting(i, j, 7, -1);
			} else if (universe.dPiece[i][j] >= 154 && universe.dPiece[i][j] <= 161) {
				DoLighting(i, j, 7, -1);
			} else if (universe.dPiece[i][j] == 150) {
				DoLighting(i, j, 7, -1);
			} else if (universe.dPiece[i][j] == 152) {
				DoLighting(i, j, 7, -1);
			}
		}
	}

	mem_free_dbg(pLevelMap);
}

void LoadPreL3Dungeon(Universe& universe, const char *sFileName, int vx, int vy)
{
	int i, j, rw, rh;
	BYTE *pLevelMap, *lm;

	InitL3Dungeon(universe);
	DRLG_InitTrans(universe);
	pLevelMap = LoadFileInMem(sFileName, NULL);

	lm = pLevelMap;
	rw = *lm;
	lm += 2;
	rh = *lm;
	lm += 2;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*lm != 0) {
				SetDungeon(universe, i, j, *lm);
			} else {
				SetDungeon(universe, i, j, 7);
			}
			lm += 2;
		}
	}
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (GetDungeon(universe, i, j) == 0) {
				SetDungeon(universe, i, j, 8);
			}
		}
	}

	memcpy(universe.pdungeon, universe.dungeon, sizeof(universe.pdungeon));
	mem_free_dbg(pLevelMap);
}
#endif
