#include "Source/analyzer/quest.h"

#include <iostream>

#include "types.h"
#include "Source/funkMapGen.h"
#include "Source/quests.h"
#include "Source/universe/universe.h"

ScannerQuest::ScannerQuest(Universe& universe) : Scanner(universe) {}

bool ScannerQuest::skipSeed()
{
	if (universe.quests[Q_LTBANNER]._qactive != QUEST_NOTAVAIL) {
		if (Config.verbose)
			std::cerr << "Game Seed: " << universe.sgGameInitInfo.dwSeed << " thrown out: Sign Quest" << std::endl;
		return true;
	}
	if (universe.quests[Q_WARLORD]._qactive != QUEST_NOTAVAIL) {
		if (Config.verbose)
			std::cerr << "Game Seed: " << universe.sgGameInitInfo.dwSeed << " thrown out: Warlord" << std::endl;
		return true;
	}

	std::cout << universe.sgGameInitInfo.dwSeed << std::endl;

	return true;
}
