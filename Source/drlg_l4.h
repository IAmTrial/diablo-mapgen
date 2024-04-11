/**
 * @file drlg_l4.h
 *
 * Interface of the hell level generation algorithms.
 */
#ifndef __DRLG_L4_H__
#define __DRLG_L4_H__

#include <optional>

#include "structs.h"
#include "types.h"
#include "Source/universe/universe.h"

std::optional<uint32_t> CreateL4Dungeon(Universe& universe, DWORD rseed, int entry, DungeonMode mode);
void DRLG_PreLoadDiabQuads();
void DRLG_FreeDiabQuads();

#endif /* __DRLG_L4_H__ */
