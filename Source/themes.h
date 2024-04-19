/**
 * @file themes.h
 *
 * Interface of the theme room placing algorithms.
 */
#ifndef __THEMES_H__
#define __THEMES_H__

#include "defs.h"
#include "structs.h"
#include "types.h"
#include "Source/universe/universe.h"

void InitThemes(Universe& universe);
void HoldThemeRooms(Universe& universe);
void CreateThemeRooms(Universe& universe);

#endif /* __THEMES_H__ */
