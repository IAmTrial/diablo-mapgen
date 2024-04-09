#pragma once

#include <cstdint>
#include <optional>

#include "Source/funkMapGen.h"

class ScannerPath : public Scanner {
public:
	bool skipSeed() override;
	bool skipLevel(int level) override;
	bool levelMatches(std::optional<uint32_t> levelSeed) override;
};
