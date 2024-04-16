
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
	switch (universe.object[oid]._otype) {
	case OBJ_CHEST1:
	case OBJ_CHEST2:
	case OBJ_CHEST3:
	case OBJ_TCHEST1:
	case OBJ_TCHEST2:
	case OBJ_TCHEST3:
		SetRndSeed(universe, universe.object[oid]._oRndSeed);
		if (universe.setlevel) {
			for (int j = 0; j < universe.object[oid]._oVar1; j++) {
				CreateRndItem(universe, universe.object[oid]._ox, universe.object[oid]._oy, TRUE, TRUE, FALSE);
			}
		} else {
			for (int j = 0; j < universe.object[oid]._oVar1; j++) {
				if (universe.object[oid]._oVar2 != 0)
					CreateRndItem(universe, universe.object[oid]._ox, universe.object[oid]._oy, FALSE, TRUE, FALSE);
				else
					CreateRndUseful(universe, 0, universe.object[oid]._ox, universe.object[oid]._oy, TRUE);
			}
		}
		break;
	case OBJ_SARC:
		SetRndSeed(universe, universe.object[oid]._oRndSeed);
		if (universe.object[oid]._oVar1 <= 2)
			CreateRndItem(universe, universe.object[oid]._ox, universe.object[oid]._oy, FALSE, TRUE, FALSE);
		break;
	case OBJ_DECAP:
		SetRndSeed(universe, universe.object[oid]._oRndSeed);
		CreateRndItem(universe, universe.object[oid]._ox, universe.object[oid]._oy, FALSE, TRUE, FALSE);
		break;
	case OBJ_BARREL:
		SetRndSeed(universe, universe.object[oid]._oRndSeed);
		if (universe.object[oid]._oVar2 <= 1) {
			if (universe.object[oid]._oVar3 == 0)
				CreateRndUseful(universe, 0, universe.object[oid]._ox, universe.object[oid]._oy, TRUE);
			else
				CreateRndItem(universe, universe.object[oid]._ox, universe.object[oid]._oy, FALSE, TRUE, FALSE);
		}
		break;
	case OBJ_SKELBOOK:
	case OBJ_BOOKSTAND:
		SetRndSeed(universe, universe.object[oid]._oRndSeed);
		if (random_(universe, 161, 5) != 0)
			CreateTypeItem(universe, universe.object[oid]._ox, universe.object[oid]._oy, FALSE, ITYPE_MISC, IMISC_SCROLL, TRUE, FALSE);
		else
			CreateTypeItem(universe, universe.object[oid]._ox, universe.object[oid]._oy, FALSE, ITYPE_MISC, IMISC_BOOK, TRUE, FALSE);
		break;
	case OBJ_BOOKCASEL:
	case OBJ_BOOKCASER:
		SetRndSeed(universe, universe.object[oid]._oRndSeed);
		CreateTypeItem(universe, universe.object[oid]._ox, universe.object[oid]._oy, FALSE, ITYPE_MISC, IMISC_BOOK, TRUE, FALSE);
		break;
	case OBJ_ARMORSTAND:
	case OBJ_WARARMOR: {
		SetRndSeed(universe, universe.object[oid]._oRndSeed);
		BOOL uniqueRnd = random_(universe, 0, 2);
		if (universe.currlevel <= 5) {
			CreateTypeItem(universe, universe.object[oid]._ox, universe.object[oid]._oy, TRUE, ITYPE_LARMOR, IMISC_NONE, TRUE, FALSE);
		} else if (universe.currlevel >= 6 && universe.currlevel <= 9) {
			CreateTypeItem(universe, universe.object[oid]._ox, universe.object[oid]._oy, uniqueRnd, ITYPE_MARMOR, IMISC_NONE, TRUE, FALSE);
		} else if (universe.currlevel >= 10 && universe.currlevel <= 12) {
			CreateTypeItem(universe, universe.object[oid]._ox, universe.object[oid]._oy, FALSE, ITYPE_HARMOR, IMISC_NONE, TRUE, FALSE);
		} else if (universe.currlevel >= 13 && universe.currlevel <= 16) {
			CreateTypeItem(universe, universe.object[oid]._ox, universe.object[oid]._oy, TRUE, ITYPE_HARMOR, IMISC_NONE, TRUE, FALSE);
		}
	} break;
	case OBJ_WARWEAP:
	case OBJ_WEAPONRACK: {
		SetRndSeed(universe, universe.object[oid]._oRndSeed);
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
			CreateTypeItem(universe, universe.object[oid]._ox, universe.object[oid]._oy, TRUE, weaponType, IMISC_NONE, TRUE, FALSE);
		else
			CreateTypeItem(universe, universe.object[oid]._ox, universe.object[oid]._oy, FALSE, weaponType, IMISC_NONE, TRUE, FALSE);
	} break;
	}
}

void DropAllItems(Universe& universe)
{
	MonsterItems = universe.numitems;
	for (int i = 0; i < universe.nummonsters; i++) {
		int mid = universe.monstactive[i];
		if (universe.monster[mid].MType->mtype == MT_GOLEM)
			continue;
		SetRndSeed(universe, universe.monster[mid]._mRndSeed);
		SpawnItem(universe, mid, universe.monster[mid]._mx, universe.monster[mid]._my, TRUE);
	}

	ObjectItems = universe.numitems;
	for (int i = 0; i < universe.nobjects; i++) {
		int oid = universe.objectactive[i];
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
	for (int i = 0; i < universe.numitems; i++) {
		int ii = universe.itemactive[i];
		if (universe.item[ii]._iMagical == ITEM_QUALITY_UNIQUE && universe.item[ii]._iUid == 60) {
			POI = { universe.item[ii]._ix, universe.item[ii]._iy };
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
		std::cerr << "Monster Count: " << universe.nummonsters << std::endl;
		for (int i = 0; i < universe.nummonsters; i++) {
			std::cerr << "Monster " << i << ": " << universe.monster[universe.monstactive[i]].mName << " (" << universe.monster[universe.monstactive[i]]._mRndSeed << ")" << std::endl;
		}
		std::cerr << std::endl;
		std::cerr << "Object Count: " << universe.nobjects << std::endl;
		for (int i = 0; i < universe.nobjects; i++) {
			int oid = universe.objectactive[i];
			char objstr[50];
			GetObjectStr(universe, oid, objstr);
			std::cerr << "Object " << i << ": " << objstr << " (" << universe.object[oid]._oRndSeed << ")" << std::endl;
		}
		std::cerr << std::endl;
		std::cerr << "Item Count: " << universe.numitems << std::endl;
		for (int i = 0; i < universe.numitems; i++) {
			std::string prefix = "";
			if (i >= ObjectItems)
				prefix = "Object ";
			else if (i >= MonsterItems)
				prefix = "Monster ";
			std::cerr << prefix << "Item " << i << ": " << universe.item[universe.itemactive[i]]._iIName << " (" << universe.item[universe.itemactive[i]]._iSeed << ")" << std::endl;
		}
	}

	std::cout << universe.sgGameInitInfo.dwSeed << std::endl;

	return true;
}
