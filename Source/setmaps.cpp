/**
 * @file setmaps.cpp
 *
 * Implementation of functionality the special quest dungeons.
 */

#include "Source/setmaps.h"

#include "types.h"
#include "Source/drlg_l1.h"
#include "Source/drlg_l2.h"
#include "Source/drlg_l3.h"
#include "Source/engine.h"
#include "Source/gendung.h"
#include "Source/quests.h"
#include "Source/objects.h"
#include "Source/trigs.h"
#include "Source/universe/universe.h"

// BUGFIX: constant data should be const
const BYTE SkelKingTrans1[] = {
	19, 47, 26, 55,
	26, 49, 30, 53
};

const BYTE SkelKingTrans2[] = {
	33, 19, 47, 29,
	37, 29, 43, 39
};

const BYTE SkelKingTrans3[] = {
	27, 53, 35, 61,
	27, 35, 34, 42,
	45, 35, 53, 43,
	45, 53, 53, 61,
	31, 39, 49, 57
};

const BYTE SkelKingTrans4[] = {
	49, 45, 58, 51,
	57, 31, 62, 37,
	63, 31, 69, 40,
	59, 41, 73, 55,
	63, 55, 69, 65,
	73, 45, 78, 51,
	79, 43, 89, 53
};

const BYTE SkelChamTrans1[] = {
	43, 19, 50, 26,
	51, 19, 59, 26,
	35, 27, 42, 34,
	43, 27, 49, 34,
	50, 27, 59, 34
};

const BYTE SkelChamTrans2[] = {
	19, 31, 34, 47,
	34, 35, 42, 42
};

const BYTE SkelChamTrans3[] = {
	43, 35, 50, 42,
	51, 35, 62, 42,
	63, 31, 66, 46,
	67, 31, 78, 34,
	67, 35, 78, 42,
	67, 43, 78, 46,
	35, 43, 42, 51,
	43, 43, 49, 51,
	50, 43, 59, 51
};

/** Maps from quest level to quest level names. */
const char *const quest_level_names[] = {
	"",
	"Skeleton King's Lair",
	"Bone Chamber",
	"Maze",
	"Poisoned Water Supply",
	"Archbishop Lazarus' Lair",
};

int ObjIndex(int x, int y)
{
	int i;
	int oi;

	for (i = 0; i < nobjects; i++) {
		oi = objectactive[i];
		if (object[oi]._ox == x && object[oi]._oy == y)
			return oi;
	}
	char msg[200];
	snprintf(msg, 200, "ObjIndex: Active object not found at (%d,%d)", x, y);
	app_fatal(msg);
	return -1;
}

#ifndef SPAWN
void AddSKingObjs()
{
	SetObjMapRange(ObjIndex(64, 34), 20, 7, 23, 10, 1);
	SetObjMapRange(ObjIndex(64, 59), 20, 14, 21, 16, 2);
	SetObjMapRange(ObjIndex(27, 37), 8, 1, 15, 11, 3);
	SetObjMapRange(ObjIndex(46, 35), 8, 1, 15, 11, 3);
	SetObjMapRange(ObjIndex(49, 53), 8, 1, 15, 11, 3);
	SetObjMapRange(ObjIndex(27, 53), 8, 1, 15, 11, 3);
}

void AddSChamObjs()
{
	SetObjMapRange(ObjIndex(37, 30), 17, 0, 21, 5, 1);
	SetObjMapRange(ObjIndex(37, 46), 13, 0, 16, 5, 2);
}

void AddVileObjs()
{
	SetObjMapRange(ObjIndex(26, 45), 1, 1, 9, 10, 1);
	SetObjMapRange(ObjIndex(45, 46), 11, 1, 20, 10, 2);
	SetObjMapRange(ObjIndex(35, 36), 7, 11, 13, 18, 3);
}

void DRLG_SetMapTrans(Universe& universe, const char *sFileName)
{
	int x, y;
	int i, j;
	BYTE *pLevelMap;
	BYTE *d;
	DWORD dwOffset;

	pLevelMap = LoadFileInMem(sFileName, NULL);
	d = pLevelMap + 2;
	x = pLevelMap[0];
	y = *d;
	dwOffset = (x * y + 1) * 2;
	x <<= 1;
	y <<= 1;
	dwOffset += 3 * x * y * 2;
	d += dwOffset;

	for (j = 0; j < y; j++) {
		for (i = 0; i < x; i++) {
			universe.dTransVal[16 + i][16 + j] = *d;
			d += 2;
		}
	}
	mem_free_dbg(pLevelMap);
}

/**
 * @brief Load a quest map, the given map is specified via the global universe.setlvlnum
 */
void LoadSetMap(Universe& universe)
{
	switch (universe.setlvlnum) {
	case SL_SKELKING:
		if (quests[Q_SKELKING]._qactive == QUEST_INIT) {
			quests[Q_SKELKING]._qactive = QUEST_ACTIVE;
			quests[Q_SKELKING]._qvar1 = 1;
		}
		LoadPreL1Dungeon(universe, "Levels\\L1Data\\SklKng1.DUN", 83, 45);
		LoadL1Dungeon(universe, "Levels\\L1Data\\SklKng2.DUN", 83, 45);
		DRLG_AreaTrans(universe, sizeof(SkelKingTrans1) / 4, &SkelKingTrans1[0]);
		DRLG_ListTrans(universe, sizeof(SkelKingTrans2) / 4, &SkelKingTrans2[0]);
		DRLG_AreaTrans(universe, sizeof(SkelKingTrans3) / 4, &SkelKingTrans3[0]);
		DRLG_ListTrans(universe, sizeof(SkelKingTrans4) / 4, &SkelKingTrans4[0]);
		AddL1Objs(universe, 0, 0, MAXDUNX, MAXDUNY);
		AddSKingObjs();
		InitSKingTriggers();
		break;
	case SL_BONECHAMB:
		LoadPreL2Dungeon(universe, "Levels\\L2Data\\Bonecha2.DUN", 69, 39);
		LoadL2Dungeon(universe, "Levels\\L2Data\\Bonecha1.DUN", 69, 39);
		DRLG_ListTrans(universe, sizeof(SkelChamTrans1) / 4, &SkelChamTrans1[0]);
		DRLG_AreaTrans(universe, sizeof(SkelChamTrans2) / 4, &SkelChamTrans2[0]);
		DRLG_ListTrans(universe, sizeof(SkelChamTrans3) / 4, &SkelChamTrans3[0]);
		AddL2Objs(universe, 0, 0, MAXDUNX, MAXDUNY);
		AddSChamObjs();
		InitSChambTriggers();
		break;
	case SL_MAZE:
		LoadPreL1Dungeon(universe, "Levels\\L1Data\\Lv1MazeA.DUN", 20, 50);
		LoadL1Dungeon(universe, "Levels\\L1Data\\Lv1MazeB.DUN", 20, 50);
		AddL1Objs(universe, 0, 0, MAXDUNX, MAXDUNY);
		DRLG_SetMapTrans(universe, "Levels\\L1Data\\Lv1MazeA.DUN");
		break;
	case SL_POISONWATER:
		if (quests[Q_PWATER]._qactive == QUEST_INIT)
			quests[Q_PWATER]._qactive = QUEST_ACTIVE;
		LoadPreL3Dungeon(universe, "Levels\\L3Data\\Foulwatr.DUN", 19, 50);
		LoadL3Dungeon(universe, "Levels\\L3Data\\Foulwatr.DUN", 20, 50);
		InitPWaterTriggers();
		break;
	case SL_VILEBETRAYER:
		if (quests[Q_BETRAYER]._qactive == QUEST_DONE) {
			quests[Q_BETRAYER]._qvar2 = 4;
		} else if (quests[Q_BETRAYER]._qactive == QUEST_ACTIVE) {
			quests[Q_BETRAYER]._qvar2 = 3;
		}
		LoadPreL1Dungeon(universe, "Levels\\L1Data\\Vile1.DUN", 35, 36);
		LoadL1Dungeon(universe, "Levels\\L1Data\\Vile2.DUN", 35, 36);
		AddL1Objs(universe, 0, 0, MAXDUNX, MAXDUNY);
		AddVileObjs();
		DRLG_SetMapTrans(universe, "Levels\\L1Data\\Vile1.DUN");
		InitNoTriggers();
		break;
	}
}
#endif
