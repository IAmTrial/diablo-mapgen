/**
 * @file items.cpp
 *
 * Implementation of item functionality.
 */

#include "Source/items.h"

#include "defs.h"
#include "structs.h"
#include "types.h"

#include "Source/engine.h"
#include "Source/gendung.h"
#include "Source/itemdat.h"
#include "Source/monster.h"
#include "Source/quests.h"
#include "Source/spelldat.h"

#ifdef HELLFIRE
#include "../3rdParty/Storm/Source/storm.h"
#endif

int itemactive[MAXITEMS];
BOOL uitemflag;
int itemavail[MAXITEMS];
ItemStruct curruitem;
ItemGetRecordStruct itemrecord[MAXITEMS];
/** Contains the items on ground in the current game. */
ItemStruct item[MAXITEMS + 1];
BOOL itemhold[3][3];
#ifdef HELLFIRE
CornerStoneStruct CornerStone;
#endif
BYTE *itemanims[ITEMTYPES];
BOOL UniqueItemFlag[128];
#ifdef HELLFIRE
int auricGold = GOLD_MAX_LIMIT * 2;
#endif
int numitems;
int gnNumGetRecords;
ItemStruct golditem;

/* data */

#ifdef HELLFIRE
const int OilLevels[] = { 1, 10, 1, 10, 4, 1, 5, 17, 1, 10 };
const int OilValues[] = { 500, 2500, 500, 2500, 1500, 100, 2500, 15000, 500, 2500 };
const int OilMagic[] = {
	IMISC_OILACC,
	IMISC_OILMAST,
	IMISC_OILSHARP,
	IMISC_OILDEATH,
	IMISC_OILSKILL,
	IMISC_OILBSMTH,
	IMISC_OILFORT,
	IMISC_OILPERM,
	IMISC_OILHARD,
	IMISC_OILIMP,
};
const char OilNames[10][25] = {
	"Oil of Accuracy",
	"Oil of Mastery",
	"Oil of Sharpness",
	"Oil of Death",
	"Oil of Skill",
	"Blacksmith Oil",
	"Oil of Fortitude",
	"Oil of Permanence",
	"Oil of Hardening",
	"Oil of Imperviousness"
};
const int MaxGold = GOLD_MAX_LIMIT;
#endif

/** Maps from item_cursor_graphic to in-memory item type. */
const BYTE ItemCAnimTbl[] = {
#ifndef HELLFIRE
	20, 16, 16, 16, 4, 4, 4, 12, 12, 12,
	12, 12, 12, 12, 12, 21, 21, 25, 12, 28,
	28, 28, 0, 0, 0, 32, 0, 0, 0, 24,
	24, 26, 2, 25, 22, 23, 24, 25, 27, 27,
	29, 0, 0, 0, 12, 12, 12, 12, 12, 0,
	8, 8, 0, 8, 8, 8, 8, 8, 8, 6,
	8, 8, 8, 6, 8, 8, 6, 8, 8, 6,
	6, 6, 8, 8, 8, 5, 9, 13, 13, 13,
	5, 5, 5, 15, 5, 5, 18, 18, 18, 30,
	5, 5, 14, 5, 14, 13, 16, 18, 5, 5,
	7, 1, 3, 17, 1, 15, 10, 14, 3, 11,
	8, 0, 1, 7, 0, 7, 15, 7, 3, 3,
	3, 6, 6, 11, 11, 11, 31, 14, 14, 14,
	6, 6, 7, 3, 8, 14, 0, 14, 14, 0,
	33, 1, 1, 1, 1, 1, 7, 7, 7, 14,
	14, 17, 17, 17, 0, 34, 1, 0, 3, 17,
	8, 8, 6, 1, 3, 3, 11, 3, 4
#else
	20, 16, 16, 16, 4, 4, 4, 12, 12, 12,
	12, 12, 12, 12, 12, 21, 21, 25, 12, 28,
	28, 28, 38, 38, 38, 32, 38, 38, 38, 24,
	24, 26, 2, 25, 22, 23, 24, 25, 27, 27,
	29, 0, 0, 0, 12, 12, 12, 12, 12, 0,
	8, 8, 0, 8, 8, 8, 8, 8, 8, 6,
	8, 8, 8, 6, 8, 8, 6, 8, 8, 6,
	6, 6, 8, 8, 8, 5, 9, 13, 13, 13,
	5, 5, 5, 15, 5, 5, 18, 18, 18, 30,
	5, 5, 14, 5, 14, 13, 16, 18, 5, 5,
	7, 1, 3, 17, 1, 15, 10, 14, 3, 11,
	8, 0, 1, 7, 0, 7, 15, 7, 3, 3,
	3, 6, 6, 11, 11, 11, 31, 14, 14, 14,
	6, 6, 7, 3, 8, 14, 0, 14, 14, 0,
	33, 1, 1, 1, 1, 1, 7, 7, 7, 14,
	14, 17, 17, 17, 0, 34, 1, 0, 3, 17,
	8, 8, 6, 1, 3, 3, 11, 3, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 35, 39, 36,
	36, 36, 37, 38, 38, 38, 38, 38, 41, 42,
	8, 8, 8, 17, 0, 6, 8, 11, 11, 3,
	3, 1, 6, 6, 6, 1, 8, 6, 11, 3,
	6, 8, 1, 6, 6, 17, 40, 0, 0
#endif
};
/** Map of item type .cel file names. */
const char *const ItemDropNames[] = {
	"Armor2",
	"Axe",
	"FBttle",
	"Bow",
	"GoldFlip",
	"Helmut",
	"Mace",
	"Shield",
	"SwrdFlip",
	"Rock",
	"Cleaver",
	"Staff",
	"Ring",
	"CrownF",
	"LArmor",
	"WShield",
	"Scroll",
	"FPlateAr",
	"FBook",
	"Food",
	"FBttleBB",
	"FBttleDY",
	"FBttleOR",
	"FBttleBR",
	"FBttleBL",
	"FBttleBY",
	"FBttleWH",
	"FBttleDB",
	"FEar",
	"FBrain",
	"FMush",
	"Innsign",
	"Bldstn",
	"Fanvil",
	"FLazStaf",
#ifdef HELLFIRE
	"bombs1",
	"halfps1",
	"wholeps1",
	"runes1",
	"teddys1",
	"cows1",
	"donkys1",
	"mooses1",
#endif
};
/** Maps of item drop animation length. */
const BYTE ItemAnimLs[] = {
	15,
	13,
	16,
	13,
	10,
	13,
	13,
	13,
	13,
	10,
	13,
	13,
	13,
	13,
	13,
	13,
	13,
	13,
	13,
	1,
	16,
	16,
	16,
	16,
	16,
	16,
	16,
	16,
	13,
	12,
	12,
	13,
	13,
	13,
	8,
#ifdef HELLFIRE
	10,
	16,
	16,
	10,
	10,
	15,
	15,
	15,
#endif
};
/** Maps of drop sounds effect of dropping the item on ground. */
const int ItemDropSnds[] = {
	IS_FHARM,
	IS_FAXE,
	IS_FPOT,
	IS_FBOW,
	IS_GOLD,
	IS_FCAP,
	IS_FSWOR,
	IS_FSHLD,
	IS_FSWOR,
	IS_FROCK,
	IS_FAXE,
	IS_FSTAF,
	IS_FRING,
	IS_FCAP,
	IS_FLARM,
	IS_FSHLD,
	IS_FSCRL,
	IS_FHARM,
	IS_FBOOK,
	IS_FLARM,
	IS_FPOT,
	IS_FPOT,
	IS_FPOT,
	IS_FPOT,
	IS_FPOT,
	IS_FPOT,
	IS_FPOT,
	IS_FPOT,
	IS_FBODY,
	IS_FBODY,
	IS_FMUSH,
	IS_ISIGN,
	IS_FBLST,
	IS_FANVL,
	IS_FSTAF,
#ifdef HELLFIRE
	IS_FROCK,
	IS_FSCRL,
	IS_FSCRL,
	IS_FROCK,
	IS_FMUSH,
	IS_FHARM,
	IS_FLARM,
	IS_FLARM,
#endif
};
/** Maps of drop sounds effect of placing the item in the inventory. */
const int ItemInvSnds[] = {
	IS_IHARM,
	IS_IAXE,
	IS_IPOT,
	IS_IBOW,
	IS_GOLD,
	IS_ICAP,
	IS_ISWORD,
	IS_ISHIEL,
	IS_ISWORD,
	IS_IROCK,
	IS_IAXE,
	IS_ISTAF,
	IS_IRING,
	IS_ICAP,
	IS_ILARM,
	IS_ISHIEL,
	IS_ISCROL,
	IS_IHARM,
	IS_IBOOK,
	IS_IHARM,
	IS_IPOT,
	IS_IPOT,
	IS_IPOT,
	IS_IPOT,
	IS_IPOT,
	IS_IPOT,
	IS_IPOT,
	IS_IPOT,
	IS_IBODY,
	IS_IBODY,
	IS_IMUSH,
	IS_ISIGN,
	IS_IBLST,
	IS_IANVL,
	IS_ISTAF,
#ifdef HELLFIRE
	IS_IROCK,
	IS_ISCROL,
	IS_ISCROL,
	IS_IROCK,
	IS_IMUSH,
	IS_IHARM,
	IS_ILARM,
	IS_ILARM,
#endif
};
#ifdef HELLFIRE
char *CornerStoneRegKey = "SItem";
#endif
/** Specifies the current Y-coordinate used for validation of items on ground. */
const int idoppely = 16;
/** Maps from Griswold premium item number to a quality level delta as added to the base quality level. */
const int premiumlvladd[SMITH_PREMIUM_ITEMS] = {
	// clang-format off
	-1,
	-1,
#ifdef HELLFIRE
	-1,
#endif
	 0,
	 0,
#ifdef HELLFIRE
	 0,
	 0,
	 1,
	 1,
	 1,
#endif
	 1,
	 2,
#ifdef HELLFIRE
	 2,
	 3,
	 3,
#endif
	// clang-format on
};

BOOL ItemPlace(Universe& universe, int xp, int yp)
{
	if (universe.dMonster[xp][yp] != 0)
		return FALSE;
	if (universe.dPlayer[xp][yp] != 0)
		return FALSE;
	if (universe.dItem[xp][yp] != 0)
		return FALSE;
	if (universe.dObject[xp][yp] != 0)
		return FALSE;
	if (universe.dFlags[xp][yp] & BFLAG_POPULATED)
		return FALSE;
	if (universe.nSolidTable[universe.dPiece[xp][yp]])
		return FALSE;

	return TRUE;
}

void AddInitItems(Universe& universe)
{
	int x, y, i, j, rnd;

#ifdef HELLFIRE
	int curlv = items_get_currlevel();
#endif
	rnd = random_(universe, 11, 3) + 3;
	for (j = 0; j < rnd; j++) {
		i = itemavail[0];
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = i;
		x = random_(universe, 12, 80) + 16;
		y = random_(universe, 12, 80) + 16;
		while (!ItemPlace(universe, x, y)) {
			x = random_(universe, 12, 80) + 16;
			y = random_(universe, 12, 80) + 16;
		}
		item[i]._ix = x;
		item[i]._iy = y;
		universe.dItem[x][y] = i + 1;
		item[i]._iSeed = GetRndSeed(universe);
		SetRndSeed(universe, item[i]._iSeed);
#ifdef HELLFIRE
		if (random_(universe, 12, 2) != 0)
			GetItemAttrs(universe, i, IDI_HEAL, curlv);
		else
			GetItemAttrs(universe, i, IDI_MANA, curlv);
		item[i]._iCreateInfo = curlv + CF_PREGEN;
#else
		if (random_(universe, 12, 2) != 0)
			GetItemAttrs(universe, i, IDI_HEAL, universe.currlevel);
		else
			GetItemAttrs(universe, i, IDI_MANA, universe.currlevel);
		item[i]._iCreateInfo = universe.currlevel + CF_PREGEN;
#endif
		SetupItem(universe, i);
		item[i]._iAnimFrame = item[i]._iAnimLen;
		item[i]._iAnimFlag = FALSE;
		item[i]._iSelFlag = 1;
		numitems++;
	}
}

void InitItems(Universe& universe)
{
	int i;
	long s;

	GetItemAttrs(universe, 0, IDI_GOLD, 1);
	golditem = item[0];
	golditem._iStatFlag = TRUE;
	numitems = 0;

	for (i = 0; i < MAXITEMS; i++) {
		item[i]._itype = ITYPE_MISC; // BUGFIX Should be ITYPE_NONE
		item[i]._ix = 0;
		item[i]._iy = 0;
		item[i]._iAnimFlag = FALSE;
		item[i]._iSelFlag = 0;
		item[i]._iIdentified = FALSE;
		item[i]._iPostDraw = FALSE;
	}

	for (i = 0; i < MAXITEMS; i++) {
		itemavail[i] = i;
		itemactive[i] = 0;
	}

	if (!universe.setlevel) {
		s = GetRndSeed(universe); /* unused */
		if (QuestStatus(universe, Q_ROCK))
			SpawnRock(universe);
		if (QuestStatus(universe, Q_ANVIL))
			SpawnQuestItem(universe, IDI_ANVIL, 2 * universe.setpc_x + 27, 2 * universe.setpc_y + 27, 0, 1);
#ifdef HELLFIRE
		if (UseCowFarmer && universe.currlevel == 20)
			SpawnQuestItem(universe, IDI_BROWNSUIT, 25, 25, 3, 1);
		if (UseCowFarmer && universe.currlevel == 19)
			SpawnQuestItem(universe, IDI_GREYSUIT, 25, 25, 3, 1);
#endif
		if (universe.currlevel > 0 && universe.currlevel < 16)
			AddInitItems(universe);
#ifdef HELLFIRE
		if (universe.currlevel >= 21 && universe.currlevel <= 23)
			SpawnNote();
#endif
	}

	uitemflag = FALSE;

	// BUGFIX: item get records not reset when resetting items.
}

void GetPlrHandSeed(Universe& universe, ItemStruct *h)
{
	h->_iSeed = GetRndSeed(universe);
}

void GetGoldSeed(Universe& universe, int pnum, ItemStruct *h)
{
	int i, ii, s;
	BOOL doneflag;

	do {
		doneflag = TRUE;
		s = GetRndSeed(universe);
		for (i = 0; i < numitems; i++) {
			ii = itemactive[i];
			if (item[ii]._iSeed == s)
				doneflag = FALSE;
		}
		if (pnum == myplr) {
			for (i = 0; i < universe.plr[pnum]._pNumInv; i++) {
				if (universe.plr[pnum].InvList[i]._iSeed == s)
					doneflag = FALSE;
			}
		}
	} while (!doneflag);

	h->_iSeed = s;
}

void SetPlrHandSeed(ItemStruct *h, int iseed)
{
	h->_iSeed = iseed;
}

void SetPlrHandGoldCurs(ItemStruct *h)
{
	if (h->_ivalue >= GOLD_MEDIUM_LIMIT)
		h->_iCurs = ICURS_GOLD_LARGE;
	else if (h->_ivalue <= GOLD_SMALL_LIMIT)
		h->_iCurs = ICURS_GOLD_SMALL;
	else
		h->_iCurs = ICURS_GOLD_MEDIUM;
}

BOOL ItemSpaceOk(Universe& universe, int i, int j)
{
	int oi;

	if (i < 0 || i >= MAXDUNX || j < 0 || j >= MAXDUNY)
		return FALSE;

	if (universe.dMonster[i][j] != 0)
		return FALSE;

	if (universe.dPlayer[i][j] != 0)
		return FALSE;

	if (universe.dItem[i][j] != 0)
		return FALSE;

	if (universe.dObject[i][j] != 0) {
		oi = universe.dObject[i][j] > 0 ? universe.dObject[i][j] - 1 : -(universe.dObject[i][j] + 1);
		if (object[oi]._oSolidFlag)
			return FALSE;
	}

	if (universe.dObject[i + 1][j + 1] > 0 && object[universe.dObject[i + 1][j + 1] - 1]._oSelFlag != 0) /// BUGFIX: check for universe.dObject OOB
		return FALSE;

	if (universe.dObject[i + 1][j + 1] < 0 && object[-(universe.dObject[i + 1][j + 1] + 1)]._oSelFlag != 0) /// BUGFIX: check for universe.dObject OOB
		return FALSE;

	if (universe.dObject[i + 1][j] > 0 /// BUGFIX: check for universe.dObject OOB
	    && universe.dObject[i][j + 1] > 0 /// BUGFIX: check for universe.dObject OOB
	    && object[universe.dObject[i + 1][j] - 1]._oSelFlag != 0
	    && object[universe.dObject[i][j + 1] - 1]._oSelFlag != 0) {
		return FALSE;
	}

	return !universe.nSolidTable[universe.dPiece[i][j]];
}

BOOL GetItemSpace(Universe& universe, int x, int y, char inum)
{
	int i, j, rs;
	int xx, yy;
	BOOL savail;

	yy = 0;
	for (j = y - 1; j <= y + 1; j++) {
		xx = 0;
		for (i = x - 1; i <= x + 1; i++) {
			itemhold[xx][yy] = ItemSpaceOk(universe, i, j);
			xx++;
		}
		yy++;
	}

	savail = FALSE;
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 3; i++) {
			if (itemhold[i][j])
				savail = TRUE;
		}
	}

	rs = random_(universe, 13, 15) + 1;

	if (!savail)
		return FALSE;

	xx = 0;
	yy = 0;
	while (rs > 0) {
		if (itemhold[xx][yy])
			rs--;
		if (rs > 0) {
			xx++;
			if (xx == 3) {
				xx = 0;
				yy++;
				if (yy == 3)
					yy = 0;
			}
		}
	}

	xx += x - 1;
	yy += y - 1;
	item[inum]._ix = xx;
	item[inum]._iy = yy;
	universe.dItem[xx][yy] = inum + 1;

	return TRUE;
}

void GetSuperItemSpace(Universe& universe, int x, int y, char inum)
{
	int xx, yy;
	int i, j, k;

	if (!GetItemSpace(universe, x, y, inum)) {
		for (k = 2; k < 50; k++) {
			for (j = -k; j <= k; j++) {
				yy = y + j;
				for (i = -k; i <= k; i++) {
					xx = i + x;
					if (ItemSpaceOk(universe, xx, yy)) {
						item[inum]._ix = xx;
						item[inum]._iy = yy;
						universe.dItem[xx][yy] = inum + 1;
						return;
					}
				}
			}
		}
	}
}

void GetSuperItemLoc(Universe& universe, int x, int y, int &xx, int &yy)
{
	int i, j, k;

	for (k = 1; k < 50; k++) {
		for (j = -k; j <= k; j++) {
			yy = y + j;
			for (i = -k; i <= k; i++) {
				xx = i + x;
				if (ItemSpaceOk(universe, xx, yy)) {
					return;
				}
			}
		}
	}
}

void CalcItemValue(int i)
{
	int v;

	v = item[i]._iVMult1 + item[i]._iVMult2;
	if (v > 0) {
		v *= item[i]._ivalue;
	}
	if (v < 0) {
		v = item[i]._ivalue / v;
	}
	v = item[i]._iVAdd1 + item[i]._iVAdd2 + v;
	if (v <= 0) {
		v = 1;
	}
	item[i]._iIvalue = v;
}

void GetBookSpell(Universe& universe, int i, int lvl)
{
	int rv, s, bs;

	if (lvl == 0)
		lvl = 1;
	rv = random_(universe, 14, MAX_SPELLS) + 1;
#ifdef SPAWN
	if (lvl > 5)
		lvl = 5;
#endif
	s = SPL_FIREBOLT;
#ifdef HELLFIRE
	bs = SPL_FIREBOLT;
#endif
	while (rv > 0) {
		if (spelldata[s].sBookLvl != -1 && lvl >= spelldata[s].sBookLvl) {
			rv--;
			bs = s;
		}
		s++;
		if (universe.gbMaxPlayers == 1) {
			if (s == SPL_RESURRECT)
				s = SPL_TELEKINESIS;
		}
		if (universe.gbMaxPlayers == 1) {
			if (s == SPL_HEALOTHER)
				s = SPL_FLARE;
		}
		if (s == MAX_SPELLS)
			s = 1;
	}
	strcat(item[i]._iName, spelldata[bs].sNameText);
	strcat(item[i]._iIName, spelldata[bs].sNameText);
	item[i]._iSpell = bs;
	item[i]._iMinMag = spelldata[bs].sMinInt;
	item[i]._ivalue += spelldata[bs].sBookCost;
	item[i]._iIvalue += spelldata[bs].sBookCost;
	if (spelldata[bs].sType == STYPE_FIRE)
		item[i]._iCurs = ICURS_BOOK_RED;
#ifdef HELLFIRE
	else
#endif
	    if (spelldata[bs].sType == STYPE_LIGHTNING)
		item[i]._iCurs = ICURS_BOOK_BLUE;
#ifdef HELLFIRE
	else
#endif
	    if (spelldata[bs].sType == STYPE_MAGIC)
		item[i]._iCurs = ICURS_BOOK_GREY;
}

void GetStaffPower(Universe& universe, int i, int lvl, int bs, BOOL onlygood)
{
	int l[256];
	char istr[128];
	int nl, j, preidx;
	BOOL addok;
	int tmp;

	tmp = random_(universe, 15, 10);
	preidx = -1;
	if (tmp == 0 || onlygood) {
		nl = 0;
		for (j = 0; PL_Prefix[j].PLPower != IPL_INVALID; j++) {
			if (PL_Prefix[j].PLIType & PLT_STAFF && PL_Prefix[j].PLMinLvl <= lvl) {
				addok = TRUE;
				if (onlygood && !PL_Prefix[j].PLOk)
					addok = FALSE;
				if (addok) {
					l[nl] = j;
					nl++;
					if (PL_Prefix[j].PLDouble) {
						l[nl] = j;
						nl++;
					}
				}
			}
		}
		if (nl != 0) {
			preidx = l[random_(universe, 16, nl)];
			sprintf(istr, "%s %s", PL_Prefix[preidx].PLName, item[i]._iIName);
			strcpy(item[i]._iIName, istr);
			item[i]._iMagical = ITEM_QUALITY_MAGIC;
			SaveItemPower(
			    universe,
			    i,
			    PL_Prefix[preidx].PLPower,
			    PL_Prefix[preidx].PLParam1,
			    PL_Prefix[preidx].PLParam2,
			    PL_Prefix[preidx].PLMinVal,
			    PL_Prefix[preidx].PLMaxVal,
			    PL_Prefix[preidx].PLMultVal);
			item[i]._iPrePower = PL_Prefix[preidx].PLPower;
		}
	}
	CalcItemValue(i);
}

void GetStaffSpell(Universe& universe, int i, int lvl, BOOL onlygood)
{
	int l, rv, s, minc, maxc, v, bs;
	char istr[64];

#ifndef HELLFIRE
	if (random_(universe, 17, 4) == 0) {
		GetItemPower(universe, i, lvl >> 1, lvl, PLT_STAFF, onlygood);
	} else
#endif
	{
		l = lvl >> 1;
		if (l == 0)
			l = 1;
		rv = random_(universe, 18, MAX_SPELLS) + 1;
#ifdef SPAWN
		if (lvl > 10)
			lvl = 10;
#endif
		s = SPL_FIREBOLT;
		while (rv > 0) {
			if (spelldata[s].sStaffLvl != -1 && l >= spelldata[s].sStaffLvl) {
				rv--;
				bs = s;
			}
			s++;
			if (universe.gbMaxPlayers == 1 && s == SPL_RESURRECT)
				s = SPL_TELEKINESIS;
			if (universe.gbMaxPlayers == 1 && s == SPL_HEALOTHER)
				s = SPL_FLARE;
			if (s == MAX_SPELLS)
				s = SPL_FIREBOLT;
		}
		sprintf(istr, "%s of %s", item[i]._iName, spelldata[bs].sNameText);
		strcpy(item[i]._iName, istr);
		strcpy(item[i]._iIName, istr);

		minc = spelldata[bs].sStaffMin;
		maxc = spelldata[bs].sStaffMax - minc + 1;
		item[i]._iSpell = bs;
		item[i]._iCharges = minc + random_(universe, 19, maxc);
		item[i]._iMaxCharges = item[i]._iCharges;

		item[i]._iMinMag = spelldata[bs].sMinInt;
		v = item[i]._iCharges * spelldata[bs].sStaffCost / 5;
		item[i]._ivalue += v;
		item[i]._iIvalue += v;
		GetStaffPower(universe, i, lvl, bs, onlygood);
	}
}

void GetItemAttrs(Universe& universe, int i, int idata, int lvl)
{
	int rndv;
#ifdef HELLFIRE
	int itemlevel;
#endif

	item[i]._itype = AllItemsList[idata].itype;
	item[i]._iCurs = AllItemsList[idata].iCurs;
	strcpy(item[i]._iName, AllItemsList[idata].iName);
	strcpy(item[i]._iIName, AllItemsList[idata].iName);
	item[i]._iLoc = AllItemsList[idata].iLoc;
	item[i]._iClass = AllItemsList[idata].iClass;
	item[i]._iMinDam = AllItemsList[idata].iMinDam;
	item[i]._iMaxDam = AllItemsList[idata].iMaxDam;
	item[i]._iAC = AllItemsList[idata].iMinAC + random_(universe, 20, AllItemsList[idata].iMaxAC - AllItemsList[idata].iMinAC + 1);
#ifndef HELLFIRE
	item[i]._iFlags = AllItemsList[idata].iFlags;
#endif
	item[i]._iMiscId = AllItemsList[idata].iMiscId;
	item[i]._iSpell = AllItemsList[idata].iSpell;
	item[i]._iMagical = ITEM_QUALITY_NORMAL;
	item[i]._ivalue = AllItemsList[idata].iValue;
	item[i]._iIvalue = AllItemsList[idata].iValue;
	item[i]._iVAdd1 = 0;
	item[i]._iVMult1 = 0;
	item[i]._iVAdd2 = 0;
	item[i]._iVMult2 = 0;
	item[i]._iPLDam = 0;
	item[i]._iPLToHit = 0;
	item[i]._iPLAC = 0;
	item[i]._iPLStr = 0;
	item[i]._iPLMag = 0;
	item[i]._iPLDex = 0;
	item[i]._iPLVit = 0;
	item[i]._iCharges = 0;
	item[i]._iMaxCharges = 0;
	item[i]._iDurability = AllItemsList[idata].iDurability;
	item[i]._iMaxDur = AllItemsList[idata].iDurability;
	item[i]._iMinStr = AllItemsList[idata].iMinStr;
	item[i]._iMinMag = AllItemsList[idata].iMinMag;
	item[i]._iMinDex = AllItemsList[idata].iMinDex;
	item[i]._iPLFR = 0;
	item[i]._iPLLR = 0;
	item[i]._iPLMR = 0;
	item[i].IDidx = idata;
	item[i]._iPLDamMod = 0;
	item[i]._iPLGetHit = 0;
	item[i]._iPLLight = 0;
	item[i]._iSplLvlAdd = 0;
	item[i]._iRequest = FALSE;
	item[i]._iFMinDam = 0;
	item[i]._iFMaxDam = 0;
	item[i]._iLMinDam = 0;
	item[i]._iLMaxDam = 0;
	item[i]._iPLEnAc = 0;
	item[i]._iPLMana = 0;
	item[i]._iPLHP = 0;
	item[i]._iPrePower = -1;
	item[i]._iSufPower = -1;

#ifndef HELLFIRE
	if (item[i]._iMiscId == IMISC_BOOK)
		GetBookSpell(universe, i, lvl);

	if (item[i]._itype == ITYPE_GOLD) {
		if (universe.gnDifficulty == DIFF_NORMAL)
			rndv = 5 * universe.currlevel + random_(universe, 21, 10 * universe.currlevel);
		if (universe.gnDifficulty == DIFF_NIGHTMARE)
			rndv = 5 * (universe.currlevel + 16) + random_(universe, 21, 10 * (universe.currlevel + 16));
		if (universe.gnDifficulty == DIFF_HELL)
			rndv = 5 * (universe.currlevel + 32) + random_(universe, 21, 10 * (universe.currlevel + 32));
#else
	item[i]._iFlags = 0;
	item[i]._iDamAcFlags = 0;

	if (item[i]._iMiscId == IMISC_BOOK)
		GetBookSpell(universe, i, lvl);

	if (item[i]._iMiscId == IMISC_OILOF)
		GetOilType(i, lvl);

	itemlevel = items_get_currlevel();
	if (item[i]._itype == ITYPE_GOLD) {
		if (universe.gnDifficulty == DIFF_NORMAL)
			rndv = 5 * itemlevel + random_(universe, 21, 10 * itemlevel);
		else if (universe.gnDifficulty == DIFF_NIGHTMARE)
			rndv = 5 * (itemlevel + 16) + random_(universe, 21, 10 * (itemlevel + 16));
		else if (universe.gnDifficulty == DIFF_HELL)
			rndv = 5 * (itemlevel + 32) + random_(universe, 21, 10 * (itemlevel + 32));
#endif
		if (universe.leveltype == DTYPE_HELL)
			rndv += rndv >> 3;
		if (rndv > GOLD_MAX_LIMIT)
			rndv = GOLD_MAX_LIMIT;

		item[i]._ivalue = rndv;

		if (rndv >= GOLD_MEDIUM_LIMIT)
			item[i]._iCurs = ICURS_GOLD_LARGE;
		else
			item[i]._iCurs = (rndv > GOLD_SMALL_LIMIT) + 4;
	}
}

int RndPL(Universe& universe, int param1, int param2)
{
	return param1 + random_(universe, 22, param2 - param1 + 1);
}

int PLVal(int pv, int p1, int p2, int minv, int maxv)
{
	if (p1 == p2)
		return minv;
	if (minv == maxv)
		return minv;
	return minv + (maxv - minv) * (100 * (pv - p1) / (p2 - p1)) / 100;
}

void SaveItemPower(Universe& universe, int i, int power, int param1, int param2, int minval, int maxval, int multval)
{
	int r, r2;

	r = RndPL(universe, param1, param2);
	switch (power) {
	case IPL_TOHIT:
		item[i]._iPLToHit += r;
		break;
	case IPL_TOHIT_CURSE:
		item[i]._iPLToHit -= r;
		break;
	case IPL_DAMP:
		item[i]._iPLDam += r;
		break;
	case IPL_DAMP_CURSE:
		item[i]._iPLDam -= r;
		break;
#ifdef HELLFIRE
	case IPL_DOPPELGANGER:
		item[i]._iDamAcFlags |= ISPLHF_DOPPELGANGER;
		// no break
#endif
	case IPL_TOHIT_DAMP:
		r = RndPL(universe, param1, param2);
		item[i]._iPLDam += r;
		if (param1 == 20)
			r2 = RndPL(universe, 1, 5);
		if (param1 == 36)
			r2 = RndPL(universe, 6, 10);
		if (param1 == 51)
			r2 = RndPL(universe, 11, 15);
		if (param1 == 66)
			r2 = RndPL(universe, 16, 20);
		if (param1 == 81)
			r2 = RndPL(universe, 21, 30);
		if (param1 == 96)
			r2 = RndPL(universe, 31, 40);
		if (param1 == 111)
			r2 = RndPL(universe, 41, 50);
		if (param1 == 126)
			r2 = RndPL(universe, 51, 75);
		if (param1 == 151)
			r2 = RndPL(universe, 76, 100);
		item[i]._iPLToHit += r2;
		break;
	case IPL_TOHIT_DAMP_CURSE:
		item[i]._iPLDam -= r;
		if (param1 == 25)
			r2 = RndPL(universe, 1, 5);
		if (param1 == 50)
			r2 = RndPL(universe, 6, 10);
		item[i]._iPLToHit -= r2;
		break;
	case IPL_ACP:
		item[i]._iPLAC += r;
		break;
	case IPL_ACP_CURSE:
		item[i]._iPLAC -= r;
		break;
	case IPL_SETAC:
		item[i]._iAC = r;
		break;
	case IPL_AC_CURSE:
		item[i]._iAC -= r;
		break;
	case IPL_FIRERES:
		item[i]._iPLFR += r;
		break;
	case IPL_LIGHTRES:
		item[i]._iPLLR += r;
		break;
	case IPL_MAGICRES:
		item[i]._iPLMR += r;
		break;
	case IPL_ALLRES:
		item[i]._iPLFR += r;
		item[i]._iPLLR += r;
		item[i]._iPLMR += r;
		if (item[i]._iPLFR < 0)
			item[i]._iPLFR = 0;
		if (item[i]._iPLLR < 0)
			item[i]._iPLLR = 0;
		if (item[i]._iPLMR < 0)
			item[i]._iPLMR = 0;
		break;
	case IPL_SPLLVLADD:
		item[i]._iSplLvlAdd = r;
		break;
	case IPL_CHARGES:
		item[i]._iCharges *= param1;
		item[i]._iMaxCharges = item[i]._iCharges;
		break;
	case IPL_SPELL:
		item[i]._iSpell = param1;
#ifdef HELLFIRE
		item[i]._iCharges = param2;
#else
		item[i]._iCharges = param1; // BUGFIX: should be param2. This code was correct in v1.04, and the bug was introduced between 1.04 and 1.09b.
#endif
		item[i]._iMaxCharges = param2;
		break;
	case IPL_FIREDAM:
		item[i]._iFlags |= ISPL_FIREDAM;
#ifdef HELLFIRE
		item[i]._iFlags &= ~ISPL_LIGHTDAM;
#endif
		item[i]._iFMinDam = param1;
		item[i]._iFMaxDam = param2;
#ifdef HELLFIRE
		item[i]._iLMinDam = 0;
		item[i]._iLMaxDam = 0;
#endif
		break;
	case IPL_LIGHTDAM:
		item[i]._iFlags |= ISPL_LIGHTDAM;
#ifdef HELLFIRE
		item[i]._iFlags &= ~ISPL_FIREDAM;
#endif
		item[i]._iLMinDam = param1;
		item[i]._iLMaxDam = param2;
#ifdef HELLFIRE
		item[i]._iFMinDam = 0;
		item[i]._iFMaxDam = 0;
#endif
		break;
	case IPL_STR:
		item[i]._iPLStr += r;
		break;
	case IPL_STR_CURSE:
		item[i]._iPLStr -= r;
		break;
	case IPL_MAG:
		item[i]._iPLMag += r;
		break;
	case IPL_MAG_CURSE:
		item[i]._iPLMag -= r;
		break;
	case IPL_DEX:
		item[i]._iPLDex += r;
		break;
	case IPL_DEX_CURSE:
		item[i]._iPLDex -= r;
		break;
	case IPL_VIT:
		item[i]._iPLVit += r;
		break;
	case IPL_VIT_CURSE:
		item[i]._iPLVit -= r;
		break;
	case IPL_ATTRIBS:
		item[i]._iPLStr += r;
		item[i]._iPLMag += r;
		item[i]._iPLDex += r;
		item[i]._iPLVit += r;
		break;
	case IPL_ATTRIBS_CURSE:
		item[i]._iPLStr -= r;
		item[i]._iPLMag -= r;
		item[i]._iPLDex -= r;
		item[i]._iPLVit -= r;
		break;
	case IPL_GETHIT_CURSE:
		item[i]._iPLGetHit += r;
		break;
	case IPL_GETHIT:
		item[i]._iPLGetHit -= r;
		break;
	case IPL_LIFE:
		item[i]._iPLHP += r << 6;
		break;
	case IPL_LIFE_CURSE:
		item[i]._iPLHP -= r << 6;
		break;
	case IPL_MANA:
		item[i]._iPLMana += r << 6;
		break;
	case IPL_MANA_CURSE:
		item[i]._iPLMana -= r << 6;
		break;
	case IPL_DUR:
		r2 = r * item[i]._iMaxDur / 100;
		item[i]._iMaxDur += r2;
		item[i]._iDurability += r2;
		break;
#ifdef HELLFIRE
	case IPL_CRYSTALLINE:
		item[i]._iPLDam += 140 + r * 2;
		// no break
#endif
	case IPL_DUR_CURSE:
		item[i]._iMaxDur -= r * item[i]._iMaxDur / 100;
		if (item[i]._iMaxDur < 1)
			item[i]._iMaxDur = 1;
		item[i]._iDurability = item[i]._iMaxDur;
		break;
	case IPL_INDESTRUCTIBLE:
		item[i]._iDurability = DUR_INDESTRUCTIBLE;
		item[i]._iMaxDur = DUR_INDESTRUCTIBLE;
		break;
	case IPL_LIGHT:
		item[i]._iPLLight += param1;
		break;
	case IPL_LIGHT_CURSE:
		item[i]._iPLLight -= param1;
		break;
#ifdef HELLFIRE
	case IPL_MULT_ARROWS:
		item[i]._iFlags |= ISPL_MULT_ARROWS;
		break;
#endif
	case IPL_FIRE_ARROWS:
		item[i]._iFlags |= ISPL_FIRE_ARROWS;
#ifdef HELLFIRE
		item[i]._iFlags &= ~ISPL_LIGHT_ARROWS;
#endif
		item[i]._iFMinDam = param1;
		item[i]._iFMaxDam = param2;
#ifdef HELLFIRE
		item[i]._iLMinDam = 0;
		item[i]._iLMaxDam = 0;
#endif
		break;
	case IPL_LIGHT_ARROWS:
		item[i]._iFlags |= ISPL_LIGHT_ARROWS;
#ifdef HELLFIRE
		item[i]._iFlags &= ~ISPL_FIRE_ARROWS;
#endif
		item[i]._iLMinDam = param1;
		item[i]._iLMaxDam = param2;
#ifdef HELLFIRE
		item[i]._iFMinDam = 0;
		item[i]._iFMaxDam = 0;
#endif
		break;
#ifdef HELLFIRE
	case IPL_FIREBALL:
		item[i]._iFlags |= (ISPL_LIGHT_ARROWS | ISPL_FIRE_ARROWS);
		item[i]._iFMinDam = param1;
		item[i]._iFMaxDam = param2;
		item[i]._iLMinDam = 0;
		item[i]._iLMaxDam = 0;
		break;
#endif
	case IPL_THORNS:
		item[i]._iFlags |= ISPL_THORNS;
		break;
	case IPL_NOMANA:
		item[i]._iFlags |= ISPL_NOMANA;
		break;
	case IPL_NOHEALPLR:
		item[i]._iFlags |= ISPL_NOHEALPLR;
		break;
	case IPL_ABSHALFTRAP:
		item[i]._iFlags |= ISPL_ABSHALFTRAP;
		break;
	case IPL_KNOCKBACK:
		item[i]._iFlags |= ISPL_KNOCKBACK;
		break;
	case IPL_3XDAMVDEM:
		item[i]._iFlags |= ISPL_3XDAMVDEM;
		break;
	case IPL_ALLRESZERO:
		item[i]._iFlags |= ISPL_ALLRESZERO;
		break;
	case IPL_NOHEALMON:
		item[i]._iFlags |= ISPL_NOHEALMON;
		break;
	case IPL_STEALMANA:
		if (param1 == 3)
			item[i]._iFlags |= ISPL_STEALMANA_3;
		if (param1 == 5)
			item[i]._iFlags |= ISPL_STEALMANA_5;
		break;
	case IPL_STEALLIFE:
		if (param1 == 3)
			item[i]._iFlags |= ISPL_STEALLIFE_3;
		if (param1 == 5)
			item[i]._iFlags |= ISPL_STEALLIFE_5;
		break;
	case IPL_TARGAC:
#ifdef HELLFIRE
		item[i]._iPLEnAc = param1;
#else
		item[i]._iPLEnAc += r;
#endif
		break;
	case IPL_FASTATTACK:
		if (param1 == 1)
			item[i]._iFlags |= ISPL_QUICKATTACK;
		if (param1 == 2)
			item[i]._iFlags |= ISPL_FASTATTACK;
		if (param1 == 3)
			item[i]._iFlags |= ISPL_FASTERATTACK;
		if (param1 == 4)
			item[i]._iFlags |= ISPL_FASTESTATTACK;
		break;
	case IPL_FASTRECOVER:
		if (param1 == 1)
			item[i]._iFlags |= ISPL_FASTRECOVER;
		if (param1 == 2)
			item[i]._iFlags |= ISPL_FASTERRECOVER;
		if (param1 == 3)
			item[i]._iFlags |= ISPL_FASTESTRECOVER;
		break;
	case IPL_FASTBLOCK:
		item[i]._iFlags |= ISPL_FASTBLOCK;
		break;
	case IPL_DAMMOD:
		item[i]._iPLDamMod += r;
		break;
	case IPL_RNDARROWVEL:
		item[i]._iFlags |= ISPL_RNDARROWVEL;
		break;
	case IPL_SETDAM:
		item[i]._iMinDam = param1;
		item[i]._iMaxDam = param2;
		break;
	case IPL_SETDUR:
		item[i]._iDurability = param1;
		item[i]._iMaxDur = param1;
		break;
	case IPL_FASTSWING:
		item[i]._iFlags |= ISPL_FASTERATTACK;
		break;
	case IPL_ONEHAND:
		item[i]._iLoc = ILOC_ONEHAND;
		break;
	case IPL_DRAINLIFE:
		item[i]._iFlags |= ISPL_DRAINLIFE;
		break;
	case IPL_RNDSTEALLIFE:
		item[i]._iFlags |= ISPL_RNDSTEALLIFE;
		break;
	case IPL_INFRAVISION:
		item[i]._iFlags |= ISPL_INFRAVISION;
		break;
	case IPL_NOMINSTR:
		item[i]._iMinStr = 0;
		break;
	case IPL_INVCURS:
		item[i]._iCurs = param1;
		break;
	case IPL_ADDACLIFE:
#ifdef HELLFIRE
		item[i]._iFlags |= (ISPL_LIGHT_ARROWS | ISPL_FIRE_ARROWS);
		item[i]._iFMinDam = param1;
		item[i]._iFMaxDam = param2;
		item[i]._iLMinDam = 1;
		item[i]._iLMaxDam = 0;
#else
		item[i]._iPLHP = (universe.plr[myplr]._pIBonusAC + universe.plr[myplr]._pIAC + universe.plr[myplr]._pDexterity / 5) << 6;
#endif
		break;
	case IPL_ADDMANAAC:
#ifdef HELLFIRE
		item[i]._iFlags |= (ISPL_LIGHTDAM | ISPL_FIREDAM);
		item[i]._iFMinDam = param1;
		item[i]._iFMaxDam = param2;
		item[i]._iLMinDam = 2;
		item[i]._iLMaxDam = 0;
#else
		item[i]._iAC += (universe.plr[myplr]._pMaxManaBase >> 6) / 10;
#endif
		break;
	case IPL_FIRERESCLVL:
		item[i]._iPLFR = 30 - universe.plr[myplr]._pLevel;
		if (item[i]._iPLFR < 0)
			item[i]._iPLFR = 0;
		break;
#ifdef HELLFIRE
	case IPL_FIRERES_CURSE:
		item[i]._iPLFR -= r;
		break;
	case IPL_LIGHTRES_CURSE:
		item[i]._iPLLR -= r;
		break;
	case IPL_MAGICRES_CURSE:
		item[i]._iPLMR -= r;
		break;
	case IPL_ALLRES_CURSE:
		item[i]._iPLFR -= r;
		item[i]._iPLLR -= r;
		item[i]._iPLMR -= r;
		break;
	case IPL_DEVASTATION:
		item[i]._iDamAcFlags |= ISPLHF_DEVASTATION;
		break;
	case IPL_DECAY:
		item[i]._iDamAcFlags |= ISPLHF_DECAY;
		item[i]._iPLDam += r;
		break;
	case IPL_PERIL:
		item[i]._iDamAcFlags |= ISPLHF_PERIL;
		break;
	case IPL_JESTERS:
		item[i]._iDamAcFlags |= ISPLHF_JESTERS;
		break;
	case IPL_ACDEMON:
		item[i]._iDamAcFlags |= ISPLHF_ACDEMON;
		break;
	case IPL_ACUNDEAD:
		item[i]._iDamAcFlags |= ISPLHF_ACUNDEAD;
		break;
	case IPL_MANATOLIFE:
		r2 = ((universe.plr[myplr]._pMaxManaBase >> 6) * 50 / 100);
		item[i]._iPLMana -= (r2 << 6);
		item[i]._iPLHP += (r2 << 6);
		break;
	case IPL_LIFETOMANA:
		r2 = ((universe.plr[myplr]._pMaxHPBase >> 6) * 40 / 100);
		item[i]._iPLHP -= (r2 << 6);
		item[i]._iPLMana += (r2 << 6);
		break;
#endif
	}
	if (item[i]._iVAdd1 || item[i]._iVMult1) {
		item[i]._iVAdd2 = PLVal(r, param1, param2, minval, maxval);
		item[i]._iVMult2 = multval;
	} else {
		item[i]._iVAdd1 = PLVal(r, param1, param2, minval, maxval);
		item[i]._iVMult1 = multval;
	}
}

void GetItemPower(Universe& universe, int i, int minlvl, int maxlvl, int flgs, BOOL onlygood)
{
	int pre, post, nt, nl, j, preidx, sufidx;
	int l[256];
	char istr[128];
	BYTE goe;

	pre = random_(universe, 23, 4);
	post = random_(universe, 23, 3);
	if (pre != 0 && post == 0) {
		if (random_(universe, 23, 2) != 0)
			post = 1;
		else
			pre = 0;
	}
	preidx = -1;
	sufidx = -1;
	goe = GOE_ANY;
	if (!onlygood && random_(universe, 0, 3) != 0)
		onlygood = TRUE;
	if (pre == 0) {
		nt = 0;
		for (j = 0; PL_Prefix[j].PLPower != IPL_INVALID; j++) {
			if (flgs & PL_Prefix[j].PLIType) {
				if (PL_Prefix[j].PLMinLvl >= minlvl && PL_Prefix[j].PLMinLvl <= maxlvl && (!onlygood || PL_Prefix[j].PLOk) && (flgs != PLT_STAFF || PL_Prefix[j].PLPower != IPL_CHARGES)) {
					l[nt] = j;
					nt++;
					if (PL_Prefix[j].PLDouble) {
						l[nt] = j;
						nt++;
					}
				}
			}
		}
		if (nt != 0) {
			preidx = l[random_(universe, 23, nt)];
			sprintf(istr, "%s %s", PL_Prefix[preidx].PLName, item[i]._iIName);
			strcpy(item[i]._iIName, istr);
			item[i]._iMagical = ITEM_QUALITY_MAGIC;
			SaveItemPower(
			    universe,
			    i,
			    PL_Prefix[preidx].PLPower,
			    PL_Prefix[preidx].PLParam1,
			    PL_Prefix[preidx].PLParam2,
			    PL_Prefix[preidx].PLMinVal,
			    PL_Prefix[preidx].PLMaxVal,
			    PL_Prefix[preidx].PLMultVal);
			item[i]._iPrePower = PL_Prefix[preidx].PLPower;
			goe = PL_Prefix[preidx].PLGOE;
		}
	}
	if (post != 0) {
		nl = 0;
		for (j = 0; PL_Suffix[j].PLPower != IPL_INVALID; j++) {
			if (PL_Suffix[j].PLIType & flgs
			    && PL_Suffix[j].PLMinLvl >= minlvl && PL_Suffix[j].PLMinLvl <= maxlvl
			    && (goe | PL_Suffix[j].PLGOE) != (GOE_GOOD | GOE_EVIL)
			    && (!onlygood || PL_Suffix[j].PLOk)) {
				l[nl] = j;
				nl++;
			}
		}
		if (nl != 0) {
			sufidx = l[random_(universe, 23, nl)];
			sprintf(istr, "%s of %s", item[i]._iIName, PL_Suffix[sufidx].PLName);
			strcpy(item[i]._iIName, istr);
			item[i]._iMagical = ITEM_QUALITY_MAGIC;
			SaveItemPower(
			    universe,
			    i,
			    PL_Suffix[sufidx].PLPower,
			    PL_Suffix[sufidx].PLParam1,
			    PL_Suffix[sufidx].PLParam2,
			    PL_Suffix[sufidx].PLMinVal,
			    PL_Suffix[sufidx].PLMaxVal,
			    PL_Suffix[sufidx].PLMultVal);
			item[i]._iSufPower = PL_Suffix[sufidx].PLPower;
		}
	}
	if (preidx != -1 || sufidx != -1)
		CalcItemValue(i);
}

#ifdef HELLFIRE
void GetItemBonus(int i, int idata, int minlvl, int maxlvl, BOOL onlygood, BOOLEAN allowspells)
#else
void GetItemBonus(Universe& universe, int i, int idata, int minlvl, int maxlvl, BOOL onlygood)
#endif
{
	if (item[i]._iClass != ICLASS_GOLD) {
		if (minlvl > 25)
			minlvl = 25;

		switch (item[i]._itype) {
		case ITYPE_SWORD:
		case ITYPE_AXE:
		case ITYPE_MACE:
			GetItemPower(universe, i, minlvl, maxlvl, PLT_WEAP, onlygood);
			break;
		case ITYPE_BOW:
			GetItemPower(universe, i, minlvl, maxlvl, PLT_BOW, onlygood);
			break;
		case ITYPE_SHIELD:
			GetItemPower(universe, i, minlvl, maxlvl, PLT_SHLD, onlygood);
			break;
		case ITYPE_LARMOR:
		case ITYPE_HELM:
		case ITYPE_MARMOR:
		case ITYPE_HARMOR:
			GetItemPower(universe, i, minlvl, maxlvl, PLT_ARMO, onlygood);
			break;
		case ITYPE_STAFF:
#ifdef HELLFIRE
			if (allowspells)
#endif
				GetStaffSpell(universe, i, maxlvl, onlygood);
#ifdef HELLFIRE
			else
				GetItemPower(universe, i, minlvl, maxlvl, PLT_STAFF, onlygood);
#endif
			break;
		case ITYPE_RING:
		case ITYPE_AMULET:
			GetItemPower(universe, i, minlvl, maxlvl, PLT_MISC, onlygood);
			break;
		}
	}
}

void SetupItem(Universe& universe, int i)
{
	int it;

	it = ItemCAnimTbl[item[i]._iCurs];
	item[i]._iAnimData = itemanims[it];
	item[i]._iAnimLen = ItemAnimLs[it];
	item[i]._iAnimWidth = 96;
	item[i]._iAnimWidth2 = 16;
	item[i]._iIdentified = FALSE;
	item[i]._iPostDraw = FALSE;

	if (!universe.plr[myplr].pLvlLoad) {
		item[i]._iAnimFrame = 1;
		item[i]._iAnimFlag = TRUE;
		item[i]._iSelFlag = 0;
	} else {
		item[i]._iAnimFrame = item[i]._iAnimLen;
		item[i]._iAnimFlag = FALSE;
		item[i]._iSelFlag = 1;
	}
}

int RndItem(Universe& universe, int m)
{
	int i, ri, r;
	int ril[512];

	if ((monster[m].MData->mTreasure & 0x8000) != 0)
		return -1 - (monster[m].MData->mTreasure & 0xFFF);

	if (monster[m].MData->mTreasure & 0x4000)
		return 0;

	if (random_(universe, 24, 100) > 40)
		return 0;

	if (random_(universe, 24, 100) > 25)
		return IDI_GOLD + 1;

	ri = 0;
	for (i = 0; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
		if (AllItemsList[i].iRnd == IDROP_DOUBLE && monster[m].mLevel >= AllItemsList[i].iMinMLvl
#ifdef HELLFIRE
		    && ri < 512
#endif
		) {
			ril[ri] = i;
			ri++;
		}
		if (AllItemsList[i].iRnd != IDROP_NEVER && monster[m].mLevel >= AllItemsList[i].iMinMLvl
#ifdef HELLFIRE
		    && ri < 512
#endif
		) {
			ril[ri] = i;
			ri++;
		}
		// BUGFIX: ri decremented even for IDROP_NEVER, thus Scroll of Resurrect
		// (IDI_RESURRECT) decrements ri, unintentionally removing gold drop in
		// Single Player (gold drop is still valid in Multi Player).
		if (AllItemsList[i].iSpell == SPL_RESURRECT && universe.gbMaxPlayers == 1)
			ri--;
		if (AllItemsList[i].iSpell == SPL_HEALOTHER && universe.gbMaxPlayers == 1)
			ri--;
	}

	r = random_(universe, 24, ri);
	return ril[r] + 1;
}

int RndUItem(Universe& universe, int m)
{
	int i, ri;
	int ril[512];
	BOOL okflag;

	if (m != -1 && (monster[m].MData->mTreasure & 0x8000) != 0 && universe.gbMaxPlayers == 1)
		return -1 - (monster[m].MData->mTreasure & 0xFFF);

#ifdef HELLFIRE
	int curlv = items_get_currlevel();
#endif
	ri = 0;
	for (i = 0; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
		okflag = TRUE;
		if (AllItemsList[i].iRnd == IDROP_NEVER)
			okflag = FALSE;
		if (m != -1) {
			if (monster[m].mLevel < AllItemsList[i].iMinMLvl)
				okflag = FALSE;
		} else {
#ifdef HELLFIRE
			if (2 * curlv < AllItemsList[i].iMinMLvl)
#else
			if (2 * universe.currlevel < AllItemsList[i].iMinMLvl)
#endif
				okflag = FALSE;
		}
		if (AllItemsList[i].itype == ITYPE_MISC)
			okflag = FALSE;
		if (AllItemsList[i].itype == ITYPE_GOLD)
			okflag = FALSE;
		if (AllItemsList[i].itype == ITYPE_FOOD)
			okflag = FALSE;
		if (AllItemsList[i].iMiscId == IMISC_BOOK)
			okflag = TRUE;
		if (AllItemsList[i].iSpell == SPL_RESURRECT && universe.gbMaxPlayers == 1)
			okflag = FALSE;
		if (AllItemsList[i].iSpell == SPL_HEALOTHER && universe.gbMaxPlayers == 1)
			okflag = FALSE;
#ifdef HELLFIRE
		if (okflag && ri < 512) {
#else
		if (okflag) {
#endif
			ril[ri] = i;
			ri++;
		}
	}

	return ril[random_(universe, 25, ri)];
}

int RndAllItems(Universe& universe)
{
	int i, ri;
	int ril[512];

	if (random_(universe, 26, 100) > 25)
		return 0;

#ifdef HELLFIRE
	int curlv = items_get_currlevel();
#endif
	ri = 0;
	for (i = 0; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
#ifdef HELLFIRE
		if (AllItemsList[i].iRnd != IDROP_NEVER && 2 * curlv >= AllItemsList[i].iMinMLvl && ri < 512) {
#else
		if (AllItemsList[i].iRnd != IDROP_NEVER && 2 * universe.currlevel >= AllItemsList[i].iMinMLvl) {
#endif
			ril[ri] = i;
			ri++;
		}
		if (AllItemsList[i].iSpell == SPL_RESURRECT && universe.gbMaxPlayers == 1)
			ri--;
		if (AllItemsList[i].iSpell == SPL_HEALOTHER && universe.gbMaxPlayers == 1)
			ri--;
	}

	return ril[random_(universe, 26, ri)];
}

#ifdef HELLFIRE
int RndTypeItems(Universe& universe, int itype, int imid, int lvl)
#else
int RndTypeItems(Universe& universe, int itype, int imid)
#endif
{
	int i, ri;
	BOOL okflag;
	int ril[512];

	ri = 0;
	for (i = 0; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
		okflag = TRUE;
		if (AllItemsList[i].iRnd == IDROP_NEVER)
			okflag = FALSE;
#ifdef HELLFIRE
		if (lvl << 1 < AllItemsList[i].iMinMLvl)
#else
		if (universe.currlevel << 1 < AllItemsList[i].iMinMLvl)
#endif
			okflag = FALSE;
		if (AllItemsList[i].itype != itype)
			okflag = FALSE;
		if (imid != -1 && AllItemsList[i].iMiscId != imid)
			okflag = FALSE;
#ifdef HELLFIRE
		if (okflag && ri < 512) {
#else
		if (okflag) {
#endif
			ril[ri] = i;
			ri++;
		}
	}

	return ril[random_(universe, 27, ri)];
}

int CheckUnique(Universe& universe, int i, int lvl, int uper, BOOL recreate)
{
	int j, idata, numu;
	BOOLEAN uok[128];

	if (random_(universe, 28, 100) > uper)
		return UITYPE_INVALID;

	numu = 0;
	memset(uok, 0, sizeof(uok));
	for (j = 0; UniqueItemList[j].UIItemId != UITYPE_INVALID; j++) {
		if (UniqueItemList[j].UIItemId == AllItemsList[item[i].IDidx].iItemId
		    && lvl >= UniqueItemList[j].UIMinLvl
		    && (recreate || !UniqueItemFlag[j] || universe.gbMaxPlayers != 1)) {
			uok[j] = TRUE;
			numu++;
		}
	}

	if (numu == 0)
		return UITYPE_INVALID;

	random_(universe, 29, 10); /// BUGFIX: unused, last unique in array always gets chosen
	idata = 0;
	while (numu > 0) {
		if (uok[idata])
			numu--;
		if (numu > 0) {
			idata++;
			if (idata == 128)
				idata = 0;
		}
	}

	return idata;
}

void GetUniqueItem(Universe& universe, int i, int uid)
{
	UniqueItemFlag[uid] = TRUE;
	SaveItemPower(universe, i, UniqueItemList[uid].UIPower1, UniqueItemList[uid].UIParam1, UniqueItemList[uid].UIParam2, 0, 0, 1);

	if (UniqueItemList[uid].UINumPL > 1)
		SaveItemPower(universe, i, UniqueItemList[uid].UIPower2, UniqueItemList[uid].UIParam3, UniqueItemList[uid].UIParam4, 0, 0, 1);
	if (UniqueItemList[uid].UINumPL > 2)
		SaveItemPower(universe, i, UniqueItemList[uid].UIPower3, UniqueItemList[uid].UIParam5, UniqueItemList[uid].UIParam6, 0, 0, 1);
	if (UniqueItemList[uid].UINumPL > 3)
		SaveItemPower(universe, i, UniqueItemList[uid].UIPower4, UniqueItemList[uid].UIParam7, UniqueItemList[uid].UIParam8, 0, 0, 1);
	if (UniqueItemList[uid].UINumPL > 4)
		SaveItemPower(universe, i, UniqueItemList[uid].UIPower5, UniqueItemList[uid].UIParam9, UniqueItemList[uid].UIParam10, 0, 0, 1);
	if (UniqueItemList[uid].UINumPL > 5)
		SaveItemPower(universe, i, UniqueItemList[uid].UIPower6, UniqueItemList[uid].UIParam11, UniqueItemList[uid].UIParam12, 0, 0, 1);

	strcpy(item[i]._iIName, UniqueItemList[uid].UIName);
	item[i]._iIvalue = UniqueItemList[uid].UIValue;

	if (item[i]._iMiscId == IMISC_UNIQUE)
		item[i]._iSeed = uid;

	item[i]._iUid = uid;
	item[i]._iMagical = ITEM_QUALITY_UNIQUE;
	item[i]._iCreateInfo |= CF_UNIQUE;
}

void SpawnUnique(Universe& universe, int uid, int x, int y)
{
	int ii, itype;

#ifdef HELLFIRE
	int curlv = items_get_currlevel();
#endif
	if (numitems >= MAXITEMS)
		return;

	ii = itemavail[0];
	GetSuperItemSpace(universe, x, y, ii);
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	itemactive[numitems] = ii;

	itype = 0;
	while (AllItemsList[itype].iItemId != UniqueItemList[uid].UIItemId) {
		itype++;
	}

#ifdef HELLFIRE
	GetItemAttrs(universe, ii, itype, curlv);
#else
	GetItemAttrs(universe, ii, itype, universe.currlevel);
#endif
	GetUniqueItem(universe, ii, uid);
	SetupItem(universe, ii);
	numitems++;
}

void ItemRndDur(Universe& universe, int ii)
{
	if (item[ii]._iDurability && item[ii]._iDurability != DUR_INDESTRUCTIBLE)
		item[ii]._iDurability = random_(universe, 0, item[ii]._iMaxDur >> 1) + (item[ii]._iMaxDur >> 2) + 1;
}

void SetupAllItems(Universe& universe, int ii, int idx, int iseed, int lvl, int uper, BOOL onlygood, BOOL recreate, BOOL pregen)
{
	int iblvl, uid;

	item[ii]._iSeed = iseed;
	SetRndSeed(universe, iseed);
	GetItemAttrs(universe, ii, idx, lvl >> 1);
	item[ii]._iCreateInfo = lvl;

	if (pregen)
		item[ii]._iCreateInfo = lvl | CF_PREGEN;
	if (onlygood)
		item[ii]._iCreateInfo |= CF_ONLYGOOD;

	if (uper == 15)
		item[ii]._iCreateInfo |= CF_UPER15;
	else if (uper == 1)
		item[ii]._iCreateInfo |= CF_UPER1;

	if (item[ii]._iMiscId != IMISC_UNIQUE) {
		iblvl = -1;
		if (random_(universe, 32, 100) <= 10 || random_(universe, 33, 100) <= lvl) {
			iblvl = lvl;
		}
		if (iblvl == -1 && item[ii]._iMiscId == IMISC_STAFF) {
			iblvl = lvl;
		}
		if (iblvl == -1 && item[ii]._iMiscId == IMISC_RING) {
			iblvl = lvl;
		}
		if (iblvl == -1 && item[ii]._iMiscId == IMISC_AMULET) {
			iblvl = lvl;
		}
		if (onlygood)
			iblvl = lvl;
		if (uper == 15)
			iblvl = lvl + 4;
		if (iblvl != -1) {
			uid = CheckUnique(universe, ii, iblvl, uper, recreate);
			if (uid == UITYPE_INVALID) {
#ifdef HELLFIRE
				GetItemBonus(ii, idx, iblvl >> 1, iblvl, onlygood, TRUE);
#else
				GetItemBonus(universe, ii, idx, iblvl >> 1, iblvl, onlygood);
#endif
			} else {
				GetUniqueItem(universe, ii, uid);
				item[ii]._iCreateInfo |= CF_UNIQUE;
			}
		}
		if (item[ii]._iMagical != ITEM_QUALITY_UNIQUE)
			ItemRndDur(universe, ii);
	} else {
		if (item[ii]._iLoc != ILOC_UNEQUIPABLE) {
			//uid = CheckUnique(universe, ii, iblvl, uper, recreate);
			//if (uid != UITYPE_INVALID) {
			//	GetUniqueItem(universe, ii, uid);
			//}
			GetUniqueItem(universe, ii, iseed); // BUG: the second argument to GetUniqueItem should be uid.
		}
	}
	SetupItem(universe, ii);
}

void SpawnItem(Universe& universe, int m, int x, int y, BOOL sendmsg)
{
	int ii, idx;
	// BUGFIX: onlygood may be used uninitialized in call to SetupAllItems.universe, 
	BOOL onlygood;

	if (monster[m]._uniqtype || ((monster[m].MData->mTreasure & 0x8000) && universe.gbMaxPlayers != 1)) {
		idx = RndUItem(universe, m);
		if (idx < 0) {
			SpawnUnique(universe, -(idx + 1), x, y);
			return;
		}
		onlygood = TRUE;
	} else if (quests[Q_MUSHROOM]._qactive != QUEST_ACTIVE || quests[Q_MUSHROOM]._qvar1 != QS_MUSHGIVEN) {
		idx = RndItem(universe, m);
		if (!idx)
			return;
		if (idx > 0) {
			idx--;
			onlygood = FALSE;
		} else {
			SpawnUnique(universe, -(idx + 1), x, y);
			return;
		}
	} else {
		idx = IDI_BRAIN;
		quests[Q_MUSHROOM]._qvar1 = QS_BRAINSPAWNED;
	}

	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(universe, x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
		if (monster[m]._uniqtype) {
			SetupAllItems(universe, ii, idx, GetRndSeed(universe), monster[m].MData->mLevel, 15, onlygood, FALSE, FALSE);
		} else {
			SetupAllItems(universe, ii, idx, GetRndSeed(universe), monster[m].MData->mLevel, 1, onlygood, FALSE, FALSE);
		}
		numitems++;
	}
}

void CreateItem(Universe& universe, int uid, int x, int y)
{
	int ii, idx;

#ifdef HELLFIRE
	int curlv = items_get_currlevel();
#endif
	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(universe, x, y, ii);
		idx = 0;
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;

		while (AllItemsList[idx].iItemId != UniqueItemList[uid].UIItemId) {
			idx++;
		}

#ifdef HELLFIRE
		GetItemAttrs(universe, ii, idx, curlv);
#else
		GetItemAttrs(universe, ii, idx, universe.currlevel);
#endif
		GetUniqueItem(universe, ii, uid);
		SetupItem(universe, ii);
		item[ii]._iMagical = ITEM_QUALITY_UNIQUE;
		numitems++;
	}
}

void CreateRndItem(Universe& universe, int x, int y, BOOL onlygood, BOOL sendmsg, BOOL delta)
{
	int idx, ii;

#ifdef HELLFIRE
	int curlv = items_get_currlevel();
#endif
	if (onlygood)
		idx = RndUItem(universe, -1);
	else
		idx = RndAllItems(universe);

	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(universe, x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
#ifdef HELLFIRE
		SetupAllItems(universe, ii, idx, GetRndSeed(universe), 2 * curlv, 1, onlygood, FALSE, delta);
#else
		SetupAllItems(universe, ii, idx, GetRndSeed(universe), 2 * universe.currlevel, 1, onlygood, FALSE, delta);
#endif
		numitems++;
	}
}

void SetupAllUseful(Universe& universe, int ii, int iseed, int lvl)
{
	int idx;

	item[ii]._iSeed = iseed;
	SetRndSeed(universe, iseed);

#ifdef HELLFIRE
	idx = random_(universe, 34, 7);
	switch (idx) {
	case 0:
		idx = IDI_PORTAL;
		if ((lvl <= 1))
			idx = IDI_HEAL;
		break;
	case 1:
	case 2:
		idx = IDI_HEAL;
		break;
	case 3:
		idx = IDI_PORTAL;
		if ((lvl <= 1))
			idx = IDI_MANA;
		break;
	case 4:
	case 5:
		idx = IDI_MANA;
		break;
	case 6:
		idx = IDI_OIL;
		break;
	default:
		idx = IDI_OIL;
		break;
	}
#else
	if (random_(universe, 34, 2) != 0)
		idx = IDI_HEAL;
	else
		idx = IDI_MANA;

	if (lvl > 1 && random_(universe, 34, 3) == 0)
		idx = IDI_PORTAL;
#endif

	GetItemAttrs(universe, ii, idx, lvl);
	item[ii]._iCreateInfo = lvl + CF_USEFUL;
	SetupItem(universe, ii);
}

void CreateRndUseful(Universe& universe, int pnum, int x, int y, BOOL sendmsg)
{
	int ii;

#ifdef HELLFIRE
	int curlv = items_get_currlevel();
#endif
	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(universe, x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
#ifdef HELLFIRE
		SetupAllUseful(ii, GetRndSeed(universe), curlv);
#else
		SetupAllUseful(universe, ii, GetRndSeed(universe), universe.currlevel);
#endif
		numitems++;
	}
}

void CreateTypeItem(Universe& universe, int x, int y, BOOL onlygood, int itype, int imisc, BOOL sendmsg, BOOL delta)
{
	int idx, ii;

#ifdef HELLFIRE
	int curlv = items_get_currlevel();
	if (itype != ITYPE_GOLD)
		idx = RndTypeItems(universe, itype, imisc, curlv);
#else
	if (itype != ITYPE_GOLD)
		idx = RndTypeItems(universe, itype, imisc);
#endif
	else
		idx = IDI_GOLD;

	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(universe, x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
#ifdef HELLFIRE
		SetupAllItems(universe, ii, idx, GetRndSeed(universe), 2 * curlv, 1, onlygood, FALSE, delta);
#else
		SetupAllItems(universe, ii, idx, GetRndSeed(universe), 2 * universe.currlevel, 1, onlygood, FALSE, delta);
#endif

		numitems++;
	}
}

void SpawnQuestItem(Universe& universe, int itemid, int x, int y, int randarea, int selflag)
{
	BOOL failed;
	int i, j, tries;

#ifdef HELLFIRE
	int curlv = items_get_currlevel();
#endif
	if (randarea) {
		tries = 0;
		while (1) {
			tries++;
			if (tries > 1000 && randarea > 1)
				randarea--;
			x = random_(universe, 0, MAXDUNX);
			y = random_(universe, 0, MAXDUNY);
			failed = FALSE;
			for (i = 0; i < randarea && !failed; i++) {
				for (j = 0; j < randarea && !failed; j++) {
					failed = !ItemSpaceOk(universe, i + x, j + y);
				}
			}
			if (!failed)
				break;
		}
	}

	if (numitems < MAXITEMS) {
		i = itemavail[0];
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = i;
		item[i]._ix = x;
		item[i]._iy = y;
		universe.dItem[x][y] = i + 1;
#ifdef HELLFIRE
		GetItemAttrs(universe, i, itemid, curlv);
#else
		GetItemAttrs(universe, i, itemid, universe.currlevel);
#endif
		SetupItem(universe, i);
		item[i]._iPostDraw = TRUE;
		if (selflag) {
			item[i]._iSelFlag = selflag;
			item[i]._iAnimFrame = item[i]._iAnimLen;
			item[i]._iAnimFlag = FALSE;
		}
		numitems++;
	}
}

void SpawnRock(Universe& universe)
{
	int i, ii;
	int xx, yy;
	int ostand;

	ostand = FALSE;
	for (i = 0; i < nobjects && !ostand; i++) {
		ii = objectactive[i];
		ostand = object[ii]._otype == OBJ_STAND;
	}
#ifdef HELLFIRE
	int curlv = items_get_currlevel();
#endif
	if (ostand) {
		i = itemavail[0];
		itemavail[0] = itemavail[127 - numitems - 1];
		itemactive[numitems] = i;
		xx = object[ii]._ox;
		yy = object[ii]._oy;
		item[i]._ix = xx;
		item[i]._iy = yy;
		universe.dItem[xx][item[i]._iy] = i + 1;
#ifdef HELLFIRE
		GetItemAttrs(universe, i, IDI_ROCK, curlv);
#else
		GetItemAttrs(universe, i, IDI_ROCK, universe.currlevel);
#endif
		SetupItem(universe, i);
		item[i]._iSelFlag = 2;
		item[i]._iPostDraw = TRUE;
		item[i]._iAnimFrame = 11;
		numitems++;
	}
}

BOOL StoreStatOk(Universe& universe, ItemStruct *h)
{
	BOOL sf;

	sf = TRUE;
	if (universe.plr[myplr]._pStrength < h->_iMinStr)
		sf = FALSE;
	if (universe.plr[myplr]._pMagic < h->_iMinMag)
		sf = FALSE;
	if (universe.plr[myplr]._pDexterity < h->_iMinDex)
		sf = FALSE;

	return sf;
}

BOOL SmithItemOk(int i)
{
	BOOL rv;

	rv = TRUE;
	if (AllItemsList[i].itype == ITYPE_MISC)
		rv = FALSE;
	if (AllItemsList[i].itype == ITYPE_GOLD)
		rv = FALSE;
	if (AllItemsList[i].itype == ITYPE_FOOD)
		rv = FALSE;
#ifdef HELLFIRE
	if (AllItemsList[i].itype == ITYPE_STAFF && AllItemsList[i].iSpell)
#else
	if (AllItemsList[i].itype == ITYPE_STAFF)
#endif
		rv = FALSE;
	if (AllItemsList[i].itype == ITYPE_RING)
		rv = FALSE;
	if (AllItemsList[i].itype == ITYPE_AMULET)
		rv = FALSE;

	return rv;
}

int RndSmithItem(Universe& universe, int lvl)
{
	int i, ri;
	int ril[512];

	ri = 0;
	for (i = 1; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
		if (AllItemsList[i].iRnd != IDROP_NEVER && SmithItemOk(i) && lvl >= AllItemsList[i].iMinMLvl
#ifdef HELLFIRE
		    && ri < 512
#endif
		) {
			ril[ri] = i;
			ri++;
			if (AllItemsList[i].iRnd == IDROP_DOUBLE
#ifdef HELLFIRE
			    && ri < 512
#endif
			) {
				ril[ri] = i;
				ri++;
			}
		}
	}

	return ril[random_(universe, 50, ri)] + 1;
}

void BubbleSwapItem(ItemStruct *a, ItemStruct *b)
{
	ItemStruct h;

	h = *a;
	*a = *b;
	*b = h;
}

//void SortSmith()
//{
//	int j, k;
//	BOOL sorted;
//
//	j = 0;
//	while (smithitem[j + 1]._itype != ITYPE_NONE) {
//		j++;
//	}
//
//	sorted = FALSE;
//	while (j > 0 && !sorted) {
//		sorted = TRUE;
//		for (k = 0; k < j; k++) {
//			if (smithitem[k].IDidx > smithitem[k + 1].IDidx) {
//				BubbleSwapItem(&smithitem[k], &smithitem[k + 1]);
//				sorted = FALSE;
//			}
//		}
//		j--;
//	}
//}

int ItemNoFlippy()
{
	int r;

	r = itemactive[numitems - 1];
	item[r]._iAnimFrame = item[r]._iAnimLen;
	item[r]._iAnimFlag = FALSE;
	item[r]._iSelFlag = 1;

	return r;
}

void CreateSpellBook(Universe& universe, int x, int y, int ispell, BOOL sendmsg, BOOL delta)
{
	int ii, idx;
	BOOL done;

	done = FALSE;
#ifdef HELLFIRE
	int lvl = spelldata[ispell].sBookLvl + 1;
	if (lvl < 1) {
		return;
	}
	idx = RndTypeItems(universe, ITYPE_MISC, IMISC_BOOK, lvl);
#else
	idx = RndTypeItems(universe, ITYPE_MISC, IMISC_BOOK);
#endif
	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(universe, x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
		while (!done) {
#ifdef HELLFIRE
			SetupAllItems(universe, ii, idx, GetRndSeed(universe), 2 * lvl, 1, TRUE, FALSE, delta);
#else
			SetupAllItems(universe, ii, idx, GetRndSeed(universe), 2 * universe.currlevel, 1, TRUE, FALSE, delta);
#endif
			if (item[ii]._iMiscId == IMISC_BOOK && item[ii]._iSpell == ispell)
				done = TRUE;
		}
		numitems++;
	}
}

void CreateMagicArmor(Universe& universe, int x, int y, int imisc, int icurs, BOOL sendmsg, BOOL delta)
{
	int ii, idx;
	BOOL done;

	done = FALSE;
#ifdef HELLFIRE
	int curlv = items_get_currlevel();
#endif
	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(universe, x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
#ifdef HELLFIRE
		idx = RndTypeItems(universe, imisc, IMISC_NONE, curlv);
#else
		idx = RndTypeItems(universe, imisc, IMISC_NONE);
#endif
		while (!done) {
#ifdef HELLFIRE
			SetupAllItems(universe, ii, idx, GetRndSeed(universe), 2 * curlv, 1, TRUE, FALSE, delta);
#else
			SetupAllItems(universe, ii, idx, GetRndSeed(universe), 2 * universe.currlevel, 1, TRUE, FALSE, delta);
#endif
			if (item[ii]._iCurs == icurs)
				done = TRUE;
			else
#ifdef HELLFIRE
				idx = RndTypeItems(universe, imisc, IMISC_NONE, curlv);
#else
				idx = RndTypeItems(universe, imisc, IMISC_NONE);
#endif
		}
		numitems++;
	}
}

void CreateMagicWeapon(Universe& universe, int x, int y, int imisc, int icurs, BOOL sendmsg, BOOL delta)
{
	int ii, idx;
	BOOL done;

	done = FALSE;
#ifdef HELLFIRE
	int imid;
	if (imisc == ITYPE_STAFF)
		imid = IMISC_STAFF;
	else
		imid = IMISC_NONE;
	int curlv = items_get_currlevel();
#endif
	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(universe, x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
#ifdef HELLFIRE
		idx = RndTypeItems(universe, imisc, imid, curlv);
#else
		idx = RndTypeItems(universe, imisc, IMISC_NONE);
#endif
		while (!done) {
#ifdef HELLFIRE
			SetupAllItems(universe, ii, idx, GetRndSeed(universe), 2 * curlv, 1, TRUE, FALSE, delta);
#else
			SetupAllItems(universe, ii, idx, GetRndSeed(universe), 2 * universe.currlevel, 1, TRUE, FALSE, delta);
#endif
			if (item[ii]._iCurs == icurs)
				done = TRUE;
			else
#ifdef HELLFIRE
				idx = RndTypeItems(universe, imisc, imid, curlv);
#else
				idx = RndTypeItems(universe, imisc, IMISC_NONE);
#endif
		}
		numitems++;
	}
}
