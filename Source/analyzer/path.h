#pragma once

#include <cstdint>
#include <optional>

#include "Source/funkMapGen.h"
#include "Source/universe/universe.h"

class ScannerPath : public Scanner {
public:
	explicit ScannerPath(Universe& universe);
	bool skipSeed() override;
	bool skipLevel(int level) override;
	bool levelMatches(std::optional<uint32_t> levelSeed) override;
};
