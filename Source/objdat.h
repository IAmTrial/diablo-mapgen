/**
 * @file objdat.h
 *
 * Interface of all object data.
 */
#ifndef __OBJDAT_H__
#define __OBJDAT_H__

#include "structs.h"

extern const int ObjTypeConv[];
extern const ObjDataStruct AllObjects[];
extern const char *const ObjMasterLoadList[];
#ifdef HELLFIRE
extern const char *ObjCryptLoadList[];
extern const char *ObjHiveLoadList[];
#endif

#endif /* __OBJDAT_H__ */
