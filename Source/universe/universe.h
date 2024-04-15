
#pragma once

/**
 * State of the universe. (aka all global variables)
 */

#include "defs.h"
#include "enums.h"
#include "structs.h"
#include "types.h"

struct Universe {
	constexpr Universe() = default;

	// Remove the copy constructor and operator, preventing accidental copies
	Universe(Universe&) = delete;
	Universe& operator=(Universe&) = delete;

	constexpr Universe(Universe&&) noexcept = default;
	Universe& operator=(Universe&&) noexcept = default;

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

	// drlg_l3.cpp
#ifndef SPAWN
	/** This will be true if a lava pool has been generated for the level */
	BOOLEAN lavapool;
	/** unused */
	int abyssx;
	int lockoutcnt;
	BOOLEAN lockout[DMAXX][DMAXY];
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

	// engine.h
	BYTE gbMaxPlayers = 1;
	BOOL leveldebug = false;
	BOOL light4flag;
	DWORD glSeedTbl[NUMLEVELS];
	_gamedata sgGameInitInfo;
	PlayerStruct plr[MAX_PLRS];
	bool zoomflag = false;
	int questdebug = -1;
	bool oobread = false;
	bool oobwrite = false;

	// engine.cpp
	/** Seed value before the most recent call to SetRndSeed() */
	int orgseed;
	/** Current game seed */
	int sglGameSeed;
	/** Number of times the current seed has been fetched */
	int SeedCount;

	// gendung.h
	/** Contains the tile IDs of the map. */
	BYTE dungeon[DMAXX][DMAXY];
	/** Contains a backup of the tile IDs of the map. */
	BYTE pdungeon[DMAXX][DMAXY];
	char dflags[DMAXX][DMAXY];
	/** Specifies the active set level X-coordinate of the map. */
	int setpc_x;
	/** Specifies the active set level Y-coordinate of the map. */
	int setpc_y;
	/** Specifies the width of the active set level of the map. */
	int setpc_w;
	/** Specifies the height of the active set level of the map. */
	int setpc_h;
	/** Specifies whether a single player quest DUN has been loaded. */
	BOOL setloadflag;
	/**
	 * Returns the frame number of the speed CEL, an in memory decoding
	 * of level CEL frames, based on original frame number and light index.
	 * Note, given light index 0, the original frame number is returned.
	 */
	int SpeedFrameTbl[128][16];
	/**
	 * List of transparancy masks to use for dPieces
	 */
	char block_lvid[MAXTILES + 1];
	/**
	 * List of light blocking dPieces
	 */
	BOOLEAN nBlockTable[MAXTILES + 1];
	/**
	 * List of path blocking dPieces
	 */
	BOOLEAN nSolidTable[MAXTILES + 1];
	/**
	 * List of transparent dPieces
	 */
	BOOLEAN nTransTable[MAXTILES + 1];
	/**
	 * List of missile blocking dPieces
	 */
	BOOLEAN nMissileTable[MAXTILES + 1];
	BOOLEAN nTrapTable[MAXTILES + 1];
	/** Specifies the minimum X-coordinate of the map. */
	int dminx;
	/** Specifies the minimum Y-coordinate of the map. */
	int dminy;
	/** Specifies the maximum X-coordinate of the map. */
	int dmaxx;
	/** Specifies the maximum Y-coordinate of the map. */
	int dmaxy;
	int gnDifficulty;
	/** Specifies the active dungeon type of the current game. */
	BYTE leveltype;
	/** Specifies the active dungeon level of the current game. */
	BYTE currlevel;
	BOOLEAN setlevel;
	/** Specifies the active quest level of the current game. */
	BYTE setlvlnum;
	char setlvltype;
	/** Specifies the player viewpoint X-coordinate of the map. */
	int ViewX;
	/** Specifies the player viewpoint Y-coordinate of the map. */
	int ViewY;
	int ViewBX;
	int ViewBY;
	int ViewDX;
	int ViewDY;
	ScrollStruct ScrollInfo;
	/** Specifies the level viewpoint X-coordinate of the map. */
	int LvlViewX;
	/** Specifies the level viewpoint Y-coordinate of the map. */
	int LvlViewY;
	int MicroTileLen;
	char TransVal;
	/** Specifies the active transparency indices. */
	BOOLEAN TransList[256];
	/** Contains the piece IDs of each tile on the map. */
	int dPiece[MAXDUNX][MAXDUNY];
	/** Specifies the dungeon piece information for a given coordinate and block number. */
	MICROS dpiece_defs_map_2[MAXDUNX][MAXDUNY];
	/** Specifies the dungeon piece information for a given coordinate and block number, optimized for diagonal access. */
	MICROS dpiece_defs_map_1[MAXDUNX * MAXDUNY];
	/** Specifies the transparency at each coordinate of the map. */
	char dTransVal[MAXDUNX][MAXDUNY];
	char dLight[MAXDUNX][MAXDUNY];
	char dPreLight[MAXDUNX][MAXDUNY];
	char dFlags[MAXDUNX][MAXDUNY];
	/** Contains the player numbers (players array indices) of the map. */
	char dPlayer[MAXDUNX][MAXDUNY];
	/**
	 * Contains the NPC numbers of the map. The NPC number represents a
	 * towner number (towners array index) in Tristram and a monster number
	 * (monsters array index) in the dungeon.
	 */
	int dMonster[MAXDUNX][MAXDUNY];
	/**
	 * Contains the dead numbers (deads array indices) and dead direction of
	 * the map, encoded as specified by the pseudo-code below.
	 * dDead[x][y] & 0x1F - index of dead
	 * dDead[x][y] >> 0x5 - direction
	 */
	char dDead[MAXDUNX][MAXDUNY];
	/** Contains the object numbers (objects array indices) of the map. */
	char dObject[MAXDUNX][MAXDUNY];
	/** Contains the item numbers (items array indices) of the map. */
	char dItem[MAXDUNX][MAXDUNY];
	/** Contains the missile numbers (missiles array indices) of the map. */
	char dMissile[MAXDUNX][MAXDUNY];
	/**
	 * Contains the arch frame numbers of the map from the special tileset
	 * (e.g. "levels/l1data/l1s.cel"). Note, the special tileset of Tristram (i.e.
	 * "levels/towndata/towns.cel") contains trees rather than arches.
	 */
	char dSpecial[MAXDUNX][MAXDUNY];
	int themeCount;
	THEME_LOC themeLoc[MAXTHEMES];
};
