#pragma once

#include <optional>
#include <string>

#include "engine.h"

enum class Scanners {
	None,
	Path,
	Quest,
	Puzzler,
	Stairs,
	Warp,
	Pattern,
	GameSeed,
};

struct Configuration {
	uint32_t startSeed = 0;
	uint32_t seedCount = 1;
	std::string seedFile;
	Scanners scanner = Scanners::None;
	bool quiet = false;
	bool asciiLevels = false;
	bool exportLevels = false;
	std::optional<uint32_t> target = std::nullopt;
	bool verbose = false;
};

// TODO: Mark these as OpenCL __global
extern Configuration Config;

class Scanner {
protected:
	Universe& universe;

public:
	explicit Scanner(Universe& universe_) : universe(universe_) {}

	virtual DungeonMode getDungeonMode()
	{
		return DungeonMode::Full;
	};

	virtual bool skipSeed()
	{
		return false;
	};

	virtual bool skipLevel(int level)
	{
		return Config.target && level != *Config.target;
	};

	virtual bool levelMatches(std::optional<uint32_t> levelSeed)
	{
		return true;
	};

	virtual ~Scanner() = default;
};


void InitDungeonMonsters(Universe& universe);
