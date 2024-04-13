#include "Source/analyzer/warp.h"

#include <cstdint>
#include <iostream>
#include <optional>

#include "types.h"
#include "Source/funkMapGen.h"
#include "Source/universe/universe.h"

ScannerWarp::ScannerWarp(Universe& universe) : Scanner(universe) {}

bool ScannerWarp::skipLevel(int level)
{
	return level != 15;
}

bool ScannerWarp::levelMatches(std::optional<uint32_t> levelSeed)
{
	if (POI == Point { -1, -1 })
		return false;

	std::cout << universe.sgGameInitInfo.dwSeed << std::endl;

	return true;
}
