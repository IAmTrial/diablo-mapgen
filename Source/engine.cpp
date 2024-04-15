
#include "Source/engine.h"

#include <stdio.h>
#include <stdlib.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "structs.h"
#include "types.h"
#include "Source/gendung.h"
#include "Source/objects.h"
#include "Source/universe/universe.h"

const int myplr = 0;

const uint32_t RndMult = 0x015A4E35;
const uint32_t RndInc = 1;

uint64_t micros()
{
	auto now = std::chrono::steady_clock::now();
	auto since_epoch = now.time_since_epoch();
	return std::chrono::duration_cast<std::chrono::microseconds>(since_epoch).count();
}

BOOL delta_quest_inited(int i)
{
	return false;
}

/**
 * @brief Set the RNG seed
 * @param s RNG seed
 */
void SetRndSeed(Universe& universe, int s)
{
	universe.SeedCount = 0;
	universe.sglGameSeed = s;
	universe.orgseed = s;
}

/**
 * @brief Get the current RNG seed
 * @return RNG seed
 */
int GetRndSeed(Universe& universe)
{
	universe.SeedCount++;
	universe.sglGameSeed = RndMult * universe.sglGameSeed + RndInc;
	int32_t seed = (int32_t)universe.sglGameSeed;
	return seed == std::numeric_limits<int32_t>::min() ? std::numeric_limits<int32_t>::min() : std::abs(seed);
}

int GetRndState(Universe& universe)
{
	return universe.sglGameSeed;
}

int random_(Universe& universe, BYTE idx, int v)
{
	if (v <= 0)
		return 0;
	if (v < 0xFFFF)
		return (GetRndSeed(universe) >> 16) % v;
	return GetRndSeed(universe) % v;
}

BYTE *DiabloAllocPtr(DWORD dwBytes)
{
	return (BYTE *)malloc(dwBytes);
}

/**
 * @brief Multithreaded safe memfree
 * @param p Memory pointer to free
 */
void mem_free_dbg(void *p)
{
	free(p);
}

/**
 * @brief Load a file into a buffer
 * @param pszName Path of file
 * @param pdwFileLen Will be set to file size if non-NULL
 * @return Buffer with content of file
 */
BYTE *LoadFileInMem(std::string pszName, DWORD *pdwFileLen)
{
	BYTE *buf;
	int fileLen;

#ifndef WIN32
	// Convert to lowercase
	std::transform(pszName.begin(), pszName.end(), pszName.begin(), [](unsigned char c) { return std::tolower(c); });

	// Replace backslashes with forward slashes
	std::replace(pszName.begin(), pszName.end(), '\\', '/');
#endif

	FILE *filestream = fopen(pszName.c_str(), "r+b");

	if (!filestream) {
		char cwd[MAX_PATH];
		cwd[0] = '\0';
#ifdef _WIN32
		GetCurrentDirectoryA(sizeof(cwd), cwd);
#else
		getcwd(cwd, sizeof(cwd));
#endif
		std::cerr << cwd << std::endl;
		app_fatal("FILE NOT FOUND");
	}
	fseek(filestream, 0, SEEK_END); // seek to end of file
	fileLen = ftell(filestream);    // get current file pointer
	fseek(filestream, 0, SEEK_SET); // seek back to beginning of file

	if (pdwFileLen)
		*pdwFileLen = fileLen;

	if (!fileLen)
		app_fatal("Zero length SFILE:\n%s");

	buf = (BYTE *)DiabloAllocPtr(fileLen);

	int read = fread(buf, 1, fileLen, filestream);

	fclose(filestream);

	return buf;
}

void LoadLvlGFX(Universe& universe)
{
	free(pMegaTiles);
	free(pLevelPieces);

	switch (universe.leveltype) {
	case DTYPE_CATHEDRAL:
		pMegaTiles = LoadFileInMem("Levels\\L1Data\\L1.TIL", NULL);
		pLevelPieces = LoadFileInMem("Levels\\L1Data\\L1.MIN", NULL);
		return;
	case DTYPE_CATACOMBS:
		pMegaTiles = LoadFileInMem("Levels\\L2Data\\L2.TIL", NULL);
		pLevelPieces = LoadFileInMem("Levels\\L2Data\\L2.MIN", NULL);
		return;
	case DTYPE_CAVES:
		pMegaTiles = LoadFileInMem("Levels\\L3Data\\L3.TIL", NULL);
		pLevelPieces = LoadFileInMem("Levels\\L3Data\\L3.MIN", NULL);
		return;
	case DTYPE_HELL:
		pMegaTiles = LoadFileInMem("Levels\\L4Data\\L4.TIL", NULL);
		pLevelPieces = LoadFileInMem("Levels\\L4Data\\L4.MIN", NULL);
		return;
	default:
		app_fatal("LoadLvlGFX");
	}
}

void app_fatal(const char *dummystring)
{
	std::cerr << dummystring << std::endl;
	exit(1);
}

void SetAutomapView(int nXPos, int nYPos)
{
}
