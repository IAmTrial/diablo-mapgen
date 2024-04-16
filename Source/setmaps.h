/**
 * @file setmaps.cpp
 *
 * Interface of functionality for the special quest dungeons.
 */
#ifndef __SETMAPS_H__
#define __SETMAPS_H__

#include "Source/universe/universe.h"

int ObjIndex(Universe& universe, int x, int y);
void LoadSetMap();

/* rdata */
extern const char *const quest_level_names[];

#endif /* __SETMAPS_H__ */
