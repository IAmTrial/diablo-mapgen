#pragma once

#include <cstdint>
#include <optional>

#include "Source/funkMapGen.h"

class ScannerPuzzler : public Scanner {
public:
	bool skipLevel(int level) override;
	bool levelMatches(std::optional<uint32_t> levelSeed) override;
};

void LocatePuzzler();
