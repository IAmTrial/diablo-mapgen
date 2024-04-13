/**
 * @file quests.cpp
 *
 * Interface of functionality for handling quests.
 */
#ifndef __QUESTS_H__
#define __QUESTS_H__

#include "defs.h"
#include "structs.h"
#include "types.h"
#include "Source/universe/universe.h"

extern BOOL questlog;
extern BYTE *pQLogCel;
extern QuestStruct quests[MAXQUESTS];
extern int ReturnLvlX;
extern int ReturnLvlY;
extern int ReturnLvlT;
extern int ReturnLvl;

void InitQuests(Universe& universe);
void CheckQuests();
BOOL ForceQuests();
BOOL QuestStatus(int i);
void CheckQuestKill(int m, BOOL sendmsg);
void DRLG_CheckQuests(Universe& universe, int x, int y);
void SetReturnLvlPos();
void GetReturnLvlPos();
void ResyncMPQuests();
void ResyncQuests();
void DrawQuestLog();
void StartQuestlog();
void QuestlogUp();
void QuestlogDown();
void QuestlogEnter();
void QuestlogESC();
void SetMultiQuest(int q, int s, int l, int v1);

/* rdata */
extern const QuestData questlist[MAXQUESTS];

#endif /* __QUESTS_H__ */
