#pragma once

#include <cstdint>
#include <optional>

#include "Source/funkMapGen.h"
#include "Source/universe/universe.h"

class ScannerWarp : public Scanner {
public:
	explicit ScannerWarp(Universe& universe);

	bool skipLevel(int level) override;
	bool levelMatches(std::optional<uint32_t> levelSeed) override;
};
