/**
 * @file trigs.cpp
 *
 * Implementation of functionality for triggering events when the player enters an area.
 */

#include "Source/trigs.h"

#include "defs.h"
#include "structs.h"
#include "types.h"
#include "Source/gendung.h"
#include "Source/quests.h"
#include "Source/universe/universe.h"

/** Specifies the dungeon piece IDs which constitute stairways leading down to the cathedral from town. */
const int TownDownList[] = { 716, 715, 719, 720, 721, 723, 724, 725, 726, 727, -1 };
/** Specifies the dungeon piece IDs which constitute stairways leading down to the catacombs from town. */
const int TownWarp1List[] = { 1171, 1172, 1173, 1174, 1175, 1176, 1177, 1178, 1179, 1181, 1183, 1185, -1 };
#ifdef HELLFIRE
const int TownCryptList[] = { 1331, 1332, 1333, 1334, 1335, 1336, 1337, 1338, -1 };
const int TownHiveList[] = { 1307, 1308, 1309, 1310, -1 };
#endif
/** Specifies the dungeon piece IDs which constitute stairways leading up from the cathedral. */
const int L1UpList[] = { 127, 129, 130, 131, 132, 133, 135, 137, 138, 139, 140, -1 };
/** Specifies the dungeon piece IDs which constitute stairways leading down from the cathedral. */
const int L1DownList[] = { 106, 107, 108, 109, 110, 112, 114, 115, 118, -1 };
/** Specifies the dungeon piece IDs which constitute stairways leading up from the catacombs. */
const int L2UpList[] = { 266, 267, -1 };
/** Specifies the dungeon piece IDs which constitute stairways leading down from the catacombs. */
const int L2DownList[] = { 269, 270, 271, 272, -1 };
/** Specifies the dungeon piece IDs which constitute stairways leading up to town from the catacombs. */
const int L2TWarpUpList[] = { 558, 559, -1 };
/** Specifies the dungeon piece IDs which constitute stairways leading up from the caves. */
const int L3UpList[] = { 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, -1 };
/** Specifies the dungeon piece IDs which constitute stairways leading down from the caves. */
const int L3DownList[] = { 162, 163, 164, 165, 166, 167, 168, 169, -1 };
/** Specifies the dungeon piece IDs which constitute stairways leading up to town from the caves. */
const int L3TWarpUpList[] = { 548, 549, 550, 551, 552, 553, 554, 555, 556, 557, 558, 559, 560, -1 };
/** Specifies the dungeon piece IDs which constitute stairways leading up from hell. */
const int L4UpList[] = { 82, 83, 90, -1 };
/** Specifies the dungeon piece IDs which constitute stairways leading down from hell. */
const int L4DownList[] = { 120, 130, 131, 132, 133, -1 };
/** Specifies the dungeon piece IDs which constitute stairways leading up to town from hell. */
const int L4TWarpUpList[] = { 421, 422, 429, -1 };
/** Specifies the dungeon piece IDs which constitute stairways leading down to Diablo from hell. */
const int L4PentaList[] = { 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, -1 };
#ifdef HELLFIRE
const int L5TWarpUpList[] = { 172, 173, 174, 175, 176, 177, 178, 179, 184, -1 };
const int L5UpList[] = { 149, 150, 151, 152, 153, 154, 155, 157, 158, 159, -1 };
const int L5DownList[] = { 125, 126, 129, 131, 132, 135, 136, 140, 142, -1 };
const int L6TWarpUpList[] = { 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, -1 };
const int L6UpList[] = { 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, -1 };
const int L6DownList[] = { 57, 58, 59, 60, 61, 62, 63, 64, -1 };
#endif

void InitNoTriggers(Universe& universe)
{
	universe.numtrigs = 0;
	universe.trigflag = FALSE;
}

void InitL1Triggers(Universe& universe)
{
	int i, j;

	universe.numtrigs = 0;
#ifdef HELLFIRE
	if (currlevel < 17) {
#endif
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++) {
				if (universe.dPiece[i][j] == 129) {
					universe.trigs[universe.numtrigs]._tx = i;
					universe.trigs[universe.numtrigs]._ty = j;
					universe.trigs[universe.numtrigs]._tmsg = WM_DIABPREVLVL;
					universe.numtrigs++;
				}
				if (universe.dPiece[i][j] == 115) {
					universe.trigs[universe.numtrigs]._tx = i;
					universe.trigs[universe.numtrigs]._ty = j;
					universe.trigs[universe.numtrigs]._tmsg = WM_DIABNEXTLVL;
					universe.numtrigs++;
				}
			}
		}
#ifdef HELLFIRE
	} else {
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++) {
				if (universe.dPiece[i][j] == 184) {
					universe.trigs[universe.numtrigs]._tx = i;
					universe.trigs[universe.numtrigs]._ty = j;
					universe.trigs[universe.numtrigs]._tmsg = WM_DIABTWARPUP;
					universe.trigs[universe.numtrigs]._tlvl = 0;
					universe.numtrigs++;
				}
				if (universe.dPiece[i][j] == 158) {
					universe.trigs[universe.numtrigs]._tx = i;
					universe.trigs[universe.numtrigs]._ty = j;
					universe.trigs[universe.numtrigs]._tmsg = WM_DIABPREVLVL;
					universe.numtrigs++;
				}
				if (universe.dPiece[i][j] == 126) {
					universe.trigs[universe.numtrigs]._tx = i;
					universe.trigs[universe.numtrigs]._ty = j;
					universe.trigs[universe.numtrigs]._tmsg = WM_DIABNEXTLVL;
					universe.numtrigs++;
				}
			}
		}
	}
#endif
	universe.trigflag = FALSE;
}

void InitL2Triggers(Universe& universe)
{
	int i, j;

	universe.numtrigs = 0;
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (universe.dPiece[i][j] == 267 && (i != universe.quests[Q_SCHAMB]._qtx || j != universe.quests[Q_SCHAMB]._qty)) {
				universe.trigs[universe.numtrigs]._tx = i;
				universe.trigs[universe.numtrigs]._ty = j;
				universe.trigs[universe.numtrigs]._tmsg = WM_DIABPREVLVL;
				universe.numtrigs++;
			}

			if (universe.dPiece[i][j] == 559) {
				universe.trigs[universe.numtrigs]._tx = i;
				universe.trigs[universe.numtrigs]._ty = j;
				universe.trigs[universe.numtrigs]._tmsg = WM_DIABTWARPUP;
				universe.trigs[universe.numtrigs]._tlvl = 0;
				universe.numtrigs++;
			}

			if (universe.dPiece[i][j] == 271) {
				universe.trigs[universe.numtrigs]._tx = i;
				universe.trigs[universe.numtrigs]._ty = j;
				universe.trigs[universe.numtrigs]._tmsg = WM_DIABNEXTLVL;
				universe.numtrigs++;
			}
		}
	}
	universe.trigflag = FALSE;
}

void InitL3Triggers(Universe& universe)
{
	int i, j;

#ifdef HELLFIRE
	if (currlevel < 17) {
#endif
		universe.numtrigs = 0;
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++) {
				if (universe.dPiece[i][j] == 171) {
					universe.trigs[universe.numtrigs]._tx = i;
					universe.trigs[universe.numtrigs]._ty = j;
					universe.trigs[universe.numtrigs]._tmsg = WM_DIABPREVLVL;
					universe.numtrigs++;
				}

				if (universe.dPiece[i][j] == 168) {
					universe.trigs[universe.numtrigs]._tx = i;
					universe.trigs[universe.numtrigs]._ty = j;
					universe.trigs[universe.numtrigs]._tmsg = WM_DIABNEXTLVL;
					universe.numtrigs++;
				}

				if (universe.dPiece[i][j] == 549) {
					universe.trigs[universe.numtrigs]._tx = i;
					universe.trigs[universe.numtrigs]._ty = j;
					universe.trigs[universe.numtrigs]._tmsg = WM_DIABTWARPUP;
					universe.numtrigs++;
				}
			}
		}
#ifdef HELLFIRE
	} else {
		universe.numtrigs = 0;
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++) {
				if (universe.dPiece[i][j] == 66) {
					universe.trigs[universe.numtrigs]._tx = i;
					universe.trigs[universe.numtrigs]._ty = j;
					universe.trigs[universe.numtrigs]._tmsg = WM_DIABPREVLVL;
					universe.numtrigs++;
				}

				if (universe.dPiece[i][j] == 63) {
					universe.trigs[universe.numtrigs]._tx = i;
					universe.trigs[universe.numtrigs]._ty = j;
					universe.trigs[universe.numtrigs]._tmsg = WM_DIABNEXTLVL;
					universe.numtrigs++;
				}

				if (universe.dPiece[i][j] == 80) {
					universe.trigs[universe.numtrigs]._tx = i;
					universe.trigs[universe.numtrigs]._ty = j;
					universe.trigs[universe.numtrigs]._tmsg = WM_DIABTWARPUP;
					universe.numtrigs++;
				}
			}
		}
	}
#endif
	universe.trigflag = FALSE;
}

void InitL4Triggers(Universe& universe)
{
	int i, j;

	universe.numtrigs = 0;
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (universe.dPiece[i][j] == 83) {
				universe.trigs[universe.numtrigs]._tx = i;
				universe.trigs[universe.numtrigs]._ty = j;
				universe.trigs[universe.numtrigs]._tmsg = WM_DIABPREVLVL;
				universe.numtrigs++;
			}

			if (universe.dPiece[i][j] == 422) {
				universe.trigs[universe.numtrigs]._tx = i;
				universe.trigs[universe.numtrigs]._ty = j;
				universe.trigs[universe.numtrigs]._tmsg = WM_DIABTWARPUP;
				universe.trigs[universe.numtrigs]._tlvl = 0;
				universe.numtrigs++;
			}

			if (universe.dPiece[i][j] == 120) {
				universe.trigs[universe.numtrigs]._tx = i;
				universe.trigs[universe.numtrigs]._ty = j;
				universe.trigs[universe.numtrigs]._tmsg = WM_DIABNEXTLVL;
				universe.numtrigs++;
			}
			if (universe.dPiece[i][j] == 336 || universe.dPiece[i][j] == 370) {
				universe.trigs[universe.numtrigs]._tx = i;
				universe.trigs[universe.numtrigs]._ty = j;
				universe.trigs[universe.numtrigs]._tmsg = WM_DIABNEXTLVL;
				universe.numtrigs++;
			}
		}
	}
	universe.trigflag = FALSE;
}

void InitSKingTriggers(Universe& universe)
{
	universe.trigflag = FALSE;
	universe.numtrigs = 1;
	universe.trigs[0]._tx = 82;
	universe.trigs[0]._ty = 42;
	universe.trigs[0]._tmsg = WM_DIABRTNLVL;
}

void InitSChambTriggers(Universe& universe)
{
	universe.trigflag = FALSE;
	universe.numtrigs = 1;
	universe.trigs[0]._tx = 70;
	universe.trigs[0]._ty = 39;
	universe.trigs[0]._tmsg = WM_DIABRTNLVL;
}

void InitPWaterTriggers(Universe& universe)
{
	universe.trigflag = FALSE;
	universe.numtrigs = 1;
	universe.trigs[0]._tx = 30;
	universe.trigs[0]._ty = 83;
	universe.trigs[0]._tmsg = WM_DIABRTNLVL;
}

void InitVPTriggers(Universe& universe)
{
	universe.trigflag = FALSE;
	universe.numtrigs = 1;
	universe.trigs[0]._tx = 35;
	universe.trigs[0]._ty = 32;
	universe.trigs[0]._tmsg = WM_DIABRTNLVL;
}

void Freeupstairs(Universe& universe)
{
	int i, tx, ty, xx, yy;

	for (i = 0; i < universe.numtrigs; i++) {
		tx = universe.trigs[i]._tx;
		ty = universe.trigs[i]._ty;

		for (yy = -2; yy <= 2; yy++) {
			for (xx = -2; xx <= 2; xx++) {
				universe.dFlags[tx + xx][ty + yy] |= BFLAG_POPULATED;
			}
		}
	}
}
