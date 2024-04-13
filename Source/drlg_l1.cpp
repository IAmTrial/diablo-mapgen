/**
 * @file drlg_l1.cpp
 *
 * Implementation of the cathedral level generation algorithms.
 */

#include "Source/drlg_l1.h"

#include <optional>

#include "defs.h"
#include "structs.h"
#include "types.h"
#include "Source/engine.h"
#include "Source/lighting.h"
#include "Source/quests.h"
#include "Source/universe/universe.h"

/** Contains shadows for 2x2 blocks of base tile IDs in the Cathedral. */
const ShadowStruct SPATS[37] = {
	// clang-format off
	// strig, s1, s2, s3, nv1, nv2, nv3
	{      7, 13,  0, 13, 144,   0, 142 },
	{     16, 13,  0, 13, 144,   0, 142 },
	{     15, 13,  0, 13, 145,   0, 142 },
	{      5, 13, 13, 13, 152, 140, 139 },
	{      5, 13,  1, 13, 143, 146, 139 },
	{      5, 13, 13,  2, 143, 140, 148 },
	{      5,  0,  1,  2,   0, 146, 148 },
	{      5, 13, 11, 13, 143, 147, 139 },
	{      5, 13, 13, 12, 143, 140, 149 },
	{      5, 13, 11, 12, 150, 147, 149 },
	{      5, 13,  1, 12, 143, 146, 149 },
	{      5, 13, 11,  2, 143, 147, 148 },
	{      9, 13, 13, 13, 144, 140, 142 },
	{      9, 13,  1, 13, 144, 146, 142 },
	{      9, 13, 11, 13, 151, 147, 142 },
	{      8, 13,  0, 13, 144,   0, 139 },
	{      8, 13,  0, 12, 143,   0, 149 },
	{      8,  0,  0,  2,   0,   0, 148 },
	{     11,  0,  0, 13,   0,   0, 139 },
	{     11, 13,  0, 13, 139,   0, 139 },
	{     11,  2,  0, 13, 148,   0, 139 },
	{     11, 12,  0, 13, 149,   0, 139 },
	{     11, 13, 11, 12, 139,   0, 149 },
	{     14,  0,  0, 13,   0,   0, 139 },
	{     14, 13,  0, 13, 139,   0, 139 },
	{     14,  2,  0, 13, 148,   0, 139 },
	{     14, 12,  0, 13, 149,   0, 139 },
	{     14, 13, 11, 12, 139,   0, 149 },
	{     10,  0, 13,  0,   0, 140,   0 },
	{     10, 13, 13,  0, 140, 140,   0 },
	{     10,  0,  1,  0,   0, 146,   0 },
	{     10, 13, 11,  0, 140, 147,   0 },
	{     12,  0, 13,  0,   0, 140,   0 },
	{     12, 13, 13,  0, 140, 140,   0 },
	{     12,  0,  1,  0,   0, 146,   0 },
	{     12, 13, 11,  0, 140, 147,   0 },
	{      3, 13, 11, 12, 150,   0,   0 }
	// clang-format on
};

// BUGFIX: This array should contain an additional 0 (207 elements).
/** Maps tile IDs to their corresponding base tile ID. */
const BYTE BSTYPES[207] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 0, 0,
	0, 0, 0, 0, 0, 1, 2, 10, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 14, 5, 14,
	10, 4, 14, 4, 5, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	2, 3, 4, 1, 6, 7, 16, 17, 2, 1,
	1, 2, 2, 1, 1, 2, 2, 2, 2, 2,
	1, 1, 11, 1, 13, 13, 13, 1, 2, 1,
	2, 1, 2, 1, 2, 2, 2, 2, 12, 0,
	0, 11, 1, 11, 1, 13, 0, 0, 0, 0,
	0, 0, 0, 13, 13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13, 1, 11, 2, 12,
	13, 13, 13, 12, 2, 1, 2, 2, 4, 14,
	4, 10, 13, 13, 4, 4, 1, 1, 4, 2,
	2, 13, 13, 13, 13, 25, 26, 28, 30, 31,
	41, 43, 40, 41, 42, 43, 25, 41, 43, 28,
	28, 1, 2, 25, 26, 22, 22, 25, 26, 0,
	0, 0, 0, 0, 0, 0, 0
};

// BUGFIX: This array should contain an additional 0 (207 elements).
/** Maps tile IDs to their corresponding undecorated tile ID. */
const BYTE L5BTYPES[207] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 0, 0,
	0, 0, 0, 0, 0, 25, 26, 0, 28, 0,
	30, 31, 0, 0, 0, 0, 0, 0, 0, 0,
	40, 41, 42, 43, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 79,
	80, 0, 82, 0, 0, 0, 0, 0, 0, 79,
	0, 80, 0, 0, 79, 80, 0, 2, 2, 2,
	1, 1, 11, 25, 13, 13, 13, 1, 2, 1,
	2, 1, 2, 1, 2, 2, 2, 2, 12, 0,
	0, 11, 1, 11, 1, 13, 0, 0, 0, 0,
	0, 0, 0, 13, 13, 13, 13, 13, 13, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 4
};
/** Miniset: stairs up on a corner wall. */
const BYTE STAIRSUP[] = {
	// clang-format off
	4, 4, // width, height

	13, 13, 13, 13, // search
	 2,  2,  2,  2,
	13, 13, 13, 13,
	13, 13, 13, 13,

	 0, 66,  6,  0, // replace
	63, 64, 65,  0,
	 0, 67, 68,  0,
	 0,  0,  0,  0,
	// clang-format on
};
#ifdef HELLFIRE
const BYTE L5STAIRSUP[] = {
	// clang-format off
	4, 5, // width, height

	22, 22, 22, 22, // search
	22, 22, 22, 22,
	 2,  2,  2,  2,
	13, 13, 13, 13,
	13, 13, 13, 13,

	 0, 54, 23,  0, // replace
	 0, 53, 18,  0,
	55, 56, 57,  0,
	58, 59, 60,  0,
	 0,  0,  0,  0
	// clang-format on
};
#else
/** Miniset: stairs up. */
const BYTE L5STAIRSUP[] = {
	// clang-format off
	4, 4, // width, height

	22, 22, 22, 22, // search
	 2,  2,  2,  2,
	13, 13, 13, 13,
	13, 13, 13, 13,

	 0, 66, 23,  0, // replace
	63, 64, 65,  0,
	 0, 67, 68,  0,
	 0,  0,  0,  0,
	// clang-format on
};
#endif
/** Miniset: stairs down. */
const BYTE STAIRSDOWN[] = {
	// clang-format off
	4, 3, // width, height

	13, 13, 13, 13, // search
	13, 13, 13, 13,
	13, 13, 13, 13,

	62, 57, 58,  0, // replace
	61, 59, 60,  0,
	 0,  0,  0,  0,
	// clang-format on
};
#ifdef HELLFIRE
const BYTE L5STAIRSDOWN[] = {
	// clang-format off
	4, 5, // width, height

	13, 13, 13, 13, // search
	13, 13, 13, 13,
	13, 13, 13, 13,
	13, 13, 13, 13,
	13, 13, 13, 13,

	 0,  0, 52,  0, // replace
	 0, 48, 51,  0,
	 0, 47, 50,  0,
	45, 46, 49,  0,
	 0,  0,  0,  0,
	// clang-format on
};
const BYTE L5STAIRSTOWN[] = {
	// clang-format off
	4, 5, // width, height

	22, 22, 22, 22, // search
	22, 22, 22, 22,
	 2,  2,  2,  2,
	13, 13, 13, 13,
	13, 13, 13, 13,

	 0, 62, 23,  0, // replace
	 0, 61, 18,  0,
	63, 64, 65,  0,
	66, 67, 68,  0,
	 0,  0,  0,  0,
	// clang-format on
};
#endif
/** Miniset: candlestick. */
const BYTE LAMPS[] = {
	// clang-format off
	2, 2, // width, height

	13,  0, // search
	13, 13,

	129,   0, // replace
	130, 128,
	// clang-format on
};
/** Miniset: Poisoned Water Supply entrance. */
const BYTE PWATERIN[] = {
	// clang-format off
	6, 6, // width, height

	13, 13, 13, 13, 13, 13, // search
	13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13,

	 0,   0,   0,   0,   0, 0, // replace
	 0, 202, 200, 200,  84, 0,
	 0, 199, 203, 203,  83, 0,
	 0,  85, 206,  80,  81, 0,
	 0,   0, 134, 135,   0, 0,
	 0,   0,   0,   0,   0, 0,
	// clang-format on
};
#ifdef HELLFIRE
const BYTE byte_48A1B4[4] = { 1, 1, 11, 95 };
const BYTE byte_48A1B8[8] = { 1, 1, 12, 96 };
const BYTE byte_48A1C0[8] = {
	// clang-format off
	1, 3, // width, height

	1, // search
	1,
	1,

	91, // replace
	90,
	89,
	// clang-format on
};
const BYTE byte_48A1C8[8] = {
	// clang-format off
	3, 1, // width, height

	 2,  2,  2, // search

	94, 93, 92, // replace
	// clang-format on
};
const BYTE byte_48A1D0[4] = { 1, 1, 13, 97 };
const BYTE byte_48A1D4[4] = { 1, 1, 13, 98 };
const BYTE byte_48A1D8[4] = { 1, 1, 13, 99 };
const BYTE byte_48A1DC[4] = { 1, 1, 13, 100 };
const BYTE byte_48A1E0[20] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 101, 0,
	0,   0, 0,
	// clang-format on
};
const BYTE byte_48A1F4[4] = { 1, 1, 11, 185 };
const BYTE byte_48A1F8[4] = { 1, 1, 11, 186 };
const BYTE byte_48A1FC[4] = { 1, 1, 12, 187 };
const BYTE byte_48A200[4] = { 1, 1, 12, 188 };
const BYTE byte_48A204[4] = { 1, 1, 89, 173 };
const BYTE byte_48A208[4] = { 1, 1, 89, 174 };
const BYTE byte_48A20C[4] = { 1, 1, 90, 175 };
const BYTE byte_48A210[4] = { 1, 1, 90, 176 };
const BYTE byte_48A214[4] = { 1, 1, 91, 177 };
const BYTE byte_48A218[4] = { 1, 1, 91, 178 };
const BYTE byte_48A21C[4] = { 1, 1, 92, 179 };
const BYTE byte_48A220[4] = { 1, 1, 92, 180 };
const BYTE byte_48A224[4] = { 1, 1, 92, 181 };
const BYTE byte_48A228[4] = { 1, 1, 92, 182 };
const BYTE byte_48A22C[4] = { 1, 1, 92, 183 };
const BYTE byte_48A230[4] = { 1, 1, 92, 184 };
const BYTE byte_48A234[4] = { 1, 1, 98, 189 };
const BYTE byte_48A238[4] = { 1, 1, 98, 190 };
const BYTE byte_48A23C[4] = { 1, 1, 97, 191 };
const BYTE byte_48A240[4] = { 1, 1, 15, 192 };
const BYTE byte_48A244[4] = { 1, 1, 99, 193 };
const BYTE byte_48A248[4] = { 1, 1, 99, 194 };
const BYTE byte_48A24C[4] = { 1, 1, 100, 195 };
const BYTE byte_48A250[4] = { 1, 1, 101, 196 };
const BYTE byte_48A254[4] = { 1, 1, 101, 197 };
const BYTE byte_48A258[8] = { 1, 1, 101, 198 };
const BYTE byte_48A260[24] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 167, 0,
	0,   0, 0,
	// clang-format on
};
const BYTE byte_48A278[24] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 168, 0,
	0,   0, 0,
	// clang-format on
};
const BYTE byte_48A290[24] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 169, 0,
	0,   0, 0,
};
const BYTE byte_48A2A8[24] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 170, 0,
	0,   0, 0,
	// clang-format on
};
const BYTE byte_48A2C0[24] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 171, 0,
	0,   0, 0,
	// clang-format on
};
const BYTE byte_48A2D8[20] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	 0,   0, 0, // replace
	 0, 172, 0,
	 0,   0, 0,
	// clang-format on
};
const BYTE byte_48A2EC[4] = { 1, 1, 13, 163 };
const BYTE byte_48A2F0[4] = { 1, 1, 13, 164 };
const BYTE byte_48A2F4[4] = { 1, 1, 13, 165 };
const BYTE byte_48A2F8[4] = { 1, 1, 13, 166 };
const BYTE byte_48A2FC[4] = { 1, 1, 1, 112 };
const BYTE byte_48A300[4] = { 1, 1, 2, 113 };
const BYTE byte_48A304[4] = { 1, 1, 3, 114 };
const BYTE byte_48A308[4] = { 1, 1, 4, 115 };
const BYTE byte_48A30C[4] = { 1, 1, 5, 116 };
const BYTE byte_48A310[4] = { 1, 1, 6, 117 };
const BYTE byte_48A314[4] = { 1, 1, 7, 118 };
const BYTE byte_48A318[4] = { 1, 1, 8, 119 };
const BYTE byte_48A31C[4] = { 1, 1, 9, 120 };
const BYTE byte_48A320[4] = { 1, 1, 10, 121 };
const BYTE byte_48A324[4] = { 1, 1, 11, 122 };
const BYTE byte_48A328[4] = { 1, 1, 12, 123 };
const BYTE byte_48A32C[4] = { 1, 1, 13, 124 };
const BYTE byte_48A330[4] = { 1, 1, 14, 125 };
const BYTE byte_48A334[4] = { 1, 1, 15, 126 };
const BYTE byte_48A338[4] = { 1, 1, 16, 127 };
const BYTE byte_48A33C[4] = { 1, 1, 17, 128 };
const BYTE byte_48A340[4] = { 1, 1, 1, 129 };
const BYTE byte_48A344[4] = { 1, 1, 2, 130 };
const BYTE byte_48A348[4] = { 1, 1, 3, 131 };
const BYTE byte_48A34C[4] = { 1, 1, 4, 132 };
const BYTE byte_48A350[4] = { 1, 1, 5, 133 };
const BYTE byte_48A354[4] = { 1, 1, 6, 134 };
const BYTE byte_48A358[4] = { 1, 1, 7, 135 };
const BYTE byte_48A35C[4] = { 1, 1, 8, 136 };
const BYTE byte_48A360[4] = { 1, 1, 9, 137 };
const BYTE byte_48A364[4] = { 1, 1, 10, 138 };
const BYTE byte_48A368[4] = { 1, 1, 11, 139 };
const BYTE byte_48A36C[4] = { 1, 1, 12, 140 };
const BYTE byte_48A370[4] = { 1, 1, 13, 141 };
const BYTE byte_48A374[4] = { 1, 1, 14, 142 };
const BYTE byte_48A378[4] = { 1, 1, 15, 143 };
const BYTE byte_48A37C[4] = { 1, 1, 16, 144 };
const BYTE byte_48A380[4] = { 1, 1, 17, 145 };
const BYTE byte_48A384[4] = { 1, 1, 1, 146 };
const BYTE byte_48A388[4] = { 1, 1, 2, 147 };
const BYTE byte_48A38C[4] = { 1, 1, 3, 148 };
const BYTE byte_48A390[4] = { 1, 1, 4, 149 };
const BYTE byte_48A394[4] = { 1, 1, 5, 150 };
const BYTE byte_48A398[4] = { 1, 1, 6, 151 };
const BYTE byte_48A39C[4] = { 1, 1, 7, 152 };
const BYTE byte_48A3A0[4] = { 1, 1, 8, 153 };
const BYTE byte_48A3A4[4] = { 1, 1, 9, 154 };
const BYTE byte_48A3A8[4] = { 1, 1, 10, 155 };
const BYTE byte_48A3AC[4] = { 1, 1, 11, 156 };
const BYTE byte_48A3B0[4] = { 1, 1, 12, 157 };
const BYTE byte_48A3B4[4] = { 1, 1, 13, 158 };
const BYTE byte_48A3B8[4] = { 1, 1, 14, 159 };
const BYTE byte_48A3BC[4] = { 1, 1, 15, 160 };
const BYTE byte_48A3C0[4] = { 1, 1, 16, 161 };
const BYTE byte_48A3C4[4] = { 1, 1, 17, 162 };
const BYTE byte_48A3C8[4] = { 1, 1, 1, 199 };
const BYTE byte_48A3CC[4] = { 1, 1, 1, 201 };
const BYTE byte_48A3D0[4] = { 1, 1, 2, 200 };
const BYTE byte_48A3D4[4] = { 1, 1, 2, 202 };
#endif

/* data */

#ifdef HELLFIRE
const BYTE UberRoomPattern[26] = {
	// clang-format off
	4, 6, // width, height

	115, 130,   6, 13, // pattern
	129, 108,   1, 13,
	  1, 107, 103, 13,
	146, 106, 102, 13,
	129, 168,   1, 13,
	  7,   2,   3, 13,
	// clang-format on
};
const BYTE CornerstoneRoomPattern[27] = {
	// clang-format off
	5, 5, // width, height

	4,   2,   2, 2,  6, // pattern
	1, 111, 172, 0,  1,
	1, 172,   0, 0, 25,
	1,   0,   0, 0,  1,
	7,   2,   2, 2,  3,
	// clang-format on
};
#endif
/**
 * A lookup table for the 16 possible patterns of a 2x2 area,
 * where each cell either contains a SW wall or it doesn't.
 */
const BYTE L5ConvTbl[16] = { 22, 13, 1, 13, 2, 13, 13, 13, 4, 13, 1, 13, 2, 13, 16, 13 };

#ifdef HELLFIRE
void DRLG_InitL5Vals()
{
	int i, j, pc;

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dPiece[i][j] == 77) {
				pc = 1;
			} else if (dPiece[i][j] == 80) {
				pc = 2;
			} else {
				continue;
			}
			dSpecial[i][j] = pc;
		}
	}
}
#endif

static void DRLG_PlaceDoor(Universe& universe, int x, int y)
{
	if ((universe.L5dflags[x][y] & DLRG_PROTECTED) == 0) {
		BYTE df = universe.L5dflags[x][y] & 0x7F;
		BYTE c = GetDungeon(universe, x, y);

		if (df == 1) {
			if (y != 1 && c == 2)
				SetDungeon(universe, x, y, 26);
			if (y != 1 && c == 7)
				SetDungeon(universe, x, y, 31);
			if (y != 1 && c == 14)
				SetDungeon(universe, x, y, 42);
			if (y != 1 && c == 4)
				SetDungeon(universe, x, y, 43);
			if (x != 1 && c == 1)
				SetDungeon(universe, x, y, 25);
			if (x != 1 && c == 10)
				SetDungeon(universe, x, y, 40);
			if (x != 1 && c == 6)
				SetDungeon(universe, x, y, 30);
		}
		if (df == 2) {
			if (x != 1 && c == 1)
				SetDungeon(universe, x, y, 25);
			if (x != 1 && c == 6)
				SetDungeon(universe, x, y, 30);
			if (x != 1 && c == 10)
				SetDungeon(universe, x, y, 40);
			if (x != 1 && c == 4)
				SetDungeon(universe, x, y, 41);
			if (y != 1 && c == 2)
				SetDungeon(universe, x, y, 26);
			if (y != 1 && c == 14)
				SetDungeon(universe, x, y, 42);
			if (y != 1 && c == 7)
				SetDungeon(universe, x, y, 31);
		}
		if (df == 3) {
			if (x != 1 && y != 1 && c == 4)
				SetDungeon(universe, x, y, 28);
			if (x != 1 && c == 10)
				SetDungeon(universe, x, y, 40);
			if (y != 1 && c == 14)
				SetDungeon(universe, x, y, 42);
			if (y != 1 && c == 2)
				SetDungeon(universe, x, y, 26);
			if (x != 1 && c == 1)
				SetDungeon(universe, x, y, 25);
			if (y != 1 && c == 7)
				SetDungeon(universe, x, y, 31);
			if (x != 1 && c == 6)
				SetDungeon(universe, x, y, 30);
		}
	}

	universe.L5dflags[x][y] = DLRG_PROTECTED;
}

#ifdef HELLFIRE
void drlg_l1_crypt_lavafloor()
{
	int i, j;

	for (j = 1; j < 40; j++) {
		for (i = 1; i < 40; i++) {
			switch (GetDungeon(universe, i, j)) {
			case 5:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 203);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 204);
				if (GetDungeon(universe, i, j - 1) == 13)
					SetDungeon(universe, i, j - 1, 205);
				break;
			case 7:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 206);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 207);
				break;
			case 8:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 203);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 204);
				break;
			case 9:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 206);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 207);
				if (GetDungeon(universe, i, j - 1) == 13)
					SetDungeon(universe, i, j - 1, 205);
				break;
			case 10:
				if (GetDungeon(universe, i, j - 1) == 13)
					SetDungeon(universe, i, j - 1, 205);
				break;
			case 11:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 203);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 204);
				break;
			case 12:
				if (GetDungeon(universe, i, j - 1) == 13)
					SetDungeon(universe, i, j - 1, 205);
				break;
			case 14:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 203);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 204);
				break;
			case 15:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 206);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 207);
				break;
			case 17:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 206);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 207);
				break;
			case 95:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 203);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 204);
				break;
			case 96:
				if (GetDungeon(universe, i, j - 1) == 13)
					SetDungeon(universe, i, j - 1, 208);
				break;
			case 116:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 203);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 204);
				if (GetDungeon(universe, i, j - 1) == 13)
					SetDungeon(universe, i, j - 1, 205);
				break;
			case 118:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 206);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 207);
				break;
			case 119:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 203);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 204);
				break;
			case 120:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 206);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 207);
				if (GetDungeon(universe, i, j - 1) == 13)
					SetDungeon(universe, i, j - 1, 205);
				break;
			case 121:
				if (GetDungeon(universe, i, j - 1) == 13)
					SetDungeon(universe, i, j - 1, 205);
				break;
			case 122:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 211);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 212);
				break;
			case 123:
				if (GetDungeon(universe, i, j - 1) == 13)
					SetDungeon(universe, i, j - 1, 205);
				break;
			case 125:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 203);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 204);
				break;
			case 126:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 206);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 207);
				break;
			case 128:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 206);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 207);
				break;
			case 133:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 203);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 204);
				if (GetDungeon(universe, i, j - 1) == 13)
					SetDungeon(universe, i, j - 1, 205);
				break;
			case 135:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 206);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 207);
				break;
			case 136:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 203);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 204);
				break;
			case 137:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 213);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 214);
				if (GetDungeon(universe, i, j - 1) == 13)
					SetDungeon(universe, i, j - 1, 205);
				break;
			case 138:
				if (GetDungeon(universe, i, j - 1) == 13)
					SetDungeon(universe, i, j - 1, 205);
				break;
			case 139:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 215);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 216);
				break;
			case 140:
				if (GetDungeon(universe, i, j - 1) == 13)
					SetDungeon(universe, i, j - 1, 217);
				break;
			case 142:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 203);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 204);
				break;
			case 143:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 213);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 214);
				break;
			case 145:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 213);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 214);
				break;
			case 150:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 203);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 204);
				if (GetDungeon(universe, i, j - 1) == 13)
					SetDungeon(universe, i, j - 1, 217);
				break;
			case 152:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 206);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 207);
				break;
			case 153:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 203);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 204);
				break;
			case 154:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 206);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 207);
				if (GetDungeon(universe, i, j - 1) == 13)
					SetDungeon(universe, i, j - 1, 205);
				break;
			case 155:
				if (GetDungeon(universe, i, j - 1) == 13)
					SetDungeon(universe, i, j - 1, 205);
				break;
			case 156:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 203);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 204);
				break;
			case 157:
				if (GetDungeon(universe, i, j - 1) == 13)
					SetDungeon(universe, i, j - 1, 217);
				break;
			case 159:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 203);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 204);
				break;
			case 160:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 206);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 207);
				break;
			case 162:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 209);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 210);
				break;
			case 167:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 209);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 210);
				break;
			case 187:
				if (GetDungeon(universe, i, j - 1) == 13)
					SetDungeon(universe, i, j - 1, 208);
				break;
			case 185:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 203);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 204);
				break;
			case 186:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 203);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 204);
				break;
			case 192:
				if (GetDungeon(universe, i - 1, j) == 13)
					SetDungeon(universe, i - 1, j, 209);
				if (GetDungeon(universe, i - 1, j - 1) == 13)
					SetDungeon(universe, i - 1, j - 1, 210);
				break;
			}
		}
	}
}
#endif

static void DRLG_L1Shadows(Universe& universe)
{
	int x, y, i;
	BYTE sd[2][2];
	BYTE tnv3;
	BOOL patflag;

	for (y = 1; y < DMAXY; y++) {
		for (x = 1; x < DMAXX; x++) {
			sd[0][0] = BSTYPES[GetDungeon(universe, x, y)];
			sd[1][0] = BSTYPES[GetDungeon(universe, x - 1, y)];
			sd[0][1] = BSTYPES[GetDungeon(universe, x, y - 1)];
			sd[1][1] = BSTYPES[GetDungeon(universe, x - 1, y - 1)];

			for (i = 0; i < 37; i++) {
				if (SPATS[i].strig == sd[0][0]) {
					patflag = TRUE;
					if (SPATS[i].s1 && SPATS[i].s1 != sd[1][1])
						patflag = FALSE;
					if (SPATS[i].s2 && SPATS[i].s2 != sd[0][1])
						patflag = FALSE;
					if (SPATS[i].s3 && SPATS[i].s3 != sd[1][0])
						patflag = FALSE;
					if (patflag == TRUE) {
						if (SPATS[i].nv1 && !universe.L5dflags[x - 1][y - 1])
							SetDungeon(universe, x - 1, y - 1, SPATS[i].nv1);
						if (SPATS[i].nv2 && !universe.L5dflags[x][y - 1])
							SetDungeon(universe, x, y - 1, SPATS[i].nv2);
						if (SPATS[i].nv3 && !universe.L5dflags[x - 1][y])
							SetDungeon(universe, x - 1, y, SPATS[i].nv3);
					}
				}
			}
		}
	}

	for (y = 1; y < DMAXY; y++) {
		for (x = 1; x < DMAXX; x++) {
			if (GetDungeon(universe, x - 1, y) == 139 && !universe.L5dflags[x - 1][y]) {
				tnv3 = 139;
				if (GetDungeon(universe, x, y) == 29)
					tnv3 = 141;
				if (GetDungeon(universe, x, y) == 32)
					tnv3 = 141;
				if (GetDungeon(universe, x, y) == 35)
					tnv3 = 141;
				if (GetDungeon(universe, x, y) == 37)
					tnv3 = 141;
				if (GetDungeon(universe, x, y) == 38)
					tnv3 = 141;
				if (GetDungeon(universe, x, y) == 39)
					tnv3 = 141;
				SetDungeon(universe, x - 1, y, tnv3);
			}
			if (GetDungeon(universe, x - 1, y) == 149 && !universe.L5dflags[x - 1][y]) {
				tnv3 = 149;
				if (GetDungeon(universe, x, y) == 29)
					tnv3 = 153;
				if (GetDungeon(universe, x, y) == 32)
					tnv3 = 153;
				if (GetDungeon(universe, x, y) == 35)
					tnv3 = 153;
				if (GetDungeon(universe, x, y) == 37)
					tnv3 = 153;
				if (GetDungeon(universe, x, y) == 38)
					tnv3 = 153;
				if (GetDungeon(universe, x, y) == 39)
					tnv3 = 153;
				SetDungeon(universe, x - 1, y, tnv3);
			}
			if (GetDungeon(universe, x - 1, y) == 148 && !universe.L5dflags[x - 1][y]) {
				tnv3 = 148;
				if (GetDungeon(universe, x, y) == 29)
					tnv3 = 154;
				if (GetDungeon(universe, x, y) == 32)
					tnv3 = 154;
				if (GetDungeon(universe, x, y) == 35)
					tnv3 = 154;
				if (GetDungeon(universe, x, y) == 37)
					tnv3 = 154;
				if (GetDungeon(universe, x, y) == 38)
					tnv3 = 154;
				if (GetDungeon(universe, x, y) == 39)
					tnv3 = 154;
				SetDungeon(universe, x - 1, y, tnv3);
			}
		}
	}
}

static int DRLG_PlaceMiniSet(Universe& universe, const BYTE *miniset, int tmin, int tmax, int cx, int cy, BOOL setview, int noquad, int ldir)
{
	int sx, sy, sw, sh, xx, yy, i, ii, numt, found, t;
	BOOL abort;

	sw = miniset[0];
	sh = miniset[1];

	if (tmax - tmin == 0)
		numt = 1;
	else
		numt = random_(0, tmax - tmin) + tmin;

	for (i = 0; i < numt; i++) {
		sx = random_(0, DMAXX - sw);
		sy = random_(0, DMAXY - sh);
		abort = FALSE;
		found = 0;

		while (abort == FALSE) {
			abort = TRUE;
			// BUGFIX: This code has no purpose but causes the set piece to never appear in x 0-13 or y 0-13
			if (cx != -1 && sx >= cx - sw && sx <= cx + 12) {
				sx++;
				abort = FALSE;
			}
			if (cy != -1 && sy >= cy - sh && sy <= cy + 12) {
				sy++;
				abort = FALSE;
			}

			switch (noquad) {
			case 0:
				if (sx < cx && sy < cy)
					abort = FALSE;
				break;
			case 1:
				if (sx > cx && sy < cy)
					abort = FALSE;
				break;
			case 2:
				if (sx < cx && sy > cy)
					abort = FALSE;
				break;
			case 3:
				if (sx > cx && sy > cy)
					abort = FALSE;
				break;
			}

			ii = 2;

			for (yy = 0; yy < sh && abort == TRUE; yy++) {
				for (xx = 0; xx < sw && abort == TRUE; xx++) {
					if (miniset[ii] && GetDungeon(universe, xx + sx, sy + yy) != miniset[ii])
						abort = FALSE;
					if (universe.L5dflags[xx + sx][sy + yy])
						abort = FALSE;
					ii++;
				}
			}

			if (abort == FALSE) {
				if (++sx == DMAXX - sw) {
					sx = 0;
					if (++sy == DMAXY - sh)
						sy = 0;
				}
				if (++found > 4000)
					return -1;
			}
		}

		ii = sw * sh + 2;

		for (yy = 0; yy < sh; yy++) {
			for (xx = 0; xx < sw; xx++) {
				if (miniset[ii])
					SetDungeon(universe, xx + sx, sy + yy, miniset[ii]);
				ii++;
			}
		}
	}

	if (miniset == PWATERIN) {
		t = TransVal;
		TransVal = 0;
		DRLG_MRectTrans(sx, sy + 2, sx + 5, sy + 4);
		TransVal = t;

		quests[Q_PWATER]._qtx = 2 * sx + 21;
		quests[Q_PWATER]._qty = 2 * sy + 22;
	}

	if (setview == TRUE) {
		ViewX = 2 * sx + 19;
		ViewY = 2 * sy + 20;
	}

	if (ldir == 0) {
		LvlViewX = 2 * sx + 19;
		LvlViewY = 2 * sy + 20;
	}

	if (sx < cx && sy < cy)
		return 0;
	if (sx > cx && sy < cy)
		return 1;
	if (sx < cx && sy > cy)
		return 2;
	else
		return 3;
}

static void DRLG_L1Floor(Universe& universe)
{
	int i, j;
	LONG rv;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (universe.L5dflags[i][j] == 0 && GetDungeon(universe, i, j) == 13) {
				rv = random_(0, 3);

				if (rv == 1)
					SetDungeon(universe, i, j, 162);
				if (rv == 2)
					SetDungeon(universe, i, j, 163);
			}
		}
	}
}

static void DRLG_L1Pass3(Universe& universe)
{
	int i, j, xx, yy;
	long v1, v2, v3, v4, lv;

	lv = 22 - 1;

#ifdef USE_ASM
	__asm {
		mov		esi, pMegaTiles
		mov		eax, lv
		shl		eax, 3
		add		esi, eax
		xor		eax, eax
		lodsw
		inc		eax
		mov		v1, eax
		lodsw
		inc		eax
		mov		v2, eax
		lodsw
		inc		eax
		mov		v3, eax
		lodsw
		inc		eax
		mov		v4, eax
	}
#else
	v1 = *((WORD *)&pMegaTiles[lv * 8] + 0) + 1;
	v2 = *((WORD *)&pMegaTiles[lv * 8] + 1) + 1;
	v3 = *((WORD *)&pMegaTiles[lv * 8] + 2) + 1;
	v4 = *((WORD *)&pMegaTiles[lv * 8] + 3) + 1;
#endif

	for (j = 0; j < MAXDUNY; j += 2)
	{
		for (i = 0; i < MAXDUNX; i += 2) {
			dPiece[i][j] = v1;
			dPiece[i + 1][j] = v2;
			dPiece[i][j + 1] = v3;
			dPiece[i + 1][j + 1] = v4;
		}
	}

	yy = 16;
	for (j = 0; j < DMAXY; j++) {
		xx = 16;
		for (i = 0; i < DMAXX; i++) {
			lv = GetDungeon(universe, i, j) - 1;
			/// ASSERT: assert(lv >= 0);
#ifdef USE_ASM
			__asm {
				mov		esi, pMegaTiles
				mov		eax, lv
				shl		eax, 3
				add		esi, eax
				xor		eax, eax
				lodsw
				inc		eax
				mov		v1, eax
				lodsw
				inc		eax
				mov		v2, eax
				lodsw
				inc		eax
				mov		v3, eax
				lodsw
				inc		eax
				mov		v4, eax
			}
#else
			v1 = *((WORD *)&pMegaTiles[lv * 8] + 0) + 1;
			v2 = *((WORD *)&pMegaTiles[lv * 8] + 1) + 1;
			v3 = *((WORD *)&pMegaTiles[lv * 8] + 2) + 1;
			v4 = *((WORD *)&pMegaTiles[lv * 8] + 3) + 1;
#endif
			dPiece[xx][yy] = v1;
			dPiece[xx + 1][yy] = v2;
			dPiece[xx][yy + 1] = v3;
			dPiece[xx + 1][yy + 1] = v4;
			xx += 2;
		}
		yy += 2;
	}
}

static void DRLG_LoadL1SP(Universe& universe)
{
	universe.L5setloadflag = FALSE;
	if (QuestStatus(Q_BUTCHER)) {
		universe.L5pSetPiece = LoadFileInMem("Levels\\L1Data\\rnd6.DUN", NULL);
		universe.L5setloadflag = TRUE;
	}
	if (QuestStatus(Q_SKELKING) && universe.gbMaxPlayers == 1) {
		universe.L5pSetPiece = LoadFileInMem("Levels\\L1Data\\SKngDO.DUN", NULL);
		universe.L5setloadflag = TRUE;
	}
	if (QuestStatus(Q_LTBANNER)) {
		universe.L5pSetPiece = LoadFileInMem("Levels\\L1Data\\Banner2.DUN", NULL);
		universe.L5setloadflag = TRUE;
	}
}

static void DRLG_FreeL1SP(Universe& universe)
{
	MemFreeDbg(universe.L5pSetPiece);
}

void DRLG_Init_Globals(Universe& universe)
{
	char c;

	memset(dFlags, 0, sizeof(dFlags));
	memset(dPlayer, 0, sizeof(dPlayer));
	memset(dMonster, 0, sizeof(dMonster));
	memset(dDead, 0, sizeof(dDead));
	memset(dObject, 0, sizeof(dObject));
	memset(dItem, 0, sizeof(dItem));
	memset(dMissile, 0, sizeof(dMissile));
	if (!lightflag) {
		if (universe.light4flag)
			c = 3;
		else
			c = 15;
	} else {
		c = 0;
	}
	memset(dLight, c, sizeof(dLight));
}

static void DRLG_InitL1Vals()
{
	int i, j, pc;

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dPiece[i][j] == 12) {
				pc = 1;
			} else if (dPiece[i][j] == 11) {
				pc = 2;
			} else if (dPiece[i][j] == 71) {
				pc = 1;
			} else if (dPiece[i][j] == 259) {
				pc = 5;
			} else if (dPiece[i][j] == 249) {
				pc = 2;
			} else if (dPiece[i][j] == 325) {
				pc = 2;
			} else if (dPiece[i][j] == 321) {
				pc = 1;
			} else if (dPiece[i][j] == 255) {
				pc = 4;
			} else if (dPiece[i][j] == 211) {
				pc = 1;
			} else if (dPiece[i][j] == 344) {
				pc = 2;
			} else if (dPiece[i][j] == 341) {
				pc = 1;
			} else if (dPiece[i][j] == 331) {
				pc = 2;
			} else if (dPiece[i][j] == 418) {
				pc = 1;
			} else if (dPiece[i][j] == 421) {
				pc = 2;
			} else {
				continue;
			}
			dSpecial[i][j] = pc;
		}
	}
}

#ifndef SPAWN
void LoadL1Dungeon(Universe& universe, const char *sFileName, int vx, int vy)
{
	int i, j, rw, rh;
	BYTE *pLevelMap, *lm;

	dminx = 16;
	dminy = 16;
	dmaxx = 96;
	dmaxy = 96;

	DRLG_InitTrans();
	pLevelMap = LoadFileInMem(sFileName, NULL);

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			SetDungeon(universe, i, j, 22);
			universe.L5dflags[i][j] = 0;
		}
	}

	lm = pLevelMap;
	rw = *lm;
	lm += 2;
	rh = *lm;
	lm += 2;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*lm != 0) {
				SetDungeon(universe, i, j, *lm);
				universe.L5dflags[i][j] |= DLRG_PROTECTED;
			} else {
				SetDungeon(universe, i, j, 13);
			}
			lm += 2;
		}
	}

	DRLG_L1Floor(universe);
	ViewX = vx;
	ViewY = vy;
	DRLG_L1Pass3(universe);
	DRLG_Init_Globals(universe);
	SetMapMonsters(universe, pLevelMap, 0, 0);
	SetMapObjects(universe, pLevelMap, 0, 0);
	mem_free_dbg(pLevelMap);
}

void LoadPreL1Dungeon(Universe& universe, const char *sFileName, int vx, int vy)
{
	int i, j, rw, rh;
	BYTE *pLevelMap, *lm;

	dminx = 16;
	dminy = 16;
	dmaxx = 96;
	dmaxy = 96;

	pLevelMap = LoadFileInMem(sFileName, NULL);

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			SetDungeon(universe, i, j, 22);
			universe.L5dflags[i][j] = 0;
		}
	}

	lm = pLevelMap;
	rw = *lm;
	lm += 2;
	rh = *lm;
	lm += 2;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*lm != 0) {
				SetDungeon(universe, i, j, *lm);
				universe.L5dflags[i][j] |= DLRG_PROTECTED;
			} else {
				SetDungeon(universe, i, j, 13);
			}
			lm += 2;
		}
	}

	DRLG_L1Floor(universe);

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			pdungeon[i][j] = GetDungeon(universe, i, j);
		}
	}

	mem_free_dbg(pLevelMap);
}
#endif

static void InitL5Dungeon(Universe& universe)
{
	int i, j;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			SetDungeon(universe, i, j, 0);
			universe.L5dflags[i][j] = 0;
		}
	}
}

static void L5ClearFlags(Universe& universe)
{
	int i, j;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			universe.L5dflags[i][j] &= 0xBF;
		}
	}
}

static void L5drawRoom(Universe& universe, int x, int y, int w, int h)
{
	int i, j;

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			SetDungeon(universe, x + i, y + j, 1);
		}
	}
}

static BOOL L5checkRoom(Universe& universe, int x, int y, int width, int height)
{
	int i, j;

	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			if (i + x < 0 || i + x >= DMAXX || j + y < 0 || j + y >= DMAXY)
				return FALSE;
			if (GetDungeon(universe, i + x, j + y))
				return FALSE;
		}
	}

	return TRUE;
}

static void L5roomGen(Universe& universe, int x, int y, int w, int h, int dir)
{
	int num, dirProb;
	BOOL ran, ran2;
	int width, height, rx, ry, ry2;
	int cw, ch, cx1, cy1, cx2;

	dirProb = random_(0, 4);

	if (!(dir == 1 ? dirProb != 0 : dirProb == 0)) {
		num = 0;
		do {
			cw = (random_(0, 5) + 2) & 0xFFFFFFFE;
			ch = (random_(0, 5) + 2) & 0xFFFFFFFE;
			cy1 = h / 2 + y - ch / 2;
			cx1 = x - cw;
			ran = L5checkRoom(universe, cx1 - 1, cy1 - 1, ch + 2, cw + 1); /// BUGFIX: swap args 3 and 4 ("ch+2" and "cw+1")
			num++;
		} while (ran == FALSE && num < 20);

		if (ran == TRUE)
			L5drawRoom(universe, cx1, cy1, cw, ch);
		cx2 = x + w;
		ran2 = L5checkRoom(universe, cx2, cy1 - 1, cw + 1, ch + 2);
		if (ran2 == TRUE)
			L5drawRoom(universe, cx2, cy1, cw, ch);
		if (ran == TRUE)
			L5roomGen(universe, cx1, cy1, cw, ch, 1);
		if (ran2 == TRUE)
			L5roomGen(universe, cx2, cy1, cw, ch, 1);
	} else {
		num = 0;
		do {
			width = (random_(0, 5) + 2) & 0xFFFFFFFE;
			height = (random_(0, 5) + 2) & 0xFFFFFFFE;
			rx = w / 2 + x - width / 2;
			ry = y - height;
			ran = L5checkRoom(universe, rx - 1, ry - 1, width + 2, height + 1);
			num++;
		} while (ran == FALSE && num < 20);

		if (ran == TRUE)
			L5drawRoom(universe, rx, ry, width, height);
		ry2 = y + h;
		ran2 = L5checkRoom(universe, rx - 1, ry2, width + 2, height + 1);
		if (ran2 == TRUE)
			L5drawRoom(universe, rx, ry2, width, height);
		if (ran == TRUE)
			L5roomGen(universe, rx, ry, width, height, 0);
		if (ran2 == TRUE)
			L5roomGen(universe, rx, ry2, width, height, 0);
	}
}

static void L5firstRoom(Universe& universe)
{
	int ys, ye, y;
	int xs, xe, x;

	if (random_(0, 2) == 0) {
		ys = 1;
		ye = DMAXY - 1;

		universe.VR1 = random_(0, 2);
		universe.VR2 = random_(0, 2);
		universe.VR3 = random_(0, 2);

		if (universe.VR1 + universe.VR3 <= 1)
			universe.VR2 = 1;
		if (universe.VR1)
			L5drawRoom(universe, 15, 1, 10, 10);
		else
			ys = 18;

		if (universe.VR2)
			L5drawRoom(universe, 15, 15, 10, 10);
		if (universe.VR3)
			L5drawRoom(universe, 15, 29, 10, 10);
		else
			ye = 22;

		for (y = ys; y < ye; y++) {
			SetDungeon(universe, 17, y, 1);
			SetDungeon(universe, 18, y, 1);
			SetDungeon(universe, 19, y, 1);
			SetDungeon(universe, 20, y, 1);
			SetDungeon(universe, 21, y, 1);
			SetDungeon(universe, 22, y, 1);
		}

		if (universe.VR1)
			L5roomGen(universe, 15, 1, 10, 10, 0);
		if (universe.VR2)
			L5roomGen(universe, 15, 15, 10, 10, 0);
		if (universe.VR3)
			L5roomGen(universe, 15, 29, 10, 10, 0);

		universe.HR3 = 0;
		universe.HR2 = 0;
		universe.HR1 = 0;
	} else {
		xs = 1;
		xe = DMAXX - 1;

		universe.HR1 = random_(0, 2);
		universe.HR2 = random_(0, 2);
		universe.HR3 = random_(0, 2);

		if (universe.HR1 + universe.HR3 <= 1)
			universe.HR2 = 1;
		if (universe.HR1)
			L5drawRoom(universe, 1, 15, 10, 10);
		else
			xs = 18;

		if (universe.HR2)
			L5drawRoom(universe, 15, 15, 10, 10);
		if (universe.HR3)
			L5drawRoom(universe, 29, 15, 10, 10);
		else
			xe = 22;

		for (x = xs; x < xe; x++) {
			SetDungeon(universe, x, 17, 1);
			SetDungeon(universe, x, 18, 1);
			SetDungeon(universe, x, 19, 1);
			SetDungeon(universe, x, 20, 1);
			SetDungeon(universe, x, 21, 1);
			SetDungeon(universe, x, 22, 1);
		}

		if (universe.HR1)
			L5roomGen(universe, 1, 15, 10, 10, 1);
		if (universe.HR2)
			L5roomGen(universe, 15, 15, 10, 10, 1);
		if (universe.HR3)
			L5roomGen(universe, 29, 15, 10, 10, 1);

		universe.VR3 = 0;
		universe.VR2 = 0;
		universe.VR1 = 0;
	}
}

static int L5GetArea(Universe& universe)
{
	int i, j;
	int rv;

	rv = 0;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (GetDungeon(universe, i, j) == 1)
				rv++;
		}
	}

	return rv;
}

static void L5makeDungeon(Universe& universe)
{
	int i, j;
	int i_2, j_2;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			j_2 = j << 1;
			i_2 = i << 1;
			universe.L5dungeon[i_2][j_2] = GetDungeon(universe, i, j);
			universe.L5dungeon[i_2][j_2 + 1] = GetDungeon(universe, i, j);
			universe.L5dungeon[i_2 + 1][j_2] = GetDungeon(universe, i, j);
			universe.L5dungeon[i_2 + 1][j_2 + 1] = GetDungeon(universe, i, j);
		}
	}
}

static void L5makeDmt(Universe& universe)
{
	int i, j, idx, val, dmtx, dmty;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			SetDungeon(universe, i, j, 22);
		}
	}

	for (j = 0, dmty = 1; dmty <= 77; j++, dmty += 2) {
		for (i = 0, dmtx = 1; dmtx <= 77; i++, dmtx += 2) {
			val = 8 * universe.L5dungeon[dmtx + 1][dmty + 1]
			    + 4 * universe.L5dungeon[dmtx][dmty + 1]
			    + 2 * universe.L5dungeon[dmtx + 1][dmty]
			    + universe.L5dungeon[dmtx][dmty];
			idx = L5ConvTbl[val];
			SetDungeon(universe, i, j, idx);
		}
	}
}

static int L5HWallOk(Universe& universe, int i, int j)
{
	int x;
	BOOL wallok;

	for (x = 1; GetDungeon(universe, i + x, j) == 13; x++) {
		if (GetDungeon(universe, i + x, j - 1) != 13 || GetDungeon(universe, i + x, j + 1) != 13 || universe.L5dflags[i + x][j])
			break;
	}

	wallok = FALSE;
	if (GetDungeon(universe, i + x, j) >= 3 && GetDungeon(universe, i + x, j) <= 7)
		wallok = TRUE;
	if (GetDungeon(universe, i + x, j) >= 16 && GetDungeon(universe, i + x, j) <= 24)
		wallok = TRUE;
	if (GetDungeon(universe, i + x, j) == 22)
		wallok = FALSE;
	if (x == 1)
		wallok = FALSE;

	if (wallok)
		return x;
	else
		return -1;
}

static int L5VWallOk(Universe& universe, int i, int j)
{
	int y;
	BOOL wallok;

	for (y = 1; GetDungeon(universe, i, j + y) == 13; y++) {
		if (GetDungeon(universe, i - 1, j + y) != 13 || GetDungeon(universe, i + 1, j + y) != 13 || universe.L5dflags[i][j + y])
			break;
	}

	wallok = FALSE;
	if (GetDungeon(universe, i, j + y) >= 3 && GetDungeon(universe, i, j + y) <= 7)
		wallok = TRUE;
	if (GetDungeon(universe, i, j + y) >= 16 && GetDungeon(universe, i, j + y) <= 24)
		wallok = TRUE;
	if (GetDungeon(universe, i, j + y) == 22)
		wallok = FALSE;
	if (y == 1)
		wallok = FALSE;

	if (wallok)
		return y;
	else
		return -1;
}

static void L5HorizWall(Universe& universe, int i, int j, char p, int dx)
{
	int xx;
	char wt, dt;

	switch (random_(0, 4)) {
	case 0:
	case 1:
		dt = 2;
		break;
	case 2:
		dt = 12;
		if (p == 2)
			p = 12;
		if (p == 4)
			p = 10;
		break;
	case 3:
		dt = 36;
		if (p == 2)
			p = 36;
		if (p == 4)
			p = 27;
		break;
	}

	if (random_(0, 6) == 5)
		wt = 12;
	else
		wt = 26;
	if (dt == 12)
		wt = 12;

	SetDungeon(universe, i, j, p);

	for (xx = 1; xx < dx; xx++) {
		SetDungeon(universe, i + xx, j, dt);
	}

	xx = random_(0, dx - 1) + 1;

	if (wt == 12) {
		SetDungeon(universe, i + xx, j, wt);
	} else {
		SetDungeon(universe, i + xx, j, 2);
		universe.L5dflags[i + xx][j] |= DLRG_HDOOR;
	}
}

static void L5VertWall(Universe& universe, int i, int j, char p, int dy)
{
	int yy;
	char wt, dt;

	switch (random_(0, 4)) {
	case 0:
	case 1:
		dt = 1;
		break;
	case 2:
		dt = 11;
		if (p == 1)
			p = 11;
		if (p == 4)
			p = 14;
		break;
	case 3:
		dt = 35;
		if (p == 1)
			p = 35;
		if (p == 4)
			p = 37;
		break;
	}

	if (random_(0, 6) == 5)
		wt = 11;
	else
		wt = 25;
	if (dt == 11)
		wt = 11;

	SetDungeon(universe, i, j, p);

	for (yy = 1; yy < dy; yy++) {
		SetDungeon(universe, i, j + yy, dt);
	}

	yy = random_(0, dy - 1) + 1;

	if (wt == 11) {
		SetDungeon(universe, i, j + yy, wt);
	} else {
		SetDungeon(universe, i, j + yy, 1);
		universe.L5dflags[i][j + yy] |= DLRG_VDOOR;
	}
}

static void L5AddWall(Universe& universe)
{
	int i, j, x, y;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (!universe.L5dflags[i][j]) {
				if (GetDungeon(universe, i, j) == 3 && random_(0, 100) < 100) {
					x = L5HWallOk(universe, i, j);
					if (x != -1)
						L5HorizWall(universe, i, j, 2, x);
				}
				if (GetDungeon(universe, i, j) == 3 && random_(0, 100) < 100) {
					y = L5VWallOk(universe, i, j);
					if (y != -1)
						L5VertWall(universe, i, j, 1, y);
				}
				if (GetDungeon(universe, i, j) == 6 && random_(0, 100) < 100) {
					x = L5HWallOk(universe, i, j);
					if (x != -1)
						L5HorizWall(universe, i, j, 4, x);
				}
				if (GetDungeon(universe, i, j) == 7 && random_(0, 100) < 100) {
					y = L5VWallOk(universe, i, j);
					if (y != -1)
						L5VertWall(universe, i, j, 4, y);
				}
				if (GetDungeon(universe, i, j) == 2 && random_(0, 100) < 100) {
					x = L5HWallOk(universe, i, j);
					if (x != -1)
						L5HorizWall(universe, i, j, 2, x);
				}
				if (GetDungeon(universe, i, j) == 1 && random_(0, 100) < 100) {
					y = L5VWallOk(universe, i, j);
					if (y != -1)
						L5VertWall(universe, i, j, 1, y);
				}
			}
		}
	}
}

static void DRLG_L5GChamber(Universe& universe, int sx, int sy, BOOL topflag, BOOL bottomflag, BOOL leftflag, BOOL rightflag)
{
	int i, j;

	if (topflag == TRUE) {
		SetDungeon(universe, sx + 2, sy, 12);
		SetDungeon(universe, sx + 3, sy, 12);
		SetDungeon(universe, sx + 4, sy, 3);
		SetDungeon(universe, sx + 7, sy, 9);
		SetDungeon(universe, sx + 8, sy, 12);
		SetDungeon(universe, sx + 9, sy, 2);
	}
	if (bottomflag == TRUE) {
		sy += 11;
		SetDungeon(universe, sx + 2, sy, 10);
		SetDungeon(universe, sx + 3, sy, 12);
		SetDungeon(universe, sx + 4, sy, 8);
		SetDungeon(universe, sx + 7, sy, 5);
		SetDungeon(universe, sx + 8, sy, 12);
		if (GetDungeon(universe, sx + 9, sy) != 4) {
			SetDungeon(universe, sx + 9, sy, 21);
		}
		sy -= 11;
	}
	if (leftflag == TRUE) {
		SetDungeon(universe, sx, sy + 2, 11);
		SetDungeon(universe, sx, sy + 3, 11);
		SetDungeon(universe, sx, sy + 4, 3);
		SetDungeon(universe, sx, sy + 7, 8);
		SetDungeon(universe, sx, sy + 8, 11);
		SetDungeon(universe, sx, sy + 9, 1);
	}
	if (rightflag == TRUE) {
		sx += 11;
		SetDungeon(universe, sx, sy + 2, 14);
		SetDungeon(universe, sx, sy + 3, 11);
		SetDungeon(universe, sx, sy + 4, 9);
		SetDungeon(universe, sx, sy + 7, 5);
		SetDungeon(universe, sx, sy + 8, 11);
		if (GetDungeon(universe, sx, sy + 9) != 4) {
			SetDungeon(universe, sx, sy + 9, 21);
		}
		sx -= 11;
	}

	for (j = 1; j < 11; j++) {
		for (i = 1; i < 11; i++) {
			SetDungeon(universe, i + sx, j + sy, 13);
			universe.L5dflags[i + sx][j + sy] |= DLRG_CHAMBER;
		}
	}

	SetDungeon(universe, sx + 4, sy + 4, 15);
	SetDungeon(universe, sx + 7, sy + 4, 15);
	SetDungeon(universe, sx + 4, sy + 7, 15);
	SetDungeon(universe, sx + 7, sy + 7, 15);
}

static void DRLG_L5GHall(Universe& universe, int x1, int y1, int x2, int y2)
{
	int i;

	if (y1 == y2) {
		for (i = x1; i < x2; i++) {
			SetDungeon(universe, i, y1, 12);
			SetDungeon(universe, i, y1 + 3, 12);
		}
	} else {
		for (i = y1; i < y2; i++) {
			SetDungeon(universe, x1, i, 11);
			SetDungeon(universe, x1 + 3, i, 11);
		}
	}
}

static void L5tileFix(Universe& universe)
{
	int i, j;

	// BUGFIX: Bounds checks are required in all loop bodies.
	// See https://github.com/diasurgical/devilutionX/pull/401

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (GetDungeon(universe, i, j) == 2 && GetDungeon(universe, i + 1, j) == 22)
				SetDungeon(universe, i + 1, j, 23);
			if (GetDungeon(universe, i, j) == 13 && GetDungeon(universe, i + 1, j) == 22)
				SetDungeon(universe, i + 1, j, 18);
			if (GetDungeon(universe, i, j) == 13 && GetDungeon(universe, i + 1, j) == 2)
				SetDungeon(universe, i + 1, j, 7);
			if (GetDungeon(universe, i, j) == 6 && GetDungeon(universe, i + 1, j) == 22)
				SetDungeon(universe, i + 1, j, 24);
			if (GetDungeon(universe, i, j) == 1 && GetDungeon(universe, i, j + 1) == 22)
				SetDungeon(universe, i, j + 1, 24);
			if (GetDungeon(universe, i, j) == 13 && GetDungeon(universe, i, j + 1) == 1)
				SetDungeon(universe, i, j + 1, 6);
			if (GetDungeon(universe, i, j) == 13 && GetDungeon(universe, i, j + 1) == 22)
				SetDungeon(universe, i, j + 1, 19);
		}
	}

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (GetDungeon(universe, i, j) == 13 && GetDungeon(universe, i + 1, j) == 19)
				SetDungeon(universe, i + 1, j, 21);
			if (GetDungeon(universe, i, j) == 13 && GetDungeon(universe, i + 1, j) == 22)
				SetDungeon(universe, i + 1, j, 20);
			if (GetDungeon(universe, i, j) == 7 && GetDungeon(universe, i + 1, j) == 22)
				SetDungeon(universe, i + 1, j, 23);
			if (GetDungeon(universe, i, j) == 13 && GetDungeon(universe, i + 1, j) == 24)
				SetDungeon(universe, i + 1, j, 21);
			if (GetDungeon(universe, i, j) == 19 && GetDungeon(universe, i + 1, j) == 22)
				SetDungeon(universe, i + 1, j, 20);
			if (GetDungeon(universe, i, j) == 2 && GetDungeon(universe, i + 1, j) == 19)
				SetDungeon(universe, i + 1, j, 21);
			if (GetDungeon(universe, i, j) == 19 && GetDungeon(universe, i + 1, j) == 1)
				SetDungeon(universe, i + 1, j, 6);
			if (GetDungeon(universe, i, j) == 7 && GetDungeon(universe, i + 1, j) == 19)
				SetDungeon(universe, i + 1, j, 21);
			if (GetDungeon(universe, i, j) == 2 && GetDungeon(universe, i + 1, j) == 1)
				SetDungeon(universe, i + 1, j, 6);
			if (GetDungeon(universe, i, j) == 3 && GetDungeon(universe, i + 1, j) == 22)
				SetDungeon(universe, i + 1, j, 24);
			if (GetDungeon(universe, i, j) == 21 && GetDungeon(universe, i + 1, j) == 1)
				SetDungeon(universe, i + 1, j, 6);
			if (GetDungeon(universe, i, j) == 7 && GetDungeon(universe, i + 1, j) == 1)
				SetDungeon(universe, i + 1, j, 6);
			if (GetDungeon(universe, i, j) == 7 && GetDungeon(universe, i + 1, j) == 24)
				SetDungeon(universe, i + 1, j, 21);
			if (GetDungeon(universe, i, j) == 4 && GetDungeon(universe, i + 1, j) == 16)
				SetDungeon(universe, i + 1, j, 17);
			if (GetDungeon(universe, i, j) == 7 && GetDungeon(universe, i + 1, j) == 13)
				SetDungeon(universe, i + 1, j, 17);
			if (GetDungeon(universe, i, j) == 2 && GetDungeon(universe, i + 1, j) == 24)
				SetDungeon(universe, i + 1, j, 21);
			if (GetDungeon(universe, i, j) == 2 && GetDungeon(universe, i + 1, j) == 13)
				SetDungeon(universe, i + 1, j, 17);
			if (GetDungeon(universe, i, j) == 23 && GetDungeon(universe, i - 1, j) == 22)
				SetDungeon(universe, i - 1, j, 19);
			if (GetDungeon(universe, i, j) == 19 && GetDungeon(universe, i - 1, j) == 23)
				SetDungeon(universe, i - 1, j, 21);
			if (GetDungeon(universe, i, j) == 6 && GetDungeon(universe, i - 1, j) == 22)
				SetDungeon(universe, i - 1, j, 24);
			if (GetDungeon(universe, i, j) == 6 && GetDungeon(universe, i - 1, j) == 23)
				SetDungeon(universe, i - 1, j, 21);
			if (GetDungeon(universe, i, j) == 1 && GetDungeon(universe, i, j + 1) == 2)
				SetDungeon(universe, i, j + 1, 7);
			if (GetDungeon(universe, i, j) == 6 && GetDungeon(universe, i, j + 1) == 18)
				SetDungeon(universe, i, j + 1, 21);
			if (GetDungeon(universe, i, j) == 18 && GetDungeon(universe, i, j + 1) == 2)
				SetDungeon(universe, i, j + 1, 7);
			if (GetDungeon(universe, i, j) == 6 && GetDungeon(universe, i, j + 1) == 2)
				SetDungeon(universe, i, j + 1, 7);
			if (GetDungeon(universe, i, j) == 21 && GetDungeon(universe, i, j + 1) == 2)
				SetDungeon(universe, i, j + 1, 7);
			if (GetDungeon(universe, i, j) == 6 && GetDungeon(universe, i, j + 1) == 22)
				SetDungeon(universe, i, j + 1, 24);
			if (GetDungeon(universe, i, j) == 6 && GetDungeon(universe, i, j + 1) == 13)
				SetDungeon(universe, i, j + 1, 16);
			if (GetDungeon(universe, i, j) == 1 && GetDungeon(universe, i, j + 1) == 13)
				SetDungeon(universe, i, j + 1, 16);
			if (GetDungeon(universe, i, j) == 13 && GetDungeon(universe, i, j + 1) == 16)
				SetDungeon(universe, i, j + 1, 17);
			if (GetDungeon(universe, i, j) == 6 && GetDungeon(universe, i, j - 1) == 22)
				SetDungeon(universe, i, j - 1, 7);
			if (GetDungeon(universe, i, j) == 6 && GetDungeon(universe, i, j - 1) == 22)
				SetDungeon(universe, i, j - 1, 24);
			if (GetDungeon(universe, i, j) == 7 && GetDungeon(universe, i, j - 1) == 24)
				SetDungeon(universe, i, j - 1, 21);
			if (GetDungeon(universe, i, j) == 18 && GetDungeon(universe, i, j - 1) == 24)
				SetDungeon(universe, i, j - 1, 21);
		}
	}

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (GetDungeon(universe, i, j) == 4 && GetDungeon(universe, i, j + 1) == 2)
				SetDungeon(universe, i, j + 1, 7);
			if (GetDungeon(universe, i, j) == 2 && GetDungeon(universe, i + 1, j) == 19)
				SetDungeon(universe, i + 1, j, 21);
			if (GetDungeon(universe, i, j) == 18 && GetDungeon(universe, i, j + 1) == 22)
				SetDungeon(universe, i, j + 1, 20);
		}
	}
}

#ifdef HELLFIRE
void drlg_l1_crypt_rndset(const BYTE *miniset, int rndper)
{
	int sx, sy, sw, sh, xx, yy, ii, kk;
	BOOL found;

	sw = miniset[0];
	sh = miniset[1];

	for (sy = 0; sy < DMAXY - sh; sy++) {
		for (sx = 0; sx < DMAXX - sw; sx++) {
			found = TRUE;
			ii = 2;
			for (yy = 0; yy < sh && found == TRUE; yy++) {
				for (xx = 0; xx < sw && found == TRUE; xx++) {
					if (miniset[ii] != 0 && GetDungeon(universe, xx + sx, yy + sy) != miniset[ii]) {
						found = FALSE;
					}
					if (dflags[xx + sx][yy + sy] != 0) { // BUGFIX: Should be universe.L5dflags or it will always be false
						found = FALSE;
					}
					ii++;
				}
			}
			kk = sw * sh + 2;
			// BUGFIX: This code is copied from Cave and should not be applied for crypt
			if (miniset[kk] >= 84 && miniset[kk] <= 100 && found == TRUE) {
				// BUGFIX: accesses to dungeon can go out of bounds
				// BUGFIX: Comparisons vs 100 should use same tile as comparisons vs 84.
				if (GetDungeon(universe, sx - 1, sy) >= 84 && GetDungeon(universe, sx - 1, sy) <= 100) {
					found = FALSE;
				}
				if (GetDungeon(universe, sx + 1, sy) >= 84 && GetDungeon(universe, sx - 1, sy) <= 100) {
					found = FALSE;
				}
				if (GetDungeon(universe, sx, sy + 1) >= 84 && GetDungeon(universe, sx - 1, sy) <= 100) {
					found = FALSE;
				}
				if (GetDungeon(universe, sx, sy - 1) >= 84 && GetDungeon(universe, sx - 1, sy) <= 100) {
					found = FALSE;
				}
			}
			if (found == TRUE && random_(0, 100) < rndper) {
				for (yy = 0; yy < sh; yy++) {
					for (xx = 0; xx < sw; xx++) {
						if (miniset[kk] != 0) {
							SetDungeon(universe, xx + sx, yy + sy, miniset[kk]);
						}
						kk++;
					}
				}
			}
		}
	}
}
#endif

static void DRLG_L5Subs(Universe& universe)
{
	int x, y, rv, i;

	for (y = 0; y < DMAXY; y++) {
		for (x = 0; x < DMAXX; x++) {
			if (random_(0, 4) == 0) {
				BYTE c = L5BTYPES[GetDungeon(universe, x, y)];

				if (c && !universe.L5dflags[x][y]) {
					rv = random_(0, 16);
					i = -1;

					while (rv >= 0) {
						if (++i == sizeof(L5BTYPES) - 1)
							i = 0;
						if (c == L5BTYPES[i])
							rv--;
					}

					// BUGFIX: Add `&& y > 0` to the if statement.
					if (i == 89) {
						if (L5BTYPES[GetDungeon(universe, x, y - 1)] != 79 || universe.L5dflags[x][y - 1])
							i = 79;
						else
							SetDungeon(universe, x, y - 1, 90);
					}
					// BUGFIX: Add `&& x + 1 < DMAXX` to the if statement.
					if (i == 91) {
						if (L5BTYPES[GetDungeon(universe, x + 1, y)] != 80 || universe.L5dflags[x + 1][y])
							i = 80;
						else
							SetDungeon(universe, x + 1, y, 92);
					}
					SetDungeon(universe, x, y, i);
				}
			}
		}
	}
}

static void DRLG_L5SetRoom(Universe& universe, int rx1, int ry1)
{
	int rw, rh, i, j;
	BYTE *sp;

	rw = *universe.L5pSetPiece;
	rh = *(universe.L5pSetPiece + 2);

	setpc_x = rx1;
	setpc_y = ry1;
	setpc_w = rw;
	setpc_h = rh;

	sp = universe.L5pSetPiece + 4;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*sp) {
				SetDungeon(universe, rx1 + i, ry1 + j, *sp);
				universe.L5dflags[rx1 + i][ry1 + j] |= DLRG_PROTECTED;
			} else {
				SetDungeon(universe, rx1 + i, ry1 + j, 13);
			}
			sp += 2;
		}
	}
}

static void L5FillChambers(Universe& universe)
{
	int c;

	if (universe.HR1)
		DRLG_L5GChamber(universe, 0, 14, 0, 0, 0, 1);

	if (universe.HR2) {
		if (universe.HR1 && !universe.HR3)
			DRLG_L5GChamber(universe, 14, 14, 0, 0, 1, 0);
		if (!universe.HR1 && universe.HR3)
			DRLG_L5GChamber(universe, 14, 14, 0, 0, 0, 1);
		if (universe.HR1 && universe.HR3)
			DRLG_L5GChamber(universe, 14, 14, 0, 0, 1, 1);
		if (!universe.HR1 && !universe.HR3)
			DRLG_L5GChamber(universe, 14, 14, 0, 0, 0, 0);
	}

	if (universe.HR3)
		DRLG_L5GChamber(universe, 28, 14, 0, 0, 1, 0);
	if (universe.HR1 && universe.HR2)
		DRLG_L5GHall(universe, 12, 18, 14, 18);
	if (universe.HR2 && universe.HR3)
		DRLG_L5GHall(universe, 26, 18, 28, 18);
	if (universe.HR1 && !universe.HR2 && universe.HR3)
		DRLG_L5GHall(universe, 12, 18, 28, 18);
	if (universe.VR1)
		DRLG_L5GChamber(universe, 14, 0, 0, 1, 0, 0);

	if (universe.VR2) {
		if (universe.VR1 && !universe.VR3)
			DRLG_L5GChamber(universe, 14, 14, 1, 0, 0, 0);
		if (!universe.VR1 && universe.VR3)
			DRLG_L5GChamber(universe, 14, 14, 0, 1, 0, 0);
		if (universe.VR1 && universe.VR3)
			DRLG_L5GChamber(universe, 14, 14, 1, 1, 0, 0);
		if (!universe.VR1 && !universe.VR3)
			DRLG_L5GChamber(universe, 14, 14, 0, 0, 0, 0);
	}

	if (universe.VR3)
		DRLG_L5GChamber(universe, 14, 28, 1, 0, 0, 0);
	if (universe.VR1 && universe.VR2)
		DRLG_L5GHall(universe, 18, 12, 18, 14);
	if (universe.VR2 && universe.VR3)
		DRLG_L5GHall(universe, 18, 26, 18, 28);
	if (universe.VR1 && !universe.VR2 && universe.VR3)
		DRLG_L5GHall(universe, 18, 12, 18, 28);

#ifdef HELLFIRE
	if (currlevel == 24) {
		if (universe.VR1 || universe.VR2 || universe.VR3) {
			c = 1;
			if (!universe.VR1 && universe.VR2 && universe.VR3 && random_(0, 2) != 0)
				c = 2;
			if (universe.VR1 && universe.VR2 && !universe.VR3 && random_(0, 2) != 0)
				c = 0;

			if (universe.VR1 && !universe.VR2 && universe.VR3) {
				if (random_(0, 2) != 0)
					c = 0;
				else
					c = 2;
			}

			if (universe.VR1 && universe.VR2 && universe.VR3)
				c = random_(0, 3);

			switch (c) {
			case 0:
				drlg_l1_set_crypt_room(universe, 16, 2);
				break;
			case 1:
				drlg_l1_set_crypt_room(universe, 16, 16);
				break;
			case 2:
				drlg_l1_set_crypt_room(universe, 16, 30);
				break;
			}
		} else {
			c = 1;
			if (!universe.HR1 && universe.HR2 && universe.HR3 && random_(0, 2) != 0)
				c = 2;
			if (universe.HR1 && universe.HR2 && !universe.HR3 && random_(0, 2) != 0)
				c = 0;

			if (universe.HR1 && !universe.HR2 && universe.HR3) {
				if (random_(0, 2) != 0)
					c = 0;
				else
					c = 2;
			}

			if (universe.HR1 && universe.HR2 && universe.HR3)
				c = random_(0, 3);

			switch (c) {
			case 0:
				drlg_l1_set_crypt_room(universe, 2, 16);
				break;
			case 1:
				drlg_l1_set_crypt_room(universe, 16, 16);
				break;
			case 2:
				drlg_l1_set_crypt_room(universe, 30, 16);
				break;
			}
		}
	}
	if (currlevel == 21) {
		if (universe.VR1 || universe.VR2 || universe.VR3) {
			c = 1;
			if (!universe.VR1 && universe.VR2 && universe.VR3 && random_(0, 2) != 0)
				c = 2;
			if (universe.VR1 && universe.VR2 && !universe.VR3 && random_(0, 2) != 0)
				c = 0;

			if (universe.VR1 && !universe.VR2 && universe.VR3) {
				if (random_(0, 2) != 0)
					c = 0;
				else
					c = 2;
			}

			if (universe.VR1 && universe.VR2 && universe.VR3)
				c = random_(0, 3);

			switch (c) {
			case 0:
				drlg_l1_set_corner_room(16, 2);
				break;
			case 1:
				drlg_l1_set_corner_room(16, 16);
				break;
			case 2:
				drlg_l1_set_corner_room(16, 30);
				break;
			}
		} else {
			c = 1;
			if (!universe.HR1 && universe.HR2 && universe.HR3 && random_(0, 2))
				c = 2;
			if (universe.HR1 && universe.HR2 && !universe.HR3 && random_(0, 2))
				c = 0;

			if (universe.HR1 && !universe.HR2 && universe.HR3) {
				if (random_(0, 2))
					c = 0;
				else
					c = 2;
			}

			if (universe.HR1 && universe.HR2 && universe.HR3)
				c = random_(0, 3);

			switch (c) {
			case 0:
				drlg_l1_set_corner_room(2, 16);
				break;
			case 1:
				drlg_l1_set_corner_room(16, 16);
				break;
			case 2:
				drlg_l1_set_corner_room(30, 16);
				break;
			}
		}
	}
#endif
	if (universe.L5setloadflag) {
		if (universe.VR1 || universe.VR2 || universe.VR3) {
			c = 1;
			if (!universe.VR1 && universe.VR2 && universe.VR3 && random_(0, 2) != 0)
				c = 2;
			if (universe.VR1 && universe.VR2 && !universe.VR3 && random_(0, 2) != 0)
				c = 0;

			if (universe.VR1 && !universe.VR2 && universe.VR3) {
				if (random_(0, 2) != 0)
					c = 0;
				else
					c = 2;
			}

			if (universe.VR1 && universe.VR2 && universe.VR3)
				c = random_(0, 3);

			switch (c) {
			case 0:
				DRLG_L5SetRoom(universe, 16, 2);
				break;
			case 1:
				DRLG_L5SetRoom(universe, 16, 16);
				break;
			case 2:
				DRLG_L5SetRoom(universe, 16, 30);
				break;
			}
		} else {
			c = 1;
			if (!universe.HR1 && universe.HR2 && universe.HR3 && random_(0, 2) != 0)
				c = 2;
			if (universe.HR1 && universe.HR2 && !universe.HR3 && random_(0, 2) != 0)
				c = 0;

			if (universe.HR1 && !universe.HR2 && universe.HR3) {
				if (random_(0, 2) != 0)
					c = 0;
				else
					c = 2;
			}

			if (universe.HR1 && universe.HR2 && universe.HR3)
				c = random_(0, 3);

			switch (c) {
			case 0:
				DRLG_L5SetRoom(universe, 2, 16);
				break;
			case 1:
				DRLG_L5SetRoom(universe, 16, 16);
				break;
			case 2:
				DRLG_L5SetRoom(universe, 30, 16);
				break;
			}
		}
	}
}

#ifdef HELLFIRE
void drlg_l1_set_crypt_room(Universe& universe, int rx1, int ry1)
{
	int rw, rh, i, j, sp;

	rw = UberRoomPattern[0];
	rh = UberRoomPattern[1];

	universe.UberRow = 2 * rx1 + 6;
	universe.UberCol = 2 * ry1 + 8;
	setpc_x = rx1;
	setpc_y = ry1;
	setpc_w = rw;
	setpc_h = rh;
	universe.IsUberRoomOpened = 0;
	universe.dword_577368 = 0;
	universe.IsUberLeverActivated = 0;

	sp = 2;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (UberRoomPattern[sp]) {
				SetDungeon(universe, rx1 + i, ry1 + j, UberRoomPattern[sp]);
				universe.L5dflags[rx1 + i][ry1 + j] |= DLRG_PROTECTED;
			} else {
				SetDungeon(universe, rx1 + i, ry1 + j, 13);
			}
			sp++;
		}
	}
}

void drlg_l1_set_corner_room(int rx1, int ry1)
{
	int rw, rh, i, j, sp;

	rw = CornerstoneRoomPattern[0];
	rh = CornerstoneRoomPattern[1];

	setpc_x = rx1;
	setpc_y = ry1;
	setpc_w = rw;
	setpc_h = rh;

	sp = 2;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (CornerstoneRoomPattern[sp]) {
				SetDungeon(universe, rx1 + i, ry1 + j, CornerstoneRoomPattern[sp]);
				universe.L5dflags[rx1 + i][ry1 + j] |= DLRG_PROTECTED;
			} else {
				SetDungeon(universe, rx1 + i, ry1 + j, 13);
			}
			sp++;
		}
	}
}
#endif

static void DRLG_L5FTVR(Universe& universe, int i, int j, int x, int y, int d)
{
	if (dTransVal[x][y] || GetDungeon(universe, i, j) != 13) {
		if (d == 1) {
			dTransVal[x][y] = TransVal;
			dTransVal[x][y + 1] = TransVal;
		}
		if (d == 2) {
			dTransVal[x + 1][y] = TransVal;
			dTransVal[x + 1][y + 1] = TransVal;
		}
		if (d == 3) {
			dTransVal[x][y] = TransVal;
			dTransVal[x + 1][y] = TransVal;
		}
		if (d == 4) {
			dTransVal[x][y + 1] = TransVal;
			dTransVal[x + 1][y + 1] = TransVal;
		}
		if (d == 5)
			dTransVal[x + 1][y + 1] = TransVal;
		if (d == 6)
			dTransVal[x][y + 1] = TransVal;
		if (d == 7)
			dTransVal[x + 1][y] = TransVal;
		if (d == 8)
			dTransVal[x][y] = TransVal;
	} else {
		dTransVal[x][y] = TransVal;
		dTransVal[x + 1][y] = TransVal;
		dTransVal[x][y + 1] = TransVal;
		dTransVal[x + 1][y + 1] = TransVal;
		DRLG_L5FTVR(universe, i + 1, j, x + 2, y, 1);
		DRLG_L5FTVR(universe, i - 1, j, x - 2, y, 2);
		DRLG_L5FTVR(universe, i, j + 1, x, y + 2, 3);
		DRLG_L5FTVR(universe, i, j - 1, x, y - 2, 4);
		DRLG_L5FTVR(universe, i - 1, j - 1, x - 2, y - 2, 5);
		DRLG_L5FTVR(universe, i + 1, j - 1, x + 2, y - 2, 6);
		DRLG_L5FTVR(universe, i - 1, j + 1, x - 2, y + 2, 7);
		DRLG_L5FTVR(universe, i + 1, j + 1, x + 2, y + 2, 8);
	}
}

static void DRLG_L5FloodTVal(Universe& universe)
{
	int xx, yy, i, j;

	yy = 16;

	for (j = 0; j < DMAXY; j++) {
		xx = 16;

		for (i = 0; i < DMAXX; i++) {
			if (GetDungeon(universe, i, j) == 13 && !dTransVal[xx][yy]) {
				DRLG_L5FTVR(universe, i, j, xx, yy, 0);
				TransVal++;
			}
			xx += 2;
		}
		yy += 2;
	}
}

static void DRLG_L5TransFix(Universe& universe)
{
	int xx, yy, i, j;

	yy = 16;

	for (j = 0; j < DMAXY; j++) {
		xx = 16;

		for (i = 0; i < DMAXX; i++) {
			// BUGFIX: Should check for `j > 0` first.
			if (GetDungeon(universe, i, j) == 23 && GetDungeon(universe, i, j - 1) == 18) {
				dTransVal[xx + 1][yy] = dTransVal[xx][yy];
				dTransVal[xx + 1][yy + 1] = dTransVal[xx][yy];
			}
			// BUGFIX: Should check for `i + 1 < DMAXY` first.
			if (GetDungeon(universe, i, j) == 24 && GetDungeon(universe, i + 1, j) == 19) {
				dTransVal[xx][yy + 1] = dTransVal[xx][yy];
				dTransVal[xx + 1][yy + 1] = dTransVal[xx][yy];
			}
			if (GetDungeon(universe, i, j) == 18) {
				dTransVal[xx + 1][yy] = dTransVal[xx][yy];
				dTransVal[xx + 1][yy + 1] = dTransVal[xx][yy];
			}
			if (GetDungeon(universe, i, j) == 19) {
				dTransVal[xx][yy + 1] = dTransVal[xx][yy];
				dTransVal[xx + 1][yy + 1] = dTransVal[xx][yy];
			}
			if (GetDungeon(universe, i, j) == 20) {
				dTransVal[xx + 1][yy] = dTransVal[xx][yy];
				dTransVal[xx][yy + 1] = dTransVal[xx][yy];
				dTransVal[xx + 1][yy + 1] = dTransVal[xx][yy];
			}
			xx += 2;
		}
		yy += 2;
	}
}

static void DRLG_L5DirtFix(Universe& universe)
{
	int i, j;

#ifdef HELLFIRE
	if (currlevel < 21) {
		for (j = 0; j < DMAXY - 1; j++) {
			for (i = 0; i < DMAXX - 1; i++) {
				if (GetDungeon(universe, i, j) == 21 && GetDungeon(universe, i + 1, j) != 19)
					SetDungeon(universe, i, j, 202);
				if (GetDungeon(universe, i, j) == 19 && GetDungeon(universe, i + 1, j) != 19)
					SetDungeon(universe, i, j, 200);
				if (GetDungeon(universe, i, j) == 24 && GetDungeon(universe, i + 1, j) != 19)
					SetDungeon(universe, i, j, 205);
				if (GetDungeon(universe, i, j) == 18 && GetDungeon(universe, i, j + 1) != 18)
					SetDungeon(universe, i, j, 199);
				if (GetDungeon(universe, i, j) == 21 && GetDungeon(universe, i, j + 1) != 18)
					SetDungeon(universe, i, j, 202);
				if (GetDungeon(universe, i, j) == 23 && GetDungeon(universe, i, j + 1) != 18)
					SetDungeon(universe, i, j, 204);
			}
		}
	} else {
		for (j = 0; j < DMAXY - 1; j++) {
			for (i = 0; i < DMAXX - 1; i++) {
				if (GetDungeon(universe, i, j) == 19)
					SetDungeon(universe, i, j, 83);
				if (GetDungeon(universe, i, j) == 21)
					SetDungeon(universe, i, j, 85);
				if (GetDungeon(universe, i, j) == 23)
					SetDungeon(universe, i, j, 87);
				if (GetDungeon(universe, i, j) == 24)
					SetDungeon(universe, i, j, 88);
				if (GetDungeon(universe, i, j) == 18)
					SetDungeon(universe, i, j, 82);
			}
		}
	}
#else
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (GetDungeon(universe, i, j) == 21 && GetDungeon(universe, i + 1, j) != 19)
				SetDungeon(universe, i, j, 202);
			if (GetDungeon(universe, i, j) == 19 && GetDungeon(universe, i + 1, j) != 19)
				SetDungeon(universe, i, j, 200);
			if (GetDungeon(universe, i, j) == 24 && GetDungeon(universe, i + 1, j) != 19)
				SetDungeon(universe, i, j, 205);
			if (GetDungeon(universe, i, j) == 18 && GetDungeon(universe, i, j + 1) != 18)
				SetDungeon(universe, i, j, 199);
			if (GetDungeon(universe, i, j) == 21 && GetDungeon(universe, i, j + 1) != 18)
				SetDungeon(universe, i, j, 202);
			if (GetDungeon(universe, i, j) == 23 && GetDungeon(universe, i, j + 1) != 18)
				SetDungeon(universe, i, j, 204);
		}
	}
#endif
}

static void DRLG_L5CornerFix(Universe& universe)
{
	int i, j;

	for (j = 1; j < DMAXY - 1; j++) {
		for (i = 1; i < DMAXX - 1; i++) {
			if (!(universe.L5dflags[i][j] & DLRG_PROTECTED) && GetDungeon(universe, i, j) == 17 && GetDungeon(universe, i - 1, j) == 13 && GetDungeon(universe, i, j - 1) == 1) {
				SetDungeon(universe, i, j, 16);
				universe.L5dflags[i][j - 1] &= DLRG_PROTECTED; // BUGFIX: Should be |= or it will clear all flags
			}
			if (GetDungeon(universe, i, j) == 202 && GetDungeon(universe, i + 1, j) == 13 && GetDungeon(universe, i, j + 1) == 1) {
				SetDungeon(universe, i, j, 8);
			}
		}
	}
}

static std::optional<uint32_t> DRLG_L5(Universe& universe, int entry, DungeonMode mode)
{
	int i, j;
	LONG minarea;
	BOOL doneflag;

	switch (currlevel) {
	case 1:
		minarea = 533;
		break;
	case 2:
		minarea = 693;
		break;
	case 3:
	case 4:
		minarea = 761;
		break;
#ifdef HELLFIRE
	default:
		minarea = 761;
		break;
#endif
	}

	std::optional<uint32_t> levelSeed = std::nullopt;

	do {
		DRLG_InitTrans();

		bool failed;
		do {
			levelSeed = GetRndState();
			InitL5Dungeon(universe);
			L5firstRoom(universe);
			failed = L5GetArea(universe) < minarea;
			if ((mode == DungeonMode::BreakOnFailure || mode == DungeonMode::BreakOnFailureOrNoContent) && failed)
				return std::nullopt;
		} while (failed);

		L5makeDungeon(universe);
		L5makeDmt(universe);
		L5FillChambers(universe);
		L5tileFix(universe);
		L5AddWall(universe);
		L5ClearFlags(universe);
		DRLG_L5FloodTVal(universe);

		doneflag = TRUE;

		if (QuestStatus(Q_PWATER)) {
			if (entry == ENTRY_MAIN) {
				if (DRLG_PlaceMiniSet(universe, PWATERIN, 1, 1, 0, 0, TRUE, -1, 0) < 0)
					doneflag = FALSE;
			} else {
				if (DRLG_PlaceMiniSet(universe, PWATERIN, 1, 1, 0, 0, FALSE, -1, 0) < 0)
					doneflag = FALSE;
				ViewY--;
			}
		}
		if (QuestStatus(Q_LTBANNER)) {
			if (entry == ENTRY_MAIN) {
				if (DRLG_PlaceMiniSet(universe, STAIRSUP, 1, 1, 0, 0, TRUE, -1, 0) < 0)
					doneflag = FALSE;
			} else {
				if (DRLG_PlaceMiniSet(universe, STAIRSUP, 1, 1, 0, 0, FALSE, -1, 0) < 0)
					doneflag = FALSE;
				if (entry == ENTRY_PREV) {
					ViewX = 2 * setpc_x + 20;
					ViewY = 2 * setpc_y + 28;
				} else {
					ViewY--;
				}
			}
#ifdef HELLFIRE
		} else if (entry == ENTRY_MAIN) {
			if (currlevel < 21) {
				if (DRLG_PlaceMiniSet(universe, STAIRSUP, 1, 1, 0, 0, TRUE, -1, 0) < 0)
					doneflag = FALSE;
				if (DRLG_PlaceMiniSet(universe, STAIRSDOWN, 1, 1, 0, 0, FALSE, -1, 1) < 0)
					doneflag = FALSE;
			} else if (currlevel == 21) {
				if (DRLG_PlaceMiniSet(universe, L5STAIRSTOWN, 1, 1, 0, 0, FALSE, -1, 6) < 0)
					doneflag = FALSE;
				if (DRLG_PlaceMiniSet(universe, L5STAIRSDOWN, 1, 1, 0, 0, FALSE, -1, 1) < 0)
					doneflag = FALSE;
				ViewY++;
			} else {
				if (DRLG_PlaceMiniSet(universe, L5STAIRSUP, 1, 1, 0, 0, TRUE, -1, 0) < 0)
					doneflag = FALSE;
				if (currlevel != 24) {
					if (DRLG_PlaceMiniSet(universe, L5STAIRSDOWN, 1, 1, 0, 0, FALSE, -1, 1) < 0)
						doneflag = FALSE;
				}
				ViewY++;
			}
		} else if (entry == ENTRY_PREV) {
			if (currlevel < 21) {
				if (DRLG_PlaceMiniSet(universe, STAIRSUP, 1, 1, 0, 0, FALSE, -1, 0) < 0)
					doneflag = FALSE;
				if (DRLG_PlaceMiniSet(universe, STAIRSDOWN, 1, 1, 0, 0, TRUE, -1, 1) < 0)
					doneflag = FALSE;
				ViewY--;
			} else if (currlevel == 21) {
				if (DRLG_PlaceMiniSet(universe, L5STAIRSTOWN, 1, 1, 0, 0, FALSE, -1, 6) < 0)
					doneflag = FALSE;
				if (DRLG_PlaceMiniSet(universe, L5STAIRSDOWN, 1, 1, 0, 0, TRUE, -1, 1) < 0)
					doneflag = FALSE;
				ViewY += 3;
			} else {
				if (DRLG_PlaceMiniSet(universe, L5STAIRSUP, 1, 1, 0, 0, TRUE, -1, 0) < 0)
					doneflag = FALSE;
				if (currlevel != 24) {
					if (DRLG_PlaceMiniSet(universe, L5STAIRSDOWN, 1, 1, 0, 0, TRUE, -1, 1) < 0)
						doneflag = FALSE;
				}
				ViewY += 3;
			}
		} else {
			if (currlevel < 21) {
				if (DRLG_PlaceMiniSet(universe, STAIRSUP, 1, 1, 0, 0, FALSE, -1, 0) < 0)
					doneflag = FALSE;
				if (DRLG_PlaceMiniSet(universe, STAIRSDOWN, 1, 1, 0, 0, FALSE, -1, 1) < 0)
					doneflag = FALSE;
			} else if (currlevel == 21) {
				if (DRLG_PlaceMiniSet(universe, L5STAIRSTOWN, 1, 1, 0, 0, TRUE, -1, 6) < 0)
					doneflag = FALSE;
				if (DRLG_PlaceMiniSet(universe, L5STAIRSDOWN, 1, 1, 0, 0, FALSE, -1, 1) < 0)
					doneflag = FALSE;
			} else {
				if (DRLG_PlaceMiniSet(universe, L5STAIRSUP, 1, 1, 0, 0, TRUE, -1, 0) < 0)
					doneflag = FALSE;
				if (currlevel != 24) {
					if (DRLG_PlaceMiniSet(universe, L5STAIRSDOWN, 1, 1, 0, 0, FALSE, -1, 1) < 0)
						doneflag = FALSE;
				}
			}
#else
		} else if (entry == ENTRY_MAIN) {
			if (DRLG_PlaceMiniSet(universe, L5STAIRSUP, 1, 1, 0, 0, TRUE, -1, 0) < 0)
				doneflag = FALSE;
			else if (DRLG_PlaceMiniSet(universe, STAIRSDOWN, 1, 1, 0, 0, FALSE, -1, 1) < 0)
				doneflag = FALSE;
		} else {
			if (DRLG_PlaceMiniSet(universe, L5STAIRSUP, 1, 1, 0, 0, FALSE, -1, 0) < 0)
				doneflag = FALSE;
			else if (DRLG_PlaceMiniSet(universe, STAIRSDOWN, 1, 1, 0, 0, TRUE, -1, 1) < 0)
				doneflag = FALSE;
			ViewY--;
#endif
		}
		if ((mode == DungeonMode::BreakOnFailure || mode == DungeonMode::BreakOnFailureOrNoContent) && doneflag == FALSE)
			return std::nullopt;
	} while (doneflag == FALSE);

	if (mode == DungeonMode::BreakOnSuccess)
		return levelSeed;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (GetDungeon(universe, i, j) == 64) {
				int xx = 2 * i + 16; /* todo: fix loop */
				int yy = 2 * j + 16;
				DRLG_CopyTrans(xx, yy + 1, xx, yy);
				DRLG_CopyTrans(xx + 1, yy + 1, xx + 1, yy);
			}
		}
	}

	DRLG_L5TransFix(universe);
	DRLG_L5DirtFix(universe);
	DRLG_L5CornerFix(universe);

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (universe.L5dflags[i][j] & 0x7F)
				DRLG_PlaceDoor(universe, i, j);
		}
	}

#ifdef HELLFIRE
	if (currlevel < 21) {
		DRLG_L5Subs();
	} else {
		drlg_l1_crypt_pattern1(10);
		drlg_l1_crypt_rndset(byte_48A1B4, 95);
		drlg_l1_crypt_rndset(byte_48A1B8, 95);
		drlg_l1_crypt_rndset(byte_48A1C0, 100);
		drlg_l1_crypt_rndset(byte_48A1C8, 100);
		drlg_l1_crypt_rndset(byte_48A1E0, 60);
		drlg_l1_crypt_lavafloor();
		switch (currlevel) {
		case 21:
			drlg_l1_crypt_pattern2(30);
			drlg_l1_crypt_pattern3(15);
			drlg_l1_crypt_pattern4(5);
			drlg_l1_crypt_lavafloor();
			drlg_l1_crypt_pattern7(10);
			drlg_l1_crypt_pattern6(5);
			drlg_l1_crypt_pattern5(20);
			break;
		case 22:
			drlg_l1_crypt_pattern7(10);
			drlg_l1_crypt_pattern6(10);
			drlg_l1_crypt_pattern5(20);
			drlg_l1_crypt_pattern2(30);
			drlg_l1_crypt_pattern3(20);
			drlg_l1_crypt_pattern4(10);
			drlg_l1_crypt_lavafloor();
			break;
		case 23:
			drlg_l1_crypt_pattern7(10);
			drlg_l1_crypt_pattern6(15);
			drlg_l1_crypt_pattern5(30);
			drlg_l1_crypt_pattern2(30);
			drlg_l1_crypt_pattern3(20);
			drlg_l1_crypt_pattern4(15);
			drlg_l1_crypt_lavafloor();
			break;
		default:
			drlg_l1_crypt_pattern7(10);
			drlg_l1_crypt_pattern6(20);
			drlg_l1_crypt_pattern5(30);
			drlg_l1_crypt_pattern2(30);
			drlg_l1_crypt_pattern3(20);
			drlg_l1_crypt_pattern4(20);
			drlg_l1_crypt_lavafloor();
			break;
		}
	}
#else
	DRLG_L5Subs(universe);
#endif

#ifdef HELLFIRE
	if (currlevel < 21)
#endif
		DRLG_L1Shadows(universe);
#ifdef HELLFIRE
	if (currlevel < 21)
#endif
		DRLG_PlaceMiniSet(universe, LAMPS, 5, 10, 0, 0, FALSE, -1, 4);
#ifdef HELLFIRE
	if (currlevel < 21)
#endif
		DRLG_L1Floor(universe);

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			pdungeon[i][j] = GetDungeon(universe, i, j);
		}
	}

	DRLG_Init_Globals(universe);
	DRLG_CheckQuests(universe, setpc_x, setpc_y);

	return levelSeed;
}

std::optional<uint32_t> CreateL5Dungeon(Universe& universe, DWORD rseed, int entry, DungeonMode mode)
{
#ifdef HELLFIRE
	int i, j;
#endif

	SetRndSeed(rseed);

	dminx = 16;
	dminy = 16;
	dmaxx = 96;
	dmaxy = 96;

#ifdef HELLFIRE
	universe.UberRow = 0;
	universe.UberCol = 0;
	universe.IsUberRoomOpened = 0;
	universe.dword_577368 = 0;
	universe.UberLeverRow = 0;
	universe.UberLeverCol = 0;
	universe.IsUberLeverActivated = 0;
	universe.UberDiabloMonsterIndex = 0;
#endif

	DRLG_InitTrans();
	DRLG_InitSetPC();
	DRLG_LoadL1SP(universe);
	std::optional<uint32_t> levelSeed = DRLG_L5(universe, entry, mode);
	if (mode == DungeonMode::BreakOnFailure || mode == DungeonMode::BreakOnSuccess) {
		DRLG_FreeL1SP(universe);
		return levelSeed;
	}
	if (levelSeed)
		DRLG_L1Pass3(universe);
	DRLG_FreeL1SP(universe);

#ifdef HELLFIRE
	if (currlevel < 17)
		DRLG_InitL1Vals();
	else
		DRLG_InitL5Vals();

	DRLG_SetPC();

	for (j = dminy; j < dmaxy; j++) {
		for (i = dminx; i < dmaxx; i++) {
			if (dPiece[i][j] == 290) {
				universe.UberRow = i;
				universe.UberCol = j;
			}
			if (dPiece[i][j] == 317) {
				CornerStone.x = i;
				CornerStone.y = j;
			}
		}
	}
#else
	DRLG_InitL1Vals();
	DRLG_SetPC();
#endif

	return levelSeed;
}

#ifdef HELLFIRE
void drlg_l1_crypt_pattern1(int rndper)
{
	drlg_l1_crypt_rndset(byte_48A3C8, rndper);
	drlg_l1_crypt_rndset(byte_48A3CC, rndper);
	drlg_l1_crypt_rndset(byte_48A3D0, rndper);
	drlg_l1_crypt_rndset(byte_48A3D4, rndper);
}

void drlg_l1_crypt_pattern2(int rndper)
{
	drlg_l1_crypt_rndset(byte_48A2FC, rndper);
	drlg_l1_crypt_rndset(byte_48A300, rndper);
	drlg_l1_crypt_rndset(byte_48A304, rndper);
	drlg_l1_crypt_rndset(byte_48A308, rndper);
	drlg_l1_crypt_rndset(byte_48A30C, rndper);
	drlg_l1_crypt_rndset(byte_48A310, rndper);
	drlg_l1_crypt_rndset(byte_48A314, rndper);
	drlg_l1_crypt_rndset(byte_48A318, rndper);
	drlg_l1_crypt_rndset(byte_48A31C, rndper);
	drlg_l1_crypt_rndset(byte_48A320, rndper);
	drlg_l1_crypt_rndset(byte_48A324, rndper);
	drlg_l1_crypt_rndset(byte_48A328, rndper);
	drlg_l1_crypt_rndset(byte_48A32C, rndper);
	drlg_l1_crypt_rndset(byte_48A330, rndper);
	drlg_l1_crypt_rndset(byte_48A334, rndper);
	drlg_l1_crypt_rndset(byte_48A338, rndper);
	drlg_l1_crypt_rndset(byte_48A33C, rndper);
}

void drlg_l1_crypt_pattern3(int rndper)
{
	drlg_l1_crypt_rndset(byte_48A340, rndper);
	drlg_l1_crypt_rndset(byte_48A344, rndper);
	drlg_l1_crypt_rndset(byte_48A348, rndper);
	drlg_l1_crypt_rndset(byte_48A34C, rndper);
	drlg_l1_crypt_rndset(byte_48A350, rndper);
	drlg_l1_crypt_rndset(byte_48A354, rndper);
	drlg_l1_crypt_rndset(byte_48A358, rndper);
	drlg_l1_crypt_rndset(byte_48A35C, rndper);
	drlg_l1_crypt_rndset(byte_48A360, rndper);
	drlg_l1_crypt_rndset(byte_48A364, rndper);
	drlg_l1_crypt_rndset(byte_48A368, rndper);
	drlg_l1_crypt_rndset(byte_48A36C, rndper);
	drlg_l1_crypt_rndset(byte_48A370, rndper);
	drlg_l1_crypt_rndset(byte_48A374, rndper);
	drlg_l1_crypt_rndset(byte_48A378, rndper);
	drlg_l1_crypt_rndset(byte_48A37C, rndper);
	drlg_l1_crypt_rndset(byte_48A380, rndper);
}

void drlg_l1_crypt_pattern4(int rndper)
{
	drlg_l1_crypt_rndset(byte_48A384, rndper);
	drlg_l1_crypt_rndset(byte_48A388, rndper);
	drlg_l1_crypt_rndset(byte_48A38C, rndper);
	drlg_l1_crypt_rndset(byte_48A390, rndper);
	drlg_l1_crypt_rndset(byte_48A394, rndper);
	drlg_l1_crypt_rndset(byte_48A398, rndper);
	drlg_l1_crypt_rndset(byte_48A39C, rndper);
	drlg_l1_crypt_rndset(byte_48A3A0, rndper);
	drlg_l1_crypt_rndset(byte_48A3A4, rndper);
	drlg_l1_crypt_rndset(byte_48A3A8, rndper);
	drlg_l1_crypt_rndset(byte_48A3AC, rndper);
	drlg_l1_crypt_rndset(byte_48A3B0, rndper);
	drlg_l1_crypt_rndset(byte_48A3B4, rndper);
	drlg_l1_crypt_rndset(byte_48A3B8, rndper);
	drlg_l1_crypt_rndset(byte_48A3BC, rndper);
	drlg_l1_crypt_rndset(byte_48A3C0, rndper);
	drlg_l1_crypt_rndset(byte_48A3C4, rndper);
}

void drlg_l1_crypt_pattern5(int rndper)
{
	drlg_l1_crypt_rndset(byte_48A260, rndper);
	drlg_l1_crypt_rndset(byte_48A278, rndper);
	drlg_l1_crypt_rndset(byte_48A290, rndper);
	drlg_l1_crypt_rndset(byte_48A2A8, rndper);
	drlg_l1_crypt_rndset(byte_48A2C0, rndper);
	drlg_l1_crypt_rndset(byte_48A2D8, rndper);
	drlg_l1_crypt_rndset(byte_48A2EC, rndper);
	drlg_l1_crypt_rndset(byte_48A2F0, rndper);
	drlg_l1_crypt_rndset(byte_48A2F4, rndper);
	drlg_l1_crypt_rndset(byte_48A2F8, rndper);
}

void drlg_l1_crypt_pattern6(int rndper)
{
	drlg_l1_crypt_rndset(byte_48A1F4, rndper);
	drlg_l1_crypt_rndset(byte_48A1FC, rndper);
	drlg_l1_crypt_rndset(byte_48A1F8, rndper);
	drlg_l1_crypt_rndset(byte_48A200, rndper);
	drlg_l1_crypt_rndset(byte_48A204, rndper);
	drlg_l1_crypt_rndset(byte_48A208, rndper);
	drlg_l1_crypt_rndset(byte_48A20C, rndper);
	drlg_l1_crypt_rndset(byte_48A210, rndper);
	drlg_l1_crypt_rndset(byte_48A214, rndper);
	drlg_l1_crypt_rndset(byte_48A218, rndper);
	drlg_l1_crypt_rndset(byte_48A21C, rndper);
	drlg_l1_crypt_rndset(byte_48A220, rndper);
	drlg_l1_crypt_rndset(byte_48A224, rndper);
	drlg_l1_crypt_rndset(byte_48A228, rndper);
	drlg_l1_crypt_rndset(byte_48A22C, rndper);
	drlg_l1_crypt_rndset(byte_48A230, rndper);
	drlg_l1_crypt_rndset(byte_48A234, rndper);
	drlg_l1_crypt_rndset(byte_48A238, rndper);
	drlg_l1_crypt_rndset(byte_48A23C, rndper);
	drlg_l1_crypt_rndset(byte_48A240, rndper);
	drlg_l1_crypt_rndset(byte_48A244, rndper);
	drlg_l1_crypt_rndset(byte_48A248, rndper);
	drlg_l1_crypt_rndset(byte_48A24C, rndper);
	drlg_l1_crypt_rndset(byte_48A250, rndper);
	drlg_l1_crypt_rndset(byte_48A254, rndper);
	drlg_l1_crypt_rndset(byte_48A258, rndper);
}

void drlg_l1_crypt_pattern7(int rndper)
{
	drlg_l1_crypt_rndset(byte_48A1D0, rndper);
	drlg_l1_crypt_rndset(byte_48A1D4, rndper);
	drlg_l1_crypt_rndset(byte_48A1D8, rndper);
	drlg_l1_crypt_rndset(byte_48A1DC, rndper);
}
#endif
