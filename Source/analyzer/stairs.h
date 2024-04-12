#pragma once

#include "Source/funkMapGen.h"
#include "Source/universe/universe.h"

class ScannerStairs : public Scanner {
public:
	explicit ScannerStairs(Universe& universe);

	DungeonMode getDungeonMode() override;
	bool skipSeed() override;
	bool skipLevel(int level) override;
	bool levelMatches(std::optional<uint32_t> levelSeed) override;
};
