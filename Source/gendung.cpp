/**
 * @file gendung.cpp
 *
 * Implementation of general dungeon generation code.
 */

#include "Source/gendung.h"

#include "defs.h"
#include "types.h"
#include "Source/engine.h"
#include "Source/lighting.h"
#include "Source/universe/universe.h"

/** Contains the contents of the single player quest DUN file. */
BYTE *pSetPiece;
BYTE *pSpecialCels;
/** Specifies the tile definitions of the active dungeon type; (e.g. levels/l1data/l1.til). */
BYTE *pMegaTiles;
BYTE *pLevelPieces;
BYTE *pDungeonCels;
BYTE *pSpeedCels;

void FillSolidBlockTbls(Universe& universe)
{
	BYTE bv;
	DWORD dwTiles;
	BYTE *pSBFile, *pTmp;
	int i;

	memset(universe.nBlockTable, 0, sizeof(universe.nBlockTable));
	memset(universe.nSolidTable, 0, sizeof(universe.nSolidTable));
	memset(universe.nTransTable, 0, sizeof(universe.nTransTable));
	memset(universe.nMissileTable, 0, sizeof(universe.nMissileTable));
	memset(universe.nTrapTable, 0, sizeof(universe.nTrapTable));

	switch (universe.leveltype) {
	case DTYPE_TOWN:
#ifdef HELLFIRE
		pSBFile = LoadFileInMem("NLevels\\TownData\\Town.SOL", &dwTiles);
#else
		pSBFile = LoadFileInMem("Levels\\TownData\\Town.SOL", &dwTiles);
#endif
		break;
	case DTYPE_CATHEDRAL:
#ifdef HELLFIRE
		if (currlevel < 17)
			pSBFile = LoadFileInMem("Levels\\L1Data\\L1.SOL", &dwTiles);
		else
			pSBFile = LoadFileInMem("NLevels\\L5Data\\L5.SOL", &dwTiles);
#else
		pSBFile = LoadFileInMem("Levels\\L1Data\\L1.SOL", &dwTiles);
#endif
		break;
	case DTYPE_CATACOMBS:
		pSBFile = LoadFileInMem("Levels\\L2Data\\L2.SOL", &dwTiles);
		break;
	case DTYPE_CAVES:
#ifdef HELLFIRE
		if (currlevel < 17)
			pSBFile = LoadFileInMem("Levels\\L3Data\\L3.SOL", &dwTiles);
		else
			pSBFile = LoadFileInMem("NLevels\\L6Data\\L6.SOL", &dwTiles);
#else
		pSBFile = LoadFileInMem("Levels\\L3Data\\L3.SOL", &dwTiles);
#endif
		break;
	case DTYPE_HELL:
		pSBFile = LoadFileInMem("Levels\\L4Data\\L4.SOL", &dwTiles);
		break;
	default:
		app_fatal("FillSolidBlockTbls");
	}

	pTmp = pSBFile;

	for (i = 1; i <= dwTiles; i++) {
		bv = *pTmp++;
		if (bv & 1)
			universe.nSolidTable[i] = TRUE;
		if (bv & 2)
			universe.nBlockTable[i] = TRUE;
		if (bv & 4)
			universe.nMissileTable[i] = TRUE;
		if (bv & 8)
			universe.nTransTable[i] = TRUE;
		if (bv & 0x80)
			universe.nTrapTable[i] = TRUE;
		universe.block_lvid[i] = (bv & 0x70) >> 4; /* beta: (bv >> 4) & 7 */
	}

	mem_free_dbg(pSBFile);
}

static void SwapTile(Universe& universe, int f1, int f2)
{
	int swap;

	swap = universe.level_frame_count[f1];
	universe.level_frame_count[f1] = universe.level_frame_count[f2];
	universe.level_frame_count[f2] = swap;
	swap = universe.tile_defs[f1];
	universe.tile_defs[f1] = universe.tile_defs[f2];
	universe.tile_defs[f2] = swap;
	swap = universe.level_frame_types[f1];
	universe.level_frame_types[f1] = universe.level_frame_types[f2];
	universe.level_frame_types[f2] = swap;
	swap = universe.level_frame_sizes[f1];
	universe.level_frame_sizes[f1] = universe.level_frame_sizes[f2];
	universe.level_frame_sizes[f2] = swap;
}

static void SortTiles(Universe& universe, int frames)
{
	int i;
	BOOL doneflag;

	doneflag = FALSE;
	while (frames > 0 && !doneflag) {
		doneflag = TRUE;
		for (i = 0; i < frames; i++) {
			if (universe.level_frame_count[i] < universe.level_frame_count[i + 1]) {
				SwapTile(universe, i, i + 1);
				doneflag = FALSE;
			}
		}
		frames--;
	}
}

void MakeSpeedCels(Universe& universe)
{
	int i, j, x, y, mt, t, z;
	int total_frames, blocks, total_size, frameidx, blk_cnt, nDataSize;
	WORD m;
	BOOL blood_flag;
	DWORD *pFrameTable;
	MICROS *pMap;
#ifndef USE_ASM
	int l, k;
	BYTE width, pix;
	BYTE *src, *dst, *tbl;
#endif

	for (i = 0; i < MAXTILES; i++) {
		universe.tile_defs[i] = i;
		universe.level_frame_count[i] = 0;
		universe.level_frame_types[i] = 0;
	}

	if (universe.leveltype != DTYPE_HELL)
		blocks = 10;
	else
		blocks = 12;

	for (y = 0; y < MAXDUNY; y++) {
		for (x = 0; x < MAXDUNX; x++) {
			pMap = &universe.dpiece_defs_map_2[x][y];
			for (j = 0; j < blocks; j++) {
				mt = pMap->mt[j];
				if (mt) {
					universe.level_frame_count[pMap->mt[j] & 0xFFF]++;
					universe.level_frame_types[pMap->mt[j] & 0xFFF] = mt & 0x7000;
				}
			}
		}
	}

#if defined HELLFIRE && defined USE_ASM
	__asm {
		mov		ebx, pDungeonCels
		mov		eax, [ebx]
		mov		nDataSize, eax
	}
#else
	pFrameTable = (DWORD *)pDungeonCels;
	nDataSize = pFrameTable[0];
#endif
	universe.nlevel_frames = nDataSize & 0xFFFF;

#ifdef HELLFIRE
	for (i = 0; i < universe.nlevel_frames; i++) {
#else
	for (i = 1; i < universe.nlevel_frames; i++) {
#endif
		z = i;
#ifdef USE_ASM
		__asm {
			mov		ebx, pDungeonCels
			mov		eax, z
			shl		eax, 2
			add		ebx, eax
			mov		eax, [ebx+4]
			sub		eax, [ebx]
			mov		nDataSize, eax
		}
#else
		nDataSize = pFrameTable[i + 1] - pFrameTable[i];
#endif
		universe.level_frame_sizes[i] = nDataSize & 0xFFFF;
	}

	universe.level_frame_sizes[0] = 0;

	if (universe.leveltype == DTYPE_HELL) {
		for (i = 0; i < universe.nlevel_frames; i++) {
#ifndef HELLFIRE
			if (i == 0)
				universe.level_frame_count[0] = 0;
#endif
			z = i;
			blood_flag = TRUE;
			if (universe.level_frame_count[i] != 0) {
				if (universe.level_frame_types[i] != 0x1000) {
#ifdef USE_ASM
					t = universe.level_frame_sizes[i];
					__asm {
						mov		ebx, pDungeonCels
						mov		eax, z
						shl		eax, 2
						add		ebx, eax
						mov		esi, pDungeonCels
						add		esi, [ebx]
						xor		ebx, ebx
						mov		ecx, t
						jecxz	l1_label3
					l1_label1:
						lodsb
						cmp		al, 0
						jz		l1_label2
						cmp		al, 32
						jnb		l1_label2
						mov		blood_flag, ebx
					l1_label2:
						loop	l1_label1
					l1_label3:
						nop
					}
#else
					src = &pDungeonCels[pFrameTable[i]];
					for (j = universe.level_frame_sizes[i]; j; j--) {
						pix = *src++;
						if (pix && pix < 32)
							blood_flag = FALSE;
					}
#endif
				} else {
#ifdef USE_ASM
					__asm {
						mov		ebx, pDungeonCels
						mov		eax, z
						shl		eax, 2
						add		ebx, eax
						mov		esi, pDungeonCels
						add		esi, [ebx]
						xor		ebx, ebx
						mov		ecx, 32
					l2_label1:
						push	ecx
						mov		edx, 32
					l2_label2:
						xor		eax, eax
						lodsb
						or		al, al
						js		l2_label5
						sub		edx, eax
						mov		ecx, eax
					l2_label3:
						lodsb
						cmp		al, 0
						jz		l2_label4
						cmp		al, 32
						jnb		l2_label4
						mov		blood_flag, ebx
					l2_label4:
						loop	l2_label3
						or		edx, edx
						jz		l2_label6
						jmp		l2_label2
					l2_label5:
						neg		al
						sub		edx, eax
						jnz		l2_label2
					l2_label6:
						pop		ecx
						loop	l2_label1
					}
#else
					src = &pDungeonCels[pFrameTable[i]];
					for (k = 32; k; k--) {
						for (l = 32; l;) {
							width = *src++;
							if (!(width & 0x80)) {
								l -= width;
								while (width) {
									pix = *src++;
									if (pix && pix < 32)
										blood_flag = FALSE;
									width--;
								}
							} else {
								width = -(char)width;
								l -= width;
							}
						}
					}
#endif
				}
				if (!blood_flag)
					universe.level_frame_count[i] = 0;
			}
		}
	}

	SortTiles(universe, MAXTILES - 1);
	total_size = 0;
	total_frames = 0;

	if (universe.light4flag) {
		while (total_size < 0x100000) {
			total_size += universe.level_frame_sizes[total_frames] << 1;
			total_frames++;
		}
	} else {
		while (total_size < 0x100000) {
			total_size += (universe.level_frame_sizes[total_frames] << 4) - (universe.level_frame_sizes[total_frames] << 1);
			total_frames++;
		}
	}

	total_frames--;
	if (total_frames > 128)
		total_frames = 128;

	frameidx = 0;

	if (universe.light4flag)
		blk_cnt = 3;
	else
		blk_cnt = 15;

	for (i = 0; i < total_frames; i++) {
		z = universe.tile_defs[i];
		universe.SpeedFrameTbl[i][0] = z;
		if (universe.level_frame_types[i] != 0x1000) {
			t = universe.level_frame_sizes[i];
			for (j = 1; j < blk_cnt; j++) {
				universe.SpeedFrameTbl[i][j] = frameidx;
#ifdef USE_ASM
				__asm {
					mov		ebx, pDungeonCels
					mov		eax, z
					shl		eax, 2
					add		ebx, eax
					mov		esi, pDungeonCels
					add		esi, [ebx]
					mov		edi, pSpeedCels
					add		edi, frameidx
					mov		ebx, j
					shl		ebx, 8
					add		ebx, universe.pLightTbl
					mov		ecx, t
					jecxz	l3_label2
				l3_label1:
					lodsb
					xlat
					stosb
					loop	l3_label1
				l3_label2:
					nop
				}
#else
				src = &pDungeonCels[pFrameTable[z]];
				dst = &pSpeedCels[frameidx];
				tbl = &universe.pLightTbl[256 * j];
				for (k = t; k; k--) {
					*dst++ = tbl[*src++];
				}
#endif
				frameidx += t;
			}
		} else {
			for (j = 1; j < blk_cnt; j++) {
				universe.SpeedFrameTbl[i][j] = frameidx;
#ifdef USE_ASM
				__asm {
					mov		ebx, pDungeonCels
					mov		eax, z
					shl		eax, 2
					add		ebx, eax
					mov		esi, pDungeonCels
					add		esi, [ebx]
					mov		edi, pSpeedCels
					add		edi, frameidx
					mov		ebx, j
					shl		ebx, 8
					add		ebx, universe.pLightTbl
					mov		ecx, 32
				l4_label1:
					push	ecx
					mov		edx, 32
				l4_label2:
					xor		eax, eax
					lodsb
					stosb
					or		al, al
					js		l4_label4
					sub		edx, eax
					mov		ecx, eax
				l4_label3:
					lodsb
					xlat
					stosb
					loop	l4_label3
					or		edx, edx
					jz		l4_label5
					jmp		l4_label2
				l4_label4:
					neg		al
					sub		edx, eax
					jnz		l4_label2
				l4_label5:
					pop		ecx
					loop	l4_label1
				}
#else
				src = &pDungeonCels[pFrameTable[z]];
				dst = &pSpeedCels[frameidx];
				tbl = &universe.pLightTbl[256 * j];
				for (k = 32; k; k--) {
					for (l = 32; l;) {
						width = *src++;
						*dst++ = width;
						if (!(width & 0x80)) {
							l -= width;
							while (width) {
								*dst++ = tbl[*src++];
								width--;
							}
						} else {
							width = -(char)width;
							l -= width;
						}
					}
				}
#endif
				frameidx += universe.level_frame_sizes[i];
			}
		}
	}

	for (y = 0; y < MAXDUNY; y++) {
		for (x = 0; x < MAXDUNX; x++) {
			if (universe.dPiece[x][y] != 0) {
				pMap = &universe.dpiece_defs_map_2[x][y];
				for (i = 0; i < blocks; i++) {
					if (pMap->mt[i]) {
						for (m = 0; m < total_frames; m++) {
							if ((pMap->mt[i] & 0xFFF) == universe.tile_defs[m]) {
								pMap->mt[i] = m + universe.level_frame_types[m] + 0x8000;
								m = total_frames;
							}
						}
					}
				}
			}
		}
	}
}

int IsometricCoord(int x, int y)
{
	if (x < MAXDUNY - y)
		return (y + y * y + x * (x + 2 * y + 3)) / 2;

	x = MAXDUNX - x - 1;
	y = MAXDUNY - y - 1;
	return MAXDUNX * MAXDUNY - ((y + y * y + x * (x + 2 * y + 3)) / 2) - 1;
}

void SetSpeedCels(Universe& universe)
{
	int x, y;

	for (x = 0; x < MAXDUNX; x++) {
		for (y = 0; y < MAXDUNY; y++) {
			universe.dpiece_defs_map_1[IsometricCoord(x, y)] = universe.dpiece_defs_map_2[x][y];
		}
	}
}

void SetDungeonMicros(Universe& universe)
{
	int i, x, y, lv, blocks;
	WORD *pPiece;
	MICROS *pMap;

	if (universe.leveltype != DTYPE_HELL) {
		universe.MicroTileLen = 10;
		blocks = 10;
	} else {
		universe.MicroTileLen = 12;
		blocks = 16;
	}

	for (y = 0; y < MAXDUNY; y++) {
		for (x = 0; x < MAXDUNX; x++) {
			lv = universe.dPiece[x][y];
			pMap = &universe.dpiece_defs_map_2[x][y];
			if (lv != 0) {
				lv--;
				if (universe.leveltype != DTYPE_HELL)
					pPiece = (WORD *)&pLevelPieces[20 * lv];
				else
					pPiece = (WORD *)&pLevelPieces[32 * lv];
				for (i = 0; i < blocks; i++)
					pMap->mt[i] = pPiece[(i & 1) + blocks - 2 - (i & 0xE)];
			} else {
				for (i = 0; i < blocks; i++)
					pMap->mt[i] = 0;
			}
		}
	}

	MakeSpeedCels(universe);
	SetSpeedCels(universe);

	if (universe.zoomflag) {
		universe.ViewDX = SCREEN_WIDTH;
		universe.ViewDY = VIEWPORT_HEIGHT;
		universe.ViewBX = SCREEN_WIDTH / TILE_WIDTH;
		universe.ViewBY = VIEWPORT_HEIGHT / TILE_HEIGHT;
	} else {
		universe.ViewDX = ZOOM_WIDTH;
		universe.ViewDY = ZOOM_HEIGHT;
		universe.ViewBX = ZOOM_WIDTH / TILE_WIDTH;
		universe.ViewBY = ZOOM_HEIGHT / TILE_HEIGHT;
	}
}

void DRLG_InitTrans(Universe& universe)
{
	memset(universe.dTransVal, 0, sizeof(universe.dTransVal));
	memset(universe.TransList, 0, sizeof(universe.TransList));
	universe.TransVal = 1;
}

void DRLG_MRectTrans(Universe& universe, int x1, int y1, int x2, int y2)
{
	int i, j;

	x1 = 2 * x1 + 17;
	y1 = 2 * y1 + 17;
	x2 = 2 * x2 + 16;
	y2 = 2 * y2 + 16;

	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			universe.dTransVal[i][j] = universe.TransVal;
		}
	}

	universe.TransVal++;
}

void DRLG_RectTrans(Universe& universe, int x1, int y1, int x2, int y2)
{
	int i, j;

	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			universe.dTransVal[i][j] = universe.TransVal;
		}
	}
	universe.TransVal++;
}

void DRLG_CopyTrans(Universe& universe, int sx, int sy, int dx, int dy)
{
	universe.dTransVal[dx][dy] = universe.dTransVal[sx][sy];
}

#ifndef SPAWN
void DRLG_ListTrans(Universe& universe, int num, const BYTE *List)
{
	int i;
	BYTE x1, y1, x2, y2;

	for (i = 0; i < num; i++) {
		x1 = *List++;
		y1 = *List++;
		x2 = *List++;
		y2 = *List++;
		DRLG_RectTrans(universe, x1, y1, x2, y2);
	}
}

void DRLG_AreaTrans(Universe& universe, int num, const BYTE *List)
{
	int i;
	BYTE x1, y1, x2, y2;

	for (i = 0; i < num; i++) {
		x1 = *List++;
		y1 = *List++;
		x2 = *List++;
		y2 = *List++;
		DRLG_RectTrans(universe, x1, y1, x2, y2);
		universe.TransVal--;
	}
	universe.TransVal++;
}
#endif

void DRLG_InitSetPC(Universe& universe)
{
	universe.setpc_x = 0;
	universe.setpc_y = 0;
	universe.setpc_w = 0;
	universe.setpc_h = 0;
}

void DRLG_SetPC(Universe& universe)
{
	int i, j, x, y, w, h;

	w = 2 * universe.setpc_w;
	h = 2 * universe.setpc_h;
	x = 2 * universe.setpc_x + 16;
	y = 2 * universe.setpc_y + 16;

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			universe.dFlags[i + x][j + y] |= BFLAG_POPULATED;
		}
	}
}

#ifndef SPAWN
void Make_SetPC(Universe& universe, int x, int y, int w, int h)
{
	int i, j, dx, dy, dh, dw;

	dw = 2 * w;
	dh = 2 * h;
	dx = 2 * x + 16;
	dy = 2 * y + 16;

	for (j = 0; j < dh; j++) {
		for (i = 0; i < dw; i++) {
			universe.dFlags[i + dx][j + dy] |= BFLAG_POPULATED;
		}
	}
}

BOOL DRLG_WillThemeRoomFit(Universe& universe, int floor, int x, int y, int minSize, int maxSize, int *width, int *height)
{
	int ii, xx, yy;
	int xSmallest, ySmallest;
	int xArray[20], yArray[20];
	int xCount, yCount;
	BOOL yFlag, xFlag;

	yFlag = TRUE;
	xFlag = TRUE;
	xCount = 0;
	yCount = 0;

	// BUGFIX: incorrect out-of-bounds check, should check that `universe.dungeon[xx][y + ii]` is not out-of-bounds in loop.
	if (x > DMAXX - maxSize && y > DMAXY - maxSize) {
		return FALSE;
	}
	if (!SkipThemeRoom(universe, x, y)) {
		return FALSE;
	}

	memset(xArray, 0, sizeof(xArray));
	memset(yArray, 0, sizeof(yArray));

	for (ii = 0; ii < maxSize; ii++) {
		if (xFlag) {
			for (xx = x; xx < x + maxSize; xx++) {
				if (GetDungeon(universe, xx, y + ii) != floor) {
					if (xx >= minSize) { // BUGFIX: This is comparing absolute to relative, should be `xx - x >= minSize`
						break;
					}
					xFlag = FALSE;
				} else {
					xCount++;
				}
			}
			if (xFlag) {
				xArray[ii] = xCount;
				xCount = 0;
			}
		}
		if (yFlag) {
			for (yy = y; yy < y + maxSize; yy++) {
				if (GetDungeon(universe, x + ii, yy) != floor) {
					if (yy >= minSize) { // BUGFIX: This is comparing absolute to relative, should be `yy - y >= minSize`
						break;
					}
					yFlag = FALSE;
				} else {
					yCount++;
				}
			}
			if (yFlag) {
				yArray[ii] = yCount;
				yCount = 0;
			}
		}
	}

	for (ii = 0; ii < minSize; ii++) {
		if (xArray[ii] < minSize || yArray[ii] < minSize) {
			return FALSE;
		}
	}

	xSmallest = xArray[0];
	ySmallest = yArray[0];

	for (ii = 0; ii < maxSize; ii++) {
		if (xArray[ii] < minSize || yArray[ii] < minSize) {
			break;
		}
		if (xArray[ii] < xSmallest) {
			xSmallest = xArray[ii];
		}
		if (yArray[ii] < ySmallest) {
			ySmallest = yArray[ii];
		}
	}

	*width = xSmallest - 2;
	*height = ySmallest - 2;
	return TRUE;
}

void DRLG_CreateThemeRoom(Universe& universe, int themeIndex)
{
	int xx, yy;

	for (yy = universe.themeLoc[themeIndex].y; yy < universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height; yy++) {
		for (xx = universe.themeLoc[themeIndex].x; xx < universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width; xx++) {
			if (universe.leveltype == DTYPE_CATACOMBS) {
				if (yy == universe.themeLoc[themeIndex].y
				        && xx >= universe.themeLoc[themeIndex].x
				        && xx <= universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width
				    || yy == universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height - 1
				        && xx >= universe.themeLoc[themeIndex].x
				        && xx <= universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width) {
					SetDungeon(universe, xx, yy, 2);
				} else if (xx == universe.themeLoc[themeIndex].x
				        && yy >= universe.themeLoc[themeIndex].y
				        && yy <= universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height
				    || xx == universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width - 1
				        && yy >= universe.themeLoc[themeIndex].y
				        && yy <= universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height) {
					SetDungeon(universe, xx, yy, 1);
				} else {
					SetDungeon(universe, xx, yy, 3);
				}
			}
			if (universe.leveltype == DTYPE_CAVES) {
				if (yy == universe.themeLoc[themeIndex].y
				        && xx >= universe.themeLoc[themeIndex].x
				        && xx <= universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width
				    || yy == universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height - 1
				        && xx >= universe.themeLoc[themeIndex].x
				        && xx <= universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width) {
					SetDungeon(universe, xx, yy, 134);
				} else if (xx == universe.themeLoc[themeIndex].x
				        && yy >= universe.themeLoc[themeIndex].y
				        && yy <= universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height
				    || xx == universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width - 1
				        && yy >= universe.themeLoc[themeIndex].y
				        && yy <= universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height) {
					SetDungeon(universe, xx, yy, 137);
				} else {
					SetDungeon(universe, xx, yy, 7);
				}
			}
			if (universe.leveltype == DTYPE_HELL) {
				if (yy == universe.themeLoc[themeIndex].y
				        && xx >= universe.themeLoc[themeIndex].x
				        && xx <= universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width
				    || yy == universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height - 1
				        && xx >= universe.themeLoc[themeIndex].x
				        && xx <= universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width) {
					SetDungeon(universe, xx, yy, 2);
				} else if (xx == universe.themeLoc[themeIndex].x
				        && yy >= universe.themeLoc[themeIndex].y
				        && yy <= universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height
				    || xx == universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width - 1
				        && yy >= universe.themeLoc[themeIndex].y
				        && yy <= universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height) {
					SetDungeon(universe, xx, yy, 1);
				} else {
					SetDungeon(universe, xx, yy, 6);
				}
			}
		}
	}

	if (universe.leveltype == DTYPE_CATACOMBS) {
		SetDungeon(universe, universe.themeLoc[themeIndex].x, universe.themeLoc[themeIndex].y, 8);
		SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width - 1, universe.themeLoc[themeIndex].y, 7);
		SetDungeon(universe, universe.themeLoc[themeIndex].x, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height - 1, 9);
		SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width - 1, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height - 1, 6);
	}
	if (universe.leveltype == DTYPE_CAVES) {
		SetDungeon(universe, universe.themeLoc[themeIndex].x, universe.themeLoc[themeIndex].y, 150);
		SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width - 1, universe.themeLoc[themeIndex].y, 151);
		SetDungeon(universe, universe.themeLoc[themeIndex].x, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height - 1, 152);
		SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width - 1, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height - 1, 138);
	}
	if (universe.leveltype == DTYPE_HELL) {
		SetDungeon(universe, universe.themeLoc[themeIndex].x, universe.themeLoc[themeIndex].y, 9);
		SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width - 1, universe.themeLoc[themeIndex].y, 16);
		SetDungeon(universe, universe.themeLoc[themeIndex].x, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height - 1, 15);
		SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width - 1, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height - 1, 12);
	}

	if (universe.leveltype == DTYPE_CATACOMBS) {
		switch (random_(universe, 0, 2)) {
		case 0:
			SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width - 1, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height / 2, 4);
			break;
		case 1:
			SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width / 2, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height - 1, 5);
			break;
		}
	}
	if (universe.leveltype == DTYPE_CAVES) {
		switch (random_(universe, 0, 2)) {
		case 0:
			SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width - 1, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height / 2, 147);
			break;
		case 1:
			SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width / 2, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height - 1, 146);
			break;
		}
	}
	if (universe.leveltype == DTYPE_HELL) {
		switch (random_(universe, 0, 2)) {
		case 0:
			SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width - 1, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height / 2 - 1, 53);
			SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width - 1, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height / 2, 6);
			SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width - 1, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height / 2 + 1, 52);
			SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width - 2, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height / 2 - 1, 54);
			break;
		case 1:
			SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width / 2 - 1, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height - 1, 57);
			SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width / 2, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height - 1, 6);
			SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width / 2 + 1, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height - 1, 56);
			SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width / 2, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height - 2, 59);
			SetDungeon(universe, universe.themeLoc[themeIndex].x + universe.themeLoc[themeIndex].width / 2 - 1, universe.themeLoc[themeIndex].y + universe.themeLoc[themeIndex].height - 2, 58);
			break;
		}
	}
}

void DRLG_PlaceThemeRooms(Universe& universe, int minSize, int maxSize, int floor, int freq, int rndSize)
{
	int i, j;
	int themeW, themeH;
	int rv2, min, max;

	universe.themeCount = 0;
	memset(universe.themeLoc, 0, sizeof(*universe.themeLoc));
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (universe.dungeon[i][j] == floor && !random_(universe, 0, freq) && DRLG_WillThemeRoomFit(universe, floor, i, j, minSize, maxSize, &themeW, &themeH)) {
				if (rndSize) {
					min = minSize - 2;
					max = maxSize - 2;
					rv2 = min + random_(universe, 0, random_(universe, 0, themeW - min + 1));
					if (rv2 >= min && rv2 <= max)
						themeW = rv2;
					else
						themeW = min;
					rv2 = min + random_(universe, 0, random_(universe, 0, themeH - min + 1));
					if (rv2 >= min && rv2 <= max)
						themeH = rv2;
					else
						themeH = min;
				}
				universe.themeLoc[universe.themeCount].x = i + 1;
				universe.themeLoc[universe.themeCount].y = j + 1;
				universe.themeLoc[universe.themeCount].width = themeW;
				universe.themeLoc[universe.themeCount].height = themeH;
				if (universe.leveltype == DTYPE_CAVES)
					DRLG_RectTrans(universe, 2 * i + 20, 2 * j + 20, 2 * (i + themeW) + 15, 2 * (j + themeH) + 15);
				else
					DRLG_MRectTrans(universe, i + 1, j + 1, i + themeW, j + themeH);
				universe.themeLoc[universe.themeCount].ttval = universe.TransVal - 1;
				DRLG_CreateThemeRoom(universe, universe.themeCount);
				universe.themeCount++;
			}
		}
	}
}
#endif

void DRLG_HoldThemeRooms(Universe& universe)
{
	int i, x, y, xx, yy;

	for (i = 0; i < universe.themeCount; i++) {
		for (y = universe.themeLoc[i].y; y < universe.themeLoc[i].y + universe.themeLoc[i].height - 1; y++) {
			for (x = universe.themeLoc[i].x; x < universe.themeLoc[i].x + universe.themeLoc[i].width - 1; x++) {
				xx = 2 * x + 16;
				yy = 2 * y + 16;
				universe.dFlags[xx][yy] |= BFLAG_POPULATED;
				universe.dFlags[xx + 1][yy] |= BFLAG_POPULATED;
				universe.dFlags[xx][yy + 1] |= BFLAG_POPULATED;
				universe.dFlags[xx + 1][yy + 1] |= BFLAG_POPULATED;
			}
		}
	}
}

BOOL SkipThemeRoom(Universe& universe, int x, int y)
{
	int i;

	for (i = 0; i < universe.themeCount; i++) {
		if (x >= universe.themeLoc[i].x - 2 && x <= universe.themeLoc[i].x + universe.themeLoc[i].width + 2
		    && y >= universe.themeLoc[i].y - 2 && y <= universe.themeLoc[i].y + universe.themeLoc[i].height + 2)
			return FALSE;
	}

	return TRUE;
}

void InitLevels(Universe& universe)
{
	if (!universe.leveldebug) {
		universe.currlevel = 0;
		universe.leveltype = DTYPE_TOWN;
		universe.setlevel = FALSE;
	}
}
