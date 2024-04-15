/**
 * @file lighting.h
 *
 * Interface of light and vision.
 */
#ifndef __LIGHTING_H__
#define __LIGHTING_H__

#include "types.h"
#include "structs.h"
#include "Source/universe/universe.h"

extern LightListStruct VisionList[MAXVISION];
extern BYTE lightactive[MAXLIGHTS];
extern LightListStruct LightList[MAXLIGHTS];
extern int numlights;
extern BYTE lightradius[16][128];
extern BOOL dovision;
extern int numvision;
extern char lightmax;
extern BOOL dolighting;
extern int visionid;
extern BYTE *pLightTbl;
extern BOOL lightflag;

void DoLighting(int nXPos, int nYPos, int nRadius, int Lnum);
void DoUnVision(Universe& universe, int nXPos, int nYPos, int nRadius);
void DoVision(Universe& universe, int nXPos, int nYPos, int nRadius, BOOL doautomap, BOOL visible);
void FreeLightTable();
void InitLightTable();
void MakeLightTable(Universe& universe);
#ifdef _DEBUG
void ToggleLighting_2();
void ToggleLighting();
#endif
void InitLightMax(Universe& universe);
void InitLighting();
int AddLight(int x, int y, int r);
void AddUnLight(int i);
void ChangeLightRadius(int i, int r);
void ChangeLightXY(int i, int x, int y);
void ChangeLightOff(int i, int x, int y);
void ChangeLight(int i, int x, int y, int r);
void ProcessLightList(Universe& universe);
void SavePreLighting(Universe& universe);
void InitVision(Universe& universe);
int AddVision(int x, int y, int r, BOOL mine);
void ChangeVisionRadius(int id, int r);
void ChangeVisionXY(int id, int x, int y);
void ProcessVisionList(Universe& universe);
void lighting_color_cycling(Universe& universe);

/* rdata */

extern const char CrawlTable[2749];
extern const BYTE vCrawlTable[23][30];

#endif /* __LIGHTING_H__ */
