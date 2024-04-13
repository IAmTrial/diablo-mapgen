#include "gameseed.h"

#include <cstdint>
#include <iostream>
#include <optional>

#include "structs.h"
#include "types.h"
#include "Source/funkMapGen.h"
#include "Source/monster.h"
#include "Source/universe/universe.h"

ScannerGameSeed::ScannerGameSeed(Universe& universe) : Scanner(universe) {}

DungeonMode ScannerGameSeed::getDungeonMode()
{
	return DungeonMode::BreakOnSuccess;
}

bool ScannerGameSeed::skipLevel(int level)
{
	if (level != 9)
		return true;

	InitDungeonMonsters(universe);

	bool hasLavaLoards = false;
	for (int i = 0; i < nummtypes && !hasLavaLoards; i++)
		hasLavaLoards = Monsters[i].mtype == MT_WMAGMA;

	return !hasLavaLoards;
}

bool ScannerGameSeed::levelMatches(std::optional<uint32_t> levelSeed)
{
	if (levelSeed == std::nullopt)
		return false;

	if (levelSeed == *Config.target) {
		std::cout << universe.sgGameInitInfo.dwSeed << std::endl;
		return true;
	}

	return false;
}
