/**
 * @file themes.cpp
 *
 * Implementation of the theme room placing algorithms.
 */

#include "Source/themes.h"

#include "defs.h"
#include "structs.h"
#include "types.h"

#include "Source/engine.h"
#include "Source/gendung.h"
#include "Source/items.h"
#include "Source/monster.h"
#include "Source/objects.h"
#include "Source/quests.h"
#include "Source/trigs.h"
#include "Source/universe/universe.h"

BOOL ThemeGoodIn[4];
BOOL treasureFlag;
BOOL mFountainFlag;
BOOL cauldronFlag;
BOOL tFountainFlag;
int themex;
int themey;
int themeVar1;
BOOL pFountainFlag;
BOOL bFountainFlag;
BOOL bCrossFlag;

/** Specifies the set of special theme IDs from which one will be selected at random. */
const int ThemeGood[4] = { THEME_GOATSHRINE, THEME_SHRINE, THEME_SKELROOM, THEME_LIBRARY };
/** Specifies a 5x5 area to fit theme objects. */
const int trm5x[] = {
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2,
	-2, -1, 0, 1, 2
};
/** Specifies a 5x5 area to fit theme objects. */
const int trm5y[] = {
	-2, -2, -2, -2, -2,
	-1, -1, -1, -1, -1,
	0, 0, 0, 0, 0,
	1, 1, 1, 1, 1,
	2, 2, 2, 2, 2
};
/** Specifies a 3x3 area to fit theme objects. */
const int trm3x[] = {
	-1, 0, 1,
	-1, 0, 1,
	-1, 0, 1
};
/** Specifies a 3x3 area to fit theme objects. */
const int trm3y[] = {
	-1, -1, -1,
	0, 0, 0,
	1, 1, 1
};

BOOL TFit_Shrine(Universe& universe, int i)
{
	int xp, yp, found;

	xp = 0;
	yp = 0;
	found = 0;
	while (found == 0) {
		if (universe.dTransVal[xp][yp] == universe.themes[i].ttval) {
			if (universe.nTrapTable[universe.dPiece[xp][yp - 1]]
			    && !universe.nSolidTable[universe.dPiece[xp - 1][yp]]
			    && !universe.nSolidTable[universe.dPiece[xp + 1][yp]]
			    && universe.dTransVal[xp - 1][yp] == universe.themes[i].ttval
			    && universe.dTransVal[xp + 1][yp] == universe.themes[i].ttval
			    && universe.dObject[xp - 1][yp - 1] == 0
			    && universe.dObject[xp + 1][yp - 1] == 0) {
				found = 1;
			}
			if (found == 0
			    && universe.nTrapTable[universe.dPiece[xp - 1][yp]]
			    && !universe.nSolidTable[universe.dPiece[xp][yp - 1]]
			    && !universe.nSolidTable[universe.dPiece[xp][yp + 1]]
			    && universe.dTransVal[xp][yp - 1] == universe.themes[i].ttval
			    && universe.dTransVal[xp][yp + 1] == universe.themes[i].ttval
			    && universe.dObject[xp - 1][yp - 1] == 0
			    && universe.dObject[xp - 1][yp + 1] == 0) {
				found = 2;
			}
		}
		if (found == 0) {
			xp++;
			if (xp == MAXDUNX) {
				xp = 0;
				yp++;
				if (yp == MAXDUNY)
					return FALSE;
			}
		}
	}
	themex = xp;
	themey = yp;
	themeVar1 = found;
	return TRUE;
}

BOOL TFit_Obj5(Universe& universe, int t)
{
	int xp, yp;
	int i, r, rs;
	BOOL found;

	xp = 0;
	yp = 0;
	r = random_(universe, 0, 5) + 1;
	rs = r;
	while (r > 0) {
		found = FALSE;
		if (universe.dTransVal[xp][yp] == universe.themes[t].ttval && !universe.nSolidTable[universe.dPiece[xp][yp]]) {
			found = TRUE;
			for (i = 0; found && i < 25; i++) {
				if (universe.nSolidTable[universe.dPiece[xp + trm5x[i]][yp + trm5y[i]]]) {
					found = FALSE;
				}
				if (universe.dTransVal[xp + trm5x[i]][yp + trm5y[i]] != universe.themes[t].ttval) {
					found = FALSE;
				}
			}
		}

		if (!found) {
			xp++;
			if (xp == MAXDUNX) {
				xp = 0;
				yp++;
				if (yp == MAXDUNY) {
					if (r == rs) {
						return FALSE;
					}
					yp = 0;
				}
			}
			continue;
		}

		r--;
	}

	themex = xp;
	themey = yp;

	return TRUE;
}

BOOL TFit_SkelRoom(Universe& universe, int t)
{
	int i;

	if (universe.leveltype != DTYPE_CATHEDRAL && universe.leveltype != DTYPE_CATACOMBS) {
		return FALSE;
	}

	for (i = 0; i < universe.nummtypes; i++) {
		if (IsSkel(universe.Monsters[i].mtype)) {
			themeVar1 = i;
			return TFit_Obj5(universe, t);
		}
	}

	return FALSE;
}

BOOL TFit_GoatShrine(Universe& universe, int t)
{
	int i;

	for (i = 0; i < universe.nummtypes; i++) {
		if (IsGoat(universe.Monsters[i].mtype)) {
			themeVar1 = i;
			return TFit_Obj5(universe, t);
		}
	}

	return FALSE;
}

BOOL CheckThemeObj3(Universe& universe, int xp, int yp, int t, int f)
{
	int i;

	for (i = 0; i < 9; i++) {
		if (xp + trm3x[i] < 0 || yp + trm3y[i] < 0)
			return FALSE;
		if (universe.nSolidTable[universe.dPiece[xp + trm3x[i]][yp + trm3y[i]]])
			return FALSE;
		if (universe.dTransVal[xp + trm3x[i]][yp + trm3y[i]] != universe.themes[t].ttval)
			return FALSE;
		if (universe.dObject[xp + trm3x[i]][yp + trm3y[i]])
			return FALSE;
		if (f != -1 && random_(universe, 0, f) == 0)
			return FALSE;
	}

	return TRUE;
}

BOOL TFit_Obj3(Universe& universe, int t)
{
	int xp, yp;
	char objrnd[4] = { 4, 4, 3, 5 };

	for (yp = 1; yp < MAXDUNY - 1; yp++) {
		for (xp = 1; xp < MAXDUNX - 1; xp++) {
			if (CheckThemeObj3(universe, xp, yp, t, objrnd[universe.leveltype - 1])) {
				themex = xp;
				themey = yp;
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CheckThemeReqs(Universe& universe, int t)
{
	BOOL rv;

	rv = TRUE;
	switch (t) {
	case THEME_SHRINE:
	case THEME_SKELROOM:
	case THEME_LIBRARY:
		if (universe.leveltype == DTYPE_CAVES || universe.leveltype == DTYPE_HELL) {
			rv = FALSE;
		}
		break;
	case THEME_BLOODFOUNTAIN:
		if (!bFountainFlag) {
			rv = FALSE;
		}
		break;
	case THEME_PURIFYINGFOUNTAIN:
		if (!pFountainFlag) {
			rv = FALSE;
		}
		break;
	case THEME_ARMORSTAND:
		if (universe.leveltype == DTYPE_CATHEDRAL) {
			rv = FALSE;
		}
		break;
	case THEME_CAULDRON:
		if (universe.leveltype != DTYPE_HELL || !cauldronFlag) {
			rv = FALSE;
		}
		break;
	case THEME_MURKYFOUNTAIN:
		if (!mFountainFlag) {
			rv = FALSE;
		}
		break;
	case THEME_TEARFOUNTAIN:
		if (!tFountainFlag) {
			rv = FALSE;
		}
		break;
	case THEME_WEAPONRACK:
		if (universe.leveltype == DTYPE_CATHEDRAL) {
			rv = FALSE;
		}
		break;
	}

	return rv;
}

BOOL SpecialThemeFit(Universe& universe, int i, int t)
{
	BOOL rv;

	rv = CheckThemeReqs(universe, t);
	switch (t) {
	case THEME_SHRINE:
	case THEME_LIBRARY:
		if (rv) {
			rv = TFit_Shrine(universe, i);
		}
		break;
	case THEME_SKELROOM:
		if (rv) {
			rv = TFit_SkelRoom(universe, i);
		}
		break;
	case THEME_BLOODFOUNTAIN:
		if (rv) {
			rv = TFit_Obj5(universe, i);
		}
		if (rv) {
			bFountainFlag = FALSE;
		}
		break;
	case THEME_PURIFYINGFOUNTAIN:
		if (rv) {
			rv = TFit_Obj5(universe, i);
		}
		if (rv) {
			pFountainFlag = FALSE;
		}
		break;
	case THEME_MURKYFOUNTAIN:
		if (rv) {
			rv = TFit_Obj5(universe, i);
		}
		if (rv) {
			mFountainFlag = FALSE;
		}
		break;
	case THEME_TEARFOUNTAIN:
		if (rv) {
			rv = TFit_Obj5(universe, i);
		}
		if (rv) {
			tFountainFlag = FALSE;
		}
		break;
	case THEME_CAULDRON:
		if (rv) {
			rv = TFit_Obj5(universe, i);
		}
		if (rv) {
			cauldronFlag = FALSE;
		}
		break;
	case THEME_GOATSHRINE:
		if (rv) {
			rv = TFit_GoatShrine(universe, i);
		}
		break;
	case THEME_TORTURE:
	case THEME_DECAPITATED:
	case THEME_ARMORSTAND:
	case THEME_BRNCROSS:
	case THEME_WEAPONRACK:
		if (rv) {
			rv = TFit_Obj3(universe, i);
		}
		break;
	case THEME_TREASURE:
		rv = treasureFlag;
		if (rv) {
			treasureFlag = FALSE;
		}
		break;
	}

	return rv;
}

BOOL CheckThemeRoom(Universe& universe, int tv)
{
	int i, j, tarea;

	for (i = 0; i < numtrigs; i++) {
		if (universe.dTransVal[trigs[i]._tx][trigs[i]._ty] == tv)
			return FALSE;
	}

	tarea = 0;
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (universe.dTransVal[i][j] != tv)
				continue;
			if (universe.dFlags[i][j] & BFLAG_POPULATED)
				return FALSE;

			tarea++;
			if (tarea > 100)
				return FALSE;
		}
	}

	if (tarea < 9)
		return FALSE;

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (universe.dTransVal[i][j] != tv || universe.nSolidTable[universe.dPiece[i][j]])
				continue;
			if (universe.dTransVal[i - 1][j] != tv && !universe.nSolidTable[universe.dPiece[i - 1][j]])
				return FALSE;
			if (universe.dTransVal[i + 1][j] != tv && !universe.nSolidTable[universe.dPiece[i + 1][j]])
				return FALSE;
			if (universe.dTransVal[i][j - 1] != tv && !universe.nSolidTable[universe.dPiece[i][j - 1]])
				return FALSE;
			if (universe.dTransVal[i][j + 1] != tv && !universe.nSolidTable[universe.dPiece[i][j + 1]])
				return FALSE;
		}
	}

	return TRUE;
}

void InitThemes(Universe& universe)
{
	int i, j;

	universe.zharlib = -1;
	universe.numthemes = 0;
	universe.armorFlag = TRUE;
	bFountainFlag = TRUE;
	cauldronFlag = TRUE;
	mFountainFlag = TRUE;
	pFountainFlag = TRUE;
	tFountainFlag = TRUE;
	treasureFlag = TRUE;
	bCrossFlag = FALSE;
	universe.weaponFlag = TRUE;

	if (universe.currlevel == 16)
		return;

	if (universe.leveltype == DTYPE_CATHEDRAL) {
		for (i = 0; i < sizeof(ThemeGoodIn) / sizeof(ThemeGoodIn[0]); i++)
			ThemeGoodIn[i] = FALSE;

		for (i = 0; i < universe.TransVal && universe.numthemes < MAXTHEMES; i++) {
			if (CheckThemeRoom(universe, i)) {
				universe.themes[universe.numthemes].ttval = i;
				for (j = ThemeGood[random_(universe, 0, 4)];; j = random_(universe, 0, 17)) {
					if (SpecialThemeFit(universe, universe.numthemes, j)) {
						break;
					}
				}
				universe.themes[universe.numthemes].ttype = j;
				universe.numthemes++;
			}
		}
	}
	if (universe.leveltype == DTYPE_CATACOMBS || universe.leveltype == DTYPE_CAVES || universe.leveltype == DTYPE_HELL) {
		for (i = 0; i < universe.themeCount; i++)
			universe.themes[i].ttype = THEME_NONE;
		if (QuestStatus(universe, Q_ZHAR)) {
			for (j = 0; j < universe.themeCount; j++) {
				universe.themes[j].ttval = universe.themeLoc[j].ttval;
				if (SpecialThemeFit(universe, j, THEME_LIBRARY)) {
					universe.themes[j].ttype = THEME_LIBRARY;
					universe.zharlib = j;
					break;
				}
			}
		}
		for (i = 0; i < universe.themeCount; i++) {
			if (universe.themes[i].ttype == THEME_NONE) {
				universe.themes[i].ttval = universe.themeLoc[i].ttval;
				for (j = ThemeGood[random_(universe, 0, 4)];; j = random_(universe, 0, 17)) {
					if (SpecialThemeFit(universe, i, j)) {
						break;
					}
				}
				universe.themes[i].ttype = j;
			}
		}
		universe.numthemes += universe.themeCount;
	}
}

/**
 * @brief HoldThemeRooms marks theme rooms as populated.
 */
void HoldThemeRooms(Universe& universe)
{
	int i, x, y;
	char v;

	if (universe.currlevel != 16) {
		if (universe.leveltype == DTYPE_CATHEDRAL) {
			for (i = 0; i < universe.numthemes; i++) {
				v = universe.themes[i].ttval;
				for (y = 0; y < MAXDUNY; y++) {
					for (x = 0; x < MAXDUNX; x++) {
						if (universe.dTransVal[x][y] == v) {
							universe.dFlags[x][y] |= BFLAG_POPULATED;
						}
					}
				}
			}
		} else {
			DRLG_HoldThemeRooms(universe);
		}
	}
}

/**
 * PlaceThemeMonsts universe, places theme monsters with the specified frequency.
 *
 * @param t theme number (index into universe.themes array).
 * @param f frequency (1/f likelihood of adding monster).
 */
void PlaceThemeMonsts(Universe& universe, int t, int f)
{
	int xp, yp;
#ifdef HELLFIRE
	int scattertypes[138];
#else
	int scattertypes[111];
#endif
	int numscattypes, mtype, i;

	numscattypes = 0;
	for (i = 0; i < universe.nummtypes; i++) {
		if (universe.Monsters[i].mPlaceFlags & PLACE_SCATTER) {
			scattertypes[numscattypes] = i;
			numscattypes++;
		}
	}
	mtype = scattertypes[random_(universe, 0, numscattypes)];
	for (yp = 0; yp < MAXDUNY; yp++) {
		for (xp = 0; xp < MAXDUNX; xp++) {
			if (universe.dTransVal[xp][yp] == universe.themes[t].ttval && !universe.nSolidTable[universe.dPiece[xp][yp]] && universe.dItem[xp][yp] == 0 && universe.dObject[xp][yp] == 0) {
				if (random_(universe, 0, f) == 0) {
					AddMonster(universe, xp, yp, random_(universe, 0, 8), mtype, TRUE);
				}
			}
		}
	}
}

/**
 * Theme_Barrel initializes the barrel theme.
 *
 * @param t theme number (index into universe.themes array).
 */
void Theme_Barrel(Universe& universe, int t)
{
	int xp, yp, r;
	char barrnd[4] = { 2, 6, 4, 8 };
	char monstrnd[4] = { 5, 7, 3, 9 };

	for (yp = 0; yp < MAXDUNY; yp++) {
		for (xp = 0; xp < MAXDUNX; xp++) {
			if (universe.dTransVal[xp][yp] == universe.themes[t].ttval && !universe.nSolidTable[universe.dPiece[xp][yp]]) {
				if (random_(universe, 0, barrnd[universe.leveltype - 1]) == 0) {
					if (random_(universe, 0, barrnd[universe.leveltype - 1]) == 0) {
						r = OBJ_BARREL;
					} else {
						r = OBJ_BARRELEX;
					}
					AddObject(universe, r, xp, yp);
				}
			}
		}
	}
	PlaceThemeMonsts(universe, t, monstrnd[universe.leveltype - 1]);
}

/**
 * Theme_Shrine initializes the shrine theme.
 *
 * @param t theme number (index into universe.themes array).
 */
void Theme_Shrine(Universe& universe, int t)
{
	char monstrnd[4] = { 6, 6, 3, 9 };

	TFit_Shrine(universe, t);
	if (themeVar1 == 1) {
		AddObject(universe, OBJ_CANDLE2, themex - 1, themey);
		AddObject(universe, OBJ_SHRINER, themex, themey);
		AddObject(universe, OBJ_CANDLE2, themex + 1, themey);
	} else {
		AddObject(universe, OBJ_CANDLE2, themex, themey - 1);
		AddObject(universe, OBJ_SHRINEL, themex, themey);
		AddObject(universe, OBJ_CANDLE2, themex, themey + 1);
	}
	PlaceThemeMonsts(universe, t, monstrnd[universe.leveltype - 1]);
}

/**
 * Theme_MonstPit initializes the monster pit theme.
 *
 * @param t theme number (index into universe.themes array).
 */
void Theme_MonstPit(Universe& universe, int t)
{
	int r;
	int ixp, iyp;
	char monstrnd[4] = { 6, 7, 3, 9 };

	r = random_(universe, 0, 100) + 1;
	ixp = 0;
	iyp = 0;
	while (r > 0) {
		if (universe.dTransVal[ixp][iyp] == universe.themes[t].ttval && !universe.nSolidTable[universe.dPiece[ixp][iyp]]) {
			--r;
		}
		if (r <= 0)
			continue;
		ixp++;
		if (ixp == MAXDUNX) {
			ixp = 0;
			iyp++;
			if (iyp == MAXDUNY) {
				iyp = 0;
			}
		}
	}
	CreateRndItem(universe, ixp, iyp, TRUE, FALSE, TRUE);
	ItemNoFlippy(universe);
	PlaceThemeMonsts(universe, t, monstrnd[universe.leveltype - 1]);
}

/**
 * Theme_SkelRoom initializes the skeleton room theme.
 *
 * @param t theme number (index into universe.themes array).
 */
void Theme_SkelRoom(Universe& universe, int t)
{
	int xp, yp, i;
	char monstrnd[4] = { 6, 7, 3, 9 };

	TFit_SkelRoom(universe, t);

	xp = themex;
	yp = themey;

	AddObject(universe, OBJ_SKFIRE, xp, yp);

	if (random_(universe, 0, monstrnd[universe.leveltype - 1]) != 0) {
		i = PreSpawnSkeleton(universe);
		SpawnSkeleton(universe, i, xp - 1, yp - 1);
	} else {
		AddObject(universe, OBJ_BANNERL, xp - 1, yp - 1);
	}

	i = PreSpawnSkeleton(universe);
	SpawnSkeleton(universe, i, xp, yp - 1);

	if (random_(universe, 0, monstrnd[universe.leveltype - 1]) != 0) {
		i = PreSpawnSkeleton(universe);
		SpawnSkeleton(universe, i, xp + 1, yp - 1);
	} else {
		AddObject(universe, OBJ_BANNERR, xp + 1, yp - 1);
	}
	if (random_(universe, 0, monstrnd[universe.leveltype - 1]) != 0) {
		i = PreSpawnSkeleton(universe);
		SpawnSkeleton(universe, i, xp - 1, yp);
	} else {
		AddObject(universe, OBJ_BANNERM, xp - 1, yp);
	}
	if (random_(universe, 0, monstrnd[universe.leveltype - 1]) != 0) {
		i = PreSpawnSkeleton(universe);
		SpawnSkeleton(universe, i, xp + 1, yp);
	} else {
		AddObject(universe, OBJ_BANNERM, xp + 1, yp);
	}
	if (random_(universe, 0, monstrnd[universe.leveltype - 1]) != 0) {
		i = PreSpawnSkeleton(universe);
		SpawnSkeleton(universe, i, xp - 1, yp + 1);
	} else {
		AddObject(universe, OBJ_BANNERR, xp - 1, yp + 1);
	}

	i = PreSpawnSkeleton(universe);
	SpawnSkeleton(universe, i, xp, yp + 1);

	if (random_(universe, 0, monstrnd[universe.leveltype - 1]) != 0) {
		i = PreSpawnSkeleton(universe);
		SpawnSkeleton(universe, i, xp + 1, yp + 1);
	} else {
		AddObject(universe, OBJ_BANNERL, xp + 1, yp + 1);
	}

	if (universe.dObject[xp][yp - 3] == 0) {
		AddObject(universe, OBJ_SKELBOOK, xp, yp - 2);
	}
	if (universe.dObject[xp][yp + 3] == 0) {
		AddObject(universe, OBJ_SKELBOOK, xp, yp + 2);
	}
}

/**
 * Theme_Treasure initializes the treasure theme.
 *
 * @param t theme number (index into universe.themes array).
 */
void Theme_Treasure(Universe& universe, int t)
{
	int xp, yp;
	int i;
	char treasrnd[4] = { 4, 9, 7, 10 };
	char monstrnd[4] = { 6, 8, 3, 7 };

	GetRndSeed(universe);
	for (yp = 0; yp < MAXDUNY; yp++) {
		for (xp = 0; xp < MAXDUNX; xp++) {
			if (universe.dTransVal[xp][yp] == universe.themes[t].ttval && !universe.nSolidTable[universe.dPiece[xp][yp]]) {
				int rv = random_(universe, 0, treasrnd[universe.leveltype - 1]);
				// BUGFIX: the `2*` in `2*random_(universe, 0, treasrnd...) == 0` has no effect, should probably be `random_(universe, 0, 2*treasrnd...) == 0`
				if ((2 * random_(universe, 0, treasrnd[universe.leveltype - 1])) == 0) {
					CreateTypeItem(universe, xp, yp, FALSE, ITYPE_GOLD, IMISC_NONE, FALSE, TRUE);
					ItemNoFlippy(universe);
				}
				if (rv == 0) {
					CreateRndItem(universe, xp, yp, FALSE, FALSE, TRUE);
					ItemNoFlippy(universe);
				}
				// BUGFIX: the following code is likely not working as intended.
				//
				//    `rv == 0` has no effect.
				//
				//    `rv >= treasrnd[universe.leveltype - 1] - 2` is not connected to either
				//    of the item creation branches above, thus the last (unrelated)
				//    item spawned/dropped on ground would be halved in value.
				if (rv == 0 || rv >= treasrnd[universe.leveltype - 1] - 2) {
					i = ItemNoFlippy(universe);
					if (rv >= treasrnd[universe.leveltype - 1] - 2 && universe.leveltype != DTYPE_CATHEDRAL) {
						universe.item[i]._ivalue >>= 1;
					}
				}
			}
		}
	}
	PlaceThemeMonsts(universe, t, monstrnd[universe.leveltype - 1]);
}

/**
 * Theme_Library initializes the library theme.
 *
 * @param t theme number (index into universe.themes array).
 */
void Theme_Library(Universe& universe, int t)
{
	int xp, yp, oi;
	char librnd[4] = { 1, 2, 2, 5 };
	char monstrnd[4] = { 5, 7, 3, 9 };

	TFit_Shrine(universe, t);

	if (themeVar1 == 1) {
		AddObject(universe, OBJ_BOOKCANDLE, themex - 1, themey);
		AddObject(universe, OBJ_BOOKCASER, themex, themey);
		AddObject(universe, OBJ_BOOKCANDLE, themex + 1, themey);
	} else {
		AddObject(universe, OBJ_BOOKCANDLE, themex, themey - 1);
		AddObject(universe, OBJ_BOOKCASEL, themex, themey);
		AddObject(universe, OBJ_BOOKCANDLE, themex, themey + 1);
	}

	for (yp = 1; yp < MAXDUNY - 1; yp++) {
		for (xp = 1; xp < MAXDUNX - 1; xp++) {
			if (CheckThemeObj3(universe, xp, yp, t, -1) && universe.dMonster[xp][yp] == 0 && random_(universe, 0, librnd[universe.leveltype - 1]) == 0) {
				AddObject(universe, OBJ_BOOKSTAND, xp, yp);
				if (random_(universe, 0, 2 * librnd[universe.leveltype - 1]) != 0) { /// BUGFIX: check universe.dObject[xp][yp] was populated by AddObject
					oi = universe.dObject[xp][yp] - 1;
					SetObjectSelFlag(universe, oi, 0);
					IncrementObjectFrame(universe, oi, 2);
				}
			}
		}
	}

	if (QuestStatus(universe, Q_ZHAR)) {
		if (t == universe.zharlib) {
			return;
		}
		PlaceThemeMonsts(universe, t, monstrnd[universe.leveltype]); /// BUGFIX: `universe.leveltype - 1`
	} else {
		PlaceThemeMonsts(universe, t, monstrnd[universe.leveltype]); /// BUGFIX: `universe.leveltype - 1`
	}
}

/**
 * Theme_Torture initializes the torture theme.
 *
 * @param t theme number (index into universe.themes array).
 */
void Theme_Torture(Universe& universe, int t)
{
	int xp, yp;
	char tortrnd[4] = { 6, 8, 3, 8 };
	char monstrnd[4] = { 6, 8, 3, 9 };

	for (yp = 1; yp < MAXDUNY - 1; yp++) {
		for (xp = 1; xp < MAXDUNX - 1; xp++) {
			if (universe.dTransVal[xp][yp] == universe.themes[t].ttval && !universe.nSolidTable[universe.dPiece[xp][yp]]) {
				if (CheckThemeObj3(universe, xp, yp, t, -1)) {
					if (random_(universe, 0, tortrnd[universe.leveltype - 1]) == 0) {
						AddObject(universe, OBJ_TNUDEM2, xp, yp);
					}
				}
			}
		}
	}
	PlaceThemeMonsts(universe, t, monstrnd[universe.leveltype - 1]);
}

/**
 * Theme_BloodFountain initializes the blood fountain theme.
 * @param t Theme number (index into universe.themes array).
 */
void Theme_BloodFountain(Universe& universe, int t)
{
	char monstrnd[4] = { 6, 8, 3, 9 };

	TFit_Obj5(universe, t);
	AddObject(universe, OBJ_BLOODFTN, themex, themey);
	PlaceThemeMonsts(universe, t, monstrnd[universe.leveltype - 1]);
}

/**
 * Theme_Decap initializes the decapitated theme.
 *
 * @param t theme number (index into universe.themes array).
 */
void Theme_Decap(Universe& universe, int t)
{
	int xp, yp;
	char decaprnd[4] = { 6, 8, 3, 8 };
	char monstrnd[4] = { 6, 8, 3, 9 };

	for (yp = 1; yp < MAXDUNY - 1; yp++) {
		for (xp = 1; xp < MAXDUNX - 1; xp++) {
			if (universe.dTransVal[xp][yp] == universe.themes[t].ttval && !universe.nSolidTable[universe.dPiece[xp][yp]]) {
				if (CheckThemeObj3(universe, xp, yp, t, -1)) {
					if (random_(universe, 0, decaprnd[universe.leveltype - 1]) == 0) {
						AddObject(universe, OBJ_DECAP, xp, yp);
					}
				}
			}
		}
	}
	PlaceThemeMonsts(universe, t, monstrnd[universe.leveltype - 1]);
}

/**
 * Theme_PurifyingFountain initializes the purifying fountain theme.
 *
 * @param t theme number (index into universe.themes array).
 */
void Theme_PurifyingFountain(Universe& universe, int t)
{
	char monstrnd[4] = { 6, 7, 3, 9 };

	TFit_Obj5(universe, t);
	AddObject(universe, OBJ_PURIFYINGFTN, themex, themey);
	PlaceThemeMonsts(universe, t, monstrnd[universe.leveltype - 1]);
}

/**
 * Theme_ArmorStand initializes the armor stand theme.
 *
 * @param t theme number (index into universe.themes array).
 */
void Theme_ArmorStand(Universe& universe, int t)
{
	int xp, yp;
	char armorrnd[4] = { 6, 8, 3, 8 };
	char monstrnd[4] = { 6, 7, 3, 9 };

	if (universe.armorFlag) {
		TFit_Obj3(universe, t);
		AddObject(universe, OBJ_ARMORSTAND, themex, themey);
	}
	for (yp = 0; yp < MAXDUNY; yp++) {
		for (xp = 0; xp < MAXDUNX; xp++) {
			if (universe.dTransVal[xp][yp] == universe.themes[t].ttval && !universe.nSolidTable[universe.dPiece[xp][yp]]) {
				if (CheckThemeObj3(universe, xp, yp, t, -1)) {
					if (random_(universe, 0, armorrnd[universe.leveltype - 1]) == 0) {
						AddObject(universe, OBJ_ARMORSTANDN, xp, yp);
					}
				}
			}
		}
	}
	PlaceThemeMonsts(universe, t, monstrnd[universe.leveltype - 1]);
	universe.armorFlag = FALSE;
}

/**
 * Theme_GoatShrine initializes the goat shrine theme.
 *
 * @param t theme number (index into universe.themes array).
 */
void Theme_GoatShrine(Universe& universe, int t)
{
	int xx, yy;

	TFit_GoatShrine(universe, t);
	AddObject(universe, OBJ_GOATSHRINE, themex, themey);
	for (yy = themey - 1; yy <= themey + 1; yy++) {
		for (xx = themex - 1; xx <= themex + 1; xx++) {
			if (universe.dTransVal[xx][yy] == universe.themes[t].ttval && !universe.nSolidTable[universe.dPiece[xx][yy]] && (xx != themex || yy != themey)) {
				AddMonster(universe, xx, yy, DIR_SW, themeVar1, TRUE);
			}
		}
	}
}

/**
 * Theme_Cauldron initializes the cauldron theme.
 *
 * @param t theme number (index into universe.themes array).
 */
void Theme_Cauldron(Universe& universe, int t)
{
	char monstrnd[4] = { 6, 7, 3, 9 };

	TFit_Obj5(universe, t);
	AddObject(universe, OBJ_CAULDRON, themex, themey);
	PlaceThemeMonsts(universe, t, monstrnd[universe.leveltype - 1]);
}

/**
 * Theme_MurkyFountain initializes the murky fountain theme.
 *
 * @param t theme number (index into universe.themes array).
 */
void Theme_MurkyFountain(Universe& universe, int t)
{
	char monstrnd[4] = { 6, 7, 3, 9 };

	TFit_Obj5(universe, t);
	AddObject(universe, OBJ_MURKYFTN, themex, themey);
	PlaceThemeMonsts(universe, t, monstrnd[universe.leveltype - 1]);
}

/**
 * Theme_TearFountain initializes the tear fountain theme.
 *
 * @param t theme number (index into universe.themes array).
 */
void Theme_TearFountain(Universe& universe, int t)
{
	char monstrnd[4] = { 6, 7, 3, 9 };

	TFit_Obj5(universe, t);
	AddObject(universe, OBJ_TEARFTN, themex, themey);
	PlaceThemeMonsts(universe, t, monstrnd[universe.leveltype - 1]);
}

/**
 * Theme_BrnCross initializes the burning cross theme.
 *
 * @param t theme number (index into universe.themes array).
 */
void Theme_BrnCross(Universe& universe, int t)
{
	int xp, yp;
	char monstrnd[4] = { 6, 8, 3, 9 };
	char bcrossrnd[4] = { 5, 7, 3, 8 };

	for (yp = 0; yp < MAXDUNY; yp++) {
		for (xp = 0; xp < MAXDUNX; xp++) {
			if (universe.dTransVal[xp][yp] == universe.themes[t].ttval && !universe.nSolidTable[universe.dPiece[xp][yp]]) {
				if (CheckThemeObj3(universe, xp, yp, t, -1)) {
					if (random_(universe, 0, bcrossrnd[universe.leveltype - 1]) == 0) {
						AddObject(universe, OBJ_TBCROSS, xp, yp);
					}
				}
			}
		}
	}
	PlaceThemeMonsts(universe, t, monstrnd[universe.leveltype - 1]);
	bCrossFlag = TRUE;
}

/**
 * Theme_WeaponRack initializes the weapon rack theme.
 *
 * @param t theme number (index into universe.themes array).
 */
void Theme_WeaponRack(Universe& universe, int t)
{
	int xp, yp;
	char weaponrnd[4] = { 6, 8, 5, 8 };
	char monstrnd[4] = { 6, 7, 3, 9 };

	if (universe.weaponFlag) {
		TFit_Obj3(universe, t);
		AddObject(universe, OBJ_WEAPONRACK, themex, themey);
	}
	for (yp = 0; yp < MAXDUNY; yp++) {
		for (xp = 0; xp < MAXDUNX; xp++) {
			if (universe.dTransVal[xp][yp] == universe.themes[t].ttval && !universe.nSolidTable[universe.dPiece[xp][yp]]) {
				if (CheckThemeObj3(universe, xp, yp, t, -1)) {
					if (random_(universe, 0, weaponrnd[universe.leveltype - 1]) == 0) {
						AddObject(universe, OBJ_WEAPONRACKN, xp, yp);
					}
				}
			}
		}
	}
	PlaceThemeMonsts(universe, t, monstrnd[universe.leveltype - 1]);
	universe.weaponFlag = FALSE;
}

/**
 * UpdateL4Trans sets each value of the transparency map to 1.
 */
void UpdateL4Trans(Universe& universe)
{
	int i, j;

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (universe.dTransVal[i][j] != 0) {
				universe.dTransVal[i][j] = 1;
			}
		}
	}
}

/**
 * CreateThemeRooms adds thematic elements to rooms.
 */
void CreateThemeRooms(Universe& universe)
{
	int i;

	if (universe.currlevel == 16) {
		return;
	}
	universe.InitObjFlag = TRUE;
	for (i = 0; i < universe.numthemes; i++) {
		themex = 0;
		themey = 0;
		switch (universe.themes[i].ttype) {
		case THEME_BARREL:
			Theme_Barrel(universe, i);
			break;
		case THEME_SHRINE:
			Theme_Shrine(universe, i);
			break;
		case THEME_MONSTPIT:
			Theme_MonstPit(universe, i);
			break;
		case THEME_SKELROOM:
			Theme_SkelRoom(universe, i);
			break;
		case THEME_TREASURE:
			Theme_Treasure(universe, i);
			break;
		case THEME_LIBRARY:
			Theme_Library(universe, i);
			break;
		case THEME_TORTURE:
			Theme_Torture(universe, i);
			break;
		case THEME_BLOODFOUNTAIN:
			Theme_BloodFountain(universe, i);
			break;
		case THEME_DECAPITATED:
			Theme_Decap(universe, i);
			break;
		case THEME_PURIFYINGFOUNTAIN:
			Theme_PurifyingFountain(universe, i);
			break;
		case THEME_ARMORSTAND:
			Theme_ArmorStand(universe, i);
			break;
		case THEME_GOATSHRINE:
			Theme_GoatShrine(universe, i);
			break;
		case THEME_CAULDRON:
			Theme_Cauldron(universe, i);
			break;
		case THEME_MURKYFOUNTAIN:
			Theme_MurkyFountain(universe, i);
			break;
		case THEME_TEARFOUNTAIN:
			Theme_TearFountain(universe, i);
			break;
		case THEME_BRNCROSS:
			Theme_BrnCross(universe, i);
			break;
		case THEME_WEAPONRACK:
			Theme_WeaponRack(universe, i);
			break;
		}
	}
	universe.InitObjFlag = FALSE;
	if (universe.leveltype == DTYPE_HELL && universe.themeCount > 0) {
		UpdateL4Trans(universe);
	}
}
