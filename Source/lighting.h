/**
 * @file lighting.h
 *
 * Interface of light and vision.
 */
#ifndef __LIGHTING_H__
#define __LIGHTING_H__

extern LightListStruct VisionList[MAXVISION];
extern BYTE lightactive[MAXLIGHTS];
extern LightListStruct LightList[MAXLIGHTS];
extern int numlights;
extern BYTE lightradius[16][128];
extern BOOL dovision;
extern int numvision;
extern BOOL dolighting;
extern int visionid;
extern BYTE *pLightTbl;
extern BOOL lightflag;

void DoLighting(int nXPos, int nYPos, int nRadius, int Lnum);
void DoUnVision(int nXPos, int nYPos, int nRadius);
void DoVision(int nXPos, int nYPos, int nRadius, BOOL doautomap, BOOL visible);
#ifdef _DEBUG
void ToggleLighting_2();
void ToggleLighting();
#endif
int AddLight(int x, int y, int r);
void AddUnLight(int i);
void ChangeLightRadius(int i, int r);
void ChangeLightXY(int i, int x, int y);
void ChangeLightOff(int i, int x, int y);
void ChangeLight(int i, int x, int y, int r);
void ProcessLightList();
void SavePreLighting();
void InitVision();
void ChangeVisionRadius(int id, int r);
void ChangeVisionXY(int id, int x, int y);
void ProcessVisionList();

/* rdata */

extern BYTE vCrawlTable[23][30];

#endif /* __LIGHTING_H__ */
