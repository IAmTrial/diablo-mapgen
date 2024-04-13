/**
 * @file objects.h
 *
 * Interface of object functionality, interaction, spawning, loading, etc.
 */
#ifndef __OBJECTS_H__
#define __OBJECTS_H__

#include "defs.h"
#include "structs.h"
#include "types.h"
#include "Source/universe/universe.h"

extern int objectactive[MAXOBJECTS];
extern int nobjects;
extern int objectavail[MAXOBJECTS];
extern ObjectStruct object[MAXOBJECTS];
extern BOOL InitObjFlag;

void InitObjectGFX();
void FreeObjectGFX();
void AddL1Objs(Universe& universe, int x1, int y1, int x2, int y2);
void AddL2Objs(Universe& universe, int x1, int y1, int x2, int y2);
void InitObjects(Universe& universe);
void SetMapObjects(Universe& universe, BYTE *pMap, int startx, int starty);
void SetObjMapRange(int i, int x1, int y1, int x2, int y2, int v);
void SetBookMsg(int i, int msg);
void GetRndObjLoc(int randarea, int &xx, int &yy);
void AddMushPatch(Universe& universe);
void AddSlainHero(Universe& universe);
#ifdef HELLFIRE
void AddCryptBook(int ot, int v2, int ox, int oy);
void AddCryptObject(int a1, int a2);
void AddNakrulBook(int a1, int a2, int a3);
#endif
void AddObject(Universe& universe, int ot, int ox, int oy);
void Obj_Trap(int i);
void ProcessObjects();
void ObjSetMicro(int dx, int dy, int pn);
void RedoPlayerVision(Universe& universe);
void MonstCheckDoors(int m);
void ObjChangeMap(Universe& universe, int x1, int y1, int x2, int y2);
void ObjChangeMapResync(Universe& universe, int x1, int y1, int x2, int y2);
void TryDisarm(int pnum, int i);
int ItemMiscIdIdx(int imiscid);
void OperateObject(int pnum, int i, BOOL TeleFlag);
void SyncOpObject(int pnum, int cmd, int i);
void BreakObject(int pnum, int oi);
void SyncBreakObj(int pnum, int oi);
void SyncObjectAnim(int o);
void GetObjectStr(Universe& universe, int i, char infostr[50]);
#ifdef HELLFIRE
void OperateNakrulLever();
void SyncNakrulRoom();
void AddNakrulLeaver();
DIABOOL OperateNakrulBook(int s);
#endif

#endif /* __OBJECTS_H__ */
