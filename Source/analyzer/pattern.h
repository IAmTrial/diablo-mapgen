#pragma once

#include <cstdint>
#include <optional>

#include "structs.h"
#include "Source/funkMapGen.h"
#include "Source/universe/universe.h"

class ScannerPattern : public Scanner {
public:
	explicit ScannerPattern(Universe& universe);

	DungeonMode getDungeonMode() override;
	bool skipLevel(int level) override;
	bool levelMatches(std::optional<uint32_t> levelSeed) override;
};
