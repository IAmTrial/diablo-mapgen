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

extern int numthemes;
extern BOOL armorFlag;
extern BOOL weaponFlag;
extern int zharlib;
extern ThemeStruct themes[MAXTHEMES];

void InitThemes();
void HoldThemeRooms();
void CreateThemeRooms();

#endif /* __THEMES_H__ */
