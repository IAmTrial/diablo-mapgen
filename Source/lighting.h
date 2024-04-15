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
void InitLighting(Universe& universe);
int AddLight(int x, int y, int r);
void AddUnLight(Universe& universe, int i);
void ChangeLightRadius(Universe& universe, int i, int r);
void ChangeLightXY(Universe& universe, int i, int x, int y);
void ChangeLightOff(Universe& universe, int i, int x, int y);
void ChangeLight(Universe& universe, int i, int x, int y, int r);
void ProcessLightList(Universe& universe);
void SavePreLighting(Universe& universe);
void InitVision(Universe& universe);
int AddVision(Universe& universe, int x, int y, int r, BOOL mine);
void ChangeVisionRadius(Universe& universe, int id, int r);
void ChangeVisionXY(int id, int x, int y);
void ProcessVisionList(Universe& universe);
void lighting_color_cycling(Universe& universe);

/* rdata */

extern const char CrawlTable[2749];
extern const BYTE vCrawlTable[23][30];

#endif /* __LIGHTING_H__ */
