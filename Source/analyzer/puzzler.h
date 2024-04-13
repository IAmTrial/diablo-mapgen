#pragma once

#include <cstdint>
#include <optional>

#include "Source/funkMapGen.h"
#include "Source/universe/universe.h"

class ScannerPuzzler : public Scanner {
public:
	explicit ScannerPuzzler(Universe& universe);

	bool skipLevel(int level) override;
	bool levelMatches(std::optional<uint32_t> levelSeed) override;
};

void LocatePuzzler(Universe& universe);
