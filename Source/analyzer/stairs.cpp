#include "stairs.h"

#include <iostream>

#include "types.h"
#include "Source/funkMapGen.h"
#include "Source/objects.h"
#include "Source/path.h"
#include "Source/quests.h"
#include "Source/universe/universe.h"

namespace {

int CalcStairsChebyshevDistance(Point start, Point end)
{
	if (start == Point { -1, -1 } || end == Point { -1, -1 })
		return -1;

	int horizontal = std::max(start.x, end.x) - std::min(start.x, end.x);
	int vertical = std::max(start.y, end.y) - std::min(start.y, end.y);

	return std::max(horizontal, vertical);
}

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
		char bv;
		if (universe.dObject[x][y] > 0) {
			bv = universe.dObject[x][y] - 1;
		} else {
			bv = -(universe.dObject[x][y] + 1);
		}
		if (universe.object[bv]._oSolidFlag) {
			return FALSE;
		}
	}

	return TRUE;
}

int PathLength(Universe& universe, Point start, Point end)
{
	return FindPath(universe, PosOkPlayer, 0, start.x, start.y, end.x, end.y, Path);
}

int GetDistance(Universe& universe, Point start, Point end, int maxDistance)
{
	int cDistance = CalcStairsChebyshevDistance(start, end);
	if (cDistance == -1 || cDistance > maxDistance)
		return -1;

	int stairsPath = PathLength(universe, start, end);
	if (stairsPath == 0 || stairsPath > maxDistance)
		return -1;

	return stairsPath;
}

bool Failed;

}

ScannerStairs::ScannerStairs(Universe& universe) : Scanner(universe) {}

DungeonMode ScannerStairs::getDungeonMode()
{
	return DungeonMode::NoContent;
}

bool ScannerStairs::skipSeed()
{
	Failed = false;

	return false;
}

bool ScannerStairs::skipLevel(int level)
{
	return Failed || level > 9;
}

bool ScannerStairs::levelMatches(std::optional<uint32_t> levelSeed)
{
	int maxDistance = 3;
	if (universe.leveltype == DTYPE_CATHEDRAL)
		maxDistance = 7;
	else if (universe.leveltype == DTYPE_CATACOMBS)
		maxDistance = 5;

	std::memset(Path, 0, sizeof(Path));
	int steps = GetDistance(universe, Spawn, StairsDown, maxDistance);
	if (steps == -1) {
		if (Config.verbose)
			std::cerr << "Stairs: Gave up on walking to the stairs on " << (int)universe.currlevel << std::endl;
		Failed = true;
		return false;
	}

	if (universe.currlevel >= 6)
		std::cout << universe.sgGameInitInfo.dwSeed << std::endl;

	return true;
}
