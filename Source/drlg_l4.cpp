/**
 * @file drlg_l4.cpp
 *
 * Implementation of the hell level generation algorithms.
 */

#include "Source/drlg_l4.h"

#include <optional>

#include "structs.h"
#include "types.h"
#include "Source/drlg_l1.h"
#include "Source/engine.h"
#include "Source/quests.h"
#include "Source/universe/universe.h"

#ifndef SPAWN

// TODO: Mark these as OpenCL __global
BYTE *lpSetPiece1;
BYTE *lpSetPiece2;
BYTE *lpSetPiece3;
BYTE *lpSetPiece4;

/**
 * A lookup table for the 16 possible patterns of a 2x2 area,
 * where each cell either contains a SW wall or it doesn't.
 */
const BYTE L4ConvTbl[16] = { 30, 6, 1, 6, 2, 6, 6, 6, 9, 6, 1, 6, 2, 6, 3, 6 };

/** Miniset: Stairs up. */
const BYTE L4USTAIRS[] = {
	// clang-format off
	4, 5, // width, height

	 6, 6, 6, 6, // search
	 6, 6, 6, 6,
	 6, 6, 6, 6,
	 6, 6, 6, 6,
	 6, 6, 6, 6,

	 0,  0,  0,  0, // replace
	36, 38, 35,  0,
	37, 34, 33, 32,
	 0,  0, 31,  0,
	 0,  0,  0,  0,
	// clang-format on
};
/** Miniset: Stairs up to town. */
const BYTE L4TWARP[] = {
	// clang-format off
	4, 5, // width, height

	6, 6, 6, 6, // search
	6, 6, 6, 6,
	6, 6, 6, 6,
	6, 6, 6, 6,
	6, 6, 6, 6,

	  0,   0,   0,   0, // replace
	134, 136, 133,   0,
	135, 132, 131, 130,
	  0,   0, 129,   0,
	  0,   0,   0,   0,
	// clang-format on
};
/** Miniset: Stairs down. */
const BYTE L4DSTAIRS[] = {
	// clang-format off
	5, 5, // width, height

	6, 6, 6, 6, 6, // search
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,

	0,  0,  0,  0, 0, // replace
	0,  0, 45, 41, 0,
	0, 44, 43, 40, 0,
	0, 46, 42, 39, 0,
	0,  0,  0,  0, 0,
	// clang-format on
};
/** Miniset: Pentagram. */
const BYTE L4PENTA[] = {
	// clang-format off
	5, 5, // width, height

	6, 6, 6, 6, 6, // search
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,

	0,   0,   0,   0, 0, // replace
	0,  98, 100, 103, 0,
	0,  99, 102, 105, 0,
	0, 101, 104, 106, 0,
	0,   0,   0,   0, 0,
	// clang-format on
};
/** Miniset: Pentagram portal. */
const BYTE L4PENTA2[] = {
	// clang-format off
	5, 5, // width, height

	6, 6, 6, 6, 6, // search
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,

	0,   0,   0,   0, 0, // replace
	0, 107, 109, 112, 0,
	0, 108, 111, 114, 0,
	0, 110, 113, 115, 0,
	0,   0,   0,   0, 0,
	// clang-format on
};

/** Maps tile IDs to their corresponding undecorated tile ID. */
const BYTE L4BTYPES[140] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 6,
	6, 6, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 1, 2, 1, 2, 1, 1, 2,
	2, 0, 0, 0, 0, 0, 0, 15, 16, 9,
	12, 4, 5, 7, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static BYTE GetDung(Universe& universe, int x, int y)
{
	if (x < 0 || y < 0 || x >= 20 || y >= 20) {
		int index = x * 20 + y;
		x = index / 20;
		y = index % 20;
		if (x < 0 || y < 0 || x >= 20) {
			universe.oobread = true;
			return 0;
		}
	}
	return universe.dung[x][y];
}

static void SetDung(Universe& universe, int x, int y, int value)
{
	if (x < 0 || y < 0 || x >= 20 || y >= 20) {
		int index = x * 20 + y;
		x = index / 20;
		y = index % 20;
		if (x < 0 || y < 0 || x >= 20) {
			universe.oobwrite = true;
			return;
		}
	}
	universe.dung[x][y] = value;
}

static void DRLG_L4Shadows(Universe& universe)
{
	int x, y;
	BOOL okflag;

	for (y = 1; y < DMAXY; y++) {
		for (x = 1; x < DMAXY; x++) {
			okflag = FALSE;
			if (GetDungeon(universe, x, y) == 3) {
				okflag = TRUE;
			}
			if (GetDungeon(universe, x, y) == 4) {
				okflag = TRUE;
			}
			if (GetDungeon(universe, x, y) == 8) {
				okflag = TRUE;
			}
			if (GetDungeon(universe, x, y) == 15) {
				okflag = TRUE;
			}
			if (!okflag) {
				continue;
			}
			if (GetDungeon(universe, x - 1, y) == 6) {
				SetDungeon(universe, x - 1, y, 47);
			}
			if (GetDungeon(universe, x - 1, y - 1) == 6) {
				SetDungeon(universe, x - 1, y - 1, 48);
			}
		}
	}
}

static void InitL4Dungeon(Universe& universe)
{
	int i, j;

	memset(universe.dung, 0, sizeof(universe.dung));
	memset(universe.L4dungeon, 0, sizeof(universe.L4dungeon));

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			SetDungeon(universe, i, j, 30);
			universe.dflags[i][j] = 0;
		}
	}
}

void DRLG_LoadL4SP(Universe& universe)
{
	universe.setloadflag = FALSE;
	if (QuestStatus(universe, Q_WARLORD)) {
		pSetPiece = LoadFileInMem("Levels\\L4Data\\Warlord.DUN", NULL);
		universe.setloadflag = TRUE;
	}
	if (universe.currlevel == 15 && universe.gbMaxPlayers != 1) {
		pSetPiece = LoadFileInMem("Levels\\L4Data\\Vile1.DUN", NULL);
		universe.setloadflag = TRUE;
	}
}

void DRLG_FreeL4SP()
{
	MemFreeDbg(pSetPiece);
}

void DRLG_L4SetSPRoom(Universe& universe, int rx1, int ry1)
{
	int rw, rh, i, j;
	BYTE *sp;

	rw = pSetPiece[0];
	rh = pSetPiece[2];

	universe.setpc_x = rx1;
	universe.setpc_y = ry1;
	universe.setpc_w = rw;
	universe.setpc_h = rh;

	sp = &pSetPiece[4];

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*sp != 0) {
				SetDungeon(universe, i + rx1, j + ry1, *sp);
				universe.dflags[i + rx1][j + ry1] |= DLRG_PROTECTED;
			} else {
				SetDungeon(universe, i + rx1, j + ry1, 6);
			}
			sp += 2;
		}
	}
}

static void L4makeDmt(Universe& universe)
{
	int i, j, idx, val, dmtx, dmty;

	for (j = 0, dmty = 1; dmty <= 77; j++, dmty += 2) {
		for (i = 0, dmtx = 1; dmtx <= 77; i++, dmtx += 2) {
			val = 8 * universe.L4dungeon[dmtx + 1][dmty + 1]
			    + 4 * universe.L4dungeon[dmtx][dmty + 1]
			    + 2 * universe.L4dungeon[dmtx + 1][dmty]
			    + universe.L4dungeon[dmtx][dmty];
			idx = L4ConvTbl[val];
			SetDungeon(universe, i, j, idx);
		}
	}
}

static int L4HWallOk(Universe& universe, int i, int j)
{
	int x;
	BOOL wallok;

	for (x = 1; GetDungeon(universe, i + x, j) == 6; x++) {
		if (universe.dflags[i + x][j] != 0) {
			break;
		}
		if (GetDungeon(universe, i + x, j - 1) != 6) {
			break;
		}
		if (GetDungeon(universe, i + x, j + 1) != 6) {
			break;
		}
	}

	wallok = FALSE;

	if (GetDungeon(universe, i + x, j) == 10) {
		wallok = TRUE;
	}
	if (GetDungeon(universe, i + x, j) == 12) {
		wallok = TRUE;
	}
	if (GetDungeon(universe, i + x, j) == 13) {
		wallok = TRUE;
	}
	if (GetDungeon(universe, i + x, j) == 15) {
		wallok = TRUE;
	}
	if (GetDungeon(universe, i + x, j) == 16) {
		wallok = TRUE;
	}
	if (GetDungeon(universe, i + x, j) == 21) {
		wallok = TRUE;
	}
	if (GetDungeon(universe, i + x, j) == 22) {
		wallok = TRUE;
	}
	if (x <= 3) {
		wallok = FALSE;
	}

	if (wallok) {
		return x;
	} else {
		return -1;
	}
}

static int L4VWallOk(Universe& universe, int i, int j)
{
	int y;
	BOOL wallok;

	for (y = 1; GetDungeon(universe, i, j + y) == 6; y++) {
		if (universe.dflags[i][j + y] != 0) {
			break;
		}
		if (GetDungeon(universe, i - 1, j + y) != 6) {
			break;
		}
		if (GetDungeon(universe, i + 1, j + y) != 6) {
			break;
		}
	}

	wallok = FALSE;

	if (GetDungeon(universe, i, j + y) == 8) {
		wallok = TRUE;
	}
	if (GetDungeon(universe, i, j + y) == 9) {
		wallok = TRUE;
	}
	if (GetDungeon(universe, i, j + y) == 11) {
		wallok = TRUE;
	}
	if (GetDungeon(universe, i, j + y) == 14) {
		wallok = TRUE;
	}
	if (GetDungeon(universe, i, j + y) == 15) {
		wallok = TRUE;
	}
	if (GetDungeon(universe, i, j + y) == 16) {
		wallok = TRUE;
	}
	if (GetDungeon(universe, i, j + y) == 21) {
		wallok = TRUE;
	}
	if (GetDungeon(universe, i, j + y) == 23) {
		wallok = TRUE;
	}
	if (y <= 3) {
		wallok = FALSE;
	}

	if (wallok) {
		return y;
	} else {
		return -1;
	}
}

static void L4HorizWall(Universe& universe, int i, int j, int dx)
{
	int xx;

	if (GetDungeon(universe, i, j) == 13) {
		SetDungeon(universe, i, j, 17);
	}
	if (GetDungeon(universe, i, j) == 16) {
		SetDungeon(universe, i, j, 11);
	}
	if (GetDungeon(universe, i, j) == 12) {
		SetDungeon(universe, i, j, 14);
	}

	for (xx = 1; xx < dx; xx++) {
		SetDungeon(universe, i + xx, j, 2);
	}

	if (GetDungeon(universe, i + dx, j) == 15) {
		SetDungeon(universe, i + dx, j, 14);
	}
	if (GetDungeon(universe, i + dx, j) == 10) {
		SetDungeon(universe, i + dx, j, 17);
	}
	if (GetDungeon(universe, i + dx, j) == 21) {
		SetDungeon(universe, i + dx, j, 23);
	}
	if (GetDungeon(universe, i + dx, j) == 22) {
		SetDungeon(universe, i + dx, j, 29);
	}

	xx = random_(universe, 0, dx - 3) + 1;
	SetDungeon(universe, i + xx, j, 57);
	SetDungeon(universe, i + xx + 2, j, 56);
	SetDungeon(universe, i + xx + 1, j, 60);

	if (GetDungeon(universe, i + xx, j - 1) == 6) {
		SetDungeon(universe, i + xx, j - 1, 58);
	}
	if (GetDungeon(universe, i + xx + 1, j - 1) == 6) {
		SetDungeon(universe, i + xx + 1, j - 1, 59);
	}
}

static void L4VertWall(Universe& universe, int i, int j, int dy)
{
	int yy;

	if (GetDungeon(universe, i, j) == 14) {
		SetDungeon(universe, i, j, 17);
	}
	if (GetDungeon(universe, i, j) == 8) {
		SetDungeon(universe, i, j, 9);
	}
	if (GetDungeon(universe, i, j) == 15) {
		SetDungeon(universe, i, j, 10);
	}

	for (yy = 1; yy < dy; yy++) {
		SetDungeon(universe, i, j + yy, 1);
	}

	if (GetDungeon(universe, i, j + dy) == 11) {
		SetDungeon(universe, i, j + dy, 17);
	}
	if (GetDungeon(universe, i, j + dy) == 9) {
		SetDungeon(universe, i, j + dy, 10);
	}
	if (GetDungeon(universe, i, j + dy) == 16) {
		SetDungeon(universe, i, j + dy, 13);
	}
	if (GetDungeon(universe, i, j + dy) == 21) {
		SetDungeon(universe, i, j + dy, 22);
	}
	if (GetDungeon(universe, i, j + dy) == 23) {
		SetDungeon(universe, i, j + dy, 29);
	}

	yy = random_(universe, 0, dy - 3) + 1;
	SetDungeon(universe, i, j + yy, 53);
	SetDungeon(universe, i, j + yy + 2, 52);
	SetDungeon(universe, i, j + yy + 1, 6);

	if (GetDungeon(universe, i - 1, j + yy) == 6) {
		SetDungeon(universe, i - 1, j + yy, 54);
	}
	if (GetDungeon(universe, i - 1, j + yy - 1) == 6) {
		SetDungeon(universe, i - 1, j + yy - 1, 55);
	}
}

static void L4AddWall(Universe& universe)
{
	int i, j, x, y;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (universe.dflags[i][j] != 0) {
				continue;
			}
			if (GetDungeon(universe, i, j) == 10 && random_(universe, 0, 100) < 100) {
				x = L4HWallOk(universe, i, j);
				if (x != -1) {
					L4HorizWall(universe, i, j, x);
				}
			}
			if (GetDungeon(universe, i, j) == 12 && random_(universe, 0, 100) < 100) {
				x = L4HWallOk(universe, i, j);
				if (x != -1) {
					L4HorizWall(universe, i, j, x);
				}
			}
			if (GetDungeon(universe, i, j) == 13 && random_(universe, 0, 100) < 100) {
				x = L4HWallOk(universe, i, j);
				if (x != -1) {
					L4HorizWall(universe, i, j, x);
				}
			}
			if (GetDungeon(universe, i, j) == 15 && random_(universe, 0, 100) < 100) {
				x = L4HWallOk(universe, i, j);
				if (x != -1) {
					L4HorizWall(universe, i, j, x);
				}
			}
			if (GetDungeon(universe, i, j) == 16 && random_(universe, 0, 100) < 100) {
				x = L4HWallOk(universe, i, j);
				if (x != -1) {
					L4HorizWall(universe, i, j, x);
				}
			}
			if (GetDungeon(universe, i, j) == 21 && random_(universe, 0, 100) < 100) {
				x = L4HWallOk(universe, i, j);
				if (x != -1) {
					L4HorizWall(universe, i, j, x);
				}
			}
			if (GetDungeon(universe, i, j) == 22 && random_(universe, 0, 100) < 100) {
				x = L4HWallOk(universe, i, j);
				if (x != -1) {
					L4HorizWall(universe, i, j, x);
				}
			}
			if (GetDungeon(universe, i, j) == 8 && random_(universe, 0, 100) < 100) {
				y = L4VWallOk(universe, i, j);
				if (y != -1) {
					L4VertWall(universe, i, j, y);
				}
			}
			if (GetDungeon(universe, i, j) == 9 && random_(universe, 0, 100) < 100) {
				y = L4VWallOk(universe, i, j);
				if (y != -1) {
					L4VertWall(universe, i, j, y);
				}
			}
			if (GetDungeon(universe, i, j) == 11 && random_(universe, 0, 100) < 100) {
				y = L4VWallOk(universe, i, j);
				if (y != -1) {
					L4VertWall(universe, i, j, y);
				}
			}
			if (GetDungeon(universe, i, j) == 14 && random_(universe, 0, 100) < 100) {
				y = L4VWallOk(universe, i, j);
				if (y != -1) {
					L4VertWall(universe, i, j, y);
				}
			}
			if (GetDungeon(universe, i, j) == 15 && random_(universe, 0, 100) < 100) {
				y = L4VWallOk(universe, i, j);
				if (y != -1) {
					L4VertWall(universe, i, j, y);
				}
			}
			if (GetDungeon(universe, i, j) == 16 && random_(universe, 0, 100) < 100) {
				y = L4VWallOk(universe, i, j);
				if (y != -1) {
					L4VertWall(universe, i, j, y);
				}
			}
			if (GetDungeon(universe, i, j) == 21 && random_(universe, 0, 100) < 100) {
				y = L4VWallOk(universe, i, j);
				if (y != -1) {
					L4VertWall(universe, i, j, y);
				}
			}
			if (GetDungeon(universe, i, j) == 23 && random_(universe, 0, 100) < 100) {
				y = L4VWallOk(universe, i, j);
				if (y != -1) {
					L4VertWall(universe, i, j, y);
				}
			}
		}
	}
}

static void L4tileFix(Universe& universe)
{
	int i, j;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (universe.dungeon[i][j] == 2 && universe.dungeon[i + 1][j] == 6)
				universe.dungeon[i + 1][j] = 5;
			if (universe.dungeon[i][j] == 2 && universe.dungeon[i + 1][j] == 1)
				universe.dungeon[i + 1][j] = 13;
			if (universe.dungeon[i][j] == 1 && universe.dungeon[i][j + 1] == 2)
				universe.dungeon[i][j + 1] = 14;
		}
	}
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (universe.dungeon[i][j] == 2 && universe.dungeon[i + 1][j] == 6)
				universe.dungeon[i + 1][j] = 2;
			if (universe.dungeon[i][j] == 2 && universe.dungeon[i + 1][j] == 9)
				universe.dungeon[i + 1][j] = 11;
			if (universe.dungeon[i][j] == 9 && universe.dungeon[i + 1][j] == 6)
				universe.dungeon[i + 1][j] = 12;
			if (universe.dungeon[i][j] == 14 && universe.dungeon[i + 1][j] == 1)
				universe.dungeon[i + 1][j] = 13;
			if (universe.dungeon[i][j] == 6 && universe.dungeon[i + 1][j] == 14)
				universe.dungeon[i + 1][j] = 15;
			if (universe.dungeon[i][j] == 6 && universe.dungeon[i][j + 1] == 13)
				universe.dungeon[i][j + 1] = 16;
			if (universe.dungeon[i][j] == 1 && universe.dungeon[i][j + 1] == 9)
				universe.dungeon[i][j + 1] = 10;
			if (universe.dungeon[i][j] == 6 && universe.dungeon[i][j - 1] == 1)
				universe.dungeon[i][j - 1] = 1;
		}
	}
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (universe.dungeon[i][j] == 13 && universe.dungeon[i][j + 1] == 30)
				universe.dungeon[i][j + 1] = 27;
			if (universe.dungeon[i][j] == 27 && universe.dungeon[i + 1][j] == 30)
				universe.dungeon[i + 1][j] = 19;
			if (universe.dungeon[i][j] == 1 && universe.dungeon[i][j + 1] == 30)
				universe.dungeon[i][j + 1] = 27;
			if (universe.dungeon[i][j] == 27 && universe.dungeon[i + 1][j] == 1)
				universe.dungeon[i + 1][j] = 16;
			if (universe.dungeon[i][j] == 19 && universe.dungeon[i + 1][j] == 27)
				universe.dungeon[i + 1][j] = 26;
			if (universe.dungeon[i][j] == 27 && universe.dungeon[i + 1][j] == 30)
				universe.dungeon[i + 1][j] = 19;
			if (universe.dungeon[i][j] == 2 && universe.dungeon[i + 1][j] == 15)
				universe.dungeon[i + 1][j] = 14;
			if (universe.dungeon[i][j] == 14 && universe.dungeon[i + 1][j] == 15)
				universe.dungeon[i + 1][j] = 14;
			if (universe.dungeon[i][j] == 22 && universe.dungeon[i + 1][j] == 1)
				universe.dungeon[i + 1][j] = 16;
			if (universe.dungeon[i][j] == 27 && universe.dungeon[i + 1][j] == 1)
				universe.dungeon[i + 1][j] = 16;
			if (universe.dungeon[i][j] == 6 && universe.dungeon[i + 1][j] == 27 && universe.dungeon[i + 1][j + 1] != 0) /* check */
				universe.dungeon[i + 1][j] = 22;
			if (universe.dungeon[i][j] == 22 && universe.dungeon[i + 1][j] == 30)
				universe.dungeon[i + 1][j] = 19;
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i + 1][j] == 1 && universe.dungeon[i + 1][j - 1] == 1)
				universe.dungeon[i + 1][j] = 13;
			if (universe.dungeon[i][j] == 14 && universe.dungeon[i + 1][j] == 30 && universe.dungeon[i][j + 1] == 6)
				universe.dungeon[i + 1][j] = 28;
			if (universe.dungeon[i][j] == 16 && universe.dungeon[i + 1][j] == 6 && universe.dungeon[i][j + 1] == 30)
				universe.dungeon[i][j + 1] = 27;
			if (universe.dungeon[i][j] == 16 && universe.dungeon[i][j + 1] == 30 && universe.dungeon[i + 1][j + 1] == 30)
				universe.dungeon[i][j + 1] = 27;
			if (universe.dungeon[i][j] == 6 && universe.dungeon[i + 1][j] == 30 && universe.dungeon[i + 1][j - 1] == 6)
				universe.dungeon[i + 1][j] = 21;
			if (universe.dungeon[i][j] == 2 && universe.dungeon[i + 1][j] == 27 && universe.dungeon[i + 1][j + 1] == 9)
				universe.dungeon[i + 1][j] = 29;
			if (universe.dungeon[i][j] == 9 && universe.dungeon[i + 1][j] == 15)
				universe.dungeon[i + 1][j] = 14;
			if (universe.dungeon[i][j] == 15 && universe.dungeon[i + 1][j] == 27 && universe.dungeon[i + 1][j + 1] == 2)
				universe.dungeon[i + 1][j] = 29;
			if (universe.dungeon[i][j] == 19 && universe.dungeon[i + 1][j] == 18)
				universe.dungeon[i + 1][j] = 24;
			if (universe.dungeon[i][j] == 9 && universe.dungeon[i + 1][j] == 15)
				universe.dungeon[i + 1][j] = 14;
			if (universe.dungeon[i][j] == 19 && universe.dungeon[i + 1][j] == 19 && universe.dungeon[i + 1][j - 1] == 30)
				universe.dungeon[i + 1][j] = 24;
			if (universe.dungeon[i][j] == 24 && universe.dungeon[i][j - 1] == 30 && universe.dungeon[i][j - 2] == 6)
				universe.dungeon[i][j - 1] = 21;
			if (universe.dungeon[i][j] == 2 && universe.dungeon[i + 1][j] == 30)
				universe.dungeon[i + 1][j] = 28;
			if (universe.dungeon[i][j] == 15 && universe.dungeon[i + 1][j] == 30)
				universe.dungeon[i + 1][j] = 28;
			if (universe.dungeon[i][j] == 28 && universe.dungeon[i][j + 1] == 30)
				universe.dungeon[i][j + 1] = 18;
			if (universe.dungeon[i][j] == 28 && universe.dungeon[i][j + 1] == 2)
				universe.dungeon[i][j + 1] = 15;
			if (universe.dungeon[i][j] == 19 && universe.dungeon[i + 2][j] == 2 && universe.dungeon[i + 1][j - 1] == 18 && universe.dungeon[i + 1][j + 1] == 1)
				universe.dungeon[i + 1][j] = 17;
			if (universe.dungeon[i][j] == 19 && universe.dungeon[i + 2][j] == 2 && universe.dungeon[i + 1][j - 1] == 22 && universe.dungeon[i + 1][j + 1] == 1)
				universe.dungeon[i + 1][j] = 17;
			if (universe.dungeon[i][j] == 19 && universe.dungeon[i + 2][j] == 2 && universe.dungeon[i + 1][j - 1] == 18 && universe.dungeon[i + 1][j + 1] == 13)
				universe.dungeon[i + 1][j] = 17;
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i + 2][j] == 2 && universe.dungeon[i + 1][j - 1] == 18 && universe.dungeon[i + 1][j + 1] == 1)
				universe.dungeon[i + 1][j] = 17;
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i + 1][j + 1] == 1 && universe.dungeon[i + 1][j - 1] == 22 && universe.dungeon[i + 2][j] == 3)
				universe.dungeon[i + 1][j] = 17;
			if (universe.dungeon[i][j] == 15 && universe.dungeon[i + 1][j] == 28 && universe.dungeon[i + 2][j] == 30 && universe.dungeon[i + 1][j - 1] == 6)
				universe.dungeon[i + 1][j] = 23;
			if (universe.dungeon[i][j] == 14 && universe.dungeon[i + 1][j] == 28 && universe.dungeon[i + 2][j] == 1)
				universe.dungeon[i + 1][j] = 23;
			if (universe.dungeon[i][j] == 15 && universe.dungeon[i + 1][j] == 27 && universe.dungeon[i + 1][j + 1] == 30)
				universe.dungeon[i + 1][j] = 29;
			if (universe.dungeon[i][j] == 28 && universe.dungeon[i][j + 1] == 9)
				universe.dungeon[i][j + 1] = 15;
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i + 1][j - 1] == 21)
				universe.dungeon[i + 1][j] = 24;
			if (universe.dungeon[i][j] == 2 && universe.dungeon[i + 1][j] == 27 && universe.dungeon[i + 1][j + 1] == 30)
				universe.dungeon[i + 1][j] = 29;
			if (universe.dungeon[i][j] == 2 && universe.dungeon[i + 1][j] == 18)
				universe.dungeon[i + 1][j] = 25;
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i + 1][j] == 9 && universe.dungeon[i + 2][j] == 2)
				universe.dungeon[i + 1][j] = 11;
			if (universe.dungeon[i][j] == 19 && universe.dungeon[i + 1][j] == 10)
				universe.dungeon[i + 1][j] = 17;
			if (universe.dungeon[i][j] == 15 && universe.dungeon[i][j + 1] == 3)
				universe.dungeon[i][j + 1] = 4;
			if (universe.dungeon[i][j] == 22 && universe.dungeon[i][j + 1] == 9)
				universe.dungeon[i][j + 1] = 15;
			if (universe.dungeon[i][j] == 18 && universe.dungeon[i][j + 1] == 30)
				universe.dungeon[i][j + 1] = 18;
			if (universe.dungeon[i][j] == 24 && universe.dungeon[i - 1][j] == 30)
				universe.dungeon[i - 1][j] = 19;
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i][j + 1] == 2)
				universe.dungeon[i][j + 1] = 15;
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i][j + 1] == 9)
				universe.dungeon[i][j + 1] = 10;
			if (universe.dungeon[i][j] == 22 && universe.dungeon[i][j + 1] == 30)
				universe.dungeon[i][j + 1] = 18;
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i][j + 1] == 30)
				universe.dungeon[i][j + 1] = 18;
			if (universe.dungeon[i][j] == 16 && universe.dungeon[i][j + 1] == 2)
				universe.dungeon[i][j + 1] = 15;
			if (universe.dungeon[i][j] == 13 && universe.dungeon[i][j + 1] == 2)
				universe.dungeon[i][j + 1] = 15;
			if (universe.dungeon[i][j] == 22 && universe.dungeon[i][j + 1] == 2)
				universe.dungeon[i][j + 1] = 15;
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i + 1][j] == 18 && universe.dungeon[i + 2][j] == 30)
				universe.dungeon[i + 1][j] = 24;
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i + 1][j] == 9 && universe.dungeon[i + 1][j + 1] == 1)
				universe.dungeon[i + 1][j] = 16;
			if (universe.dungeon[i][j] == 2 && universe.dungeon[i + 1][j] == 27 && universe.dungeon[i + 1][j + 1] == 2)
				universe.dungeon[i + 1][j] = 29;
			if (universe.dungeon[i][j] == 23 && universe.dungeon[i][j + 1] == 2)
				universe.dungeon[i][j + 1] = 15;
			if (universe.dungeon[i][j] == 23 && universe.dungeon[i][j + 1] == 9)
				universe.dungeon[i][j + 1] = 15;
			if (universe.dungeon[i][j] == 25 && universe.dungeon[i][j + 1] == 2)
				universe.dungeon[i][j + 1] = 15;
			if (universe.dungeon[i][j] == 22 && universe.dungeon[i + 1][j] == 9)
				universe.dungeon[i + 1][j] = 11;
			if (universe.dungeon[i][j] == 23 && universe.dungeon[i + 1][j] == 9)
				universe.dungeon[i + 1][j] = 11;
			if (universe.dungeon[i][j] == 15 && universe.dungeon[i + 1][j] == 1)
				universe.dungeon[i + 1][j] = 16;
			if (universe.dungeon[i][j] == 11 && universe.dungeon[i + 1][j] == 15)
				universe.dungeon[i + 1][j] = 14;
			if (universe.dungeon[i][j] == 23 && universe.dungeon[i + 1][j] == 1)
				universe.dungeon[i + 1][j] = 16;
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i + 1][j] == 27)
				universe.dungeon[i + 1][j] = 26;
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i + 1][j] == 18)
				universe.dungeon[i + 1][j] = 24;
			if (universe.dungeon[i][j] == 26 && universe.dungeon[i + 1][j] == 1)
				universe.dungeon[i + 1][j] = 16;
			if (universe.dungeon[i][j] == 29 && universe.dungeon[i + 1][j] == 1)
				universe.dungeon[i + 1][j] = 16;
			if (universe.dungeon[i][j] == 29 && universe.dungeon[i][j + 1] == 2)
				universe.dungeon[i][j + 1] = 15;
			if (universe.dungeon[i][j] == 1 && universe.dungeon[i][j - 1] == 15)
				universe.dungeon[i][j - 1] = 10;
			if (universe.dungeon[i][j] == 18 && universe.dungeon[i][j + 1] == 2)
				universe.dungeon[i][j + 1] = 15;
			if (universe.dungeon[i][j] == 23 && universe.dungeon[i][j + 1] == 30)
				universe.dungeon[i][j + 1] = 18;
			if (universe.dungeon[i][j] == 18 && universe.dungeon[i][j + 1] == 9)
				universe.dungeon[i][j + 1] = 10;
			if (universe.dungeon[i][j] == 14 && universe.dungeon[i + 1][j] == 30 && universe.dungeon[i + 1][j + 1] == 30)
				universe.dungeon[i + 1][j] = 23;
			if (universe.dungeon[i][j] == 2 && universe.dungeon[i + 1][j] == 28 && universe.dungeon[i + 1][j - 1] == 6)
				universe.dungeon[i + 1][j] = 23;
			if (universe.dungeon[i][j] == 23 && universe.dungeon[i + 1][j] == 18 && universe.dungeon[i][j - 1] == 6)
				universe.dungeon[i + 1][j] = 24;
			if (universe.dungeon[i][j] == 14 && universe.dungeon[i + 1][j] == 23 && universe.dungeon[i + 2][j] == 30)
				universe.dungeon[i + 1][j] = 28;
			if (universe.dungeon[i][j] == 14 && universe.dungeon[i + 1][j] == 28 && universe.dungeon[i + 2][j] == 30 && universe.dungeon[i + 1][j - 1] == 6)
				universe.dungeon[i + 1][j] = 23;
			if (universe.dungeon[i][j] == 23 && universe.dungeon[i + 1][j] == 30)
				universe.dungeon[i + 1][j] = 19;
			if (universe.dungeon[i][j] == 29 && universe.dungeon[i + 1][j] == 30)
				universe.dungeon[i + 1][j] = 19;
			if (universe.dungeon[i][j] == 29 && universe.dungeon[i][j + 1] == 30)
				universe.dungeon[i][j + 1] = 18;
			if (universe.dungeon[i][j] == 19 && universe.dungeon[i + 1][j] == 30)
				universe.dungeon[i + 1][j] = 19;
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i + 1][j] == 30)
				universe.dungeon[i + 1][j] = 19;
			if (universe.dungeon[i][j] == 26 && universe.dungeon[i + 1][j] == 30)
				universe.dungeon[i + 1][j] = 19;
			if (universe.dungeon[i][j] == 16 && universe.dungeon[i][j + 1] == 30)
				universe.dungeon[i][j + 1] = 18;
			if (universe.dungeon[i][j] == 13 && universe.dungeon[i][j + 1] == 9)
				universe.dungeon[i][j + 1] = 10;
			if (universe.dungeon[i][j] == 25 && universe.dungeon[i][j + 1] == 30)
				universe.dungeon[i][j + 1] = 18;
			if (universe.dungeon[i][j] == 18 && universe.dungeon[i][j + 1] == 2)
				universe.dungeon[i][j + 1] = 15;
			if (universe.dungeon[i][j] == 11 && universe.dungeon[i + 1][j] == 3)
				universe.dungeon[i + 1][j] = 5;
			if (universe.dungeon[i][j] == 19 && universe.dungeon[i + 1][j] == 9)
				universe.dungeon[i + 1][j] = 11;
			if (universe.dungeon[i][j] == 19 && universe.dungeon[i + 1][j] == 1)
				universe.dungeon[i + 1][j] = 13;
			if (universe.dungeon[i][j] == 19 && universe.dungeon[i + 1][j] == 13 && universe.dungeon[i + 1][j - 1] == 6)
				universe.dungeon[i + 1][j] = 16;
		}
	}
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i][j + 1] == 24 && universe.dungeon[i][j + 2] == 1)
				universe.dungeon[i][j + 1] = 17;
			if (universe.dungeon[i][j] == 15 && universe.dungeon[i + 1][j + 1] == 9 && universe.dungeon[i + 1][j - 1] == 1 && universe.dungeon[i + 2][j] == 16)
				universe.dungeon[i + 1][j] = 29;
			if (universe.dungeon[i][j] == 2 && universe.dungeon[i - 1][j] == 6)
				universe.dungeon[i - 1][j] = 8;
			if (universe.dungeon[i][j] == 1 && universe.dungeon[i][j - 1] == 6)
				universe.dungeon[i][j - 1] = 7;
			if (universe.dungeon[i][j] == 6 && universe.dungeon[i + 1][j] == 15 && universe.dungeon[i + 1][j + 1] == 4)
				universe.dungeon[i + 1][j] = 10;
			if (universe.dungeon[i][j] == 1 && universe.dungeon[i][j + 1] == 3)
				universe.dungeon[i][j + 1] = 4;
			if (universe.dungeon[i][j] == 1 && universe.dungeon[i][j + 1] == 6)
				universe.dungeon[i][j + 1] = 4;
			if (universe.dungeon[i][j] == 9 && universe.dungeon[i][j + 1] == 3)
				universe.dungeon[i][j + 1] = 4;
			if (universe.dungeon[i][j] == 10 && universe.dungeon[i][j + 1] == 3)
				universe.dungeon[i][j + 1] = 4;
			if (universe.dungeon[i][j] == 13 && universe.dungeon[i][j + 1] == 3)
				universe.dungeon[i][j + 1] = 4;
			if (universe.dungeon[i][j] == 1 && universe.dungeon[i][j + 1] == 5)
				universe.dungeon[i][j + 1] = 12;
			if (universe.dungeon[i][j] == 1 && universe.dungeon[i][j + 1] == 16)
				universe.dungeon[i][j + 1] = 13;
			if (universe.dungeon[i][j] == 6 && universe.dungeon[i][j + 1] == 13)
				universe.dungeon[i][j + 1] = 16;
			if (universe.dungeon[i][j] == 25 && universe.dungeon[i][j + 1] == 9)
				universe.dungeon[i][j + 1] = 10;
			if (universe.dungeon[i][j] == 13 && universe.dungeon[i][j + 1] == 5)
				universe.dungeon[i][j + 1] = 12;
			if (universe.dungeon[i][j] == 28 && universe.dungeon[i][j - 1] == 6 && universe.dungeon[i + 1][j] == 1)
				universe.dungeon[i + 1][j] = 23;
			if (universe.dungeon[i][j] == 19 && universe.dungeon[i + 1][j] == 10)
				universe.dungeon[i + 1][j] = 17;
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i + 1][j] == 9)
				universe.dungeon[i + 1][j] = 11;
			if (universe.dungeon[i][j] == 11 && universe.dungeon[i + 1][j] == 3)
				universe.dungeon[i + 1][j] = 5;
			if (universe.dungeon[i][j] == 10 && universe.dungeon[i + 1][j] == 4)
				universe.dungeon[i + 1][j] = 12;
			if (universe.dungeon[i][j] == 14 && universe.dungeon[i + 1][j] == 4)
				universe.dungeon[i + 1][j] = 12;
			if (universe.dungeon[i][j] == 27 && universe.dungeon[i + 1][j] == 9)
				universe.dungeon[i + 1][j] = 11;
			if (universe.dungeon[i][j] == 15 && universe.dungeon[i + 1][j] == 4)
				universe.dungeon[i + 1][j] = 12;
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i + 1][j] == 1)
				universe.dungeon[i + 1][j] = 16;
			if (universe.dungeon[i][j] == 11 && universe.dungeon[i + 1][j] == 4)
				universe.dungeon[i + 1][j] = 12;
			if (universe.dungeon[i][j] == 2 && universe.dungeon[i + 1][j] == 3)
				universe.dungeon[i + 1][j] = 5;
			if (universe.dungeon[i][j] == 9 && universe.dungeon[i + 1][j] == 3)
				universe.dungeon[i + 1][j] = 5;
			if (universe.dungeon[i][j] == 14 && universe.dungeon[i + 1][j] == 3)
				universe.dungeon[i + 1][j] = 5;
			if (universe.dungeon[i][j] == 15 && universe.dungeon[i + 1][j] == 3)
				universe.dungeon[i + 1][j] = 5;
			if (universe.dungeon[i][j] == 2 && universe.dungeon[i + 1][j] == 5 && universe.dungeon[i + 1][j - 1] == 16)
				universe.dungeon[i + 1][j] = 12;
			if (universe.dungeon[i][j] == 2 && universe.dungeon[i + 1][j] == 4)
				universe.dungeon[i + 1][j] = 12;
			if (universe.dungeon[i][j] == 9 && universe.dungeon[i + 1][j] == 4)
				universe.dungeon[i + 1][j] = 12;
			if (universe.dungeon[i][j] == 1 && universe.dungeon[i][j - 1] == 8)
				universe.dungeon[i][j - 1] = 9;
			if (universe.dungeon[i][j] == 28 && universe.dungeon[i + 1][j] == 23 && universe.dungeon[i + 1][j + 1] == 3)
				universe.dungeon[i + 1][j] = 16;
		}
	}
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i + 1][j] == 10)
				universe.dungeon[i + 1][j] = 17;
			if (universe.dungeon[i][j] == 17 && universe.dungeon[i + 1][j] == 4)
				universe.dungeon[i + 1][j] = 12;
			if (universe.dungeon[i][j] == 10 && universe.dungeon[i + 1][j] == 4)
				universe.dungeon[i + 1][j] = 12;
			if (universe.dungeon[i][j] == 17 && universe.dungeon[i][j + 1] == 5)
				universe.dungeon[i][j + 1] = 12;
			if (universe.dungeon[i][j] == 29 && universe.dungeon[i][j + 1] == 9)
				universe.dungeon[i][j + 1] = 10;
			if (universe.dungeon[i][j] == 13 && universe.dungeon[i][j + 1] == 5)
				universe.dungeon[i][j + 1] = 12;
			if (universe.dungeon[i][j] == 9 && universe.dungeon[i][j + 1] == 16)
				universe.dungeon[i][j + 1] = 13;
			if (universe.dungeon[i][j] == 10 && universe.dungeon[i][j + 1] == 16)
				universe.dungeon[i][j + 1] = 13;
			if (universe.dungeon[i][j] == 16 && universe.dungeon[i][j + 1] == 3)
				universe.dungeon[i][j + 1] = 4;
			if (universe.dungeon[i][j] == 11 && universe.dungeon[i][j + 1] == 5)
				universe.dungeon[i][j + 1] = 12;
			if (universe.dungeon[i][j] == 10 && universe.dungeon[i + 1][j] == 3 && universe.dungeon[i + 1][j - 1] == 16)
				universe.dungeon[i + 1][j] = 12;
			if (universe.dungeon[i][j] == 16 && universe.dungeon[i][j + 1] == 5)
				universe.dungeon[i][j + 1] = 12;
			if (universe.dungeon[i][j] == 1 && universe.dungeon[i][j + 1] == 6)
				universe.dungeon[i][j + 1] = 4;
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i + 1][j] == 13 && universe.dungeon[i][j + 1] == 10)
				universe.dungeon[i + 1][j + 1] = 12;
			if (universe.dungeon[i][j] == 15 && universe.dungeon[i + 1][j] == 10)
				universe.dungeon[i + 1][j] = 17;
			if (universe.dungeon[i][j] == 22 && universe.dungeon[i][j + 1] == 11)
				universe.dungeon[i][j + 1] = 17;
			if (universe.dungeon[i][j] == 15 && universe.dungeon[i + 1][j] == 28 && universe.dungeon[i + 2][j] == 16)
				universe.dungeon[i + 1][j] = 23;
			if (universe.dungeon[i][j] == 28 && universe.dungeon[i + 1][j] == 23 && universe.dungeon[i + 1][j + 1] == 1 && universe.dungeon[i + 2][j] == 6)
				universe.dungeon[i + 1][j] = 16;
		}
	}
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (universe.dungeon[i][j] == 15 && universe.dungeon[i + 1][j] == 28 && universe.dungeon[i + 2][j] == 16)
				universe.dungeon[i + 1][j] = 23;
			if (universe.dungeon[i][j] == 21 && universe.dungeon[i + 1][j - 1] == 21 && universe.dungeon[i + 1][j + 1] == 13 && universe.dungeon[i + 2][j] == 2)
				universe.dungeon[i + 1][j] = 17;
			if (universe.dungeon[i][j] == 19 && universe.dungeon[i + 1][j] == 15 && universe.dungeon[i + 1][j + 1] == 12)
				universe.dungeon[i + 1][j] = 17;
		}
	}
}

static void DRLG_L4Subs(Universe& universe)
{
	int x, y, i, rv;
	BYTE c;

	for (y = 0; y < DMAXY; y++) {
		for (x = 0; x < DMAXX; x++) {
			rv = random_(universe, 0, 3);
			if (rv == 0) {
				c = GetDungeon(universe, x, y);
				c = L4BTYPES[c];
				if (c != 0 && universe.dflags[x][y] == 0) {
					rv = random_(universe, 0, 16);
					i = -1;
					while (rv >= 0) {
						i++;
						if (i == sizeof(L4BTYPES)) {
							i = 0;
						}
						if (c == L4BTYPES[i]) {
							rv--;
						}
					}
					SetDungeon(universe, x, y, i);
				}
			}
		}
	}
	for (y = 0; y < DMAXY; y++) {
		for (x = 0; x < DMAXX; x++) {
			rv = random_(universe, 0, 10);
			if (rv == 0) {
				c = GetDungeon(universe, x, y);
				if (L4BTYPES[c] == 6 && universe.dflags[x][y] == 0) {
					SetDungeon(universe, x, y, random_(universe, 0, 3) + 95);
				}
			}
		}
	}
}

static void L4makeDungeon(Universe& universe)
{
	int i, j, k, l;

	for (j = 0; j < 20; j++) {
		for (i = 0; i < 20; i++) {
			k = i << 1;
			l = j << 1;
			universe.L4dungeon[k][l] = universe.dung[i][j];
			universe.L4dungeon[k][l + 1] = universe.dung[i][j];
			universe.L4dungeon[k + 1][l] = universe.dung[i][j];
			universe.L4dungeon[k + 1][l + 1] = universe.dung[i][j];
		}
	}
	for (j = 0; j < 20; j++) {
		for (i = 0; i < 20; i++) {
			k = i << 1;
			l = j << 1;
			universe.L4dungeon[k][l + 40] = universe.dung[i][19 - j];
			universe.L4dungeon[k][l + 41] = universe.dung[i][19 - j];
			universe.L4dungeon[k + 1][l + 40] = universe.dung[i][19 - j];
			universe.L4dungeon[k + 1][l + 41] = universe.dung[i][19 - j];
		}
	}
	for (j = 0; j < 20; j++) {
		for (i = 0; i < 20; i++) {
			k = i << 1;
			l = j << 1;
			universe.L4dungeon[k + 40][l] = universe.dung[19 - i][j];
			universe.L4dungeon[k + 40][l + 1] = universe.dung[19 - i][j];
			universe.L4dungeon[k + 41][l] = universe.dung[19 - i][j];
			universe.L4dungeon[k + 41][l + 1] = universe.dung[19 - i][j];
		}
	}
	for (j = 0; j < 20; j++) {
		for (i = 0; i < 20; i++) {
			k = i << 1;
			l = j << 1;
			universe.L4dungeon[k + 40][l + 40] = universe.dung[19 - i][19 - j];
			universe.L4dungeon[k + 40][l + 41] = universe.dung[19 - i][19 - j];
			universe.L4dungeon[k + 41][l + 40] = universe.dung[19 - i][19 - j];
			universe.L4dungeon[k + 41][l + 41] = universe.dung[19 - i][19 - j];
		}
	}
}

static void uShape(Universe& universe)
{
	int j, i, rv;

	for (j = 19; j >= 0; j--) {
		for (i = 19; i >= 0; i--) {
			if (universe.dung[i][j] != 1) {
				universe.hallok[j] = FALSE;
			}
			if (universe.dung[i][j] == 1) {
				// BUGFIX: check that i + 1 < 20 and j + 1 < 20
				if (universe.dung[i][j + 1] == 1 && universe.dung[i + 1][j + 1] == 0) {
					universe.hallok[j] = TRUE;
				} else {
					universe.hallok[j] = FALSE;
				}
				i = 0;
			}
		}
	}

	rv = random_(universe, 0, 19) + 1;
	do {
		if (universe.hallok[rv]) {
			for (i = 19; i >= 0; i--) {
				if (universe.dung[i][rv] == 1) {
					i = -1;
					rv = 0;
				} else {
					universe.dung[i][rv] = 1;
					universe.dung[i][rv + 1] = 1;
				}
			}
		} else {
			rv++;
			if (rv == 20) {
				rv = 1;
			}
		}
	} while (rv != 0);

	for (i = 19; i >= 0; i--) {
		for (j = 19; j >= 0; j--) {
			if (universe.dung[i][j] != 1) {
				universe.hallok[i] = FALSE;
			}
			if (universe.dung[i][j] == 1) {
				// BUGFIX: check that i + 1 < 20 and j + 1 < 20
				if (GetDung(universe, i + 1, j) == 1 && GetDung(universe, i + 1, j + 1) == 0) {
					universe.hallok[i] = TRUE;
				} else {
					universe.hallok[i] = FALSE;
				}
				j = 0;
			}
		}
	}

	rv = random_(universe, 0, 19) + 1;
	do {
		if (universe.hallok[rv]) {
			for (j = 19; j >= 0; j--) {
				if (universe.dung[rv][j] == 1) {
					j = -1;
					rv = 0;
				} else {
					universe.dung[rv][j] = 1;
					universe.dung[rv + 1][j] = 1;
				}
			}
		} else {
			rv++;
			if (rv == 20) {
				rv = 1;
			}
		}
	} while (rv != 0);
}

static long GetArea(Universe& universe)
{
	int i, j;
	long rv;

	rv = 0;

	for (j = 0; j < 20; j++) {
		for (i = 0; i < 20; i++) {
			if (universe.dung[i][j] == 1) {
				rv++;
			}
		}
	}

	return rv;
}

static void L4drawRoom(Universe& universe, int x, int y, int width, int height)
{
	int i, j;

	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			SetDung(universe, i + x, j + y, 1);
		}
	}
}

static BOOL L4checkRoom(Universe& universe, int x, int y, int width, int height)
{
	int i, j;

	if (x <= 0 || y <= 0) {
		return FALSE;
	}

	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			if (i + x < 0 || i + x >= 20 || j + y < 0 || j + y >= 20) {
				return FALSE;
			}
			if (universe.dung[i + x][j + y] != 0) {
				return FALSE;
			}
		}
	}

	return TRUE;
}

static void L4roomGen(Universe& universe, int x, int y, int w, int h, int dir)
{
	int num;
	BOOL ran, ran2;
	int width, height, rx, ry, ry2;
	int cw, ch, cx1, cy1, cx2;

	int dirProb = random_(universe, 0, 4);

	if (!(dir == 1 ? dirProb != 0 : dirProb == 0)) {
		num = 0;
		do {
			cw = (random_(universe, 0, 5) + 2) & ~1;
			ch = (random_(universe, 0, 5) + 2) & ~1;
			cy1 = h / 2 + y - ch / 2;
			cx1 = x - cw;
			ran = L4checkRoom(universe, cx1 - 1, cy1 - 1, ch + 2, cw + 1); /// BUGFIX: swap args 3 and 4 ("ch+2" and "cw+1")
			num++;
		} while (ran == FALSE && num < 20);

		if (ran == TRUE)
			L4drawRoom(universe, cx1, cy1, cw, ch);
		cx2 = x + w;
		ran2 = L4checkRoom(universe, cx2, cy1 - 1, cw + 1, ch + 2);
		if (ran2 == TRUE)
			L4drawRoom(universe, cx2, cy1, cw, ch);
		if (ran == TRUE)
			L4roomGen(universe, cx1, cy1, cw, ch, 1);
		if (ran2 == TRUE)
			L4roomGen(universe, cx2, cy1, cw, ch, 1);
	} else {
		num = 0;
		do {
			width = (random_(universe, 0, 5) + 2) & ~1;
			height = (random_(universe, 0, 5) + 2) & ~1;
			rx = w / 2 + x - width / 2;
			ry = y - height;
			ran = L4checkRoom(universe, rx - 1, ry - 1, width + 2, height + 1);
			num++;
		} while (ran == FALSE && num < 20);

		if (ran == TRUE)
			L4drawRoom(universe, rx, ry, width, height);
		ry2 = y + h;
		ran2 = L4checkRoom(universe, rx - 1, ry2, width + 2, height + 1);
		if (ran2 == TRUE)
			L4drawRoom(universe, rx, ry2, width, height);
		if (ran == TRUE)
			L4roomGen(universe, rx, ry, width, height, 0);
		if (ran2 == TRUE)
			L4roomGen(universe, rx, ry2, width, height, 0);
	}
}

static void L4firstRoom(Universe& universe)
{
	int x, y, w, h, rndx, rndy, xmin, xmax, ymin, ymax, tx, ty;

	if (universe.currlevel != 16) {
		if (universe.currlevel == universe.quests[Q_WARLORD]._qlevel && universe.quests[Q_WARLORD]._qactive != QUEST_NOTAVAIL) {
			/// ASSERT: assert(universe.gbMaxPlayers == 1);
			w = 11;
			h = 11;
		} else if (universe.currlevel == universe.quests[Q_BETRAYER]._qlevel && universe.gbMaxPlayers != 1) {
			w = 11;
			h = 11;
		} else {
			w = random_(universe, 0, 5) + 2;
			h = random_(universe, 0, 5) + 2;
		}
	} else {
		w = 14;
		h = 14;
	}

	xmin = (20 - w) >> 1;
	xmax = 19 - w;
	rndx = random_(universe, 0, xmax - xmin + 1) + xmin;
	if (rndx + w > 19) {
		tx = w + rndx - 19;
		x = rndx - tx + 1;
	} else {
		x = rndx;
	}
	ymin = (20 - h) >> 1;
	ymax = 19 - h;
	rndy = random_(universe, 0, ymax - ymin + 1) + ymin;
	if (rndy + h > 19) {
		ty = h + rndy - 19;
		y = rndy - ty + 1;
	} else {
		y = rndy;
	}

	if (universe.currlevel == 16) {
		universe.l4holdx = x;
		universe.l4holdy = y;
	}
	if (QuestStatus(universe, Q_WARLORD) || universe.currlevel == universe.quests[Q_BETRAYER]._qlevel && universe.gbMaxPlayers != 1) {
		universe.SP4x1 = x + 1;
		universe.SP4y1 = y + 1;
		universe.SP4x2 = universe.SP4x1 + w;
		universe.SP4y2 = universe.SP4y1 + h;
	} else {
		universe.SP4x1 = 0;
		universe.SP4y1 = 0;
		universe.SP4x2 = 0;
		universe.SP4y2 = 0;
	}

	L4drawRoom(universe, x, y, w, h);
	L4roomGen(universe, x, y, w, h, random_(universe, 0, 2));
}

void L4SaveQuads(Universe& universe)
{
	int i, j, x, y;

	x = universe.l4holdx;
	y = universe.l4holdy;

	for (j = 0; j < 14; j++) {
		for (i = 0; i < 14; i++) {
			universe.dflags[i + x][j + y] = 1;
			universe.dflags[DMAXX - 1 - i - x][j + y] = 1;
			universe.dflags[i + x][DMAXY - 1 - j - y] = 1;
			universe.dflags[DMAXX - 1 - i - x][DMAXY - 1 - j - y] = 1;
		}
	}
}

void DRLG_L4SetRoom(Universe& universe, const BYTE *pSetPiece, int rx1, int ry1)
{
	int rw, rh, i, j;
	const BYTE *sp;

	rw = pSetPiece[0];
	rh = pSetPiece[2];
	sp = &pSetPiece[4];

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*sp != 0) {
				SetDungeon(universe, i + rx1, j + ry1, *sp);
				universe.dflags[i + rx1][j + ry1] |= DLRG_PROTECTED;
			} else {
				SetDungeon(universe, i + rx1, j + ry1, 6);
			}
			sp += 2;
		}
	}
}

void DRLG_PreLoadDiabQuads()
{
	lpSetPiece1 = LoadFileInMem("Levels\\L4Data\\diab1.DUN", NULL);
	lpSetPiece2 = LoadFileInMem("Levels\\L4Data\\diab2b.DUN", NULL);
	lpSetPiece3 = LoadFileInMem("Levels\\L4Data\\diab3b.DUN", NULL);
	lpSetPiece4 = LoadFileInMem("Levels\\L4Data\\diab4b.DUN", NULL);
}

void DRLG_FreeDiabQuads()
{
	mem_free_dbg(lpSetPiece1);
	mem_free_dbg(lpSetPiece2);
	mem_free_dbg(lpSetPiece3);
	mem_free_dbg(lpSetPiece4);
}

void DRLG_LoadDiabQuads(Universe& universe, BOOL preflag)
{
	universe.diabquad1x = 4 + universe.l4holdx;
	universe.diabquad1y = 4 + universe.l4holdy;
	DRLG_L4SetRoom(universe, lpSetPiece1, universe.diabquad1x, universe.diabquad1y);

	universe.diabquad2x = 27 - universe.l4holdx;
	universe.diabquad2y = 1 + universe.l4holdy;
	DRLG_L4SetRoom(universe, lpSetPiece2, universe.diabquad2x, universe.diabquad2y);

	universe.diabquad3x = 1 + universe.l4holdx;
	universe.diabquad3y = 27 - universe.l4holdy;
	DRLG_L4SetRoom(universe, lpSetPiece3, universe.diabquad3x, universe.diabquad3y);

	universe.diabquad4x = 28 - universe.l4holdx;
	universe.diabquad4y = 28 - universe.l4holdy;
	DRLG_L4SetRoom(universe, lpSetPiece4, universe.diabquad4x, universe.diabquad4y);
}

static BOOL DRLG_L4PlaceMiniSet(Universe& universe, const BYTE *miniset, int tmin, int tmax, int cx, int cy, BOOL setview, int ldir)
{
	int sx, sy, sw, sh, xx, yy, i, ii, numt, bailcnt;
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
		for (bailcnt = 0; !found && bailcnt < 200; bailcnt++) {
			found = TRUE;
			if (sx >= universe.SP4x1 && sx <= universe.SP4x2 && sy >= universe.SP4y1 && sy <= universe.SP4y2) {
				found = FALSE;
			}
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
		if (bailcnt >= 200) {
			return FALSE;
		}
		ii = sw * sh + 2;
		for (yy = 0; yy < sh; yy++) {
			for (xx = 0; xx < sw; xx++) {
				if (miniset[ii] != 0) {
					SetDungeon(universe, xx + sx, yy + sy, miniset[ii]);
					universe.dflags[xx + sx][yy + sy] |= 8;
				}
				ii++;
			}
		}
	}

	if (universe.currlevel == 15) {
		universe.quests[Q_BETRAYER]._qtx = sx + 1;
		universe.quests[Q_BETRAYER]._qty = sy + 1;
	}
	if (setview == TRUE) {
		universe.ViewX = 2 * sx + 21;
		universe.ViewY = 2 * sy + 22;
	}
	if (ldir == 0) {
		universe.LvlViewX = 2 * sx + 21;
		universe.LvlViewY = 2 * sy + 22;
	}

	return TRUE;
}

static void DRLG_L4FTVR(Universe& universe, int i, int j, int x, int y, int d)
{
	if (universe.dTransVal[x][y] != 0 || GetDungeon(universe, i, j) != 6) {
		if (d == 1) {
			universe.dTransVal[x][y] = universe.TransVal;
			universe.dTransVal[x][y + 1] = universe.TransVal;
		}
		if (d == 2) {
			universe.dTransVal[x + 1][y] = universe.TransVal;
			universe.dTransVal[x + 1][y + 1] = universe.TransVal;
		}
		if (d == 3) {
			universe.dTransVal[x][y] = universe.TransVal;
			universe.dTransVal[x + 1][y] = universe.TransVal;
		}
		if (d == 4) {
			universe.dTransVal[x][y + 1] = universe.TransVal;
			universe.dTransVal[x + 1][y + 1] = universe.TransVal;
		}
		if (d == 5) {
			universe.dTransVal[x + 1][y + 1] = universe.TransVal;
		}
		if (d == 6) {
			universe.dTransVal[x][y + 1] = universe.TransVal;
		}
		if (d == 7) {
			universe.dTransVal[x + 1][y] = universe.TransVal;
		}
		if (d == 8) {
			universe.dTransVal[x][y] = universe.TransVal;
		}
	} else {
		universe.dTransVal[x][y] = universe.TransVal;
		universe.dTransVal[x + 1][y] = universe.TransVal;
		universe.dTransVal[x][y + 1] = universe.TransVal;
		universe.dTransVal[x + 1][y + 1] = universe.TransVal;
		DRLG_L4FTVR(universe, i + 1, j, x + 2, y, 1);
		DRLG_L4FTVR(universe, i - 1, j, x - 2, y, 2);
		DRLG_L4FTVR(universe, i, j + 1, x, y + 2, 3);
		DRLG_L4FTVR(universe, i, j - 1, x, y - 2, 4);
		DRLG_L4FTVR(universe, i - 1, j - 1, x - 2, y - 2, 5);
		DRLG_L4FTVR(universe, i + 1, j - 1, x + 2, y - 2, 6);
		DRLG_L4FTVR(universe, i - 1, j + 1, x - 2, y + 2, 7);
		DRLG_L4FTVR(universe, i + 1, j + 1, x + 2, y + 2, 8);
	}
}

static void DRLG_L4FloodTVal(Universe& universe)
{
	int i, j, xx, yy;

	yy = 16;
	for (j = 0; j < DMAXY; j++) {
		xx = 16;
		for (i = 0; i < DMAXX; i++) {
			if (GetDungeon(universe, i, j) == 6 && universe.dTransVal[xx][yy] == 0) {
				DRLG_L4FTVR(universe, i, j, xx, yy, 0);
				universe.TransVal++;
			}
			xx += 2;
		}
		yy += 2;
	}
}

BOOL IsDURWall(char d)
{
	if (d == 25) {
		return TRUE;
	}
	if (d == 28) {
		return TRUE;
	}
	if (d == 23) {
		return TRUE;
	}

	return FALSE;
}

BOOL IsDLLWall(char dd)
{
	if (dd == 27) {
		return TRUE;
	}
	if (dd == 26) {
		return TRUE;
	}
	if (dd == 22) {
		return TRUE;
	}

	return FALSE;
}

static void DRLG_L4TransFix(Universe& universe)
{
	int i, j, xx, yy;

	yy = 16;
	for (j = 0; j < DMAXY; j++) {
		xx = 16;
		for (i = 0; i < DMAXX; i++) {
			if (IsDURWall(GetDungeon(universe, i, j)) && GetDungeon(universe, i, j - 1) == 18) {
				universe.dTransVal[xx + 1][yy] = universe.dTransVal[xx][yy];
				universe.dTransVal[xx + 1][yy + 1] = universe.dTransVal[xx][yy];
			}
			if (IsDLLWall(GetDungeon(universe, i, j)) && GetDungeon(universe, i + 1, j) == 19) {
				universe.dTransVal[xx][yy + 1] = universe.dTransVal[xx][yy];
				universe.dTransVal[xx + 1][yy + 1] = universe.dTransVal[xx][yy];
			}
			if (GetDungeon(universe, i, j) == 18) {
				universe.dTransVal[xx + 1][yy] = universe.dTransVal[xx][yy];
				universe.dTransVal[xx + 1][yy + 1] = universe.dTransVal[xx][yy];
			}
			if (GetDungeon(universe, i, j) == 19) {
				universe.dTransVal[xx][yy + 1] = universe.dTransVal[xx][yy];
				universe.dTransVal[xx + 1][yy + 1] = universe.dTransVal[xx][yy];
			}
			if (GetDungeon(universe, i, j) == 24) {
				universe.dTransVal[xx + 1][yy] = universe.dTransVal[xx][yy];
				universe.dTransVal[xx][yy + 1] = universe.dTransVal[xx][yy];
				universe.dTransVal[xx + 1][yy + 1] = universe.dTransVal[xx][yy];
			}
			if (GetDungeon(universe, i, j) == 57) {
				universe.dTransVal[xx - 1][yy] = universe.dTransVal[xx][yy + 1];
				universe.dTransVal[xx][yy] = universe.dTransVal[xx][yy + 1];
			}
			if (GetDungeon(universe, i, j) == 53) {
				universe.dTransVal[xx][yy - 1] = universe.dTransVal[xx + 1][yy];
				universe.dTransVal[xx][yy] = universe.dTransVal[xx + 1][yy];
			}
			xx += 2;
		}
		yy += 2;
	}
}

static void DRLG_L4Corners(Universe& universe)
{
	int i, j;

	for (j = 1; j < DMAXY - 1; j++) {
		for (i = 1; i < DMAXX - 1; i++) {
			if (GetDungeon(universe, i, j) >= 18 && GetDungeon(universe, i, j) <= 30) {
				if (GetDungeon(universe, i + 1, j) < 18) {
					SetDungeon(universe, i, j, GetDungeon(universe, i, j) + 98);
				} else if (GetDungeon(universe, i, j + 1) < 18) {
					SetDungeon(universe, i, j, GetDungeon(universe, i, j) + 98);
				}
			}
		}
	}
}

void L4FixRim(Universe& universe)
{
	int i, j;

	for (i = 0; i < 20; i++) {
		universe.dung[i][0] = 0;
	}
	for (j = 0; j < 20; j++) {
		universe.dung[0][j] = 0;
	}
}

void DRLG_L4GeneralFix(Universe& universe)
{
	int i, j;

	for (j = 0; j < DMAXY - 1; j++) {
		for (i = 0; i < DMAXX - 1; i++) {
			if ((GetDungeon(universe, i, j) == 24 || GetDungeon(universe, i, j) == 122) && GetDungeon(universe, i + 1, j) == 2 && GetDungeon(universe, i, j + 1) == 5) {
				SetDungeon(universe, i, j, 17);
			}
		}
	}
}

std::optional<uint32_t> DRLG_L4(Universe& universe, int entry, DungeonMode mode)
{
	int i, j, spi, spj, ar;
	BOOL doneflag;

	std::optional<uint32_t> levelSeed = std::nullopt;

	do {
		DRLG_InitTrans(universe);
		do {
			levelSeed = GetRndState(universe);
			InitL4Dungeon(universe);
			L4firstRoom(universe);
			L4FixRim(universe);
			ar = GetArea(universe);
			if (ar >= 173) {
				uShape(universe);
			}
			if (mode == DungeonMode::BreakOnFailure && ar < 173)
				return std::nullopt;
		} while (ar < 173);
		L4makeDungeon(universe);
		L4makeDmt(universe);
		L4tileFix(universe);
		if (universe.currlevel == 16) {
			L4SaveQuads(universe);
		}
		if (QuestStatus(universe, Q_WARLORD) || universe.currlevel == universe.quests[Q_BETRAYER]._qlevel && universe.gbMaxPlayers != 1) {
			for (spi = universe.SP4x1; spi < universe.SP4x2; spi++) {
				for (spj = universe.SP4y1; spj < universe.SP4y2; spj++) {
					universe.dflags[spi][spj] = 1;
				}
			}
		}
		L4AddWall(universe);
		DRLG_L4FloodTVal(universe);
		DRLG_L4TransFix(universe);
		if (universe.setloadflag) {
			DRLG_L4SetSPRoom(universe, universe.SP4x1, universe.SP4y1);
		}
		if (universe.currlevel == 16) {
			DRLG_LoadDiabQuads(universe, TRUE);
		}
		if (QuestStatus(universe, Q_WARLORD)) {
			if (entry == ENTRY_MAIN) {
				doneflag = DRLG_L4PlaceMiniSet(universe, L4USTAIRS, 1, 1, -1, -1, TRUE, 0);
				if (doneflag && universe.currlevel == 13) {
					doneflag = DRLG_L4PlaceMiniSet(universe, L4TWARP, 1, 1, -1, -1, FALSE, 6);
				}
				universe.ViewX++;
			} else if (entry == ENTRY_PREV) {
				doneflag = DRLG_L4PlaceMiniSet(universe, L4USTAIRS, 1, 1, -1, -1, FALSE, 0);
				if (doneflag && universe.currlevel == 13) {
					doneflag = DRLG_L4PlaceMiniSet(universe, L4TWARP, 1, 1, -1, -1, FALSE, 6);
				}
				universe.ViewX = 2 * universe.setpc_x + 22;
				universe.ViewY = 2 * universe.setpc_y + 22;
			} else {
				doneflag = DRLG_L4PlaceMiniSet(universe, L4USTAIRS, 1, 1, -1, -1, FALSE, 0);
				if (doneflag && universe.currlevel == 13) {
					doneflag = DRLG_L4PlaceMiniSet(universe, L4TWARP, 1, 1, -1, -1, TRUE, 6);
				}
				universe.ViewX++;
			}
		} else if (universe.currlevel != 15) {
			if (entry == ENTRY_MAIN) {
				doneflag = DRLG_L4PlaceMiniSet(universe, L4USTAIRS, 1, 1, -1, -1, TRUE, 0);
				if (doneflag && universe.currlevel != 16) {
					doneflag = DRLG_L4PlaceMiniSet(universe, L4DSTAIRS, 1, 1, -1, -1, FALSE, 1);
				}
				if (doneflag && universe.currlevel == 13) {
					doneflag = DRLG_L4PlaceMiniSet(universe, L4TWARP, 1, 1, -1, -1, FALSE, 6);
				}
				universe.ViewX++;
			} else if (entry == ENTRY_PREV) {
				doneflag = DRLG_L4PlaceMiniSet(universe, L4USTAIRS, 1, 1, -1, -1, FALSE, 0);
				if (doneflag && universe.currlevel != 16) {
					doneflag = DRLG_L4PlaceMiniSet(universe, L4DSTAIRS, 1, 1, -1, -1, TRUE, 1);
				}
				if (doneflag && universe.currlevel == 13) {
					doneflag = DRLG_L4PlaceMiniSet(universe, L4TWARP, 1, 1, -1, -1, FALSE, 6);
				}
				universe.ViewY++;
			} else {
				doneflag = DRLG_L4PlaceMiniSet(universe, L4USTAIRS, 1, 1, -1, -1, FALSE, 0);
				if (doneflag && universe.currlevel != 16) {
					doneflag = DRLG_L4PlaceMiniSet(universe, L4DSTAIRS, 1, 1, -1, -1, FALSE, 1);
				}
				if (doneflag && universe.currlevel == 13) {
					doneflag = DRLG_L4PlaceMiniSet(universe, L4TWARP, 1, 1, -1, -1, TRUE, 6);
				}
				universe.ViewX++;
			}
		} else {
			if (entry == ENTRY_MAIN) {
				doneflag = DRLG_L4PlaceMiniSet(universe, L4USTAIRS, 1, 1, -1, -1, TRUE, 0);
				if (doneflag) {
					if (universe.gbMaxPlayers == 1 && universe.quests[Q_DIABLO]._qactive != QUEST_ACTIVE) {
						doneflag = DRLG_L4PlaceMiniSet(universe, L4PENTA, 1, 1, -1, -1, FALSE, 1);
					} else {
						doneflag = DRLG_L4PlaceMiniSet(universe, L4PENTA2, 1, 1, -1, -1, FALSE, 1);
					}
				}
				universe.ViewX++;
			} else {
				doneflag = DRLG_L4PlaceMiniSet(universe, L4USTAIRS, 1, 1, -1, -1, FALSE, 0);
				if (doneflag) {
					if (universe.gbMaxPlayers == 1 && universe.quests[Q_DIABLO]._qactive != QUEST_ACTIVE) {
						doneflag = DRLG_L4PlaceMiniSet(universe, L4PENTA, 1, 1, -1, -1, TRUE, 1);
					} else {
						doneflag = DRLG_L4PlaceMiniSet(universe, L4PENTA2, 1, 1, -1, -1, TRUE, 1);
					}
				}
				universe.ViewY++;
			}
		}
		if (mode == DungeonMode::BreakOnFailure && !doneflag)
			return std::nullopt;
	} while (!doneflag);

	if (mode == DungeonMode::BreakOnSuccess)
		return levelSeed;

	DRLG_L4GeneralFix(universe);

	if (universe.currlevel != 16) {
		DRLG_PlaceThemeRooms(universe, 7, 10, 6, 8, 1);
	}

	DRLG_L4Shadows(universe);
	DRLG_L4Corners(universe);
	DRLG_L4Subs(universe);
	DRLG_Init_Globals(universe);

	if (QuestStatus(universe, Q_WARLORD)) {
		for (j = 0; j < DMAXY; j++) {
			for (i = 0; i < DMAXX; i++) {
				universe.pdungeon[i][j] = GetDungeon(universe, i, j);
			}
		}
	}

	DRLG_CheckQuests(universe, universe.SP4x1, universe.SP4y1);

	if (universe.currlevel == 15) {
		for (j = 0; j < DMAXY; j++) {
			for (i = 0; i < DMAXX; i++) {
				if (GetDungeon(universe, i, j) == 98) {
					Make_SetPC(universe, i - 1, j - 1, 5, 5);
				}
				if (GetDungeon(universe, i, j) == 107) {
					Make_SetPC(universe, i - 1, j - 1, 5, 5);
				}
			}
		}
	}
	if (universe.currlevel == 16) {
		for (j = 0; j < DMAXY; j++) {
			for (i = 0; i < DMAXX; i++) {
				universe.pdungeon[i][j] = GetDungeon(universe, i, j);
			}
		}
		DRLG_LoadDiabQuads(universe, FALSE);
	}

	return levelSeed;
}

static void DRLG_L4Pass3(Universe& universe)
{
	int i, j, xx, yy;
	long v1, v2, v3, v4, lv;

	lv = 30 - 1;

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

std::optional<uint32_t> CreateL4Dungeon(Universe& universe, DWORD rseed, int entry, DungeonMode mode)
{
	SetRndSeed(universe, rseed);

	universe.dminx = 16;
	universe.dminy = 16;
	universe.dmaxx = 96;
	universe.dmaxy = 96;

	universe.ViewX = 40;
	universe.ViewY = 40;

	DRLG_InitSetPC(universe);
	DRLG_LoadL4SP(universe);
	std::optional<uint32_t> levelSeed = DRLG_L4(universe, entry, mode);
	if (mode == DungeonMode::BreakOnFailure || mode == DungeonMode::BreakOnSuccess) {
		DRLG_FreeL4SP();
		return levelSeed;
	}

	DRLG_L4Pass3(universe);
	DRLG_FreeL4SP();
	DRLG_SetPC(universe);

	return levelSeed;
}
#endif
