
#include "Source/analyzer/puzzler.h"

#include <cstdint>
#include <iostream>
#include <optional>

#include "Source/items.h"
#include "Source/monster.h"
#include "Source/objects.h"
#include "Source/universe/universe.h"

void CreateItemsFromObject(Universe& universe, int oid)
{
	switch (object[oid]._otype) {
	case OBJ_CHEST1:
	case OBJ_CHEST2:
	case OBJ_CHEST3:
	case OBJ_TCHEST1:
	case OBJ_TCHEST2:
	case OBJ_TCHEST3:
		SetRndSeed(universe, object[oid]._oRndSeed);
		if (universe.setlevel) {
			for (int j = 0; j < object[oid]._oVar1; j++) {
				CreateRndItem(universe, object[oid]._ox, object[oid]._oy, TRUE, TRUE, FALSE);
			}
		} else {
			for (int j = 0; j < object[oid]._oVar1; j++) {
				if (object[oid]._oVar2 != 0)
					CreateRndItem(universe, object[oid]._ox, object[oid]._oy, FALSE, TRUE, FALSE);
				else
					CreateRndUseful(universe, 0, object[oid]._ox, object[oid]._oy, TRUE);
			}
		}
		break;
	case OBJ_SARC:
		SetRndSeed(universe, object[oid]._oRndSeed);
		if (object[oid]._oVar1 <= 2)
			CreateRndItem(universe, object[oid]._ox, object[oid]._oy, FALSE, TRUE, FALSE);
		break;
	case OBJ_DECAP:
		SetRndSeed(universe, object[oid]._oRndSeed);
		CreateRndItem(universe, object[oid]._ox, object[oid]._oy, FALSE, TRUE, FALSE);
		break;
	case OBJ_BARREL:
		SetRndSeed(universe, object[oid]._oRndSeed);
		if (object[oid]._oVar2 <= 1) {
			if (object[oid]._oVar3 == 0)
				CreateRndUseful(universe, 0, object[oid]._ox, object[oid]._oy, TRUE);
			else
				CreateRndItem(universe, object[oid]._ox, object[oid]._oy, FALSE, TRUE, FALSE);
		}
		break;
	case OBJ_SKELBOOK:
	case OBJ_BOOKSTAND:
		SetRndSeed(universe, object[oid]._oRndSeed);
		if (random_(universe, 161, 5) != 0)
			CreateTypeItem(universe, object[oid]._ox, object[oid]._oy, FALSE, ITYPE_MISC, IMISC_SCROLL, TRUE, FALSE);
		else
			CreateTypeItem(universe, object[oid]._ox, object[oid]._oy, FALSE, ITYPE_MISC, IMISC_BOOK, TRUE, FALSE);
		break;
	case OBJ_BOOKCASEL:
	case OBJ_BOOKCASER:
		SetRndSeed(universe, object[oid]._oRndSeed);
		CreateTypeItem(universe, object[oid]._ox, object[oid]._oy, FALSE, ITYPE_MISC, IMISC_BOOK, TRUE, FALSE);
		break;
	case OBJ_ARMORSTAND:
	case OBJ_WARARMOR: {
		SetRndSeed(universe, object[oid]._oRndSeed);
		BOOL uniqueRnd = random_(universe, 0, 2);
		if (universe.currlevel <= 5) {
			CreateTypeItem(universe, object[oid]._ox, object[oid]._oy, TRUE, ITYPE_LARMOR, IMISC_NONE, TRUE, FALSE);
		} else if (universe.currlevel >= 6 && universe.currlevel <= 9) {
			CreateTypeItem(universe, object[oid]._ox, object[oid]._oy, uniqueRnd, ITYPE_MARMOR, IMISC_NONE, TRUE, FALSE);
		} else if (universe.currlevel >= 10 && universe.currlevel <= 12) {
			CreateTypeItem(universe, object[oid]._ox, object[oid]._oy, FALSE, ITYPE_HARMOR, IMISC_NONE, TRUE, FALSE);
		} else if (universe.currlevel >= 13 && universe.currlevel <= 16) {
			CreateTypeItem(universe, object[oid]._ox, object[oid]._oy, TRUE, ITYPE_HARMOR, IMISC_NONE, TRUE, FALSE);
		}
	} break;
	case OBJ_WARWEAP:
	case OBJ_WEAPONRACK: {
		SetRndSeed(universe, object[oid]._oRndSeed);
		int weaponType;

		switch (random_(universe, 0, 4) + ITYPE_SWORD) {
		case ITYPE_SWORD:
			weaponType = ITYPE_SWORD;
			break;
		case ITYPE_AXE:
			weaponType = ITYPE_AXE;
			break;
		case ITYPE_BOW:
			weaponType = ITYPE_BOW;
			break;
		case ITYPE_MACE:
			weaponType = ITYPE_MACE;
			break;
		}

		if (universe.leveltype > 1)
			CreateTypeItem(universe, object[oid]._ox, object[oid]._oy, TRUE, weaponType, IMISC_NONE, TRUE, FALSE);
		else
			CreateTypeItem(universe, object[oid]._ox, object[oid]._oy, FALSE, weaponType, IMISC_NONE, TRUE, FALSE);
	} break;
	}
}

void DropAllItems(Universe& universe)
{
	MonsterItems = numitems;
	for (int i = 0; i < nummonsters; i++) {
		int mid = monstactive[i];
		if (monster[mid].MType->mtype == MT_GOLEM)
			continue;
		SetRndSeed(universe, monster[mid]._mRndSeed);
		SpawnItem(universe, mid, monster[mid]._mx, monster[mid]._my, TRUE);
	}

	ObjectItems = numitems;
	for (int i = 0; i < nobjects; i++) {
		int oid = objectactive[i];
		CreateItemsFromObject(universe, oid);
	}
}

ScannerPuzzler::ScannerPuzzler(Universe& universe) : Scanner(universe) {}

bool ScannerPuzzler::skipLevel(int level)
{
	return level != 9;
}

void LocatePuzzler(Universe& universe)
{
	DropAllItems(universe);

	POI = { -1, -1 };
	for (int i = 0; i < numitems; i++) {
		int ii = itemactive[i];
		if (item[ii]._iMagical == ITEM_QUALITY_UNIQUE && item[ii]._iUid == 60) {
			POI = { item[ii]._ix, item[ii]._iy };
			break;
		}
	}
}

bool ScannerPuzzler::levelMatches(std::optional<uint32_t> levelSeed)
{
	LocatePuzzler(universe);
	if (POI == Point { -1, -1 })
		return false;

	if (Config.verbose) {
		std::cerr << "Monster Count: " << nummonsters << std::endl;
		for (int i = 0; i < nummonsters; i++) {
			std::cerr << "Monster " << i << ": " << monster[monstactive[i]].mName << " (" << monster[monstactive[i]]._mRndSeed << ")" << std::endl;
		}
		std::cerr << std::endl;
		std::cerr << "Object Count: " << nobjects << std::endl;
		for (int i = 0; i < nobjects; i++) {
			int oid = objectactive[i];
			char objstr[50];
			GetObjectStr(universe, oid, objstr);
			std::cerr << "Object " << i << ": " << objstr << " (" << object[oid]._oRndSeed << ")" << std::endl;
		}
		std::cerr << std::endl;
		std::cerr << "Item Count: " << numitems << std::endl;
		for (int i = 0; i < numitems; i++) {
			std::string prefix = "";
			if (i >= ObjectItems)
				prefix = "Object ";
			else if (i >= MonsterItems)
				prefix = "Monster ";
			std::cerr << prefix << "Item " << i << ": " << item[itemactive[i]]._iIName << " (" << item[itemactive[i]]._iSeed << ")" << std::endl;
		}
	}

	std::cout << universe.sgGameInitInfo.dwSeed << std::endl;

	return true;
}
