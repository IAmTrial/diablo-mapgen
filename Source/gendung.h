/**
 * @file gendung.h
 *
 * Interface of general dungeon generation code.
 */
#ifndef __GENDUNG_H__
#define __GENDUNG_H__

#include "defs.h"
#include "structs.h"
#include "types.h"
#include "Source/universe/universe.h"

// TODO: Mark these as OpenCL __global
extern BYTE *pSetPiece;
extern BYTE *pSpecialCels;
extern BYTE *pMegaTiles;
extern BYTE *pLevelPieces;
extern BYTE *pDungeonCels;
extern BYTE *pSpeedCels;

void FillSolidBlockTbls(Universe& universe);
int IsometricCoord(int x, int y);
void SetDungeonMicros(Universe& universe);
void DRLG_InitTrans(Universe& universe);
void DRLG_MRectTrans(Universe& universe, int x1, int y1, int x2, int y2);
void DRLG_RectTrans(Universe& universe, int x1, int y1, int x2, int y2);
void DRLG_CopyTrans(Universe& universe, int sx, int sy, int dx, int dy);
void DRLG_ListTrans(Universe& universe, int num, const BYTE *List);
void DRLG_AreaTrans(Universe& universe, int num, const BYTE *List);
void DRLG_InitSetPC(Universe& universe);
void DRLG_SetPC(Universe& universe);
void Make_SetPC(Universe& universe, int x, int y, int w, int h);
void DRLG_PlaceThemeRooms(Universe& universe, int minSize, int maxSize, int floor, int freq, int rndSize);
void DRLG_HoldThemeRooms(Universe& universe);
BOOL SkipThemeRoom(Universe& universe, int x, int y);
void InitLevels(Universe& universe);

#endif /* __GENDUNG_H__ */
