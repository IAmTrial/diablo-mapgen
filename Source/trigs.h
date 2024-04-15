/**
 * @file trigs.h
 *
 * Interface of functionality for triggering events when the player enters an area.
 */
#ifndef __TRIGS_H__
#define __TRIGS_H__

#include "structs.h"
#include "types.h"
#include "Source/universe/universe.h"

extern BOOL trigflag;
extern int numtrigs;
extern TriggerStruct trigs[MAXTRIGGERS];
extern int TWarpFrom;

void InitNoTriggers();
void InitL1Triggers(Universe& universe);
void InitL2Triggers(Universe& universe);
void InitL3Triggers(Universe& universe);
void InitL4Triggers(Universe& universe);
void InitSKingTriggers();
void InitSChambTriggers();
void InitPWaterTriggers();
void InitVPTriggers();
void Freeupstairs(Universe& universe);

#endif /* __TRIGS_H__ */
