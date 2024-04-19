/**
 * @file monster.cpp
 *
 * Implementation of monster functionality, AI, actions, spawning, loading, etc.
 */

#include "Source/monster.h"

#include "defs.h"
#include "structs.h"
#include "types.h"
#include "Source/drlg_l4.h"
#include "Source/engine.h"
#include "Source/gendung.h"
#include "Source/lighting.h"
#include "Source/monstdat.h"
#include "Source/quests.h"
#include "Source/themes.h"
#include "Source/trigs.h"
#include "Source/universe/universe.h"

/** Maps from walking path step to facing direction. */
const char plr2monst[9] = { 0, 5, 3, 7, 1, 4, 6, 0, 2 };
/** Maps from monster intelligence factor to missile type. */
const BYTE counsmiss[4] = { MIS_FIREBOLT, MIS_CBOLT, MIS_LIGHTCTRL, MIS_FIREBALL };

/* data */

// BUGFIX: MWVel velocity values are not rounded consistently. The correct
// formula for monster walk velocity is calculated as follows (for 16, 32 and 64
// pixel distances, respectively):
//
//    vel16 = (16 << monsterWalkShift) / nframes
//    vel32 = (32 << monsterWalkShift) / nframes
//    vel64 = (64 << monsterWalkShift) / nframes
//
// The correct monster walk velocity table is as follows:
//
//   int MWVel[24][3] = {
//      { 256, 512, 1024 },
//      { 128, 256, 512 },
//      { 85, 171, 341 },
//      { 64, 128, 256 },
//      { 51, 102, 205 },
//      { 43, 85, 171 },
//      { 37, 73, 146 },
//      { 32, 64, 128 },
//      { 28, 57, 114 },
//      { 26, 51, 102 },
//      { 23, 47, 93 },
//      { 21, 43, 85 },
//      { 20, 39, 79 },
//      { 18, 37, 73 },
//      { 17, 34, 68 },
//      { 16, 32, 64 },
//      { 15, 30, 60 },
//      { 14, 28, 57 },
//      { 13, 27, 54 },
//      { 13, 26, 51 },
//      { 12, 24, 49 },
//      { 12, 23, 47 },
//      { 11, 22, 45 },
//      { 11, 21, 43 }
//   };

/** Maps from monster walk animation frame num to monster velocity. */
const int MWVel[24][3] = {
	{ 256, 512, 1024 },
	{ 128, 256, 512 },
	{ 85, 170, 341 },
	{ 64, 128, 256 },
	{ 51, 102, 204 },
	{ 42, 85, 170 },
	{ 36, 73, 146 },
	{ 32, 64, 128 },
	{ 28, 56, 113 },
	{ 26, 51, 102 },
	{ 23, 46, 93 },
	{ 21, 42, 85 },
	{ 19, 39, 78 },
	{ 18, 36, 73 },
	{ 17, 34, 68 },
	{ 16, 32, 64 },
	{ 15, 30, 60 },
	{ 14, 28, 57 },
	{ 13, 26, 54 },
	{ 12, 25, 51 },
	{ 12, 24, 48 },
	{ 11, 23, 46 },
	{ 11, 22, 44 },
	{ 10, 21, 42 }
};
/** Maps from monster action to monster animation letter. */
const char animletter[7] = "nwahds";
/** Maps from direction to a left turn from the direction. */
const int left[8] = { 7, 0, 1, 2, 3, 4, 5, 6 };
/** Maps from direction to a right turn from the direction. */
const int right[8] = { 1, 2, 3, 4, 5, 6, 7, 0 };
/** Maps from direction to the opposite direction. */
const int opposite[8] = { 4, 5, 6, 7, 0, 1, 2, 3 };
/** Maps from direction to delta X-offset. */
const int offset_x[8] = { 1, 0, -1, -1, -1, 0, 1, 1 };
/** Maps from direction to delta Y-offset. */
const int offset_y[8] = { 1, 1, 1, 0, -1, -1, -1, 0 };

#ifdef HELLFIRE
const int HorkXAdd[8] = { 1, 0, -1, -1, -1, 0, 1, 1 }; // CODEFIX: same values as offset_x, remove it and use offset_x instead
const int HorkYAdd[8] = { 1, 1, 1, 0, -1, -1, -1, 0 }; // CODEFIX: same values as offset_y, remove it and use offset_y instead
#endif

/** unused */
const int rnd5[4] = { 5, 10, 15, 20 };
const int rnd10[4] = { 10, 15, 20, 30 };
const int rnd20[4] = { 20, 30, 40, 50 };
const int rnd60[4] = { 60, 70, 80, 90 };

///** Maps from monster AI ID to monster AI function. */
//void (*AiProc[])(int i) = {
//	&MAI_Zombie,
//	&MAI_Fat,
//	&MAI_SkelSd,
//	&MAI_SkelBow,
//	&MAI_Scav,
//	&MAI_Rhino,
//	&MAI_GoatMc,
//	&MAI_GoatBow,
//	&MAI_Fallen,
//	&MAI_Magma,
//	&MAI_SkelKing,
//	&MAI_Bat,
//	&MAI_Garg,
//	&MAI_Cleaver,
//	&MAI_Succ,
//	&MAI_Sneak,
//	&MAI_Storm,
//	&MAI_Fireman,
//	&MAI_Garbud,
//	&MAI_Acid,
//	&MAI_AcidUniq,
//	&MAI_Golum,
//	&MAI_Zhar,
//	&MAI_SnotSpil,
//	&MAI_Snake,
//	&MAI_Counselor,
//	&MAI_Mega,
//	&MAI_Diablo,
//	&MAI_Lazurus,
//	&MAI_Lazhelp,
//	&MAI_Lachdanan,
//	&MAI_Warlord,
//#ifdef HELLFIRE
//	&MAI_Firebat,
//	&MAI_Torchant,
//	&MAI_HorkDemon,
//	&MAI_Lich,
//	&MAI_ArchLich,
//	&MAI_Psychorb,
//	&MAI_Necromorb,
//	&MAI_BoneDemon
//#endif
//};

void InitLevelMonsters(Universe& universe)
{
	int i;

	universe.nummtypes = 0;
	universe.monstimgtot = 0;
	universe.MissileFileFlag = 0;

	for (i = 0; i < MAX_LVLMTYPES; i++) {
		universe.Monsters[i].mPlaceFlags = 0;
	}

	ClrAllMonsters(universe);
	universe.nummonsters = 0;
	universe.totalmonsters = MAXMONSTERS;

	for (i = 0; i < MAXMONSTERS; i++) {
		universe.monstactive[i] = i;
	}

	universe.uniquetrans = 0;
}

int AddMonsterType(Universe& universe, int type, int placeflag)
{
	BOOL done = FALSE;
	int i;

	for (i = 0; i < universe.nummtypes && !done; i++) {
		done = universe.Monsters[i].mtype == type;
	}

	i--;

	if (!done) {
		i = universe.nummtypes;
		universe.nummtypes++;
		universe.Monsters[i].mtype = type;
		universe.monstimgtot += monsterdata[type].mImage;
		InitMonsterGFX(universe, i);
	}

	universe.Monsters[i].mPlaceFlags |= placeflag;
	return i;
}

void GetLevelMTypes(Universe& universe)
{
	int i;

	// this array is merged with skeltypes down below.
	int typelist[MAXMONSTERS];
	int skeltypes[NUM_MTYPES];

	int minl; // min level
	int maxl; // max level
	char mamask;
	const int numskeltypes = 19;

	int nt; // number of types

#ifdef SPAWN
	mamask = 1; // monster availability mask
#else
	mamask = 3; // monster availability mask
#endif

	AddMonsterType(universe, MT_GOLEM, PLACE_SPECIAL);
	if (universe.currlevel == 16) {
		AddMonsterType(universe, MT_ADVOCATE, PLACE_SCATTER);
		AddMonsterType(universe, MT_RBLACK, PLACE_SCATTER);
		AddMonsterType(universe, MT_DIABLO, PLACE_SPECIAL);
		return;
	}

#ifdef HELLFIRE
	if (universe.currlevel == 18)
		AddMonsterType(universe, MT_HORKSPWN, PLACE_SCATTER);
	if (universe.currlevel == 19) {
		AddMonsterType(universe, MT_HORKSPWN, PLACE_SCATTER);
		AddMonsterType(universe, MT_HORKDMN, PLACE_UNIQUE);
	}
	if (universe.currlevel == 20)
		AddMonsterType(universe, MT_DEFILER, PLACE_UNIQUE);
	if (universe.currlevel == 24) {
		AddMonsterType(universe, MT_ARCHLICH, PLACE_SCATTER);
		AddMonsterType(universe, MT_NAKRUL, PLACE_SPECIAL);
	}
#endif

	if (!universe.setlevel) {
		if (QuestStatus(universe, Q_BUTCHER))
			AddMonsterType(universe, MT_CLEAVER, PLACE_SPECIAL);
		if (QuestStatus(universe, Q_GARBUD))
			AddMonsterType(universe, UniqMonst[UMT_GARBUD].mtype, PLACE_UNIQUE);
		if (QuestStatus(universe, Q_ZHAR))
			AddMonsterType(universe, UniqMonst[UMT_ZHAR].mtype, PLACE_UNIQUE);
		if (QuestStatus(universe, Q_LTBANNER))
			AddMonsterType(universe, UniqMonst[UMT_SNOTSPIL].mtype, PLACE_UNIQUE);
		if (QuestStatus(universe, Q_VEIL))
			AddMonsterType(universe, UniqMonst[UMT_LACHDAN].mtype, PLACE_UNIQUE);
		if (QuestStatus(universe, Q_WARLORD))
			AddMonsterType(universe, UniqMonst[UMT_WARLORD].mtype, PLACE_UNIQUE);

		if (universe.gbMaxPlayers != 1 && universe.currlevel == universe.quests[Q_SKELKING]._qlevel) {

			AddMonsterType(universe, MT_SKING, PLACE_UNIQUE);

			nt = 0;
			for (i = MT_WSKELAX; i <= MT_WSKELAX + numskeltypes; i++) {
				if (IsSkel(i)) {
					minl = 15 * monsterdata[i].mMinDLvl / 30 + 1;
					maxl = 15 * monsterdata[i].mMaxDLvl / 30 + 1;

					if (universe.currlevel >= minl && universe.currlevel <= maxl) {
						if (MonstAvailTbl[i] & mamask) {
							skeltypes[nt++] = i;
						}
					}
				}
			}
			AddMonsterType(universe, skeltypes[random_(universe, 88, nt)], PLACE_SCATTER);
		}

		nt = 0;
		for (i = 0; i < NUM_MTYPES; i++) {
			minl = 15 * monsterdata[i].mMinDLvl / 30 + 1;
			maxl = 15 * monsterdata[i].mMaxDLvl / 30 + 1;

			if (universe.currlevel >= minl && universe.currlevel <= maxl) {
				if (MonstAvailTbl[i] & mamask) {
					typelist[nt++] = i;
				}
			}
		}

		while (nt > 0 && universe.nummtypes < MAX_LVLMTYPES && universe.monstimgtot < 4000) {
			for (i = 0; i < nt;) {
				if (monsterdata[typelist[i]].mImage > 4000 - universe.monstimgtot) {
					typelist[i] = typelist[--nt];
					continue;
				}

				i++;
			}

			if (nt != 0) {
				i = random_(universe, 88, nt);
				AddMonsterType(universe, typelist[i], PLACE_SCATTER);
				typelist[i] = typelist[--nt];
			}
		}

	} else {
		if (universe.setlvlnum == SL_SKELKING) {
			AddMonsterType(universe, MT_SKING, PLACE_UNIQUE);
		}
	}
}

void InitMonsterGFX(Universe& universe, int monst)
{
	int mtype = universe.Monsters[monst].mtype;

	for (int anim = 0; anim < 6; anim++) {
		universe.Monsters[monst].Anims[anim].Frames = monsterdata[mtype].Frames[anim];
		universe.Monsters[monst].Anims[anim].Rate = monsterdata[mtype].Rate[anim];
	}

	universe.Monsters[monst].MData = &monsterdata[mtype];
}

void ClearMVars(Universe& universe, int i)
{
	universe.monster[i]._mVar1 = 0;
	universe.monster[i]._mVar2 = 0;
	universe.monster[i]._mVar3 = 0;
	universe.monster[i]._mVar4 = 0;
	universe.monster[i]._mVar5 = 0;
	universe.monster[i]._mVar6 = 0;
	universe.monster[i]._mVar7 = 0;
	universe.monster[i]._mVar8 = 0;
}

void InitMonster(Universe& universe, int i, int rd, int mtype, int x, int y)
{
	CMonster *monst = &universe.Monsters[mtype];

	universe.monster[i]._mdir = rd;
	universe.monster[i]._mx = x;
	universe.monster[i]._my = y;
	universe.monster[i]._mfutx = x;
	universe.monster[i]._mfuty = y;
	universe.monster[i]._moldx = x;
	universe.monster[i]._moldy = y;
	universe.monster[i]._mMTidx = mtype;
	universe.monster[i]._mmode = MM_STAND;
	universe.monster[i].mName = monst->MData->mName;
	universe.monster[i].MType = monst;
	universe.monster[i].MData = monst->MData;
	universe.monster[i]._mAnimData = monst->Anims[MA_STAND].Data[rd];
	universe.monster[i]._mAnimDelay = monst->Anims[MA_STAND].Rate;
	universe.monster[i]._mAnimCnt = random_(universe, 88, universe.monster[i]._mAnimDelay - 1);
	universe.monster[i]._mAnimLen = monst->Anims[MA_STAND].Frames;
	universe.monster[i]._mAnimFrame = random_(universe, 88, universe.monster[i]._mAnimLen - 1) + 1;

	if (monst->mtype == MT_DIABLO) {
#ifdef HELLFIRE
		universe.monster[i]._mmaxhp = (random_(universe, 88, 1) + 3333) << 6;
#else
		universe.monster[i]._mmaxhp = (random_(universe, 88, 1) + 1666) << 6;
#endif
	} else {
		universe.monster[i]._mmaxhp = (monst->mMinHP + random_(universe, 88, monst->mMaxHP - monst->mMinHP + 1)) << 6;
	}

	if (universe.gbMaxPlayers == 1) {
		universe.monster[i]._mmaxhp >>= 1;
		if (universe.monster[i]._mmaxhp < 64) {
			universe.monster[i]._mmaxhp = 64;
		}
	}

	universe.monster[i]._mhitpoints = universe.monster[i]._mmaxhp;
	universe.monster[i]._mAi = monst->MData->mAi;
	universe.monster[i]._mint = monst->MData->mInt;
	universe.monster[i]._mgoal = MGOAL_NORMAL;
	universe.monster[i]._mgoalvar1 = 0;
	universe.monster[i]._mgoalvar2 = 0;
	universe.monster[i]._mgoalvar3 = 0;
	universe.monster[i].field_18 = 0;
	universe.monster[i]._pathcount = 0;
	universe.monster[i]._mDelFlag = FALSE;
	universe.monster[i]._uniqtype = 0;
	universe.monster[i]._msquelch = 0;
#ifdef HELLFIRE
	universe.monster[i].mlid = 0;
#endif
	universe.monster[i]._mRndSeed = GetRndSeed(universe);
	universe.monster[i]._mAISeed = GetRndSeed(universe);
	universe.monster[i].mWhoHit = 0;
	universe.monster[i].mLevel = monst->MData->mLevel;
	universe.monster[i].mExp = monst->MData->mExp;
	universe.monster[i].mHit = monst->MData->mHit;
	universe.monster[i].mMinDamage = monst->MData->mMinDamage;
	universe.monster[i].mMaxDamage = monst->MData->mMaxDamage;
	universe.monster[i].mHit2 = monst->MData->mHit2;
	universe.monster[i].mMinDamage2 = monst->MData->mMinDamage2;
	universe.monster[i].mMaxDamage2 = monst->MData->mMaxDamage2;
	universe.monster[i].mArmorClass = monst->MData->mArmorClass;
	universe.monster[i].mMagicRes = monst->MData->mMagicRes;
	universe.monster[i].leader = 0;
	universe.monster[i].leaderflag = 0;
	universe.monster[i]._mFlags = monst->MData->mFlags;
	universe.monster[i].mtalkmsg = 0;

	if (universe.monster[i]._mAi == AI_GARG) {
		universe.monster[i]._mAnimData = monst->Anims[MA_SPECIAL].Data[rd];
		universe.monster[i]._mAnimFrame = 1;
		universe.monster[i]._mFlags |= MFLAG_ALLOW_SPECIAL;
		universe.monster[i]._mmode = MM_SATTACK;
	}

	if (universe.gnDifficulty == DIFF_NIGHTMARE) {
#ifdef HELLFIRE
		universe.monster[i]._mmaxhp = 3 * universe.monster[i]._mmaxhp + ((universe.gbMaxPlayers != 1 ? 100 : 50) << 6);
#else
		universe.monster[i]._mmaxhp = 3 * universe.monster[i]._mmaxhp + 64;
#endif
		universe.monster[i]._mhitpoints = universe.monster[i]._mmaxhp;
		universe.monster[i].mLevel += 15;
		universe.monster[i].mExp = 2 * (universe.monster[i].mExp + 1000);
		universe.monster[i].mHit += 85;
		universe.monster[i].mMinDamage = 2 * (universe.monster[i].mMinDamage + 2);
		universe.monster[i].mMaxDamage = 2 * (universe.monster[i].mMaxDamage + 2);
		universe.monster[i].mHit2 += 85;
		universe.monster[i].mMinDamage2 = 2 * (universe.monster[i].mMinDamage2 + 2);
		universe.monster[i].mMaxDamage2 = 2 * (universe.monster[i].mMaxDamage2 + 2);
		universe.monster[i].mArmorClass += 50;
	}

#ifdef HELLFIRE
	else
#endif
	    if (universe.gnDifficulty == DIFF_HELL) {
#ifdef HELLFIRE
		universe.monster[i]._mmaxhp = 4 * universe.monster[i]._mmaxhp + ((universe.gbMaxPlayers != 1 ? 200 : 100) << 6);
#else
		universe.monster[i]._mmaxhp = 4 * universe.monster[i]._mmaxhp + 192;
#endif
		universe.monster[i]._mhitpoints = universe.monster[i]._mmaxhp;
		universe.monster[i].mLevel += 30;
		universe.monster[i].mExp = 4 * (universe.monster[i].mExp + 1000);
		universe.monster[i].mHit += 120;
		universe.monster[i].mMinDamage = 4 * universe.monster[i].mMinDamage + 6;
		universe.monster[i].mMaxDamage = 4 * universe.monster[i].mMaxDamage + 6;
		universe.monster[i].mHit2 += 120;
		universe.monster[i].mMinDamage2 = 4 * universe.monster[i].mMinDamage2 + 6;
		universe.monster[i].mMaxDamage2 = 4 * universe.monster[i].mMaxDamage2 + 6;
		universe.monster[i].mArmorClass += 80;
		universe.monster[i].mMagicRes = monst->MData->mMagicRes2;
	}
}

void ClrAllMonsters(Universe& universe)
{
	int i;
	MonsterStruct *Monst;

	for (i = 0; i < MAXMONSTERS; i++) {
		Monst = &universe.monster[i];
		ClearMVars(universe, i);
		Monst->mName = "Invalid Monster";
		Monst->_mgoal = 0;
		Monst->_mmode = MM_STAND;
		Monst->_mVar1 = 0;
		Monst->_mVar2 = 0;
		Monst->_mx = 0;
		Monst->_my = 0;
		Monst->_mfutx = 0;
		Monst->_mfuty = 0;
		Monst->_moldx = 0;
		Monst->_moldy = 0;
		Monst->_mdir = random_(universe, 89, 8);
		Monst->_mxvel = 0;
		Monst->_myvel = 0;
		Monst->_mAnimData = NULL;
		Monst->_mAnimDelay = 0;
		Monst->_mAnimCnt = 0;
		Monst->_mAnimLen = 0;
		Monst->_mAnimFrame = 0;
		Monst->_mFlags = 0;
		Monst->_mDelFlag = FALSE;
		Monst->_menemy = random_(universe, 89, 1);
		// BUGFIX: `Monst->_menemy` may be referencing a player who already left the game, thus reading garbage data from `universe.plr[Monst->_menemy]._pfutx`.
		Monst->_menemyx = universe.plr[Monst->_menemy]._pfutx;
		Monst->_menemyy = universe.plr[Monst->_menemy]._pfuty;
	}
}

static BOOL SolidLoc(Universe& universe, int x, int y)
{
#ifndef HELLFIRE
	if (x < 0 || y < 0 || x >= MAXDUNX || y >= MAXDUNY) {
		return FALSE;
	}
#endif

	return universe.nSolidTable[universe.dPiece[x][y]];
}

BOOL MonstPlace(Universe& universe, int xp, int yp)
{
	char f;

	if (xp < 0 || xp >= MAXDUNX
	    || yp < 0 || yp >= MAXDUNY
	    || universe.dMonster[xp][yp] != 0
	    || universe.dPlayer[xp][yp] != 0) {
		return FALSE;
	}

	f = universe.dFlags[xp][yp];

	if (f & BFLAG_VISIBLE) {
		return FALSE;
	}

	if (f & BFLAG_POPULATED) {
		return FALSE;
	}

	return !SolidLoc(universe, xp, yp);
}

#ifdef HELLFIRE
void monster_some_crypt(Universe& universe)
{
	MonsterStruct *mon;
	int hp;

	if (universe.currlevel == 24 && universe.UberDiabloMonsterIndex >= 0 && universe.UberDiabloMonsterIndex < universe.nummonsters) {
		mon = &universe.monster[universe.UberDiabloMonsterIndex];
		PlayEffect(universe.UberDiabloMonsterIndex, 2);
		universe.quests[Q_NAKRUL]._qlog = FALSE;
		mon->mArmorClass -= 50;
		hp = mon->_mmaxhp / 2;
		mon->mMagicRes = 0;
		mon->_mhitpoints = hp;
		mon->_mmaxhp = hp;
	}
}

#endif
void PlaceMonster(Universe& universe, int i, int mtype, int x, int y)
{
	int rd;

#ifdef HELLFIRE
	if (universe.Monsters[mtype].mtype == MT_NAKRUL) {
		for (int j = 0; j < universe.nummonsters; j++) {
			if (universe.monster[j]._mMTidx == mtype) {
				return;
			}
			if (universe.monster[j].MType->mtype == MT_NAKRUL) {
				return;
			}
		}
	}
#endif
	universe.dMonster[x][y] = i + 1;

	rd = random_(universe, 90, 8);
	InitMonster(universe, i, rd, mtype, x, y);
}

#ifndef SPAWN
void PlaceUniqueMonst(Universe& universe, int uniqindex, int miniontype, int bosspacksize)
{
	int xp, yp, x, y, i;
	int uniqtype;
	int count2;
	char filestr[64];
	BOOL zharflag, done;
	const UniqMonstStruct *Uniq;
	MonsterStruct *Monst;
	int count;

	Monst = &universe.monster[universe.nummonsters];
	count = 0;
	Uniq = &UniqMonst[uniqindex];

	if ((universe.uniquetrans + 19) << 8 >= LIGHTSIZE) {
		return;
	}

	for (uniqtype = 0; uniqtype < universe.nummtypes; uniqtype++) {
		if (universe.Monsters[uniqtype].mtype == UniqMonst[uniqindex].mtype) {
			break;
		}
	}

	while (1) {
		xp = random_(universe, 91, 80) + 16;
		yp = random_(universe, 91, 80) + 16;
		count2 = 0;
		for (x = xp - 3; x < xp + 3; x++) {
			for (y = yp - 3; y < yp + 3; y++) {
				if (y >= 0 && y < MAXDUNY && x >= 0 && x < MAXDUNX && MonstPlace(universe, x, y)) {
					count2++;
				}
			}
		}

		if (count2 < 9) {
			count++;
			if (count < 1000) {
				continue;
			}
		}

		if (MonstPlace(universe, xp, yp)) {
			break;
		}
	}

	if (uniqindex == UMT_SNOTSPIL) {
		xp = 2 * universe.setpc_x + 24;
		yp = 2 * universe.setpc_y + 28;
	}
	if (uniqindex == UMT_WARLORD) {
		xp = 2 * universe.setpc_x + 22;
		yp = 2 * universe.setpc_y + 23;
	}
	if (uniqindex == UMT_ZHAR) {
		zharflag = TRUE;
		for (i = 0; i < universe.themeCount; i++) {
			if (i == zharlib && zharflag == TRUE) {
				zharflag = FALSE;
				xp = 2 * universe.themeLoc[i].x + 20;
				yp = 2 * universe.themeLoc[i].y + 20;
			}
		}
	}
	if (universe.gbMaxPlayers == 1) {
		if (uniqindex == UMT_LAZURUS) {
			xp = 32;
			yp = 46;
		}
		if (uniqindex == UMT_RED_VEX) {
			xp = 40;
			yp = 45;
		}
		if (uniqindex == UMT_BLACKJADE) {
			xp = 38;
			yp = 49;
		}
		if (uniqindex == UMT_SKELKING) {
			xp = 35;
			yp = 47;
		}
	} else {
		if (uniqindex == UMT_LAZURUS) {
			xp = 2 * universe.setpc_x + 19;
			yp = 2 * universe.setpc_y + 22;
		}
		if (uniqindex == UMT_RED_VEX) {
			xp = 2 * universe.setpc_x + 21;
			yp = 2 * universe.setpc_y + 19;
		}
		if (uniqindex == UMT_BLACKJADE) {
			xp = 2 * universe.setpc_x + 21;
			yp = 2 * universe.setpc_y + 25;
		}
	}
	if (uniqindex == UMT_BUTCHER) {
		done = FALSE;
		for (yp = 0; yp < MAXDUNY && !done; yp++) {
			for (xp = 0; xp < MAXDUNX && !done; xp++) {
				done = universe.dPiece[xp][yp] == 367;
			}
		}
	}

#ifdef HELLFIRE
	if (uniqindex == UMT_NAKRUL) {
		if (universe.UberRow == 0 || universe.UberCol == 0) {
			universe.UberDiabloMonsterIndex = -1;
			return;
		}
		xp = universe.UberRow - 2;
		yp = universe.UberCol;
		universe.UberDiabloMonsterIndex = universe.nummonsters;
	}
#endif
	PlaceMonster(universe, universe.nummonsters, uniqtype, xp, yp);
	Monst->_uniqtype = uniqindex + 1;

	if (Uniq->mlevel) {
		Monst->mLevel = 2 * Uniq->mlevel;
	} else {
		Monst->mLevel += 5;
	}

	Monst->mExp *= 2;
	Monst->mName = Uniq->mName;
	Monst->_mmaxhp = Uniq->mmaxhp << 6;

	if (universe.gbMaxPlayers == 1) {
		Monst->_mmaxhp = Monst->_mmaxhp >> 1;
		if (Monst->_mmaxhp < 64) {
			Monst->_mmaxhp = 64;
		}
	}

	Monst->_mhitpoints = Monst->_mmaxhp;
	Monst->_mAi = Uniq->mAi;
	Monst->_mint = Uniq->mint;
	Monst->mMinDamage = Uniq->mMinDamage;
	Monst->mMaxDamage = Uniq->mMaxDamage;
	Monst->mMinDamage2 = Uniq->mMinDamage;
	Monst->mMaxDamage2 = Uniq->mMaxDamage;
	Monst->mMagicRes = Uniq->mMagicRes;
	Monst->mtalkmsg = Uniq->mtalkmsg;
#ifdef HELLFIRE
	if (uniqindex == UMT_HORKDMN)
		Monst->mlid = 0;
	else
#endif
		Monst->mlid = AddLight(Monst->_mx, Monst->_my, 3);

	if (universe.gbMaxPlayers != 1) {
		if (Monst->_mAi == AI_LAZHELP)
			Monst->mtalkmsg = 0;
#ifndef HELLFIRE
		if (Monst->_mAi != AI_LAZURUS || universe.quests[Q_BETRAYER]._qvar1 <= 3) {
			if (Monst->mtalkmsg) {
				Monst->_mgoal = MGOAL_INQUIRING;
			}
		} else {
			Monst->_mgoal = MGOAL_NORMAL;
		}
#endif
	}
#ifdef HELLFIRE
	if (Monst->mtalkmsg)
#else
	else if (Monst->mtalkmsg)
#endif
		Monst->_mgoal = MGOAL_INQUIRING;

	if (universe.gnDifficulty == DIFF_NIGHTMARE) {
#ifdef HELLFIRE
		Monst->_mmaxhp = 3 * Monst->_mmaxhp + ((universe.gbMaxPlayers != 1 ? 100 : 50) << 6);
#else
		Monst->_mmaxhp = 3 * Monst->_mmaxhp + 64;
#endif
		Monst->mLevel += 15;
		Monst->_mhitpoints = Monst->_mmaxhp;
		Monst->mExp = 2 * (Monst->mExp + 1000);
		Monst->mMinDamage = 2 * (Monst->mMinDamage + 2);
		Monst->mMaxDamage = 2 * (Monst->mMaxDamage + 2);
		Monst->mMinDamage2 = 2 * (Monst->mMinDamage2 + 2);
		Monst->mMaxDamage2 = 2 * (Monst->mMaxDamage2 + 2);
	}

#ifdef HELLFIRE
	else if (universe.gnDifficulty == DIFF_HELL) {
#else
	if (universe.gnDifficulty == DIFF_HELL) {
#endif
#ifdef HELLFIRE
		Monst->_mmaxhp = 4 * Monst->_mmaxhp + ((universe.gbMaxPlayers != 1 ? 200 : 100) << 6);
#else
		Monst->_mmaxhp = 4 * Monst->_mmaxhp + 192;
#endif
		Monst->mLevel += 30;
		Monst->_mhitpoints = Monst->_mmaxhp;
		Monst->mExp = 4 * (Monst->mExp + 1000);
		Monst->mMinDamage = 4 * Monst->mMinDamage + 6;
		Monst->mMaxDamage = 4 * Monst->mMaxDamage + 6;
		Monst->mMinDamage2 = 4 * Monst->mMinDamage2 + 6;
		Monst->mMaxDamage2 = 4 * Monst->mMaxDamage2 + 6;
	}

	Monst->_uniqtrans = universe.uniquetrans++;

	if (Uniq->mUnqAttr & 4) {
		Monst->mHit = Uniq->mUnqVar1;
		Monst->mHit2 = Uniq->mUnqVar1;
	}
	if (Uniq->mUnqAttr & 8) {
		Monst->mArmorClass = Uniq->mUnqVar1;
	}

	universe.nummonsters++;

	if (Uniq->mUnqAttr & 1) {
		PlaceGroup(universe, miniontype, bosspacksize, Uniq->mUnqAttr, universe.nummonsters - 1);
	}

	if (Monst->_mAi != AI_GARG) {
		Monst->_mAnimData = Monst->MType->Anims[MA_STAND].Data[Monst->_mdir];
		Monst->_mAnimFrame = random_(universe, 88, Monst->_mAnimLen - 1) + 1;
		Monst->_mFlags &= ~MFLAG_ALLOW_SPECIAL;
		Monst->_mmode = MM_STAND;
	}
}

static void PlaceUniques(Universe& universe)
{
	int u, mt;
	BOOL done;

	for (u = 0; UniqMonst[u].mtype != -1; u++) {
		if (UniqMonst[u].mlevel != universe.currlevel)
			continue;
		done = FALSE;
		for (mt = 0; mt < universe.nummtypes; mt++) {
			if (done)
				break;
			done = (universe.Monsters[mt].mtype == UniqMonst[u].mtype);
		}
		mt--;
		if (u == UMT_GARBUD && universe.quests[Q_GARBUD]._qactive == QUEST_NOTAVAIL)
			done = FALSE;
		if (u == UMT_ZHAR && universe.quests[Q_ZHAR]._qactive == QUEST_NOTAVAIL)
			done = FALSE;
		if (u == UMT_SNOTSPIL && universe.quests[Q_LTBANNER]._qactive == QUEST_NOTAVAIL)
			done = FALSE;
		if (u == UMT_LACHDAN && universe.quests[Q_VEIL]._qactive == QUEST_NOTAVAIL)
			done = FALSE;
		if (u == UMT_WARLORD && universe.quests[Q_WARLORD]._qactive == QUEST_NOTAVAIL)
			done = FALSE;
		if (done)
			PlaceUniqueMonst(universe, u, mt, 8);
	}
}

void PlaceQuestMonsters(Universe& universe)
{
	int skeltype;
	BYTE *setp;

	if (!universe.setlevel) {
		if (QuestStatus(universe, Q_BUTCHER)) {
			PlaceUniqueMonst(universe, UMT_BUTCHER, 0, 0);
		}

		if (universe.currlevel == universe.quests[Q_SKELKING]._qlevel && universe.gbMaxPlayers != 1) {
			skeltype = 0;

			for (skeltype = 0; skeltype < universe.nummtypes; skeltype++) {
				if (IsSkel(universe.Monsters[skeltype].mtype)) {
					break;
				}
			}

			PlaceUniqueMonst(universe, UMT_SKELKING, skeltype, 30);
		}

		if (QuestStatus(universe, Q_LTBANNER)) {
			setp = LoadFileInMem("Levels\\L1Data\\Banner1.DUN", NULL);
			SetMapMonsters(universe, setp, 2 * universe.setpc_x, 2 * universe.setpc_y);
			mem_free_dbg(setp);
		}
		if (QuestStatus(universe, Q_BLOOD)) {
			setp = LoadFileInMem("Levels\\L2Data\\Blood2.DUN", NULL);
			SetMapMonsters(universe, setp, 2 * universe.setpc_x, 2 * universe.setpc_y);
			mem_free_dbg(setp);
		}
		if (QuestStatus(universe, Q_BLIND)) {
			setp = LoadFileInMem("Levels\\L2Data\\Blind2.DUN", NULL);
			SetMapMonsters(universe, setp, 2 * universe.setpc_x, 2 * universe.setpc_y);
			mem_free_dbg(setp);
		}
		if (QuestStatus(universe, Q_ANVIL)) {
			setp = LoadFileInMem("Levels\\L3Data\\Anvil.DUN", NULL);
			SetMapMonsters(universe, setp, 2 * universe.setpc_x + 2, 2 * universe.setpc_y + 2);
			mem_free_dbg(setp);
		}
		if (QuestStatus(universe, Q_WARLORD)) {
			setp = LoadFileInMem("Levels\\L4Data\\Warlord.DUN", NULL);
			SetMapMonsters(universe, setp, 2 * universe.setpc_x, 2 * universe.setpc_y);
			mem_free_dbg(setp);
			AddMonsterType(universe, UniqMonst[UMT_WARLORD].mtype, PLACE_SCATTER);
		}
		if (QuestStatus(universe, Q_VEIL)) {
			AddMonsterType(universe, UniqMonst[UMT_LACHDAN].mtype, PLACE_SCATTER);
		}
		if (QuestStatus(universe, Q_ZHAR) && zharlib == -1) {
			universe.quests[Q_ZHAR]._qactive = QUEST_NOTAVAIL;
		}

		if (universe.currlevel == universe.quests[Q_BETRAYER]._qlevel && universe.gbMaxPlayers != 1) {
			AddMonsterType(universe, UniqMonst[UMT_LAZURUS].mtype, PLACE_UNIQUE);
			AddMonsterType(universe, UniqMonst[UMT_RED_VEX].mtype, PLACE_UNIQUE);
			PlaceUniqueMonst(universe, UMT_LAZURUS, 0, 0);
			PlaceUniqueMonst(universe, UMT_RED_VEX, 0, 0);
			PlaceUniqueMonst(universe, UMT_BLACKJADE, 0, 0);
			setp = LoadFileInMem("Levels\\L4Data\\Vile1.DUN", NULL);
			SetMapMonsters(universe, setp, 2 * universe.setpc_x, 2 * universe.setpc_y);
			mem_free_dbg(setp);
		}
#ifdef HELLFIRE

		if (universe.currlevel == 24) {
			universe.UberDiabloMonsterIndex = -1;
			int i1;
			for (i1 = 0; i1 < universe.nummtypes; i1++) {
				if (universe.Monsters[i1].mtype == UniqMonst[UMT_NAKRUL].mtype)
					break;
			}

			if (i1 < universe.nummtypes) {
				for (int i2 = 0; i2 < universe.nummonsters; i2++) {
					if (universe.monster[i2]._uniqtype != 0 || universe.monster[i2]._mMTidx == i1) {
						universe.UberDiabloMonsterIndex = i2;
						break;
					}
				}
			}
			if (universe.UberDiabloMonsterIndex == -1)
				PlaceUniqueMonst(universe, UMT_NAKRUL, 0, 0);
		}
#endif
	} else if (universe.setlvlnum == SL_SKELKING) {
		PlaceUniqueMonst(universe, UMT_SKELKING, 0, 0);
	}
}
#endif

void PlaceGroup(Universe& universe, int mtype, int num, int leaderf, int leader)
{
	int placed, try1, try2, j;
	int xp, yp, x1, y1;

	placed = 0;

	for (try1 = 0; try1 < 10; try1++) {
		while (placed) {
			universe.nummonsters--;
			placed--;
			universe.dMonster[universe.monster[universe.nummonsters]._mx][universe.monster[universe.nummonsters]._my] = 0;
		}

		if (leaderf & 1) {
			int offset = random_(universe, 92, 8);
			x1 = xp = universe.monster[leader]._mx + offset_x[offset];
			y1 = yp = universe.monster[leader]._my + offset_y[offset];
		} else {
			do {
				x1 = xp = random_(universe, 93, 80) + 16;
				y1 = yp = random_(universe, 93, 80) + 16;
			} while (!MonstPlace(universe, xp, yp));
		}

		if (num + universe.nummonsters > universe.totalmonsters) {
			num = universe.totalmonsters - universe.nummonsters;
		}

		j = 0;
		for (try2 = 0; j < num && try2 < 100; xp += offset_x[random_(universe, 94, 8)], yp += offset_x[random_(universe, 94, 8)]) { /// BUGFIX: `yp += offset_y`
			if (!MonstPlace(universe, xp, yp)
			    || (universe.dTransVal[xp][yp] != universe.dTransVal[x1][y1])
			    || (leaderf & 2) && ((abs(xp - x1) >= 4) || (abs(yp - y1) >= 4))) {
				try2++;
				continue;
			}

			PlaceMonster(universe, universe.nummonsters, mtype, xp, yp);
			if (leaderf & 1) {
				universe.monster[universe.nummonsters]._mmaxhp *= 2;
				universe.monster[universe.nummonsters]._mhitpoints = universe.monster[universe.nummonsters]._mmaxhp;
				universe.monster[universe.nummonsters]._mint = universe.monster[leader]._mint;

				if (leaderf & 2) {
					universe.monster[universe.nummonsters].leader = leader;
					universe.monster[universe.nummonsters].leaderflag = 1;
					universe.monster[universe.nummonsters]._mAi = universe.monster[leader]._mAi;
				}

				if (universe.monster[universe.nummonsters]._mAi != AI_GARG) {
					universe.monster[universe.nummonsters]._mAnimData = universe.monster[universe.nummonsters].MType->Anims[MA_STAND].Data[universe.monster[universe.nummonsters]._mdir];
					universe.monster[universe.nummonsters]._mAnimFrame = random_(universe, 88, universe.monster[universe.nummonsters]._mAnimLen - 1) + 1;
					universe.monster[universe.nummonsters]._mFlags &= ~MFLAG_ALLOW_SPECIAL;
					universe.monster[universe.nummonsters]._mmode = MM_STAND;
				}
			}
			universe.nummonsters++;
			placed++;
			j++;
		}

		if (placed >= num) {
			break;
		}
	}

	if (leaderf & 2) {
		universe.monster[leader].packsize = placed;
	}
}

#ifndef SPAWN
void LoadDiabMonsts(Universe& universe)
{
	BYTE *lpSetPiece;

	lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab1.DUN", NULL);
	SetMapMonsters(universe, lpSetPiece, 2 * universe.diabquad1x, 2 * universe.diabquad1y);
	mem_free_dbg(lpSetPiece);
	lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab2a.DUN", NULL);
	SetMapMonsters(universe, lpSetPiece, 2 * universe.diabquad2x, 2 * universe.diabquad2y);
	mem_free_dbg(lpSetPiece);
	lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab3a.DUN", NULL);
	SetMapMonsters(universe, lpSetPiece, 2 * universe.diabquad3x, 2 * universe.diabquad3y);
	mem_free_dbg(lpSetPiece);
	lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab4a.DUN", NULL);
	SetMapMonsters(universe, lpSetPiece, 2 * universe.diabquad4x, 2 * universe.diabquad4y);
	mem_free_dbg(lpSetPiece);
}
#endif

void InitMonsters(Universe& universe)
{
	int na, nt;
	int i, s, t;
	int numplacemonsters;
	int mtype;
	int numscattypes;
	int scattertypes[NUM_MTYPES];

	numscattypes = 0;
	if (!universe.setlevel) {
		AddMonster(universe, 1, 0, 0, 0, FALSE);
		AddMonster(universe, 1, 0, 0, 0, FALSE);
		AddMonster(universe, 1, 0, 0, 0, FALSE);
		AddMonster(universe, 1, 0, 0, 0, FALSE);
	}
#ifndef SPAWN
	if (!universe.setlevel && universe.currlevel == 16)
		LoadDiabMonsts(universe);
#endif
	nt = numtrigs;
	if (universe.currlevel == 15)
		nt = 1;
	for (i = 0; i < nt; i++) {
		for (s = -2; s < 2; s++) {
			for (t = -2; t < 2; t++)
				DoVision(universe, s + trigs[i]._tx, t + trigs[i]._ty, 15, FALSE, FALSE);
		}
	}
#ifndef SPAWN
	PlaceQuestMonsters(universe);
#endif
	if (!universe.setlevel) {
#ifndef SPAWN
		PlaceUniques(universe);
#endif
		na = 0;
		for (s = 16; s < 96; s++)
			for (t = 16; t < 96; t++)
				if (!SolidLoc(universe, s, t))
					na++;
		numplacemonsters = na / 30;
		if (universe.gbMaxPlayers != 1)
			numplacemonsters += numplacemonsters >> 1;
		if (universe.nummonsters + numplacemonsters > MAXMONSTERS - 10)
			numplacemonsters = MAXMONSTERS - 10 - universe.nummonsters;
		universe.totalmonsters = universe.nummonsters + numplacemonsters;
		for (i = 0; i < universe.nummtypes; i++) {
			if (universe.Monsters[i].mPlaceFlags & PLACE_SCATTER) {
				scattertypes[numscattypes] = i;
				numscattypes++;
			}
		}
		while (universe.nummonsters < universe.totalmonsters) {
			mtype = scattertypes[random_(universe, 95, numscattypes)];
			if (universe.currlevel == 1 || random_(universe, 95, 2) == 0)
				na = 1;
#ifdef HELLFIRE
			else if (universe.currlevel == 2 || universe.currlevel >= 21 && universe.currlevel <= 24)
#else
			else if (universe.currlevel == 2)
#endif
				na = random_(universe, 95, 2) + 2;
			else
				na = random_(universe, 95, 3) + 3;
			PlaceGroup(universe, mtype, na, 0, 0);
		}
	}
	for (i = 0; i < nt; i++) {
		for (s = -2; s < 2; s++) {
			for (t = -2; t < 2; t++)
				DoUnVision(universe, s + trigs[i]._tx, t + trigs[i]._ty, 15);
		}
	}
}

#ifndef SPAWN
void SetMapMonsters(Universe& universe, BYTE *pMap, int startx, int starty)
{
	WORD rw, rh;
	WORD *lm;
	int i, j;
	int mtype;

	AddMonsterType(universe, MT_GOLEM, PLACE_SPECIAL);
	// See https://github.com/diasurgical/devilutionX/pull/2822
	AddMonster(universe, 1, 0, 0, 0, FALSE); // BUGFIX: add only if universe.setlevel is true
	AddMonster(universe, 1, 0, 0, 0, FALSE); // BUGFIX: add only if universe.setlevel is true
	AddMonster(universe, 1, 0, 0, 0, FALSE); // BUGFIX: add only if universe.setlevel is true
	AddMonster(universe, 1, 0, 0, 0, FALSE); // BUGFIX: add only if universe.setlevel is true
	if (universe.setlevel && universe.setlvlnum == SL_VILEBETRAYER) {
		AddMonsterType(universe, UniqMonst[UMT_LAZURUS].mtype, PLACE_UNIQUE);
		AddMonsterType(universe, UniqMonst[UMT_RED_VEX].mtype, PLACE_UNIQUE);
		AddMonsterType(universe, UniqMonst[UMT_BLACKJADE].mtype, PLACE_UNIQUE);
		PlaceUniqueMonst(universe, UMT_LAZURUS, 0, 0);
		PlaceUniqueMonst(universe, UMT_RED_VEX, 0, 0);
		PlaceUniqueMonst(universe, UMT_BLACKJADE, 0, 0);
	}
	lm = (WORD *)pMap;
	rw = *lm++;
	rh = *lm++;
	lm += rw * rh;
	rw = rw << 1;
	rh = rh << 1;
	lm += rw * rh;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*lm != 0) {
				mtype = AddMonsterType(universe, MonstConvTbl[(*lm) - 1], PLACE_SPECIAL);
				PlaceMonster(universe, universe.nummonsters++, mtype, i + startx + 16, j + starty + 16);
			}
			lm++;
		}
	}
}
#endif

void DeleteMonster(Universe& universe, int i)
{
	int temp;

	universe.nummonsters--;
	temp = universe.monstactive[universe.nummonsters];
	universe.monstactive[universe.nummonsters] = universe.monstactive[i];
	universe.monstactive[i] = temp;
}

int AddMonster(Universe& universe, int x, int y, int dir, int mtype, BOOL InMap)
{
	if (universe.nummonsters < MAXMONSTERS) {
		int i = universe.monstactive[universe.nummonsters++];
		if (InMap)
			universe.dMonster[x][y] = i + 1;
		InitMonster(universe, i, dir, mtype, x, y);
		return i;
	}

	return -1;
}

BOOL PosOkMonst(Universe& universe, int i, int x, int y)
{
#ifdef HELLFIRE
	int oi;
	BOOL ret;

	ret = !SolidLoc(universe, x, y) && universe.dPlayer[x][y] == 0 && universe.dMonster[x][y] == 0;
	oi = universe.dObject[x][y];
	if (ret && oi != 0) {
		oi = oi > 0 ? oi - 1 : -(oi + 1);
		if (universe.object[oi]._oSolidFlag)
			ret = FALSE;
	}

	if (ret)
		ret = monster_posok(i, x, y);
#else
	int oi, mi, j;
	BOOL ret, fire;

	fire = FALSE;
	ret = !SolidLoc(universe, x, y) && universe.dPlayer[x][y] == 0 && universe.dMonster[x][y] == 0;
	if (ret && universe.dObject[x][y] != 0) {
		oi = universe.dObject[x][y] > 0 ? universe.dObject[x][y] - 1 : -(universe.dObject[x][y] + 1);
		if (universe.object[oi]._oSolidFlag)
			ret = FALSE;
	}

	//if (ret && dMissile[x][y] != 0 && i >= 0) {
	//	// BUGFIX: case with multiple missiles being present on (x, y)-coordinate not handled.
	//	mi = dMissile[x][y];
	//	if (mi > 0) {
	//		if (missile[mi]._mitype == MIS_FIREWALL) { // BUGFIX: Change 'mi' to 'mi - 1'
	//			fire = TRUE;
	//		} else {
	//			for (j = 0; j < nummissiles; j++) {
	//				if (missile[missileactive[j]]._mitype == MIS_FIREWALL) // BUGFIX: Check missile x/y
	//					fire = TRUE;
	//			}
	//		}
	//	}
	//	if (fire && (!(universe.monster[i].mMagicRes & IMMUNE_FIRE) || universe.monster[i].MType->mtype == MT_DIABLO))
	//		ret = FALSE;
	//}
#endif

	return ret;
}

//BOOL PosOkMonst2(Universe& universe, int i, int x, int y)
//{
//	int oi, mi, j;
//#ifdef HELLFIRE
//	BOOL ret;
//
//	oi = universe.dObject[x][y];
//	ret = !SolidLoc(universe, x, y);
//	if (ret && oi != 0) {
//		oi = oi > 0 ? oi - 1 : -(oi + 1);
//		if (universe.object[oi]._oSolidFlag)
//			ret = FALSE;
//	}
//
//	if (ret)
//		ret = monster_posok(i, x, y);
//#else
//	BOOL ret, fire;
//
//	fire = FALSE;
//	ret = !SolidLoc(universe, x, y);
//	if (ret && universe.dObject[x][y] != 0) {
//		oi = universe.dObject[x][y] > 0 ? universe.dObject[x][y] - 1 : -(universe.dObject[x][y] + 1);
//		if (universe.object[oi]._oSolidFlag)
//			ret = FALSE;
//	}
//
//	if (ret && dMissile[x][y] != 0 && i >= 0) {
//		mi = dMissile[x][y];
//		// BUGFIX: case with multiple missiles being present on (x, y)-coordinate not handled.
//		if (mi > 0) {
//			if (missile[mi]._mitype == MIS_FIREWALL) { // BUGFIX: Change 'mi' to 'mi - 1'
//				fire = TRUE;
//			} else {
//				for (j = 0; j < nummissiles; j++) {
//					if (missile[missileactive[j]]._mitype == MIS_FIREWALL) // BUGFIX: Check missile x/y
//						fire = TRUE;
//				}
//			}
//		}
//		if (fire && (!(universe.monster[i].mMagicRes & IMMUNE_FIRE) || universe.monster[i].MType->mtype == MT_DIABLO))
//			ret = FALSE;
//	}
//#endif
//
//	return ret;
//}
//
//BOOL PosOkMonst3(int i, int x, int y)
//{
//	int j, oi, objtype, mi;
//#ifdef HELLFIRE
//	BOOL ret;
//	DIABOOL isdoor;
//
//	ret = TRUE;
//	isdoor = FALSE;
//
//	oi = universe.dObject[x][y];
//	if (ret && oi != 0) {
//		oi = oi > 0 ? oi - 1 : -(oi + 1);
//		objtype = universe.object[oi]._otype;
//		isdoor = objtype == OBJ_L1LDOOR || objtype == OBJ_L1RDOOR
//		    || objtype == OBJ_L2LDOOR || objtype == OBJ_L2RDOOR
//		    || objtype == OBJ_L3LDOOR || objtype == OBJ_L3RDOOR;
//		if (universe.object[oi]._oSolidFlag && !isdoor) {
//			ret = FALSE;
//		}
//	}
//	if (ret) {
//		ret = (!SolidLoc(universe, x, y) || isdoor) && universe.dPlayer[x][y] == 0 && universe.dMonster[x][y] == 0;
//	}
//	if (ret)
//		ret = monster_posok(i, x, y);
//#else
//	BOOL ret, fire, isdoor;
//
//	fire = FALSE;
//	ret = TRUE;
//	isdoor = FALSE;
//
//	if (ret && universe.dObject[x][y] != 0) {
//		oi = universe.dObject[x][y] > 0 ? universe.dObject[x][y] - 1 : -(universe.dObject[x][y] + 1);
//		objtype = universe.object[oi]._otype;
//		isdoor = objtype == OBJ_L1LDOOR || objtype == OBJ_L1RDOOR
//		    || objtype == OBJ_L2LDOOR || objtype == OBJ_L2RDOOR
//		    || objtype == OBJ_L3LDOOR || objtype == OBJ_L3RDOOR;
//		if (universe.object[oi]._oSolidFlag && !isdoor) {
//			ret = FALSE;
//		}
//	}
//	if (ret) {
//		ret = (!SolidLoc(universe, x, y) || isdoor) && universe.dPlayer[x][y] == 0 && universe.dMonster[x][y] == 0;
//	}
//	if (ret && dMissile[x][y] != 0 && i >= 0) {
//		mi = dMissile[x][y];
//		// BUGFIX: case with multiple missiles being present on (x, y)-coordinate not handled.
//		if (mi > 0) {
//			if (missile[mi]._mitype == MIS_FIREWALL) { // BUGFIX: Change 'mi' to 'mi - 1'
//				fire = TRUE;
//			} else {
//				for (j = 0; j < nummissiles; j++) {
//					if (missile[missileactive[j]]._mitype == MIS_FIREWALL) // BUGFIX: Check missile x/y
//						fire = TRUE;
//				}
//			}
//		}
//		if (fire && (!(universe.monster[i].mMagicRes & IMMUNE_FIRE) || universe.monster[i].MType->mtype == MT_DIABLO))
//			ret = FALSE;
//	}
//#endif
//
//	return ret;
//}

BOOL IsSkel(int mt)
{
	return mt >= MT_WSKELAX && mt <= MT_XSKELAX
	    || mt >= MT_WSKELBW && mt <= MT_XSKELBW
	    || mt >= MT_WSKELSD && mt <= MT_XSKELSD;
}

BOOL IsGoat(int mt)
{
	return mt >= MT_NGOATMC && mt <= MT_GGOATMC
	    || mt >= MT_NGOATBW && mt <= MT_GGOATBW;
}

//int M_SpawnSkel(int x, int y, int dir)
//{
//	int i, j, skeltypes, skel;
//
//	j = 0;
//	for (i = 0; i < universe.nummtypes; i++) {
//		if (IsSkel(universe.Monsters[i].mtype))
//			j++;
//	}
//
//	if (j) {
//		skeltypes = random_(universe, 136, j);
//		j = 0;
//		for (i = 0; i < universe.nummtypes && j <= skeltypes; i++) {
//			if (IsSkel(universe.Monsters[i].mtype))
//				j++;
//		}
//		skel = AddMonster(universe, x, y, dir, i - 1, TRUE);
//		if (skel != -1)
//			M_StartSpStand(skel, dir);
//
//		return skel;
//	}
//
//	return -1;
//}

void ActivateSpawn(Universe& universe, int i, int x, int y, int dir)
{
	universe.dMonster[x][y] = i + 1;
	universe.monster[i]._mx = x;
	universe.monster[i]._my = y;
	universe.monster[i]._mfutx = x;
	universe.monster[i]._mfuty = y;
	universe.monster[i]._moldx = x;
	universe.monster[i]._moldy = y;
}

BOOL SpawnSkeleton(Universe& universe, int ii, int x, int y)
{
	int dx, dy, xx, yy, dir, j, k, rs;
	BOOL savail;
	int monstok[3][3];

	if (ii == -1)
		return FALSE;

	if (PosOkMonst(universe, -1, x, y)) {
		dir = DIR_S;
		ActivateSpawn(universe, ii, x, y, dir);
		return TRUE;
	}

	savail = FALSE;
	yy = 0;
	for (j = y - 1; j <= y + 1; j++) {
		xx = 0;
		for (k = x - 1; k <= x + 1; k++) {
			monstok[xx][yy] = PosOkMonst(universe, -1, k, j);
			savail |= monstok[xx][yy];
			xx++;
		}
		yy++;
	}
	if (!savail) {
		return FALSE;
	}

	rs = random_(universe, 137, 15) + 1;
	xx = 0;
	yy = 0;
	while (rs > 0) {
		if (monstok[xx][yy])
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

	dx = x - 1 + xx;
	dy = y - 1 + yy;
	dir = DIR_S;
	ActivateSpawn(universe, ii, dx, dy, dir);

	return TRUE;
}

int PreSpawnSkeleton(Universe& universe)
{
	int i, j, skeltypes, skel;

	j = 0;

	for (i = 0; i < universe.nummtypes; i++) {
		if (IsSkel(universe.Monsters[i].mtype))
			j++;
	}

	if (j) {
		skeltypes = random_(universe, 136, j);
		j = 0;
		for (i = 0; i < universe.nummtypes && j <= skeltypes; i++) {
			if (IsSkel(universe.Monsters[i].mtype))
				j++;
		}
		skel = AddMonster(universe, 0, 0, 0, i - 1, FALSE);
		//if (skel != -1)
		//	M_StartStand(skel, 0);

		return skel;
	}

	return -1;
}
