/**
 * @file drlg_l3.h
 *
 * Interface of the caves level generation algorithms.
 */
#ifndef __DRLG_L3_H__
#define __DRLG_L3_H__

#include <optional>

#include "structs.h"
#include "types.h"
#include "Source/universe/universe.h"

std::optional<uint32_t> CreateL3Dungeon(DWORD rseed, int entry, DungeonMode mode);
void LoadL3Dungeon(Universe& universe, const char *sFileName, int vx, int vy);
void LoadPreL3Dungeon(const char *sFileName, int vx, int vy);

#endif /* __DRLG_L3_H__ */
