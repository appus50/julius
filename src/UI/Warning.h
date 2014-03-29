#ifndef UI_WARNING_H
#define UI_WARNING_H

enum {
	Warning_Orientation = 1,
	Warning_ClearLandNeeded = 2,
	Warning_RoadAccessNeeded = 11,
	Warning_WorkersNeeded = 17,
	Warning_MoreFoodNeeded = 18,
	Warning_BuildMarket = 19,
	Warning_IronNeeded = 26,
	Warning_VinesNeeded = 27,
	Warning_OlivesNeeded = 28,
	Warning_ClayNeeded = 29,
	Warning_TimberNeeded = 30,
	Warning_OpenTradeToImport = 31,
	Warning_TradeImportResource = 32,
	Warning_BuildIronMine = 33,
	Warning_BuildVinesFarm = 34,
	Warning_BuildOliveFarm = 35,
	Warning_BuildClayPit = 36,
	Warning_BuildTimberYard = 37,
	Warning_WaterPipeAccessNeeded = 38,
	Warning_PlaceReservoirNextToWater = 39,
	Warning_ConnectToReservoir = 40,
	Warning_SentriesNeedWall = 41,
	Warning_BuildBarracks = 42,
	Warning_WeaponsNeeded = 43,
	Warning_BuildActorColony = 44,
	Warning_BuildGladiatorSchool = 45,
	Warning_BuildLionHouse = 46,
	Warning_BuildChariotMaker = 47,
};

void UI_Warning_show(int warningId);
int UI_Warning_hasWarnings();
void UI_Warning_clearAll();

void UI_Warning_clearHasWarningFlag();

void UI_Warning_checkFoodStocks(int buildingType);
void UI_Warning_checkReservoirWater(int buildingType);
void UI_Warning_checkNewBuilding(int buildingType, int x, int y, int size);

#endif
