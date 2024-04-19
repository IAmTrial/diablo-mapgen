#include "funkMapGen.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <vector>

#include "Source/analyzer/gameseed.h"
#include "Source/analyzer/path.h"
#include "Source/analyzer/pattern.h"
#include "Source/analyzer/puzzler.h"
#include "Source/analyzer/quest.h"
#include "Source/analyzer/stairs.h"
#include "Source/analyzer/warp.h"
#include "Source/drlg_l1.h"
#include "Source/drlg_l2.h"
#include "Source/drlg_l3.h"
#include "Source/drlg_l4.h"
#include "Source/items.h"
#include "Source/level.h"
#include "Source/lighting.h"
#include "Source/monster.h"
#include "Source/objects.h"
#include "Source/quests.h"
#include "Source/themes.h"
#include "Source/trigs.h"
#include "Source/universe/universe.h"

int MonsterItems;
int ObjectItems;

Point Spawn = { -1, -1 };
Point StairsDown = { -1, -1 };
Point POI = { -1, -1 };

char Path[MAX_PATH_LENGTH];

Configuration Config;

namespace {

constexpr uint64_t ProgressInterval = 10 * 1000 * 1000;

BYTE previousLevelType = DTYPE_NONE;
Scanner *scanner;

void InitEngine(Universe& universe)
{
	universe.gnDifficulty = DIFF_NORMAL;
	universe.leveltype = DTYPE_NONE;

	DRLG_PreLoadL2SP();
	DRLG_PreLoadDiabQuads();

	if (Config.scanner == Scanners::None) {
		scanner = new Scanner(universe);
	} else if (Config.scanner == Scanners::Path) {
		scanner = new ScannerPath(universe);
	} else if (Config.scanner == Scanners::Quest) {
		scanner = new ScannerQuest(universe);
	} else if (Config.scanner == Scanners::Puzzler) {
		scanner = new ScannerPuzzler(universe);
	} else if (Config.scanner == Scanners::Warp) {
		scanner = new ScannerWarp(universe);
	} else if (Config.scanner == Scanners::Stairs) {
		scanner = new ScannerStairs(universe);
	} else if (Config.scanner == Scanners::Pattern) {
		scanner = new ScannerPattern(universe);
	} else if (Config.scanner == Scanners::GameSeed) {
		scanner = new ScannerGameSeed(universe);
	}
}

void ShutDownEngine()
{
	DRLG_UnloadL2SP();
	DRLG_FreeDiabQuads();
	delete scanner;
}

void InitiateLevel(Universe& universe, int level)
{
	POI = { -1, -1 };
	universe.currlevel = level;

	universe.oobread = false;
	universe.oobwrite = false;

	if (level > 12)
		universe.leveltype = DTYPE_HELL;
	else if (level > 8)
		universe.leveltype = DTYPE_CAVES;
	else if (level > 4)
		universe.leveltype = DTYPE_CATACOMBS;
	else if (level > 0)
		universe.leveltype = DTYPE_CATHEDRAL;

	InitVision(universe);

	if (universe.leveltype == previousLevelType)
		return;
	previousLevelType = universe.leveltype;

	LoadLvlGFX(universe);
	FillSolidBlockTbls(universe);
}

void InitTriggers(Universe& universe)
{
	if (universe.leveltype == DTYPE_CATHEDRAL)
		InitL1Triggers(universe);
	else if (universe.leveltype == DTYPE_CATACOMBS)
		InitL2Triggers(universe);
	else if (universe.leveltype == DTYPE_CAVES)
		InitL3Triggers(universe);
	else if (universe.leveltype == DTYPE_HELL)
		InitL4Triggers(universe);
	Freeupstairs(universe);
}

void FindStairCordinates(Universe& universe)
{
	Spawn = { -1, -1 };
	StairsDown = { -1, -1 };

	for (int i = 0; i < numtrigs; i++) {
		if (trigs[i]._tmsg == WM_DIABNEXTLVL) {
			StairsDown = { trigs[i]._tx, trigs[i]._ty };
		} else if (trigs[i]._tmsg == WM_DIABPREVLVL) {
			if (universe.leveltype == DTYPE_CATHEDRAL)
				Spawn = { trigs[i]._tx + 1, trigs[i]._ty + 2 };
			else if (universe.leveltype == DTYPE_CATACOMBS)
				Spawn = { trigs[i]._tx + 1, trigs[i]._ty + 1 };
			else if (universe.leveltype == DTYPE_CAVES)
				Spawn = { trigs[i]._tx, trigs[i]._ty + 1 };
			else if (universe.leveltype == DTYPE_HELL)
				Spawn = { trigs[i]._tx + 1, trigs[i]._ty };
		}
	}
}

void CreateDungeonContent(Universe& universe)
{
	InitDungeonMonsters(universe);

	InitThemes(universe);
	SetRndSeed(universe, universe.glSeedTbl[universe.currlevel]);
	HoldThemeRooms(universe);
	GetRndSeed(universe);

	InitMonsters(universe);
	GetRndSeed(universe);

	InitObjects(universe);

	InitItems(universe);
	CreateThemeRooms(universe);
}

std::optional<uint32_t> CreateDungeon(Universe& universe, DungeonMode mode)
{
	uint32_t lseed = universe.glSeedTbl[universe.currlevel];
	std::optional<uint32_t> levelSeed = std::nullopt;
	if (universe.leveltype == DTYPE_CATHEDRAL)
		levelSeed = CreateL5Dungeon(universe, lseed, 0, mode);
	if (universe.leveltype == DTYPE_CATACOMBS)
		levelSeed = CreateL2Dungeon(universe, lseed, 0, mode);
	if (universe.leveltype == DTYPE_CAVES)
		levelSeed = CreateL3Dungeon(universe, lseed, 0, mode);
	if (universe.leveltype == DTYPE_HELL)
		levelSeed = CreateL4Dungeon(universe, lseed, 0, mode);

	if (mode == DungeonMode::Full || mode == DungeonMode::NoContent || mode == DungeonMode::BreakOnFailureOrNoContent) {
		InitTriggers(universe);

		if (mode != DungeonMode::NoContent && mode != DungeonMode::BreakOnFailureOrNoContent)
			CreateDungeonContent(universe);

		if (universe.currlevel == 15) {
			// Locate Lazarus warp point
			Point point = { universe.quests[Q_BETRAYER]._qtx, universe.quests[Q_BETRAYER]._qty };
			if (!universe.nSolidTable[universe.dPiece[point.x][point.y]])
				POI = point;
		}

		FindStairCordinates(universe);
	}

	if (Config.verbose && universe.oobwrite)
		std::cerr << "Game Seed: " << universe.sgGameInitInfo.dwSeed << " OOB write detected" << std::endl;

	return levelSeed;
}

/**
 * @brief GET MAIN SEED, GET ALL MAP SEEDS
 * @return nothing, but updates RNG seeds list universe.glSeedTbl[i]
 */
void SetGameSeed(Universe& universe, uint32_t seed)
{
	universe.sgGameInitInfo.dwSeed = seed;
	SetRndSeed(universe, universe.sgGameInitInfo.dwSeed);

	for (int i = 0; i < NUMLEVELS; i++) {
		universe.glSeedTbl[i] = GetRndSeed(universe);
	}

	InitQuests(universe);
	memset(universe.UniqueItemFlag, 0, sizeof(universe.UniqueItemFlag));
}

std::vector<uint32_t> SeedsFromFile = {};

void readFromFile()
{
	if (Config.seedFile.empty())
		return;

	std::ifstream file(Config.seedFile);
	if (!file.is_open()) {
		std::cerr << "Unable to read seeds file: " << Config.seedFile << std::endl;
		exit(255);
	}

	if (!Config.quiet)
		std::cerr << "Loading seeds from: " << Config.seedFile << std::endl;

	std::string line;
	while (std::getline(file, line))
		SeedsFromFile.push_back(std::stoll(line.substr(0, line.find(' '))));

	Config.seedCount = std::min(Config.seedCount, uint32_t(SeedsFromFile.size() - Config.startSeed));

	if (file.is_open())
		file.close();
}

int ProgressseedMicros;
uint32_t ProgressseedIndex = 0;

void printProgress(uint32_t seedIndex, uint32_t seed)
{
	if (Config.verbose)
		std::cerr << "Processing: " << seed << std::endl;
	if (Config.quiet)
		return;

	int elapsed = micros() - ProgressseedMicros;
	if (elapsed < ProgressInterval)
		return;
	ProgressseedMicros += elapsed;

	uint64_t pct = 100 * (uint64_t)seedIndex / Config.seedCount;
	int speed = (seedIndex - ProgressseedIndex) / 10;
	int seconds = (Config.seedCount - seedIndex) / speed;
	ProgressseedIndex = seedIndex;

	int days = seconds / (24 * 3600);
	seconds %= (24 * 3600);
	int hours = seconds / 3600;
	seconds %= 3600;
	int minutes = seconds / 60;
	seconds %= 60;

	std::ostringstream oss;
	oss << "Progress: " << pct << "% eta: ";
	if (days > 0)
		oss << days << "d";
	if (hours > 0 || days > 0)
		oss << std::setw(2) << std::setfill('0') << hours << "h";
	if (minutes > 0 || hours > 0 || days > 0)
		oss << std::setw(2) << std::setfill('0') << minutes << "m";
	oss << std::setw(2) << std::setfill('0') << seconds << "s (" << speed << " seed/s)" << std::endl;

	std::cerr << oss.str();
}

void printHelp()
{
	std::cout << "--help         Print this message and exit" << std::endl;
	std::cout << "--ascii        Print ASCII version of levels" << std::endl;
	std::cout << "--export       Export levels as .dun files" << std::endl;
	std::cout << "--scanner <#>  How to analyze levels [default: none]" << std::endl;
	std::cout << "                   none: No analyzing" << std::endl;
	std::cout << "                   puzzler: Search for Naj's Puzzler on level 9" << std::endl;
	std::cout << "                   path: Estimate the time to complete the game" << std::endl;
	std::cout << "                   warp: Find seeds with a warp on level 15" << std::endl;
	std::cout << "                   stairs: Look for stairs with a very short distance to level 9" << std::endl;
	std::cout << "                   pattern: Search a set tile pattern" << std::endl;
	std::cout << "                   gameseed: Search for GameSeeds with LevelSeed" << std::endl;
	std::cout << "--start <#>    The seed to start from" << std::endl;
	std::cout << "--count <#>    The number of seeds to process" << std::endl;
	std::cout << "--seeds <#>    A file to read seeds from" << std::endl;
	std::cout << "--target <#>   The target for the current filter [default: 420]" << std::endl;
	std::cout << "--quiet        Do print status messages" << std::endl;
	std::cout << "--verbose      Print out details about seeds" << std::endl;
}

void ParseArguments(int argc, char **argv)
{
	bool fromFile = false;
	bool hasCount = false;

	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];
		if (arg == "--help") {
			printHelp();
			exit(0);
		} else if (arg == "--quiet") {
			Config.quiet = true;
		} else if (arg == "--ascii") {
			Config.asciiLevels = true;
		} else if (arg == "--export") {
			Config.exportLevels = true;
		} else if (arg == "--scanner") {
			i++;
			if (argc <= i) {
				std::cerr << "Missing value for --scanner" << std::endl;
				exit(255);
			}
			std::string scanner = argv[i];
			if (scanner == "none") {
				Config.scanner = Scanners::None;
			} else if (scanner == "puzzler") {
				Config.scanner = Scanners::Puzzler;
			} else if (scanner == "path") {
				Config.scanner = Scanners::Path;
			} else if (scanner == "quest") {
				Config.scanner = Scanners::Quest;
			} else if (scanner == "warp") {
				Config.scanner = Scanners::Warp;
			} else if (scanner == "stairs") {
				Config.scanner = Scanners::Stairs;
			} else if (scanner == "pattern") {
				Config.scanner = Scanners::Pattern;
			} else if (scanner == "gameseed") {
				Config.scanner = Scanners::GameSeed;
			} else {
				std::cerr << "Unknown scanner: " << scanner << std::endl;
				exit(255);
			}
		} else if (arg == "--seeds") {
			i++;
			if (argc <= i) {
				std::cerr << "Missing filename for --seeds" << std::endl;
				exit(255);
			}
			fromFile = true;
			Config.seedFile = argv[i];
		} else if (arg == "--start") {
			i++;
			if (argc <= i) {
				std::cerr << "Missing value for --start" << std::endl;
				exit(255);
			}
			Config.startSeed = std::stoll(argv[i]);
		} else if (arg == "--count") {
			i++;
			if (argc <= i) {
				std::cerr << "Missing value for --count" << std::endl;
				exit(255);
			}
			hasCount = true;
			Config.seedCount = std::stoll(argv[i]);
		} else if (arg == "--target") {
			i++;
			if (argc <= i) {
				std::cerr << "Missing value for --target" << std::endl;
				exit(255);
			}
			Config.target = std::stoll(argv[i]);
		} else if (arg == "--verbose") {
			Config.verbose = true;
		} else {
			std::cerr << "Unknown argument: " << arg << std::endl;
			exit(255);
		}
	}

	if (fromFile && !hasCount) {
		Config.seedCount = std::numeric_limits<uint32_t>::max();
	}
}

}

void InitDungeonMonsters(Universe& universe)
{
	InitLevelMonsters(universe);
	SetRndSeed(universe, universe.glSeedTbl[universe.currlevel]);
	GetLevelMTypes(universe);
}

int main(int argc, char **argv)
{
	Universe universe;

	ParseArguments(argc, argv);
	InitEngine(universe);
	readFromFile();

	ProgressseedMicros = micros();
	for (uint32_t seedIndex = 0; seedIndex < Config.seedCount; seedIndex++) {

		uint32_t seed = seedIndex + Config.startSeed;
		if (!SeedsFromFile.empty()) {
			seed = SeedsFromFile[seed];
		}
		printProgress(seedIndex, seed);

		SetGameSeed(universe, seed);
		if (scanner->skipSeed())
			continue;

		for (int level = 1; level < NUMLEVELS; level++) {
			if (scanner->skipLevel(level))
				continue;

			InitiateLevel(universe, level);
			std::optional<uint32_t> levelSeed = CreateDungeon(universe, scanner->getDungeonMode());
			if (!scanner->levelMatches(levelSeed))
				continue;

			if (Config.asciiLevels)
				printAsciiLevel(universe);
			if (Config.exportLevels)
				ExportDun(universe, seed);
		}
	}

	ShutDownEngine();

	return 0;
}
