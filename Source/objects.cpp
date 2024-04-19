/**
 * @file objects.cpp
 *
 * Implementation of object functionality, interaction, spawning, loading, etc.
 */

#include "Source/objects.h"

#include "defs.h"
#include "structs.h"
#include "types.h"
#include "Source/drlg_l4.h"
#include "Source/engine.h"
#include "Source/lighting.h"
#include "Source/monster.h"
#include "Source/objdat.h"
#include "Source/quests.h"
#include "Source/setmaps.h"
#include "Source/themes.h"
#include "Source/universe/universe.h"

// TODO: Mark these as OpenCL __global
BYTE *pObjCels[40];

/** Specifies the X-coordinate delta between barrels. */
const int bxadd[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
/** Specifies the Y-coordinate delta between barrels. */
const int byadd[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
/** Maps from shrine_id to shrine name. */
const char *const shrinestrs[NUM_SHRINETYPE] = {
	"Mysterious",
	"Hidden",
	"Gloomy",
	"Weird",
	"Magical",
	"Stone",
	"Religious",
	"Enchanted",
	"Thaumaturgic",
	"Fascinating",
	"Cryptic",
	"Magical",
	"Eldritch",
	"Eerie",
	"Divine",
	"Holy",
	"Sacred",
	"Spiritual",
	"Spooky",
	"Abandoned",
	"Creepy",
	"Quiet",
	"Secluded",
	"Ornate",
	"Glimmering",
	"Tainted",
#ifdef HELLFIRE
	"Oily",
	"Glowing",
	"Mendicant's",
	"Sparkling",
	"Town",
	"Shimmering",
	"Solar",
	"Murphy's",
#endif
};
/** Specifies the minimum dungeon level on which each shrine will appear. */
const char shrinemin[NUM_SHRINETYPE] = {
	1, // Mysterious
	1, // Hidden
	1, // Gloomy
	1, // Weird
	1, // Magical
	1, // Stone
	1, // Religious
	1, // Enchanted
	1, // Thaumaturgic
	1, // Fascinating
	1, // Cryptic
	1, // Magical
	1, // Eldritch
	1, // Eerie
	1, // Divine
	1, // Holy
	1, // Sacred
	1, // Spiritual
	1, // Spooky
	1, // Abandoned
	1, // Creepy
	1, // Quiet
	1, // Secluded
	1, // Ornate
	1, // Glimmering
	1, // Tainted
#ifdef HELLFIRE
	1, // Oily
	1, // Glowing
	1, // Mendicant's
	1, // Sparkling
	1, // Town
	1, // Shimmering
	1, // Solar,
	1, // Murphy's
#endif
};
/** Specifies the maximum dungeon level on which each shrine will appear. */
const char shrinemax[NUM_SHRINETYPE] = {
	MAX_LVLS, // Mysterious
	MAX_LVLS, // Hidden
	MAX_LVLS, // Gloomy
	MAX_LVLS, // Weird
	MAX_LVLS, // Magical
	MAX_LVLS, // Stone
	MAX_LVLS, // Religious
	8,        // Enchanted
	MAX_LVLS, // Thaumaturgic
	MAX_LVLS, // Fascinating
	MAX_LVLS, // Cryptic
	MAX_LVLS, // Magical
	MAX_LVLS, // Eldritch
	MAX_LVLS, // Eerie
	MAX_LVLS, // Divine
	MAX_LVLS, // Holy
	MAX_LVLS, // Sacred
	MAX_LVLS, // Spiritual
	MAX_LVLS, // Spooky
	MAX_LVLS, // Abandoned
	MAX_LVLS, // Creepy
	MAX_LVLS, // Quiet
	MAX_LVLS, // Secluded
	MAX_LVLS, // Ornate
	MAX_LVLS, // Glimmering
	MAX_LVLS, // Tainted
#ifdef HELLFIRE
	MAX_LVLS, // Oily
	MAX_LVLS, // Glowing
	MAX_LVLS, // Mendicant's
	MAX_LVLS, // Sparkling
	MAX_LVLS, // Town
	MAX_LVLS, // Shimmering
	MAX_LVLS, // Solar,
	MAX_LVLS, // Murphy's
#endif
};
/**
 * Specifies the game type for which each shrine may appear.
 * SHRINETYPE_ANY - 0 - sp & mp
 * SHRINETYPE_SINGLE - 1 - sp only
 * SHRINETYPE_MULTI - 2 - mp only
 */

const BYTE shrineavail[NUM_SHRINETYPE] = {
	SHRINETYPE_ANY,    // SHRINE_MYSTERIOUS
	SHRINETYPE_ANY,    // SHRINE_HIDDEN
	SHRINETYPE_SINGLE, // SHRINE_GLOOMY
	SHRINETYPE_SINGLE, // SHRINE_WEIRD
	SHRINETYPE_ANY,    // SHRINE_MAGICAL
	SHRINETYPE_ANY,    // SHRINE_STONE
	SHRINETYPE_ANY,    // SHRINE_RELIGIOUS
	SHRINETYPE_ANY,    // SHRINE_ENCHANTED
	SHRINETYPE_SINGLE, // SHRINE_THAUMATURGIC
	SHRINETYPE_ANY,    // SHRINE_FASCINATING
	SHRINETYPE_ANY,    // SHRINE_CRYPTIC
	SHRINETYPE_ANY,    // SHRINE_MAGICAL2
	SHRINETYPE_ANY,    // SHRINE_ELDRITCH
	SHRINETYPE_ANY,    // SHRINE_EERIE
	SHRINETYPE_ANY,    // SHRINE_DIVINE
	SHRINETYPE_ANY,    // SHRINE_HOLY
	SHRINETYPE_ANY,    // SHRINE_SACRED
	SHRINETYPE_ANY,    // SHRINE_SPIRITUAL
	SHRINETYPE_MULTI,  // SHRINE_SPOOKY
	SHRINETYPE_ANY,    // SHRINE_ABANDONED
	SHRINETYPE_ANY,    // SHRINE_CREEPY
	SHRINETYPE_ANY,    // SHRINE_QUIET
	SHRINETYPE_ANY,    // SHRINE_SECLUDED
	SHRINETYPE_ANY,    // SHRINE_ORNATE
	SHRINETYPE_ANY,    // SHRINE_GLIMMERING
	SHRINETYPE_MULTI,  // SHRINE_TAINTED
#ifdef HELLFIRE
	SHRINETYPE_ANY,    // SHRINE_OILY
	SHRINETYPE_ANY,    // SHRINE_GLOWING
	SHRINETYPE_ANY,    // SHRINE_MENDICANT
	SHRINETYPE_ANY,    // SHRINE_SPARKLING
	SHRINETYPE_ANY,    // SHRINE_TOWN
	SHRINETYPE_ANY,    // SHRINE_SHIMMERING
	SHRINETYPE_SINGLE, // SHRINE_SOLAR
	SHRINETYPE_ANY,    // SHRINE_MURPHYS
#endif
};
/** Maps from book_id to book name. */
const char *const StoryBookName[] = {
	"The Great Conflict",
	"The Wages of Sin are War",
	"The Tale of the Horadrim",
	"The Dark Exile",
	"The Sin War",
	"The Binding of the Three",
	"The Realms Beyond",
	"Tale of the Three",
	"The Black King",
#ifdef HELLFIRE
	"Journal: The Ensorcellment",
	"Journal: The Meeting",
	"Journal: The Tirade",
	"Journal: His Power Grows",
	"Journal: NA-KRUL",
	"Journal: The End",
	"A Spellbook",
#endif
};
/** Specifies the speech IDs of each dungeon type narrator book, for each player class. */
const int StoryText[3][3] = {
	{ TEXT_BOOK11, TEXT_BOOK12, TEXT_BOOK13 },
	{ TEXT_BOOK21, TEXT_BOOK22, TEXT_BOOK23 },
	{ TEXT_BOOK31, TEXT_BOOK32, TEXT_BOOK33 }
};

DIABOOL RndLocOk(Universe& universe, int xp, int yp)
{
	if (universe.dMonster[xp][yp] != 0)
		return FALSE;
	if (universe.dPlayer[xp][yp] != 0)
		return FALSE;
	if (universe.dObject[xp][yp] != 0)
		return FALSE;
	if (universe.dFlags[xp][yp] & BFLAG_POPULATED)
		return FALSE;
	if (universe.nSolidTable[universe.dPiece[xp][yp]])
		return FALSE;
	if (universe.leveltype != DTYPE_CATHEDRAL || universe.dPiece[xp][yp] <= 126 || universe.dPiece[xp][yp] >= 144)
		return TRUE;
	return FALSE;
}

static DIABOOL WallTrapLocOkK(Universe& universe, int xp, int yp)
{
	if (universe.dFlags[xp][yp] & BFLAG_POPULATED)
		return FALSE;

	if (universe.nTrapTable[universe.dPiece[xp][yp]] != FALSE)
		return TRUE;
	else
		return FALSE;
}

void InitRndLocObj(Universe& universe, int min, int max, int objtype)
{
	int i, xp, yp, numobjs;

	numobjs = random_(universe, 139, max - min) + min;

	for (i = 0; i < numobjs; i++) {
		while (1) {
			xp = random_(universe, 139, 80) + 16;
			yp = random_(universe, 139, 80) + 16;
			if (RndLocOk(universe, xp - 1, yp - 1)
			    && RndLocOk(universe, xp, yp - 1)
			    && RndLocOk(universe, xp + 1, yp - 1)
			    && RndLocOk(universe, xp - 1, yp)
			    && RndLocOk(universe, xp, yp)
			    && RndLocOk(universe, xp + 1, yp)
			    && RndLocOk(universe, xp - 1, yp + 1)
			    && RndLocOk(universe, xp, yp + 1)
			    && RndLocOk(universe, xp + 1, yp + 1)) {
				AddObject(universe, objtype, xp, yp);
				break;
			}
		}
	}
}

void InitRndLocBigObj(Universe& universe, int min, int max, int objtype)
{
	int i, xp, yp, numobjs;

	numobjs = random_(universe, 140, max - min) + min;
	for (i = 0; i < numobjs; i++) {
		while (1) {
			xp = random_(universe, 140, 80) + 16;
			yp = random_(universe, 140, 80) + 16;
			if (RndLocOk(universe, xp - 1, yp - 2)
			    && RndLocOk(universe, xp, yp - 2)
			    && RndLocOk(universe, xp + 1, yp - 2)
			    && RndLocOk(universe, xp - 1, yp - 1)
			    && RndLocOk(universe, xp, yp - 1)
			    && RndLocOk(universe, xp + 1, yp - 1)
			    && RndLocOk(universe, xp - 1, yp)
			    && RndLocOk(universe, xp, yp)
			    && RndLocOk(universe, xp + 1, yp)
			    && RndLocOk(universe, xp - 1, yp + 1)
			    && RndLocOk(universe, xp, yp + 1)
			    && RndLocOk(universe, xp + 1, yp + 1)) {
				AddObject(universe, objtype, xp, yp);
				break;
			}
		}
	}
}

void InitRndLocObj5x5(Universe& universe, int min, int max, int objtype)
{
	DIABOOL exit;
	int xp, yp, numobjs, i, cnt, m, n;

	numobjs = min + random_(universe, 139, max - min);
	for (i = 0; i < numobjs; i++) {
		cnt = 0;
		exit = FALSE;
		while (!exit) {
			exit = TRUE;
			xp = random_(universe, 139, 80) + 16;
			yp = random_(universe, 139, 80) + 16;
			for (n = -2; n <= 2; n++) {
				for (m = -2; m <= 2; m++) {
					if (!RndLocOk(universe, xp + m, yp + n))
						exit = FALSE;
				}
			}
			if (!exit) {
				cnt++;
				if (cnt > 20000)
					return;
			}
		}
		AddObject(universe, objtype, xp, yp);
	}
}

void ClrAllObjects(Universe& universe)
{
	int i;

#ifdef HELLFIRE
	memset(universe.object, 0, sizeof(universe.object));
#else
	for (i = 0; i < MAXOBJECTS; i++) {
		universe.object[i]._ox = 0;
		universe.object[i]._oy = 0;
		universe.object[i]._oAnimData = 0;
		universe.object[i]._oAnimDelay = 0;
		universe.object[i]._oAnimCnt = 0;
		universe.object[i]._oAnimLen = 0;
		universe.object[i]._oAnimFrame = 0;
		universe.object[i]._oDelFlag = FALSE;
		universe.object[i]._oVar1 = 0;
		universe.object[i]._oVar2 = 0;
		universe.object[i]._oVar3 = 0;
		universe.object[i]._oVar4 = 0;
	}
#endif
	universe.nobjects = 0;
	for (i = 0; i < MAXOBJECTS; i++) {
		universe.objectavail[i] = i;
#ifndef HELLFIRE
		universe.objectactive[i] = 0;
#endif
	}
#ifdef HELLFIRE
	memset(universe.objectactive, 0, sizeof(universe.objectactive));
#endif
	universe.trapdir = 0;
	universe.trapid = 1;
	universe.leverid = 1;
}

void AddTortures(Universe& universe)
{
	int ox, oy;

	for (oy = 0; oy < MAXDUNY; oy++) {
		for (ox = 0; ox < MAXDUNX; ox++) {
			if (universe.dPiece[ox][oy] == 367) {
				AddObject(universe, OBJ_TORTURE1, ox, oy + 1);
				AddObject(universe, OBJ_TORTURE3, ox + 2, oy - 1);
				AddObject(universe, OBJ_TORTURE2, ox, oy + 3);
				AddObject(universe, OBJ_TORTURE4, ox + 4, oy - 1);
				AddObject(universe, OBJ_TORTURE5, ox, oy + 5);
				AddObject(universe, OBJ_TNUDEM1, ox + 1, oy + 3);
				AddObject(universe, OBJ_TNUDEM2, ox + 4, oy + 5);
				AddObject(universe, OBJ_TNUDEM3, ox + 2, oy);
				AddObject(universe, OBJ_TNUDEM4, ox + 3, oy + 2);
				AddObject(universe, OBJ_TNUDEW1, ox + 2, oy + 4);
				AddObject(universe, OBJ_TNUDEW2, ox + 2, oy + 1);
				AddObject(universe, OBJ_TNUDEW3, ox + 4, oy + 2);
			}
		}
	}
}
void AddCandles(Universe& universe)
{
	int tx, ty;

	tx = universe.quests[Q_PWATER]._qtx;
	ty = universe.quests[Q_PWATER]._qty;
	AddObject(universe, OBJ_STORYCANDLE, tx - 2, ty + 1);
	AddObject(universe, OBJ_STORYCANDLE, tx + 3, ty + 1);
	AddObject(universe, OBJ_STORYCANDLE, tx - 1, ty + 2);
	AddObject(universe, OBJ_STORYCANDLE, tx + 2, ty + 2);
}

void AddBookLever(Universe& universe, int lx1, int ly1, int lx2, int ly2, int x1, int y1, int x2, int y2, int msg)
{
	DIABOOL exit;
	int xp, yp, ob, cnt, m, n;

	cnt = 0;
	exit = FALSE;
	while (!exit) {
		exit = TRUE;
		xp = random_(universe, 139, 80) + 16;
		yp = random_(universe, 139, 80) + 16;
		for (n = -2; n <= 2; n++) {
			for (m = -2; m <= 2; m++) {
				if (!RndLocOk(universe, xp + m, yp + n))
					exit = FALSE;
			}
		}
		if (!exit) {
			cnt++;
			if (cnt > 20000)
				return;
		}
	}

	if (QuestStatus(universe, Q_BLIND))
		AddObject(universe, OBJ_BLINDBOOK, xp, yp);
	if (QuestStatus(universe, Q_WARLORD))
		AddObject(universe, OBJ_STEELTOME, xp, yp);
	if (QuestStatus(universe, Q_BLOOD)) {
		xp = 2 * universe.setpc_x + 25;
		yp = 2 * universe.setpc_y + 40;
		AddObject(universe, OBJ_BLOODBOOK, xp, yp);
	}
	ob = universe.dObject[xp][yp] - 1;
	SetObjMapRange(universe, ob, x1, y1, x2, y2, universe.leverid);
	SetBookMsg(universe, ob, msg);
	universe.leverid++;
	universe.object[ob]._oVar6 = universe.object[ob]._oAnimFrame + 1;
}

void InitRndBarrels(Universe& universe)
{
	int numobjs; // number of groups of barrels to generate
	int xp, yp;
	_object_id o;
	DIABOOL found;
	int p; // regulates chance to stop placing barrels in current group
	int dir;
	int t; // number of tries of placing next barrel in current group
	int c; // number of barrels in current group
	int i;

	numobjs = random_(universe, 143, 5) + 3;
	for (i = 0; i < numobjs; i++) {
		do {
			xp = random_(universe, 143, 80) + 16;
			yp = random_(universe, 143, 80) + 16;
		} while (!RndLocOk(universe, xp, yp));
		o = (random_(universe, 143, 4) != 0) ? OBJ_BARREL : OBJ_BARRELEX;
		AddObject(universe, o, xp, yp);
		found = TRUE;
		p = 0;
		c = 1;
		while (random_(universe, 143, p) == 0 && found) {
			t = 0;
			found = FALSE;
			while (TRUE) {
				if (t >= 3)
					break;
				dir = random_(universe, 143, 8);
				xp += bxadd[dir];
				yp += byadd[dir];
				found = RndLocOk(universe, xp, yp);
				t++;
				if (found)
					break;
			}
			if (found) {
				o = (random_(universe, 143, 5) != 0) ? OBJ_BARREL : OBJ_BARRELEX;
				AddObject(universe, o, xp, yp);
				c++;
			}
			p = c >> 1;
		}
	}
}

void AddL1Objs(Universe& universe, int x1, int y1, int x2, int y2)
{
	int i, j, pn;

	for (j = y1; j < y2; j++) {
		for (i = x1; i < x2; i++) {
			pn = universe.dPiece[i][j];
			if (pn == 270)
				AddObject(universe, OBJ_L1LIGHT, i, j);
			if (pn == 44 || pn == 51 || pn == 214)
				AddObject(universe, OBJ_L1LDOOR, i, j);
			if (pn == 46 || pn == 56)
				AddObject(universe, OBJ_L1RDOOR, i, j);
		}
	}
}

#ifdef HELLFIRE
void add_crypt_objs(int x1, int y1, int x2, int y2)
{
	int i, j, pn;

	for (j = y1; j < y2; j++) {
		for (i = x1; i < x2; i++) {
			pn = universe.dPiece[i][j];
			if (pn == 77)
				AddObject(universe, OBJ_L1LDOOR, i, j);
			if (pn == 80)
				AddObject(universe, OBJ_L1RDOOR, i, j);
		}
	}
}

#endif
void AddL2Objs(Universe& universe, int x1, int y1, int x2, int y2)
{
	int i, j, pn;

	for (j = y1; j < y2; j++) {
		for (i = x1; i < x2; i++) {
			pn = universe.dPiece[i][j];
			if (pn == 13 || pn == 541)
				AddObject(universe, OBJ_L2LDOOR, i, j);
			if (pn == 17 || pn == 542)
				AddObject(universe, OBJ_L2RDOOR, i, j);
		}
	}
}

void AddL3Objs(Universe& universe, int x1, int y1, int x2, int y2)
{
	int i, j, pn;

	for (j = y1; j < y2; j++) {
		for (i = x1; i < x2; i++) {
			pn = universe.dPiece[i][j];
			if (pn == 531)
				AddObject(universe, OBJ_L3LDOOR, i, j);
			if (pn == 534)
				AddObject(universe, OBJ_L3RDOOR, i, j);
		}
	}
}

DIABOOL TorchLocOK(Universe& universe, int xp, int yp)
{
	if (universe.dFlags[xp][yp] & BFLAG_POPULATED)
		return FALSE;
	return TRUE;
}

void AddL2Torches(Universe& universe)
{
	int i, j, pn;

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (!TorchLocOK(universe, i, j))
				continue;

			pn = universe.dPiece[i][j];
			if (pn == 1 && random_(universe, 145, 3) == 0)
				AddObject(universe, OBJ_TORCHL2, i, j);

			if (pn == 5 && random_(universe, 145, 3) == 0)
				AddObject(universe, OBJ_TORCHR2, i, j);

			if (pn == 37 && random_(universe, 145, 10) == 0 && universe.dObject[i - 1][j] == 0)
				AddObject(universe, OBJ_TORCHL, i - 1, j);

			if (pn == 41 && random_(universe, 145, 10) == 0 && universe.dObject[i][j - 1] == 0)
				AddObject(universe, OBJ_TORCHR, i, j - 1);
		}
	}
}

void AddObjTraps(Universe& universe)
{
	char oi_trap, oi;
	int i, j;
	int xp, yp;
	int rndv;

	if (universe.currlevel == 1)
		rndv = 10;
	if (universe.currlevel >= 2)
		rndv = 15;
	if (universe.currlevel >= 5)
		rndv = 20;
	if (universe.currlevel >= 7)
		rndv = 25;
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (universe.dObject[i][j] <= 0 || random_(universe, 144, 100) >= rndv)
				continue;

			oi = universe.dObject[i][j] - 1;
			if (!AllObjects[universe.object[oi]._otype].oTrapFlag)
				continue;

			if (random_(universe, 144, 2) == 0) {
				xp = i - 1;
				while (!universe.nSolidTable[GetdPiece(universe, xp, j)])
					xp--;

				if (!WallTrapLocOkK(universe, xp, j) || i - xp <= 1)
					continue;

				AddObject(universe, OBJ_TRAPL, xp, j);
				oi_trap = universe.dObject[xp][j] - 1;
				universe.object[oi_trap]._oVar1 = i;
				universe.object[oi_trap]._oVar2 = j;
				universe.object[oi]._oTrapFlag = TRUE;
			} else {
				yp = j - 1;
				while (!universe.nSolidTable[universe.dPiece[i][yp]])
					yp--;

				if (!WallTrapLocOkK(universe, i, yp) || j - yp <= 1)
					continue;

				AddObject(universe, OBJ_TRAPR, i, yp);
				oi_trap = universe.dObject[i][yp] - 1;
				universe.object[oi_trap]._oVar1 = i;
				universe.object[oi_trap]._oVar2 = j;
				universe.object[oi]._oTrapFlag = TRUE;
			}
		}
	}
}

void AddChestTraps(Universe& universe)
{
	int i, j;
	char oi;

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (universe.dObject[i][j] > 0) {
				oi = universe.dObject[i][j] - 1;
				if (universe.object[oi]._otype >= OBJ_CHEST1 && universe.object[oi]._otype <= OBJ_CHEST3 && !universe.object[oi]._oTrapFlag && random_(universe, 0, 100) < 10) {
					universe.object[oi]._otype += OBJ_TCHEST1 - OBJ_CHEST1;
					universe.object[oi]._oTrapFlag = TRUE;
					if (universe.leveltype == DTYPE_CATACOMBS) {
						universe.object[oi]._oVar4 = random_(universe, 0, 2);
					} else {
#ifdef HELLFIRE
						universe.object[oi]._oVar4 = random_(universe, 0, 6);
#else
						universe.object[oi]._oVar4 = random_(universe, 0, 3);
#endif
					}
				}
			}
		}
	}
}

void LoadMapObjects(Universe& universe, BYTE *pMap, int startx, int starty, int x1, int y1, int w, int h, int leveridx)
{
	int rw, rh, i, j, oi, type;
	BYTE *lm;
	long mapoff;

	universe.InitObjFlag = TRUE;

	lm = pMap;
	rw = *lm;
	lm += 2;
	rh = *lm;
	mapoff = (rw * rh + 1) * 2;
	rw <<= 1;
	rh <<= 1;
	mapoff += rw * 2 * rh * 2;
	lm += mapoff;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*lm) {
				type = *lm;
				AddObject(universe, ObjTypeConv[type], startx + 16 + i, starty + 16 + j);
				oi = ObjIndex(universe, startx + 16 + i, starty + 16 + j);
				SetObjMapRange(universe, oi, x1, y1, x1 + w, y1 + h, leveridx);
			}
			lm += 2;
		}
	}
	universe.InitObjFlag = FALSE;
}

void LoadMapObjs(Universe& universe, BYTE *pMap, int startx, int starty)
{
	int rw, rh;
	int i, j;
	BYTE *lm;
	long mapoff;

	universe.InitObjFlag = TRUE;
	lm = pMap;
	rw = *lm;
	lm += 2;
	rh = *lm;
	mapoff = (rw * rh + 1) * 2;
	rw <<= 1;
	rh <<= 1;
	mapoff += 2 * rw * rh * 2;
	lm += mapoff;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*lm) {
				AddObject(universe, ObjTypeConv[*lm], startx + 16 + i, starty + 16 + j);
			}
			lm += 2;
		}
	}
	universe.InitObjFlag = FALSE;
}

void AddDiabObjs(Universe& universe)
{
	BYTE *lpSetPiece;

	lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab1.DUN", NULL);
	LoadMapObjects(universe, lpSetPiece, 2 * universe.diabquad1x, 2 * universe.diabquad1y, universe.diabquad2x, universe.diabquad2y, 11, 12, 1);
	mem_free_dbg(lpSetPiece);
	lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab2a.DUN", NULL);
	LoadMapObjects(universe, lpSetPiece, 2 * universe.diabquad2x, 2 * universe.diabquad2y, universe.diabquad3x, universe.diabquad3y, 11, 11, 2);
	mem_free_dbg(lpSetPiece);
	lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab3a.DUN", NULL);
	LoadMapObjects(universe, lpSetPiece, 2 * universe.diabquad3x, 2 * universe.diabquad3y, universe.diabquad4x, universe.diabquad4y, 9, 9, 3);
	mem_free_dbg(lpSetPiece);
}

#ifdef HELLFIRE
void AddCryptStoryBook(Universe& universe, int s)
{
	DIABOOL exit;
	int xp, yp, cnt, m, n;

	cnt = 0;
	exit = FALSE;
	while (!exit) {
		exit = TRUE;
		xp = random_(universe, 139, 80) + 16;
		yp = random_(universe, 139, 80) + 16;
		for (n = -2; n <= 2; n++) {
			for (m = -3; m <= 3; m++) {
				if (!RndLocOk(universe, xp + m, yp + n))
					exit = FALSE;
			}
		}
		if (!exit) {
			cnt++;
			if (cnt > 20000)
				return;
		}
	}
	AddCryptBook(universe, OBJ_STORYBOOK, s, xp, yp);
	AddObject(universe, OBJ_STORYCANDLE, xp - 2, yp + 1);
	AddObject(universe, OBJ_STORYCANDLE, xp - 2, yp);
	AddObject(universe, OBJ_STORYCANDLE, xp - 1, yp - 1);
	AddObject(universe, OBJ_STORYCANDLE, xp + 1, yp - 1);
	AddObject(universe, OBJ_STORYCANDLE, xp + 2, yp);
	AddObject(universe, OBJ_STORYCANDLE, xp + 2, yp + 1);
}

void AddNakrulGate(Universe& universe)
{
	AddNakrulLeaver();
	switch (random_(universe, 0, 6)) {
	case 0:
		AddNakrulBook(universe, 6, universe.UberRow + 3, universe.UberCol);
		AddNakrulBook(universe, 7, universe.UberRow + 2, universe.UberCol - 3);
		AddNakrulBook(universe, 8, universe.UberRow + 2, universe.UberCol + 2);
		break;
	case 1:
		AddNakrulBook(universe, 6, universe.UberRow + 3, universe.UberCol);
		AddNakrulBook(universe, 8, universe.UberRow + 2, universe.UberCol - 3);
		AddNakrulBook(universe, 7, universe.UberRow + 2, universe.UberCol + 2);
		break;
	case 2:
		AddNakrulBook(universe, 7, universe.UberRow + 3, universe.UberCol);
		AddNakrulBook(universe, 6, universe.UberRow + 2, universe.UberCol - 3);
		AddNakrulBook(universe, 8, universe.UberRow + 2, universe.UberCol + 2);
		break;
	case 3:
		AddNakrulBook(universe, 7, universe.UberRow + 3, universe.UberCol);
		AddNakrulBook(universe, 8, universe.UberRow + 2, universe.UberCol - 3);
		AddNakrulBook(universe, 6, universe.UberRow + 2, universe.UberCol + 2);
		break;
	case 4:
		AddNakrulBook(universe, 8, universe.UberRow + 3, universe.UberCol);
		AddNakrulBook(universe, 7, universe.UberRow + 2, universe.UberCol - 3);
		AddNakrulBook(universe, 6, universe.UberRow + 2, universe.UberCol + 2);
		break;
	case 5:
		AddNakrulBook(universe, 8, universe.UberRow + 3, universe.UberCol);
		AddNakrulBook(universe, 6, universe.UberRow + 2, universe.UberCol - 3);
		AddNakrulBook(universe, 7, universe.UberRow + 2, universe.UberCol + 2);
		break;
	}
}

void AddNakrulBook(Universe& universe, int a1, int a2, int a3)
{
	AddCryptBook(universe, OBJ_STORYBOOK, a1, a2, a3);
}
#endif

void AddStoryBooks(Universe& universe)
{
	int xp, yp, xx, yy;
	int cnt;
	DIABOOL done;

	cnt = 0;
	done = FALSE;
	while (!done) {
		done = TRUE;
		xp = random_(universe, 139, 80) + 16;
		yp = random_(universe, 139, 80) + 16;
		for (yy = -2; yy <= 2; yy++) {
			for (xx = -3; xx <= 3; xx++) {
				if (!RndLocOk(universe, xx + xp, yy + yp))
					done = FALSE;
			}
		}
		if (!done) {
			cnt++;
			if (cnt > 20000)
				return;
		}
	}
	AddObject(universe, OBJ_STORYBOOK, xp, yp);
	AddObject(universe, OBJ_STORYCANDLE, xp - 2, yp + 1);
	AddObject(universe, OBJ_STORYCANDLE, xp - 2, yp);
	AddObject(universe, OBJ_STORYCANDLE, xp - 1, yp - 1);
	AddObject(universe, OBJ_STORYCANDLE, xp + 1, yp - 1);
	AddObject(universe, OBJ_STORYCANDLE, xp + 2, yp);
	AddObject(universe, OBJ_STORYCANDLE, xp + 2, yp + 1);
}

void AddHookedBodies(Universe& universe, int freq)
{
	int i, j, ii, jj;

	for (j = 0; j < DMAXY; j++) {
		jj = 16 + j * 2;
		for (i = 0; i < DMAXX; i++) {
			ii = 16 + i * 2;
			if (GetDungeon(universe, i, j) != 1 && GetDungeon(universe, i, j) != 2)
				continue;
			if (random_(universe, 0, freq) != 0)
				continue;
			if (!SkipThemeRoom(universe, i, j))
				continue;
			if (GetDungeon(universe, i, j) == 1 && GetDungeon(universe, i + 1, j) == 6) {
				switch (random_(universe, 0, 3)) {
				case 0:
					AddObject(universe, OBJ_TORTURE1, ii + 1, jj);
					break;
				case 1:
					AddObject(universe, OBJ_TORTURE2, ii + 1, jj);
					break;
				case 2:
					AddObject(universe, OBJ_TORTURE5, ii + 1, jj);
					break;
				}
				continue;
			}
			if (GetDungeon(universe, i, j) == 2 && GetDungeon(universe, i, j + 1) == 6) {
				switch (random_(universe, 0, 2)) {
				case 0:
					AddObject(universe, OBJ_TORTURE3, ii, jj);
					break;
				case 1:
					AddObject(universe, OBJ_TORTURE4, ii, jj);
					break;
				}
			}
		}
	}
}

void AddL4Goodies(Universe& universe)
{
	AddHookedBodies(universe, 6);
	InitRndLocObj(universe, 2, 6, OBJ_TNUDEM1);
	InitRndLocObj(universe, 2, 6, OBJ_TNUDEM2);
	InitRndLocObj(universe, 2, 6, OBJ_TNUDEM3);
	InitRndLocObj(universe, 2, 6, OBJ_TNUDEM4);
	InitRndLocObj(universe, 2, 6, OBJ_TNUDEW1);
	InitRndLocObj(universe, 2, 6, OBJ_TNUDEW2);
	InitRndLocObj(universe, 2, 6, OBJ_TNUDEW3);
	InitRndLocObj(universe, 2, 6, OBJ_DECAP);
	InitRndLocObj(universe, 1, 3, OBJ_CAULDRON);
}

void AddLazStand(Universe& universe)
{
	int xp, yp, xx, yy;
	int cnt;
	DIABOOL found;

	cnt = 0;
	found = FALSE;
	while (!found) {
		found = TRUE;
		xp = random_(universe, 139, 80) + 16;
		yp = random_(universe, 139, 80) + 16;
		for (yy = -3; yy <= 3; yy++) {
			for (xx = -2; xx <= 3; xx++) {
				if (!RndLocOk(universe, xp + xx, yp + yy))
					found = FALSE;
			}
		}
		if (!found) {
			cnt++;
			if (cnt > 10000) {
				InitRndLocObj(universe, 1, 1, OBJ_LAZSTAND);
				return;
			}
		}
	}
	AddObject(universe, OBJ_LAZSTAND, xp, yp);
	AddObject(universe, OBJ_TNUDEM2, xp, yp + 2);
	AddObject(universe, OBJ_STORYCANDLE, xp + 1, yp + 2);
	AddObject(universe, OBJ_TNUDEM3, xp + 2, yp + 2);
	AddObject(universe, OBJ_TNUDEW1, xp, yp - 2);
	AddObject(universe, OBJ_STORYCANDLE, xp + 1, yp - 2);
	AddObject(universe, OBJ_TNUDEW2, xp + 2, yp - 2);
	AddObject(universe, OBJ_STORYCANDLE, xp - 1, yp - 1);
	AddObject(universe, OBJ_TNUDEW3, xp - 1, yp);
	AddObject(universe, OBJ_STORYCANDLE, xp - 1, yp + 1);
}

void InitObjects(Universe& universe)
{
	int sp_id;
	BYTE *mem;

	ClrAllObjects(universe);
#ifdef HELLFIRE
	universe.dword_6DE0E0 = 0;
#endif
	if (universe.currlevel == 16) {
		AddDiabObjs(universe);
	} else {
		universe.InitObjFlag = TRUE;
		GetRndSeed(universe);
		if (universe.currlevel == 9 && universe.gbMaxPlayers == 1)
			AddSlainHero(universe);
		if (universe.currlevel == universe.quests[Q_MUSHROOM]._qlevel && universe.quests[Q_MUSHROOM]._qactive == QUEST_INIT)
			AddMushPatch(universe);

#ifdef HELLFIRE
		if (universe.currlevel == 4 || universe.currlevel == 8 || universe.currlevel == 12)
			AddStoryBooks();
		if (universe.currlevel == 21) {
			AddCryptStoryBook(universe, 1);
		} else if (universe.currlevel == 22) {
			AddCryptStoryBook(universe, 2);
			AddCryptStoryBook(universe, 3);
		} else if (universe.currlevel == 23) {
			AddCryptStoryBook(universe, 4);
			AddCryptStoryBook(universe, 5);
		}
		if (universe.currlevel == 24) {
			AddNakrulGate(universe);
		}
#else
		if (universe.currlevel == 4)
			AddStoryBooks(universe);
		if (universe.currlevel == 8)
			AddStoryBooks(universe);
		if (universe.currlevel == 12)
			AddStoryBooks(universe);
#endif
		if (universe.leveltype == DTYPE_CATHEDRAL) {
			if (QuestStatus(universe, Q_BUTCHER))
				AddTortures(universe);
			if (QuestStatus(universe, Q_PWATER))
				AddCandles(universe);
			if (QuestStatus(universe, Q_LTBANNER))
				AddObject(universe, OBJ_SIGNCHEST, 2 * universe.setpc_x + 26, 2 * universe.setpc_y + 19);
			InitRndLocBigObj(universe, 10, 15, OBJ_SARC);
#ifdef HELLFIRE
			if (universe.currlevel >= 21)
				add_crypt_objs(0, 0, MAXDUNX, MAXDUNY);
			else
#endif
				AddL1Objs(universe, 0, 0, MAXDUNX, MAXDUNY);
			InitRndBarrels(universe);
		}
		if (universe.leveltype == DTYPE_CATACOMBS) {
			if (QuestStatus(universe, Q_ROCK))
				InitRndLocObj5x5(universe, 1, 1, OBJ_STAND);
			if (QuestStatus(universe, Q_SCHAMB))
				InitRndLocObj5x5(universe, 1, 1, OBJ_BOOK2R);
			AddL2Objs(universe, 0, 0, MAXDUNX, MAXDUNY);
			AddL2Torches(universe);
			if (QuestStatus(universe, Q_BLIND)) {
				if (universe.plr[myplr]._pClass == PC_WARRIOR) {
					sp_id = TEXT_BLINDING;
				} else if (universe.plr[myplr]._pClass == PC_ROGUE) {
					sp_id = TEXT_RBLINDING;
				} else if (universe.plr[myplr]._pClass == PC_SORCERER) {
					sp_id = TEXT_MBLINDING;
#ifdef HELLFIRE
				} else if (universe.plr[myplr]._pClass == PC_MONK) {
					sp_id = TEXT_HBLINDING;
				} else if (universe.plr[myplr]._pClass == PC_BARD) {
					sp_id = TEXT_BBLINDING;
				} else if (universe.plr[myplr]._pClass == PC_BARBARIAN) {
					sp_id = TEXT_BLINDING;
#endif
				}
				universe.quests[Q_BLIND]._qmsg = sp_id;
				AddBookLever(universe, 0, 0, MAXDUNX, MAXDUNY, universe.setpc_x, universe.setpc_y, universe.setpc_w + universe.setpc_x + 1, universe.setpc_h + universe.setpc_y + 1, sp_id);
				mem = LoadFileInMem("Levels\\L2Data\\Blind2.DUN", NULL);
				// BUGFIX: should not invoke LoadMapObjs for Blind2.DUN, as Blind2.DUN is missing an objects layer.
				LoadMapObjs(universe, mem, 2 * universe.setpc_x, 2 * universe.setpc_y);
				mem_free_dbg(mem);
			}
			if (QuestStatus(universe, Q_BLOOD)) {
				if (universe.plr[myplr]._pClass == PC_WARRIOR) {
					sp_id = TEXT_BLOODY;
				} else if (universe.plr[myplr]._pClass == PC_ROGUE) {
					sp_id = TEXT_RBLOODY;
				} else if (universe.plr[myplr]._pClass == PC_SORCERER) {
					sp_id = TEXT_MBLOODY;
#ifdef HELLFIRE
				} else if (universe.plr[myplr]._pClass == PC_MONK) {
					sp_id = TEXT_HBLOODY;
				} else if (universe.plr[myplr]._pClass == PC_BARD) {
					sp_id = TEXT_BBLOODY;
				} else if (universe.plr[myplr]._pClass == PC_BARBARIAN) {
					sp_id = TEXT_BLOODY;
#endif
				}
				universe.quests[Q_BLOOD]._qmsg = sp_id;
				AddBookLever(universe, 0, 0, MAXDUNX, MAXDUNY, universe.setpc_x, universe.setpc_y + 3, universe.setpc_x + 2, universe.setpc_y + 7, sp_id);
				AddObject(universe, OBJ_PEDISTAL, 2 * universe.setpc_x + 25, 2 * universe.setpc_y + 32);
			}
			InitRndBarrels(universe);
		}
		if (universe.leveltype == DTYPE_CAVES) {
			AddL3Objs(universe, 0, 0, MAXDUNX, MAXDUNY);
			InitRndBarrels(universe);
		}
		if (universe.leveltype == DTYPE_HELL) {
			if (QuestStatus(universe, Q_WARLORD)) {
				if (universe.plr[myplr]._pClass == PC_WARRIOR) {
					sp_id = TEXT_BLOODWAR;
				} else if (universe.plr[myplr]._pClass == PC_ROGUE) {
					sp_id = TEXT_RBLOODWAR;
				} else if (universe.plr[myplr]._pClass == PC_SORCERER) {
					sp_id = TEXT_MBLOODWAR;
#ifdef HELLFIRE
				} else if (universe.plr[myplr]._pClass == PC_MONK) {
					sp_id = TEXT_HBLOODWAR;
				} else if (universe.plr[myplr]._pClass == PC_BARD) {
					sp_id = TEXT_BBLOODWAR;
				} else if (universe.plr[myplr]._pClass == PC_BARBARIAN) {
					sp_id = TEXT_BLOODWAR;
#endif
				}
				universe.quests[Q_WARLORD]._qmsg = sp_id;
				AddBookLever(universe, 0, 0, MAXDUNX, MAXDUNY, universe.setpc_x, universe.setpc_y, universe.setpc_x + universe.setpc_w, universe.setpc_y + universe.setpc_h, sp_id);
				mem = LoadFileInMem("Levels\\L4Data\\Warlord.DUN", NULL);
				LoadMapObjs(universe, mem, 2 * universe.setpc_x, 2 * universe.setpc_y);
				mem_free_dbg(mem);
			}
			if (QuestStatus(universe, Q_BETRAYER) && universe.gbMaxPlayers == 1)
				AddLazStand(universe);
			InitRndBarrels(universe);
			AddL4Goodies(universe);
		}
		InitRndLocObj(universe, 5, 10, OBJ_CHEST1);
		InitRndLocObj(universe, 3, 6, OBJ_CHEST2);
		InitRndLocObj(universe, 1, 5, OBJ_CHEST3);
		if (universe.leveltype != DTYPE_HELL)
			AddObjTraps(universe);
		if (universe.leveltype > DTYPE_CATHEDRAL)
			AddChestTraps(universe);
		universe.InitObjFlag = FALSE;
	}
}

#ifndef SPAWN
void SetMapObjects(Universe& universe, BYTE *pMap, int startx, int starty)
{
	int rw, rh;
	int i, j;
	BYTE *lm, *h;
	long mapoff;
	int fileload[56];
	char filestr[32];

	ClrAllObjects(universe);
	for (i = 0; i < 56; i++)
		fileload[i] = FALSE;
	universe.InitObjFlag = TRUE;

	for (i = 0; AllObjects[i].oload != -1; i++) {
		if (AllObjects[i].oload == 1 && universe.leveltype == AllObjects[i].olvltype)
			fileload[AllObjects[i].ofindex] = TRUE;
	}

	lm = pMap;
	rw = *lm;
	lm += 2;
	rh = *lm;
	mapoff = (rw * rh + 1) * 2;
	rw <<= 1;
	rh <<= 1;
	mapoff += 2 * rw * rh * 2;
	lm += mapoff;
	h = lm;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*lm) {
				fileload[AllObjects[ObjTypeConv[*lm]].ofindex] = TRUE;
			}
			lm += 2;
		}
	}

	for (i = 0; i < 56; i++) {
		if (!fileload[i])
			continue;

		universe.ObjFileList[universe.numobjfiles] = i;
		sprintf(filestr, "Objects\\%s.CEL", ObjMasterLoadList[i]);
		pObjCels[universe.numobjfiles] = LoadFileInMem(filestr, NULL);
		universe.numobjfiles++;
	}

	lm = h;
	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*lm)
				AddObject(universe, ObjTypeConv[*lm], startx + 16 + i, starty + 16 + j);
			lm += 2;
		}
	}
	universe.InitObjFlag = FALSE;
}
#endif

void DeleteObject_(Universe& universe, int oi, int i)
{
	int ox, oy;

	ox = universe.object[oi]._ox;
	oy = universe.object[oi]._oy;
	universe.dObject[ox][oy] = 0;
	universe.objectavail[-universe.nobjects + MAXOBJECTS] = oi;
	universe.nobjects--;
	if (universe.nobjects > 0 && i != universe.nobjects)
		universe.objectactive[i] = universe.objectactive[universe.nobjects];
}

void SetupObject(Universe& universe, int i, int x, int y, int ot)
{
	int ofi;
	int j;

	universe.object[i]._otype = ot;
	ofi = AllObjects[ot].ofindex;
	universe.object[i]._ox = x;
	universe.object[i]._oy = y;
	universe.object[i]._oAnimFlag = AllObjects[ot].oAnimFlag;
	if (AllObjects[ot].oAnimFlag) {
		universe.object[i]._oAnimDelay = AllObjects[ot].oAnimDelay;
		universe.object[i]._oAnimCnt = random_(universe, 146, AllObjects[ot].oAnimDelay);
		universe.object[i]._oAnimLen = AllObjects[ot].oAnimLen;
		universe.object[i]._oAnimFrame = random_(universe, 146, AllObjects[ot].oAnimLen - 1) + 1;
	} else {
		universe.object[i]._oAnimDelay = 1000;
		universe.object[i]._oAnimCnt = 0;
		universe.object[i]._oAnimLen = AllObjects[ot].oAnimLen;
		universe.object[i]._oAnimFrame = AllObjects[ot].oAnimDelay;
	}
	universe.object[i]._oAnimWidth = AllObjects[ot].oAnimWidth;
	universe.object[i]._oSolidFlag = AllObjects[ot].oSolidFlag;
	universe.object[i]._oMissFlag = AllObjects[ot].oMissFlag;
	universe.object[i]._oLight = AllObjects[ot].oLightFlag;
	universe.object[i]._oDelFlag = FALSE;
	universe.object[i]._oBreak = AllObjects[ot].oBreak;
	universe.object[i]._oSelFlag = AllObjects[ot].oSelFlag;
	universe.object[i]._oPreFlag = FALSE;
	universe.object[i]._oTrapFlag = FALSE;
	universe.object[i]._oDoorFlag = FALSE;
}

void SetObjMapRange(Universe& universe, int i, int x1, int y1, int x2, int y2, int v)
{
	universe.object[i]._oVar1 = x1;
	universe.object[i]._oVar2 = y1;
	universe.object[i]._oVar3 = x2;
	universe.object[i]._oVar4 = y2;
	universe.object[i]._oVar8 = v;
}

void SetBookMsg(Universe& universe, int i, int msg)
{
	universe.object[i]._oVar7 = msg;
}

void AddL1Door(Universe& universe, int i, int x, int y, int ot)
{
	universe.object[i]._oDoorFlag = TRUE;
	if (ot == 1) {
		universe.object[i]._oVar1 = universe.dPiece[x][y];
		universe.object[i]._oVar2 = universe.dPiece[x][y - 1];
	} else {
		universe.object[i]._oVar1 = universe.dPiece[x][y];
		universe.object[i]._oVar2 = universe.dPiece[x - 1][y];
	}
	universe.object[i]._oVar4 = 0;
}

void AddSCambBook(Universe& universe, int i)
{
	universe.object[i]._oVar1 = universe.setpc_x;
	universe.object[i]._oVar2 = universe.setpc_y;
	universe.object[i]._oVar3 = universe.setpc_w + universe.setpc_x + 1;
	universe.object[i]._oVar4 = universe.setpc_h + universe.setpc_y + 1;
	universe.object[i]._oVar6 = universe.object[i]._oAnimFrame + 1;
}

void AddChest(Universe& universe, int i, int t)
{
	if (random_(universe, 147, 2) == 0)
		universe.object[i]._oAnimFrame += 3;
	universe.object[i]._oRndSeed = GetRndSeed(universe);
	switch (t) {
	case OBJ_CHEST1:
	case OBJ_TCHEST1:
		if (universe.setlevel) {
			universe.object[i]._oVar1 = 1;
			break;
		}
		universe.object[i]._oVar1 = random_(universe, 147, 2);
		break;
	case OBJ_TCHEST2:
	case OBJ_CHEST2:
		if (universe.setlevel) {
			universe.object[i]._oVar1 = 2;
			break;
		}
		universe.object[i]._oVar1 = random_(universe, 147, 3);
		break;
	case OBJ_TCHEST3:
	case OBJ_CHEST3:
		if (universe.setlevel) {
			universe.object[i]._oVar1 = 3;
			break;
		}
		universe.object[i]._oVar1 = random_(universe, 147, 4);
		break;
	}
	universe.object[i]._oVar2 = random_(universe, 147, 8);
}

void AddL2Door(Universe& universe, int i, int x, int y, int ot)
{
	universe.object[i]._oDoorFlag = TRUE;
	if (ot == OBJ_L2LDOOR)
		ObjSetMicro(universe, x, y, 538);
	else
		ObjSetMicro(universe, x, y, 540);
	universe.object[i]._oVar4 = 0;
}

void AddL3Door(Universe& universe, int i, int x, int y, int ot)
{
	universe.object[i]._oDoorFlag = TRUE;
	if (ot == OBJ_L3LDOOR)
		ObjSetMicro(universe, x, y, 531);
	else
		ObjSetMicro(universe, x, y, 534);
	universe.object[i]._oVar4 = 0;
}

void AddSarc(Universe& universe, int i)
{
	universe.dObject[universe.object[i]._ox][universe.object[i]._oy - 1] = -(i + 1);
	universe.object[i]._oVar1 = random_(universe, 153, 10);
	universe.object[i]._oRndSeed = GetRndSeed(universe);
	if (universe.object[i]._oVar1 >= 8)
		universe.object[i]._oVar2 = PreSpawnSkeleton(universe);
}

void AddFlameTrap(Universe& universe, int i)
{
	universe.object[i]._oVar1 = universe.trapid;
	universe.object[i]._oVar2 = 0;
	universe.object[i]._oVar3 = universe.trapdir;
	universe.object[i]._oVar4 = 0;
}

void AddFlameLvr(Universe& universe, int i)
{
	universe.object[i]._oVar1 = universe.trapid;
	universe.object[i]._oVar2 = MIS_FLAMEC;
}

void AddTrap(Universe& universe, int i, int ot)
{
	int mt;

	mt = universe.currlevel / 3 + 1;
#ifdef HELLFIRE
	if (universe.currlevel > 16) {
		mt = (universe.currlevel - 4) / 3 + 1;
	}
	if (universe.currlevel > 20) {
		mt = (universe.currlevel - 8) / 3 + 1;
	}
#endif
	mt = random_(universe, 148, mt);
	if (mt == 0)
		universe.object[i]._oVar3 = MIS_ARROW;
	if (mt == 1)
		universe.object[i]._oVar3 = MIS_FIREBOLT;
	if (mt == 2)
		universe.object[i]._oVar3 = MIS_LIGHTCTRL;
	universe.object[i]._oVar4 = 0;
}

void AddObjLight(Universe& universe, int i, int r)
{
	if (universe.InitObjFlag) {
		DoLighting(universe.object[i]._ox, universe.object[i]._oy, r, -1);
		universe.object[i]._oVar1 = -1;
	} else {
		universe.object[i]._oVar1 = 0;
	}
}

void AddBarrel(Universe& universe, int i, int t)
{
	universe.object[i]._oVar1 = 0;
	universe.object[i]._oRndSeed = GetRndSeed(universe);
	universe.object[i]._oVar2 = random_(universe, 149, 10);
	universe.object[i]._oVar3 = random_(universe, 149, 3);

	if (universe.object[i]._oVar2 >= 8)
		universe.object[i]._oVar4 = PreSpawnSkeleton(universe);
}

void AddShrine(Universe& universe, int i)
{
	int val;
	DIABOOL slist[NUM_SHRINETYPE];
#ifdef HELLFIRE
	unsigned int j;
#else
	int j;
#endif

	// BUGFIX: the seed of shrine objects (universe.object[i]._oRndSeed) was never
	// initialized. This lead to undefined behaviour, as the shrine object would
	// use whatever value was present in memory (often the seed of an object with
	// the same object index of a previous dungeon level).

	universe.object[i]._oPreFlag = TRUE;
	for (j = 0; j < NUM_SHRINETYPE; j++) {
		if (universe.currlevel < shrinemin[j] || universe.currlevel > shrinemax[j]) {
			slist[j] = 0;
		} else {
			slist[j] = 1;
		}
		if (universe.gbMaxPlayers != 1 && shrineavail[j] == 1) {
			slist[j] = 0;
		}
		if (universe.gbMaxPlayers == 1 && shrineavail[j] == 2) {
			slist[j] = 0;
		}
	}
	do {
		val = random_(universe, 150, NUM_SHRINETYPE);
	} while (!slist[val]);

	universe.object[i]._oVar1 = val;
	if (random_(universe, 150, 2) != 0) {
		universe.object[i]._oAnimFrame = 12;
		universe.object[i]._oAnimLen = 22;
	}
}

void AddBookcase(Universe& universe, int i)
{
	universe.object[i]._oRndSeed = GetRndSeed(universe);
	universe.object[i]._oPreFlag = TRUE;
}

void AddBookstand(Universe& universe, int i)
{
	universe.object[i]._oRndSeed = GetRndSeed(universe);
}

void AddBloodFtn(Universe& universe, int i)
{
	universe.object[i]._oRndSeed = GetRndSeed(universe);
}

void AddPurifyingFountain(Universe& universe, int i)
{
	int ox, oy;

	ox = universe.object[i]._ox;
	oy = universe.object[i]._oy;
	universe.dObject[ox][oy - 1] = -1 - i;
	universe.dObject[ox - 1][oy] = -1 - i;
	universe.dObject[ox - 1][oy - 1] = -1 - i;
	universe.object[i]._oRndSeed = GetRndSeed(universe);
}

void AddArmorStand(Universe& universe, int i)
{
	if (!armorFlag) {
		universe.object[i]._oAnimFlag = 2;
		universe.object[i]._oSelFlag = 0;
	}

	universe.object[i]._oRndSeed = GetRndSeed(universe);
}

void AddGoatShrine(Universe& universe, int i)
{
	universe.object[i]._oRndSeed = GetRndSeed(universe);
}

void AddCauldron(Universe& universe, int i)
{
	universe.object[i]._oRndSeed = GetRndSeed(universe);
}

void AddMurkyFountain(Universe& universe, int i)
{
	int ox, oy;

	ox = universe.object[i]._ox;
	oy = universe.object[i]._oy;
	universe.dObject[ox][oy - 1] = -1 - i;
	universe.dObject[ox - 1][oy] = -1 - i;
	universe.dObject[ox - 1][oy - 1] = -1 - i;
	universe.object[i]._oRndSeed = GetRndSeed(universe);
}

void AddTearFountain(Universe& universe, int i)
{
	universe.object[i]._oRndSeed = GetRndSeed(universe);
}

void AddDecap(Universe& universe, int i)
{
	universe.object[i]._oRndSeed = GetRndSeed(universe);
	universe.object[i]._oAnimFrame = random_(universe, 151, 8) + 1;
	universe.object[i]._oPreFlag = TRUE;
}

void AddVilebook(Universe& universe, int i)
{
	if (universe.setlevel && universe.setlvlnum == SL_VILEBETRAYER) {
		universe.object[i]._oAnimFrame = 4;
	}
}

void AddMagicCircle(Universe& universe, int i)
{
	universe.object[i]._oRndSeed = GetRndSeed(universe);
	universe.object[i]._oPreFlag = TRUE;
	universe.object[i]._oVar6 = 0;
	universe.object[i]._oVar5 = 1;
}

void AddBrnCross(Universe& universe, int i)
{
	universe.object[i]._oRndSeed = GetRndSeed(universe);
}

void AddPedistal(Universe& universe, int i)
{
	universe.object[i]._oVar1 = universe.setpc_x;
	universe.object[i]._oVar2 = universe.setpc_y;
	universe.object[i]._oVar3 = universe.setpc_x + universe.setpc_w;
	universe.object[i]._oVar4 = universe.setpc_y + universe.setpc_h;
}

void AddStoryBook(Universe& universe, int i)
{
	SetRndSeed(universe, universe.glSeedTbl[16]);

	universe.object[i]._oVar1 = random_(universe, 0, 3);
	if (universe.currlevel == 4)
		universe.object[i]._oVar2 = StoryText[universe.object[i]._oVar1][0];
#ifdef HELLFIRE
	if (universe.currlevel == 8)
#else
	else if (universe.currlevel == 8)
#endif
		universe.object[i]._oVar2 = StoryText[universe.object[i]._oVar1][1];
#ifdef HELLFIRE
	if (universe.currlevel == 12)
#else
	else if (universe.currlevel == 12)
#endif
		universe.object[i]._oVar2 = StoryText[universe.object[i]._oVar1][2];
	universe.object[i]._oVar3 = (universe.currlevel >> 2) + 3 * universe.object[i]._oVar1 - 1;
	universe.object[i]._oAnimFrame = 5 - 2 * universe.object[i]._oVar1;
	universe.object[i]._oVar4 = universe.object[i]._oAnimFrame + 1;
}

void AddWeaponRack(Universe& universe, int i)
{
	if (!weaponFlag) {
		universe.object[i]._oAnimFlag = 2;
		universe.object[i]._oSelFlag = 0;
	}
	universe.object[i]._oRndSeed = GetRndSeed(universe);
}

void AddTorturedBody(Universe& universe, int i)
{
	universe.object[i]._oRndSeed = GetRndSeed(universe);
	universe.object[i]._oAnimFrame = random_(universe, 0, 4) + 1;
	universe.object[i]._oPreFlag = TRUE;
}

void GetRndObjLoc(Universe& universe, int randarea, int &xx, int &yy)
{
	DIABOOL failed;
	int i, j, tries;

	if (randarea == 0)
		return;

	tries = 0;
	while (1) {
		tries++;
		if (tries > 1000 && randarea > 1)
			randarea--;
		xx = random_(universe, 0, MAXDUNX);
		yy = random_(universe, 0, MAXDUNY);
		failed = FALSE;
		for (i = 0; i < randarea && !failed; i++) {
			for (j = 0; j < randarea && !failed; j++) {
				failed = !RndLocOk(universe, i + xx, j + yy);
			}
		}
		if (!failed)
			break;
	}
}

void AddMushPatch(Universe& universe)
{
	int i;
	int y, x;

	if (universe.nobjects < MAXOBJECTS) {
		i = universe.objectavail[0];
		GetRndObjLoc(universe, 5, x, y);
		universe.dObject[x + 1][y + 1] = -1 - i;
		universe.dObject[x + 2][y + 1] = -1 - i;
		universe.dObject[x + 1][y + 2] = -1 - i;
		AddObject(universe, OBJ_MUSHPATCH, x + 2, y + 2);
	}
}

void AddSlainHero(Universe& universe)
{
	int x, y;

	GetRndObjLoc(universe, 5, x, y);
	AddObject(universe, OBJ_SLAINHERO, x + 2, y + 2);
}

#ifdef HELLFIRE
void AddCryptBook(Universe& universe, int ot, int v2, int ox, int oy)
{
	int oi;

	if (universe.nobjects >= MAXOBJECTS)
		return;

	oi = universe.objectavail[0];
	universe.objectavail[0] = universe.objectavail[MAXOBJECTS - 1 - universe.nobjects];
	universe.objectactive[universe.nobjects] = oi;
	universe.dObject[ox][oy] = oi + 1;
	SetupObject(universe, oi, ox, oy, ot);
	AddCryptObject(universe, oi, v2);
	universe.object[oi]._oAnimWidth2 = (universe.object[oi]._oAnimWidth - 64) >> 1;
	universe.nobjects++;
}

void AddCryptObject(Universe& universe, int i, int a2)
{
	int v8, v9;
	if (a2 > 5) {
		universe.object[i]._oVar8 = a2;
		switch (a2) {
		case 6:
			if (universe.plr[myplr]._pClass == PC_WARRIOR) {
				universe.object[i]._oVar2 = 323;
			} else if (universe.plr[myplr]._pClass == PC_ROGUE) {
				universe.object[i]._oVar2 = 332;
			} else if (universe.plr[myplr]._pClass == PC_SORCERER) {
				universe.object[i]._oVar2 = 329;
			} else if (universe.plr[myplr]._pClass == PC_MONK) {
				universe.object[i]._oVar2 = 326;
			} else if (universe.plr[myplr]._pClass == PC_BARD) {
				universe.object[i]._oVar2 = 335;
			} else if (universe.plr[myplr]._pClass == PC_BARBARIAN) {
				universe.object[i]._oVar2 = 323;
			}
			break;
		case 7:
			if (universe.plr[myplr]._pClass == PC_WARRIOR) {
				universe.object[i]._oVar2 = 324;
			} else if (universe.plr[myplr]._pClass == PC_ROGUE) {
				universe.object[i]._oVar2 = 333;
			} else if (universe.plr[myplr]._pClass == PC_SORCERER) {
				universe.object[i]._oVar2 = 330;
			} else if (universe.plr[myplr]._pClass == PC_MONK) {
				universe.object[i]._oVar2 = 327;
			} else if (universe.plr[myplr]._pClass == PC_BARD) {
				universe.object[i]._oVar2 = 336;
			} else if (universe.plr[myplr]._pClass == PC_BARBARIAN) {
				universe.object[i]._oVar2 = 324;
			}
			break;
		case 8:
			if (universe.plr[myplr]._pClass == PC_WARRIOR) {
				universe.object[i]._oVar2 = 325;
			} else if (universe.plr[myplr]._pClass == PC_ROGUE) {
				universe.object[i]._oVar2 = 334;
			} else if (universe.plr[myplr]._pClass == PC_SORCERER) {
				universe.object[i]._oVar2 = 331;
			} else if (universe.plr[myplr]._pClass == PC_MONK) {
				universe.object[i]._oVar2 = 328;
			} else if (universe.plr[myplr]._pClass == PC_BARD) {
				universe.object[i]._oVar2 = 337;
			} else if (universe.plr[myplr]._pClass == PC_BARBARIAN) {
				universe.object[i]._oVar2 = 325;
			}
			break;
		}
		universe.object[i]._oVar1 = 1;
		universe.object[i]._oVar3 = 15;
		v8 = 2 * universe.object[i]._oVar1;
		universe.object[i]._oAnimFrame = 5 - v8;
		universe.object[i]._oVar4 = universe.object[i]._oAnimFrame + 1;
	} else {

		universe.object[i]._oVar1 = 1;
		universe.object[i]._oVar2 = a2 + 316;
		universe.object[i]._oVar3 = a2 + 9;
		v9 = 2 * universe.object[i]._oVar1;
		universe.object[i]._oAnimFrame = 5 - v9;
		universe.object[i]._oVar4 = universe.object[i]._oAnimFrame + 1;
		universe.object[i]._oVar8 = 0;
	}
}
#endif

void AddObject(Universe& universe, int ot, int ox, int oy)
{
	int oi;

	if (universe.nobjects >= MAXOBJECTS)
		return;

	oi = universe.objectavail[0];
	universe.objectavail[0] = universe.objectavail[MAXOBJECTS - 1 - universe.nobjects];
	universe.objectactive[universe.nobjects] = oi;
	universe.dObject[ox][oy] = oi + 1;
	SetupObject(universe, oi, ox, oy, ot);
	switch (ot) {
	case OBJ_L1LIGHT:
		AddObjLight(universe, oi, 5);
		break;
	case OBJ_SKFIRE:
	case OBJ_CANDLE1:
	case OBJ_CANDLE2:
	case OBJ_BOOKCANDLE:
		AddObjLight(universe, oi, 5);
		break;
	case OBJ_STORYCANDLE:
		AddObjLight(universe, oi, 3);
		break;
	case OBJ_TORCHL:
	case OBJ_TORCHR:
	case OBJ_TORCHL2:
	case OBJ_TORCHR2:
		AddObjLight(universe, oi, 8);
		break;
	case OBJ_L1LDOOR:
	case OBJ_L1RDOOR:
		AddL1Door(universe, oi, ox, oy, ot);
		break;
	case OBJ_L2LDOOR:
	case OBJ_L2RDOOR:
		AddL2Door(universe, oi, ox, oy, ot);
		break;
	case OBJ_L3LDOOR:
	case OBJ_L3RDOOR:
		AddL3Door(universe, oi, ox, oy, ot);
		break;
	case OBJ_BOOK2R:
		AddSCambBook(universe, oi);
		break;
	case OBJ_CHEST1:
	case OBJ_CHEST2:
	case OBJ_CHEST3:
	case OBJ_TCHEST1:
	case OBJ_TCHEST2:
	case OBJ_TCHEST3:
		AddChest(universe, oi, ot);
		break;
	case OBJ_SARC:
		AddSarc(universe, oi);
		break;
	case OBJ_FLAMEHOLE:
		AddFlameTrap(universe, oi);
		break;
	case OBJ_FLAMELVR:
		AddFlameLvr(universe, oi);
		break;
	case OBJ_WATER:
		universe.object[oi]._oAnimFrame = 1;
		break;
	case OBJ_TRAPL:
	case OBJ_TRAPR:
		AddTrap(universe, oi, ot);
		break;
	case OBJ_BARREL:
	case OBJ_BARRELEX:
		AddBarrel(universe, oi, ot);
		break;
	case OBJ_SHRINEL:
	case OBJ_SHRINER:
		AddShrine(universe, oi);
		break;
	case OBJ_BOOKCASEL:
	case OBJ_BOOKCASER:
		AddBookcase(universe, oi);
		break;
	case OBJ_SKELBOOK:
	case OBJ_BOOKSTAND:
		AddBookstand(universe, oi);
		break;
	case OBJ_BLOODFTN:
		AddBloodFtn(universe, oi);
		break;
	case OBJ_DECAP:
		AddDecap(universe, oi);
		break;
	case OBJ_PURIFYINGFTN:
		AddPurifyingFountain(universe, oi);
		break;
	case OBJ_ARMORSTAND:
	case OBJ_WARARMOR:
		AddArmorStand(universe, oi);
		break;
	case OBJ_GOATSHRINE:
		AddGoatShrine(universe, oi);
		break;
	case OBJ_CAULDRON:
		AddCauldron(universe, oi);
		break;
	case OBJ_MURKYFTN:
		AddMurkyFountain(universe, oi);
		break;
	case OBJ_TEARFTN:
		AddTearFountain(universe, oi);
		break;
	case OBJ_BOOK2L:
		AddVilebook(universe, oi);
		break;
	case OBJ_MCIRCLE1:
	case OBJ_MCIRCLE2:
		AddMagicCircle(universe, oi);
		break;
	case OBJ_STORYBOOK:
		AddStoryBook(universe, oi);
		break;
	case OBJ_BCROSS:
	case OBJ_TBCROSS:
		AddBrnCross(universe, oi);
		AddObjLight(universe, oi, 5);
		break;
	case OBJ_PEDISTAL:
		AddPedistal(universe, oi);
		break;
	case OBJ_WARWEAP:
	case OBJ_WEAPONRACK:
		AddWeaponRack(universe, oi);
		break;
	case OBJ_TNUDEM2:
		AddTorturedBody(universe, oi);
		break;
	}
	universe.object[oi]._oAnimWidth2 = (universe.object[oi]._oAnimWidth - 64) >> 1;
	universe.nobjects++;
}

void ObjSetMicro(Universe& universe, int dx, int dy, int pn)
{
	WORD *v;
	MICROS *defs;
	int i;

	universe.dPiece[dx][dy] = pn;
	pn--;
	defs = &universe.dpiece_defs_map_1[IsometricCoord(dx, dy)];
	if (universe.leveltype != DTYPE_HELL) {
		v = (WORD *)pLevelPieces + 10 * pn;
		for (i = 0; i < 10; i++) {
			defs->mt[i] = v[(i & 1) - (i & 0xE) + 8];
		}
	} else {
		v = (WORD *)pLevelPieces + 16 * pn;
		for (i = 0; i < 16; i++) {
			defs->mt[i] = v[(i & 1) - (i & 0xE) + 14];
		}
	}
}

void objects_set_door_piece(Universe& universe, int x, int y)
{
	int pn;
	long v1, v2;

	pn = universe.dPiece[x][y] - 1;

#ifdef USE_ASM
	__asm {
	mov		esi, pLevelPieces
	xor		eax, eax
	mov		ax, word ptr pn
	mov		ebx, 20
	mul		ebx
	add		esi, eax
	add		esi, 16
	xor		eax, eax
	lodsw
	mov		word ptr v1, ax
	lodsw
	mov		word ptr v2, ax
	}
#else
	v1 = *((WORD *)pLevelPieces + 10 * pn + 8);
	v2 = *((WORD *)pLevelPieces + 10 * pn + 9);
#endif
	universe.dpiece_defs_map_1[IsometricCoord(x, y)].mt[0] = v1;
	universe.dpiece_defs_map_1[IsometricCoord(x, y)].mt[1] = v2;
}

void ObjSetMini(Universe& universe, int x, int y, int v)
{
	int xx, yy;
	long v1, v2, v3, v4;

#ifdef USE_ASM
	__asm {
		mov		esi, pMegaTiles
		xor		eax, eax
		mov		ax, word ptr v
		dec		eax
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
	v1 = *((WORD *)&pMegaTiles[((WORD)v - 1) * 8]) + 1;
	v2 = *((WORD *)&pMegaTiles[((WORD)v - 1) * 8] + 1) + 1;
	v3 = *((WORD *)&pMegaTiles[((WORD)v - 1) * 8] + 2) + 1;
	v4 = *((WORD *)&pMegaTiles[((WORD)v - 1) * 8] + 3) + 1;
#endif

	xx = 2 * x + 16;
	yy = 2 * y + 16;
	ObjSetMicro(universe, xx, yy, v1);
	ObjSetMicro(universe, xx + 1, yy, v2);
	ObjSetMicro(universe, xx, yy + 1, v3);
	ObjSetMicro(universe, xx + 1, yy + 1, v4);
}

void ObjL1Special(Universe& universe, int x1, int y1, int x2, int y2)
{
	int i, j;

	for (i = y1; i <= y2; ++i) {
		for (j = x1; j <= x2; ++j) {
			universe.dSpecial[j][i] = 0;
			if (universe.dPiece[j][i] == 12)
				universe.dSpecial[j][i] = 1;
			if (universe.dPiece[j][i] == 11)
				universe.dSpecial[j][i] = 2;
			if (universe.dPiece[j][i] == 71)
				universe.dSpecial[j][i] = 1;
			if (universe.dPiece[j][i] == 259)
				universe.dSpecial[j][i] = 5;
			if (universe.dPiece[j][i] == 249)
				universe.dSpecial[j][i] = 2;
			if (universe.dPiece[j][i] == 325)
				universe.dSpecial[j][i] = 2;
			if (universe.dPiece[j][i] == 321)
				universe.dSpecial[j][i] = 1;
			if (universe.dPiece[j][i] == 255)
				universe.dSpecial[j][i] = 4;
			if (universe.dPiece[j][i] == 211)
				universe.dSpecial[j][i] = 1;
			if (universe.dPiece[j][i] == 344)
				universe.dSpecial[j][i] = 2;
			if (universe.dPiece[j][i] == 341)
				universe.dSpecial[j][i] = 1;
			if (universe.dPiece[j][i] == 331)
				universe.dSpecial[j][i] = 2;
			if (universe.dPiece[j][i] == 418)
				universe.dSpecial[j][i] = 1;
			if (universe.dPiece[j][i] == 421)
				universe.dSpecial[j][i] = 2;
		}
	}
}

void ObjL2Special(Universe& universe, int x1, int y1, int x2, int y2)
{
	int i, j;

	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			universe.dSpecial[i][j] = 0;
			if (universe.dPiece[i][j] == 541)
				universe.dSpecial[i][j] = 5;
			if (universe.dPiece[i][j] == 178)
				universe.dSpecial[i][j] = 5;
			if (universe.dPiece[i][j] == 551)
				universe.dSpecial[i][j] = 5;
			if (universe.dPiece[i][j] == 542)
				universe.dSpecial[i][j] = 6;
			if (universe.dPiece[i][j] == 553)
				universe.dSpecial[i][j] = 6;
			if (universe.dPiece[i][j] == 13)
				universe.dSpecial[i][j] = 5;
			if (universe.dPiece[i][j] == 17)
				universe.dSpecial[i][j] = 6;
		}
	}
	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			if (universe.dPiece[i][j] == 132) {
				universe.dSpecial[i][j + 1] = 2;
				universe.dSpecial[i][j + 2] = 1;
			}
			if (universe.dPiece[i][j] == 135 || universe.dPiece[i][j] == 139) {
				universe.dSpecial[i + 1][j] = 3;
				universe.dSpecial[i + 2][j] = 4;
			}
		}
	}
}

void DoorSet(Universe& universe, int oi, int dx, int dy)
{
	int pn;

	pn = universe.dPiece[dx][dy];
#ifdef HELLFIRE
	if (universe.currlevel < 17) {
#endif
		if (pn == 43)
			ObjSetMicro(universe, dx, dy, 392);
		if (pn == 45)
			ObjSetMicro(universe, dx, dy, 394);
		if (pn == 50 && universe.object[oi]._otype == OBJ_L1LDOOR)
			ObjSetMicro(universe, dx, dy, 411);
		if (pn == 50 && universe.object[oi]._otype == OBJ_L1RDOOR)
			ObjSetMicro(universe, dx, dy, 412);
		if (pn == 54)
			ObjSetMicro(universe, dx, dy, 397);
		if (pn == 55)
			ObjSetMicro(universe, dx, dy, 398);
		if (pn == 61)
			ObjSetMicro(universe, dx, dy, 399);
		if (pn == 67)
			ObjSetMicro(universe, dx, dy, 400);
		if (pn == 68)
			ObjSetMicro(universe, dx, dy, 401);
		if (pn == 69)
			ObjSetMicro(universe, dx, dy, 403);
		if (pn == 70)
			ObjSetMicro(universe, dx, dy, 404);
		if (pn == 72)
			ObjSetMicro(universe, dx, dy, 406);
		if (pn == 212)
			ObjSetMicro(universe, dx, dy, 407);
		if (pn == 354)
			ObjSetMicro(universe, dx, dy, 409);
		if (pn == 355)
			ObjSetMicro(universe, dx, dy, 410);
		if (pn == 411)
			ObjSetMicro(universe, dx, dy, 396);
		if (pn == 412)
			ObjSetMicro(universe, dx, dy, 396);
#ifdef HELLFIRE
	} else {
		if (pn == 75)
			ObjSetMicro(universe, dx, dy, 204);
		if (pn == 79)
			ObjSetMicro(universe, dx, dy, 208);
		if (pn == 86 && universe.object[oi]._otype == OBJ_L1LDOOR) {
			ObjSetMicro(universe, dx, dy, 232);
		}
		if (pn == 86 && universe.object[oi]._otype == OBJ_L1RDOOR) {
			ObjSetMicro(universe, dx, dy, 234);
		}
		if (pn == 91)
			ObjSetMicro(universe, dx, dy, 215);
		if (pn == 93)
			ObjSetMicro(universe, dx, dy, 218);
		if (pn == 99)
			ObjSetMicro(universe, dx, dy, 220);
		if (pn == 111)
			ObjSetMicro(universe, dx, dy, 222);
		if (pn == 113)
			ObjSetMicro(universe, dx, dy, 224);
		if (pn == 115)
			ObjSetMicro(universe, dx, dy, 226);
		if (pn == 117)
			ObjSetMicro(universe, dx, dy, 228);
		if (pn == 119)
			ObjSetMicro(universe, dx, dy, 230);
		if (pn == 232)
			ObjSetMicro(universe, dx, dy, 212);
		if (pn == 234)
			ObjSetMicro(universe, dx, dy, 212);
	}
#endif
}

void RedoPlayerVision(Universe& universe)
{
	int p;

	for (p = 0; p < MAX_PLRS; p++) {
		if (universe.plr[p].plractive && universe.currlevel == universe.plr[p].plrlevel) {
			ChangeVisionXY(universe.plr[p]._pvid, universe.plr[p]._px, universe.plr[p]._py);
		}
	}
}

void ObjChangeMap(Universe& universe, int x1, int y1, int x2, int y2)
{
	int i, j;

	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			ObjSetMini(universe, i, j, universe.pdungeon[i][j]);
			SetDungeon(universe, i, j, universe.pdungeon[i][j]);
		}
	}
#ifdef HELLFIRE
	if (universe.leveltype == DTYPE_CATHEDRAL && universe.currlevel < 17) {
#else
	if (universe.leveltype == DTYPE_CATHEDRAL) {
#endif
		ObjL1Special(universe, 2 * x1 + 16, 2 * y1 + 16, 2 * x2 + 17, 2 * y2 + 17);
		AddL1Objs(universe, 2 * x1 + 16, 2 * y1 + 16, 2 * x2 + 17, 2 * y2 + 17);
	}
	if (universe.leveltype == DTYPE_CATACOMBS) {
		ObjL2Special(universe, 2 * x1 + 16, 2 * y1 + 16, 2 * x2 + 17, 2 * y2 + 17);
		AddL2Objs(universe, 2 * x1 + 16, 2 * y1 + 16, 2 * x2 + 17, 2 * y2 + 17);
	}
}

void ObjChangeMapResync(Universe& universe, int x1, int y1, int x2, int y2)
{
	int i, j;

	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			ObjSetMini(universe, i, j, universe.pdungeon[i][j]);
			SetDungeon(universe, i, j, universe.pdungeon[i][j]);
		}
	}
#ifdef HELLFIRE
	if (universe.leveltype == DTYPE_CATHEDRAL && universe.currlevel < 17) {
#else
	if (universe.leveltype == DTYPE_CATHEDRAL) {
#endif
		ObjL1Special(universe, 2 * x1 + 16, 2 * y1 + 16, 2 * x2 + 17, 2 * y2 + 17);
	}
	if (universe.leveltype == DTYPE_CATACOMBS) {
		ObjL2Special(universe, 2 * x1 + 16, 2 * y1 + 16, 2 * x2 + 17, 2 * y2 + 17);
	}
}

int FindValidShrine(Universe& universe, int i)
{
	int rv;
	DIABOOL done;

	done = FALSE;
	do {
		rv = random_(universe, 0, NUM_SHRINETYPE);
		if (universe.currlevel >= shrinemin[rv] && universe.currlevel <= shrinemax[rv] && rv != SHRINE_THAUMATURGIC) {
			done = TRUE;
		}
		if (done) {
			if (universe.gbMaxPlayers != 1) {
				if (shrineavail[rv] == 1) {
					done = FALSE;
					continue;
				}
			}
			if (universe.gbMaxPlayers == 1) {
				if (shrineavail[rv] == 2) {
					done = FALSE;
					continue;
				}
			}
			done = TRUE;
		}
	} while (!done);
	return rv;
}

void GetObjectStr(Universe& universe, int i, char infostr[50])
{
	char tempstr[50];
	switch (universe.object[i]._otype) {
	case OBJ_CRUX1:
	case OBJ_CRUX2:
	case OBJ_CRUX3:
		strcpy(infostr, "Crucified Skeleton");
		break;
	case OBJ_LEVER:
	case OBJ_FLAMELVR:
		strcpy(infostr, "Lever");
		break;
	case OBJ_L1LDOOR:
	case OBJ_L1RDOOR:
	case OBJ_L2LDOOR:
	case OBJ_L2RDOOR:
	case OBJ_L3LDOOR:
	case OBJ_L3RDOOR:
		if (universe.object[i]._oVar4 == 1)
			strcpy(infostr, "Open Door");
		if (universe.object[i]._oVar4 == 0)
			strcpy(infostr, "Closed Door");
		if (universe.object[i]._oVar4 == 2)
			strcpy(infostr, "Blocked Door");
		break;
	case OBJ_BOOK2L:
		if (universe.setlevel) {
			if (universe.setlvlnum == SL_BONECHAMB) {
				strcpy(infostr, "Ancient Tome");
			} else if (universe.setlvlnum == SL_VILEBETRAYER) {
				strcpy(infostr, "Book of Vileness");
			}
		}
		break;
	case OBJ_SWITCHSKL:
		strcpy(infostr, "Skull Lever");
		break;
	case OBJ_BOOK2R:
		strcpy(infostr, "Mythical Book");
		break;
	case OBJ_CHEST1:
	case OBJ_TCHEST1:
		strcpy(infostr, "Small Chest");
		break;
	case OBJ_CHEST2:
	case OBJ_TCHEST2:
		strcpy(infostr, "Chest");
		break;
	case OBJ_CHEST3:
	case OBJ_TCHEST3:
	case OBJ_SIGNCHEST:
		strcpy(infostr, "Large Chest");
		break;
	case OBJ_SARC:
		strcpy(infostr, "Sarcophagus");
		break;
	case OBJ_BOOKSHELF:
		strcpy(infostr, "Bookshelf");
		break;
	case OBJ_BOOKCASEL:
	case OBJ_BOOKCASER:
		strcpy(infostr, "Bookcase");
		break;
	case OBJ_BARREL:
	case OBJ_BARRELEX:
#ifdef HELLFIRE
		if (universe.currlevel >= 17 && universe.currlevel <= 20)      // for hive levels
			strcpy(infostr, "Pod");                  //Then a barrel is called a pod
		else if (universe.currlevel >= 21 && universe.currlevel <= 24) // for crypt levels
			strcpy(infostr, "Urn");                  //Then a barrel is called an urn
		else
#endif
			strcpy(infostr, "Barrel");
		break;
	case OBJ_SHRINEL:
	case OBJ_SHRINER:
		sprintf(tempstr, "%s Shrine", shrinestrs[universe.object[i]._oVar1]);
		strcpy(infostr, tempstr);
		break;
	case OBJ_SKELBOOK:
		strcpy(infostr, "Skeleton Tome");
		break;
	case OBJ_BOOKSTAND:
		strcpy(infostr, "Library Book");
		break;
	case OBJ_BLOODFTN:
		strcpy(infostr, "Blood Fountain");
		break;
	case OBJ_DECAP:
		strcpy(infostr, "Decapitated Body");
		break;
	case OBJ_BLINDBOOK:
		strcpy(infostr, "Book of the Blind");
		break;
	case OBJ_BLOODBOOK:
		strcpy(infostr, "Book of Blood");
		break;
	case OBJ_PURIFYINGFTN:
		strcpy(infostr, "Purifying Spring");
		break;
	case OBJ_ARMORSTAND:
	case OBJ_WARARMOR:
		strcpy(infostr, "Armor");
		break;
	case OBJ_WARWEAP:
		strcpy(infostr, "Weapon Rack");
		break;
	case OBJ_GOATSHRINE:
		strcpy(infostr, "Goat Shrine");
		break;
	case OBJ_CAULDRON:
		strcpy(infostr, "Cauldron");
		break;
	case OBJ_MURKYFTN:
		strcpy(infostr, "Murky Pool");
		break;
	case OBJ_TEARFTN:
		strcpy(infostr, "Fountain of Tears");
		break;
	case OBJ_STEELTOME:
		strcpy(infostr, "Steel Tome");
		break;
	case OBJ_PEDISTAL:
		strcpy(infostr, "Pedestal of Blood");
		break;
	case OBJ_STORYBOOK:
		strcpy(infostr, StoryBookName[universe.object[i]._oVar3]);
		break;
	case OBJ_WEAPONRACK:
		strcpy(infostr, "Weapon Rack");
		break;
	case OBJ_MUSHPATCH:
		strcpy(infostr, "Mushroom Patch");
		break;
	case OBJ_LAZSTAND:
		strcpy(infostr, "Vile Stand");
		break;
	case OBJ_SLAINHERO:
		strcpy(infostr, "Slain Hero");
		break;
	}
	if (universe.plr[myplr]._pClass == PC_ROGUE) {
		if (universe.object[i]._oTrapFlag) {
			sprintf(tempstr, "Trapped %s", infostr);
			strcpy(infostr, tempstr);
		}
	}
}
