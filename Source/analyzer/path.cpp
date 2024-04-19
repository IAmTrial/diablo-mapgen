#include "path.h"

#include <cstdint>
#include <iostream>
#include <optional>

#include "types.h"
#include "Source/analyzer/puzzler.h"
#include "Source/funkMapGen.h"
#include "Source/objects.h"
#include "Source/path.h"
#include "Source/quests.h"
#include "Source/universe/universe.h"

namespace {

#define MAXVIEWX 21
#define MAXVIEWY 21

const bool isVisible[MAXVIEWY][MAXVIEWX] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //	-y
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
	{ 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0 },
	{ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, // -x	origin(10,10)	+x
	{ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
	{ 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //	+y
};

Point StairsDownPrevious;

BOOL PosOkPlayer(Universe& universe, int pnum, int x, int y)
{
	if (x < 0 || y < 0 || x >= MAXDUNX || y >= MAXDUNY)
		return FALSE;
	int tileId = universe.dPiece[x][y];
	if (tileId == 0)
		return FALSE;
	if (universe.nSolidTable[tileId])
		return FALSE;

	if (universe.dObject[x][y] != 0) {
		ObjectStruct *obj = &universe.object[abs(universe.dObject[x][y]) - 1];
		if (obj->_oSolidFlag && !obj->_oBreak) {
			return FALSE;
		}
	}

	return TRUE;
}

int PathLength(Universe& universe, Point start, Point end)
{
	return FindPath(universe, PosOkPlayer, 0, start.x, start.y, end.x, end.y, universe.Path);
}

/**
 * Unused
 */
bool IsVisible(Point start, Point end)
{
	if (start == Point { -1, -1 } || end == Point { -1, -1 }) {
		return false;
	}

	int horizontal = end.x - start.x + 10;
	int vertical = end.y - start.y + 10;

	if (horizontal < 0 || horizontal > MAXVIEWX)
		return false;
	if (vertical < 0 || vertical > MAXVIEWY)
		return false;
	return isVisible[vertical][horizontal];
}

int TotalTickLenth;

std::string formatTime()
{
	float time = (float)TotalTickLenth / 20;
	int min = time / 60;
	char fmt[12];
	sprintf(fmt, "%u:%05.2lf", min, time - min * 60);

	return fmt;
}

int CalcStairsChebyshevDistance(Point start, Point end)
{
	if (start == Point { -1, -1 } || end == Point { -1, -1 })
		return -1;

	int horizontal = std::max(start.x, end.x) - std::min(start.x, end.x);
	int vertical = std::max(start.y, end.y) - std::min(start.y, end.y);

	return std::max(horizontal, vertical);
}

int GetWalkTime(Universe& universe, Point start, Point end)
{
	constexpr int ticksToWalkATile = 8;

	int cDistance = CalcStairsChebyshevDistance(start, end);
	if (cDistance == -1 || cDistance > MAX_PATH_LENGTH)
		return -1;

	int stairsPath = PathLength(universe, start, end);
	if (stairsPath == 0)
		return -1;

	return stairsPath * ticksToWalkATile;
}

int GetTeleportTime(Point start, Point end)
{
	constexpr int ticksToTeleport = 12;

	int cDistance = CalcStairsChebyshevDistance(start, end);
	if (cDistance == -1) {
		return -1;
	}

	if (IsVisible(start, end))
		cDistance = 5;

	/** @todo Take teleport limits in to considerations instead of just estimating 5 steps */
	return cDistance * ticksToTeleport / 5;
}

int GetShortestTeleportTime(Point startA, Point startB, Point end)
{
	int teleportTime = GetTeleportTime(startA, end);
	int teleportTimePrevious = GetTeleportTime(startB, end);
	if (teleportTime == -1)
		teleportTime = teleportTimePrevious;
	if (teleportTime == -1)
		return -1;

	return std::min(teleportTime, teleportTimePrevious);
}

void setDoorSolidState(Universe& universe, BOOLEAN doorState)
{
	if (universe.leveltype == DTYPE_CATHEDRAL) {
		universe.nSolidTable[44] = doorState;
		universe.nSolidTable[46] = doorState;
		universe.nSolidTable[51] = doorState;
		universe.nSolidTable[56] = doorState;
		universe.nSolidTable[214] = doorState;
		universe.nSolidTable[270] = doorState;
	} else if (universe.leveltype == DTYPE_CATACOMBS) {
		universe.nSolidTable[55] = doorState;
		universe.nSolidTable[58] = doorState;
		universe.nSolidTable[538] = doorState;
		universe.nSolidTable[540] = doorState;
	} else if (universe.leveltype == DTYPE_CAVES) {
		universe.nSolidTable[531] = doorState;
		universe.nSolidTable[534] = doorState;
	}
}

bool IsGoodLevelSoursororStrategy(Universe& universe)
{
	int tickLenth = 0;
	tickLenth += 20; // Load screens

	if (universe.currlevel < 9) {
		int walkTicks = GetWalkTime(universe, universe.Spawn, universe.StairsDown);
		if (walkTicks == -1) {
			if (Config.verbose)
				std::cerr << "Path: Gave up on walking to the stairs" << std::endl;
			return false;
		}
		tickLenth += walkTicks;
	} else if (universe.currlevel == 9) {
		LocatePuzzler(universe);
		int pathToPuzzler = -1;
		if (universe.POI != Point { -1, -1 }) {
			int walkTicks = GetWalkTime(universe, universe.Spawn, universe.POI);
			if (walkTicks != -1) {
				int teleportTime = GetTeleportTime(universe.Spawn, universe.StairsDown);
				if (teleportTime != -1) {
					pathToPuzzler += walkTicks;
					pathToPuzzler += 40; // Pick up Puzzler
					pathToPuzzler += teleportTime;
				}
			}
		}

		if (pathToPuzzler != -1) {
			if (Config.verbose)
				std::cerr << "Path: Found Naj's Puzzler" << std::endl;
			tickLenth += pathToPuzzler;
		} else {
			if (Config.verbose)
				std::cerr << "Path: Went to town to get a book of teleport" << std::endl;
			tickLenth += 880; // Buying a book of teleport
			int walkTicks = GetTeleportTime(universe.Spawn, universe.StairsDown);
			if (walkTicks == -1) {
				if (Config.verbose)
					std::cerr << "Path: Couldn't find the stairs" << std::endl;
				return false;
			}
			tickLenth += walkTicks;
		}

		tickLenth += 1100; // Fight monsters to get Puzzler or level up to read the book
	} else if (universe.currlevel == 15) {
		Point target = { -1, -1 };

		// Locate Lazarus staff
		for (int i = 0; i < universe.nobjects; i++) {
			int oid = universe.objectactive[i];
			ObjectStruct stand = universe.object[oid];
			if (stand._otype != OBJ_LAZSTAND)
				continue;
			target = { stand._ox, stand._oy };
		}

		// Locate Lazarous warp
		if (universe.POI != Point { -1, -1 }) {
			if (Config.verbose)
				std::cerr << "Path: Found the warp to Lazarus" << std::endl;
			target = universe.POI;
		}

		int teleportTime = GetShortestTeleportTime(universe.Spawn, StairsDownPrevious, target);
		if (teleportTime == -1) {
			if (Config.verbose)
				std::cerr << "Path: Couldn't find the stairs" << std::endl;
			return false;
		}
		tickLenth += teleportTime;

		if (universe.POI == Point { -1, -1 }) {
			if (Config.verbose)
				std::cerr << "Path: Told Cain about Lazarus" << std::endl;
			tickLenth += 20;  // Pick up staff
			tickLenth += 580; // Show staff to Cain
		}

		tickLenth += 460; // Defeat Lazarus

		int teleportTime2 = GetTeleportTime(target, universe.StairsDown);
		if (teleportTime2 == -1) {
			if (Config.verbose)
				std::cerr << "Path: Couldn't find the stairs" << std::endl;
			return false;
		}
		tickLenth += teleportTime2;
	} else {
		int teleportTime = GetShortestTeleportTime(universe.Spawn, StairsDownPrevious, universe.StairsDown);
		if (teleportTime == -1) {
			if (Config.verbose)
				std::cerr << "Path: Couldn't find the stairs" << std::endl;
			return false;
		}
		tickLenth += teleportTime;

		if (universe.currlevel == 16) {
			tickLenth += 180; // Defeat Diablo
		}
	}

	TotalTickLenth += tickLenth;

	if (Config.verbose)
		std::cerr << "Path: Compleated dlvl " << (int)universe.currlevel << " @ " << formatTime() << std::endl;

	if (TotalTickLenth > *Config.target * 20) {
		if (Config.verbose)
			std::cerr << "Path: It's to slow to beat this one, giving up" << std::endl;
		return false;
	}

	return true;
}

bool IsGoodLevel(Universe& universe)
{
	setDoorSolidState(universe, FALSE); // Open doors
	bool result = IsGoodLevelSoursororStrategy(universe);
	setDoorSolidState(universe, TRUE); // Close doors

	return result;
}

bool Ended;

}

ScannerPath::ScannerPath(Universe& universe) : Scanner(universe) {}

bool ScannerPath::skipSeed()
{
	if (universe.quests[Q_LTBANNER]._qactive != QUEST_NOTAVAIL) {
		if (Config.verbose)
			std::cerr << "Game Seed: " << universe.sgGameInitInfo.dwSeed << " thrown out: Sign Quest" << std::endl;
		return true;
	}

	TotalTickLenth = 0;
	TotalTickLenth += 435;  // Walk to Adria
	TotalTickLenth += 1400; // Dup gold
	TotalTickLenth += 250;  // Buy to books
	TotalTickLenth += 200;  // Buy full manas
	TotalTickLenth += 540;  // Walk to church
	Ended = false;

	return false;
}

bool ScannerPath::skipLevel(int level)
{
	return Ended;
}

bool ScannerPath::levelMatches(std::optional<uint32_t> levelSeed)
{
	std::memset(universe.Path, 0, sizeof(universe.Path));
	if (!IsGoodLevel(universe)) {
		Ended = true;
		return Config.verbose;
	}

	StairsDownPrevious = universe.StairsDown;

	int level = universe.currlevel;
	if (level == 16) {
		std::cout << universe.sgGameInitInfo.dwSeed << " (etc " << formatTime() << ")" << std::endl;
		Ended = true;
	}

	return true;
}
