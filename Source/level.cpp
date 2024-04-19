#include "level.h"

#include <cstdint>
#include <iostream>
#include <stdio.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include "../types.h"
#include "engine.h"
#include "funkMapGen.h"
#include "gendung.h"
#include "monstdat.h"
#include "monster.h"
#include "objdat.h"
#include "objects.h"
#include "Source/universe/universe.h"

inline void WriteLE16(FILE *out, uint16_t val)
{
	fwrite(&val, 1, 2, out);
}

void ExportDun(Universe& universe, uint32_t seed)
{
	char fileName[32];
	sprintf(fileName, "%u-%u-%u.dun", seed, universe.currlevel, universe.glSeedTbl[universe.currlevel]);
	FILE *dunFile = fopen(fileName, "wb");

	WriteLE16(dunFile, DMAXX);
	WriteLE16(dunFile, DMAXY);

	/** Tiles. */
	for (int y = 0; y < DMAXY; y++) {
		for (int x = 0; x < DMAXX; x++) {
			WriteLE16(dunFile, universe.dungeon[x][y]);
		}
	}

	/** Padding */
	for (int y = 16; y < MAXDUNY - 16; y++) {
		for (int x = 16; x < MAXDUNX - 16; x++) {
			WriteLE16(dunFile, 0);
		}
	}

	/** Monsters */
	for (int y = 16; y < MAXDUNY - 16; y++) {
		for (int x = 16; x < MAXDUNX - 16; x++) {
			uint16_t monsterId = 0;
			if (universe.dMonster[x][y] > 0) {
				for (int i = 0; i < 128; i++) {
					if (MonstConvTbl[i] == universe.monster[universe.dMonster[x][y] - 1].MType->mtype) {
						monsterId = i + 1;
						break;
					}
				}
			}
			WriteLE16(dunFile, monsterId);
		}
	}

	/** Objects */
	for (int y = 16; y < MAXDUNY - 16; y++) {
		for (int x = 16; x < MAXDUNX - 16; x++) {
			uint16_t objectId = 0;
			if (universe.dObject[x][y] > 0) {
				for (int i = 0; i < 139; i++) {
					if (ObjTypeConv[i] == universe.object[universe.dObject[x][y] - 1]._otype) {
						objectId = i;
						break;
					}
				}
			}
			WriteLE16(dunFile, objectId);
		}
	}

	/** Transparency */
	for (int y = 16; y < MAXDUNY - 16; y++) {
		for (int x = 16; x < MAXDUNX - 16; x++) {
			WriteLE16(dunFile, universe.dTransVal[x][y]);
		}
	}
	fclose(dunFile);
}

std::string red(std::string text)
{
#ifdef _WIN32
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
	return text;
#else
	return "\033[0;31m" + text + "\033[0m";
#endif
}

std::string green(std::string text)
{
#ifdef _WIN32
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
	return text;
#else
	return "\033[1;32m" + text + "\033[0m";
#endif
}

std::string yellow(std::string text)
{
#ifdef _WIN32
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN);
	return text;
#else
	return "\033[1;33m" + text + "\033[0m";
#endif
}

std::string gray(std::string text)
{
#ifdef _WIN32
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	return text;
#else
	return "\033[0;90m" + text + "\033[0m";
#endif
}

std::string cyan(std::string text)
{
#ifdef _WIN32
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE);
	return text;
#else
	return "\033[0;36m" + text + "\033[0m";
#endif
}

void printAsciiLevel(Universe& universe)
{
	bool steps[MAXDUNX][MAXDUNY];

	for (int i = 0; i < MAXDUNY; ++i) {
		for (int j = 0; j < MAXDUNX; ++j) {
			steps[i][j] = false;
		}
	}

	Point position = universe.Spawn;
	steps[position.x][position.y] = true;

	const char pathxdir[9] = { 0, 0, -1, 1, 0, -1, 1, 1, -1 };
	const char pathydir[9] = { 0, -1, 0, 0, 1, -1, -1, 1, 1 };

	for (int i = 0; i < MAX_PATH_LENGTH; ++i) {
		if (universe.Path[i] == 0)
			break;
		position.x += pathxdir[universe.Path[i]];
		position.y += pathydir[universe.Path[i]];
		steps[position.x][position.y] = true;
	}

	for (int boby = 16; boby < MAXDUNY - 17; boby++) {
		for (int bobx = 16; bobx < MAXDUNX - 17; bobx++) {
			if (Point { bobx, boby } == universe.Spawn)
				std::cout << red("^");
			else if (Point { bobx, boby } == universe.StairsDown)
				std::cout << green("v");
			else if (universe.dObject[bobx][boby] && universe.nSolidTable[universe.dPiece[bobx][boby]])
				std::cout << yellow("#");
			else if (universe.dMonster[bobx][boby])
				std::cout << red("m");
			else if (Point { bobx, boby } == universe.POI && !universe.nSolidTable[universe.dPiece[bobx][boby]])
				std::cout << red("!");
			else if (universe.dObject[bobx][boby])
				std::cout << yellow("*");
			else if (steps[bobx][boby])
				std::cout << cyan("=");
			else if (universe.nSolidTable[universe.dPiece[bobx][boby]])
				std::cout << gray("#");
			else
				std::cout << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;

#ifdef _WIN32
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
}
