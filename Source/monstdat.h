/**
 * @file monstdat.h
 *
 * Interface of all monster data.
 */
#ifndef __MONSTDAT_H__
#define __MONSTDAT_H__

#include "structs.h"

extern const MonsterData monsterdata[];
#ifdef HELLFIRE
extern const int MonstConvTbl[];
extern int MonstAvailTbl[];
#else
extern const char MonstConvTbl[];
extern const char MonstAvailTbl[];
#endif
extern const UniqMonstStruct UniqMonst[];

#endif /* __MONSTDAT_H__ */
