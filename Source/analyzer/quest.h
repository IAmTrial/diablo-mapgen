#pragma once

#include "Source/funkMapGen.h"
#include "Source/universe/universe.h"

class ScannerQuest : public Scanner {
public:
	explicit ScannerQuest(Universe& universe);

	bool skipSeed() override;
};
