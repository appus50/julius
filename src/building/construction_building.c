#include "construction_building.h"

#include "building/building.h"
#include "building/construction_warning.h"
#include "building/count.h"
#include "building/dock.h"
#include "building/properties.h"
#include "building/storage.h"
#include "city/warning.h"
#include "core/random.h"
#include "figure/formation.h"
#include "graphics/image.h"
#include "map/grid.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/water.h"

#include "Data/State.h"
#include "Data/CityInfo.h"
#include "../Building.h"
#include "../Formation.h"
#include "../SidebarMenu.h"
#include "../Terrain.h"
#include "../TerrainGraphics.h"
#include "../Undo.h"

static void add_fort(int type, building *b)
{
    b->prevPartBuildingId = 0;
    Terrain_addBuildingToGrids(b->id, b->x, b->y, b->size, image_group(GROUP_BUILDING_FORT), TERRAIN_BUILDING);
    b->formationId = Formation_createLegion(b);
    if (type == BUILDING_FORT_LEGIONARIES) {
        b->subtype.fortFigureType = FIGURE_FORT_LEGIONARY;
        formation_set_figure_type(b->formationId, FIGURE_FORT_LEGIONARY);
    }
    if (type == BUILDING_FORT_JAVELIN) {
        b->subtype.fortFigureType = FIGURE_FORT_JAVELIN;
        formation_set_figure_type(b->formationId, FIGURE_FORT_JAVELIN);
    }
    if (type == BUILDING_FORT_MOUNTED) {
        b->subtype.fortFigureType = FIGURE_FORT_MOUNTED;
        formation_set_figure_type(b->formationId, FIGURE_FORT_MOUNTED);
    }
    // create parade ground
    building *ground = building_create(BUILDING_FORT_GROUND, b->x + 3, b->y - 1);
    Undo_addBuildingToList(ground->id);
    ground->formationId = b->formationId;
    ground->prevPartBuildingId = b->id;
    b->nextPartBuildingId = ground->id;
    ground->nextPartBuildingId = 0;
    Terrain_addBuildingToGrids(ground->id, b->x + 3, b->y - 1, 4,
        image_group(GROUP_BUILDING_FORT) + 1, TERRAIN_BUILDING);
}

static void add_hippodrome(building *b)
{
    int image1 = image_group(GROUP_BUILDING_HIPPODROME_1);
    int image2 = image_group(GROUP_BUILDING_HIPPODROME_2);
    Data_CityInfo.buildingHippodromePlaced = 1;

    building *part1 = b;
    if (Data_State.map.orientation == DIR_0_TOP || Data_State.map.orientation == DIR_4_BOTTOM) {
        part1->subtype.orientation = 0;
    } else {
        part1->subtype.orientation = 3;
    }
    part1->prevPartBuildingId = 0;
    int image_id;
    switch (Data_State.map.orientation) {
        case DIR_0_TOP:    image_id = image2; break;
        case DIR_2_RIGHT:  image_id = image1 + 4; break;
        case DIR_4_BOTTOM: image_id = image2 + 4; break;
        case DIR_6_LEFT:   image_id = image1; break;
        default: return;
    }
    Terrain_addBuildingToGrids(b->id, b->x, b->y, b->size, image_id, TERRAIN_BUILDING);

    building *part2 = building_create(BUILDING_HIPPODROME, b->x + 5, b->y);
    Undo_addBuildingToList(part2->id);
    if (Data_State.map.orientation == DIR_0_TOP || Data_State.map.orientation == DIR_4_BOTTOM) {
        part2->subtype.orientation = 1;
    } else {
        part2->subtype.orientation = 4;
    }
    part2->prevPartBuildingId = part1->id;
    part1->nextPartBuildingId = part2->id;
    part2->nextPartBuildingId = 0;
    switch (Data_State.map.orientation) {
        case DIR_0_TOP: case DIR_4_BOTTOM: 
        image_id = image2 + 2; break;
        case DIR_2_RIGHT: case DIR_6_LEFT: 
        image_id = image1 + 2; break;
    }
    Terrain_addBuildingToGrids(part2->id, b->x + 5, b->y, b->size, image_id, TERRAIN_BUILDING);

    building *part3 = building_create(BUILDING_HIPPODROME, b->x + 10, b->y);
    Undo_addBuildingToList(part3->id);
    if (Data_State.map.orientation == DIR_0_TOP || Data_State.map.orientation == DIR_4_BOTTOM) {
        part3->subtype.orientation = 2;
    } else {
        part3->subtype.orientation = 5;
    }
    part3->prevPartBuildingId = part2->id;
    part2->nextPartBuildingId = part3->id;
    part3->nextPartBuildingId = 0;
    switch (Data_State.map.orientation) {
        case DIR_0_TOP: 
        image_id = image2 + 4; break;
        case DIR_2_RIGHT: 
        image_id = image1; break;
        case DIR_4_BOTTOM: 
        image_id = image2; break;
        case DIR_6_LEFT: 
        image_id = image1 + 4; break;
    }
    Terrain_addBuildingToGrids(part3->id, b->x + 10, b->y, b->size, image_id, TERRAIN_BUILDING);
}

static building *add_warehouse_space(int x, int y, building *prev)
{
    building *b = building_create(BUILDING_WAREHOUSE_SPACE, x, y);
    Undo_addBuildingToList(b->id);
    b->prevPartBuildingId = prev->id;
    prev->nextPartBuildingId = b->id;
    Terrain_addBuildingToGrids(b->id, x, y, 1,
        image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_EMPTY), TERRAIN_BUILDING);
    return b;
}

static void add_warehouse(building *b)
{
    b->storage_id = building_storage_create();
    b->prevPartBuildingId = 0;
    Terrain_addBuildingToGrids(b->id, b->x, b->y, 1, image_group(GROUP_BUILDING_WAREHOUSE), TERRAIN_BUILDING);

    building *prev = b;
    prev = add_warehouse_space(b->x + 1, b->y, prev);
    prev = add_warehouse_space(b->x + 2, b->y, prev);
    prev = add_warehouse_space(b->x, b->y + 1, prev);
    prev = add_warehouse_space(b->x + 1, b->y + 1, prev);
    prev = add_warehouse_space(b->x + 2, b->y + 1, prev);
    prev = add_warehouse_space(b->x, b->y + 2, prev);
    prev = add_warehouse_space(b->x + 1, b->y + 2, prev);
    prev = add_warehouse_space(b->x + 2, b->y + 2, prev);
    prev->nextPartBuildingId = 0;
}

static void add_building(building *b, int image_id)
{
    Terrain_addBuildingToGrids(b->id, b->x, b->y, b->size, image_id, TERRAIN_BUILDING);
}

static void add_to_map(int type, building *b, int size,
    int orientation, int waterside_orientation_abs, int waterside_orientation_rel)
{
    switch (type) {
        // houses
        case BUILDING_HOUSE_LARGE_TENT:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_TENT) + 2);
            break;
        case BUILDING_HOUSE_SMALL_SHACK:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_SHACK));
            break;
        case BUILDING_HOUSE_LARGE_SHACK:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_SHACK) + 2);
            break;
        case BUILDING_HOUSE_SMALL_HOVEL:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_HOVEL));
            break;
        case BUILDING_HOUSE_LARGE_HOVEL:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_HOVEL) + 2);
            break;
        case BUILDING_HOUSE_SMALL_CASA:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_CASA));
            break;
        case BUILDING_HOUSE_LARGE_CASA:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_CASA) + 2);
            break;
        case BUILDING_HOUSE_SMALL_INSULA:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_INSULA_1));
            break;
        case BUILDING_HOUSE_MEDIUM_INSULA:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_INSULA_1) + 2);
            break;
        case BUILDING_HOUSE_LARGE_INSULA:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_INSULA_2));
            break;
        case BUILDING_HOUSE_GRAND_INSULA:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_INSULA_2) + 2);
            break;
        case BUILDING_HOUSE_SMALL_VILLA:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_VILLA_1));
            break;
        case BUILDING_HOUSE_MEDIUM_VILLA:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_VILLA_1) + 2);
            break;
        case BUILDING_HOUSE_LARGE_VILLA:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_VILLA_2));
            break;
        case BUILDING_HOUSE_GRAND_VILLA:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_VILLA_2) + 1);
            break;
        case BUILDING_HOUSE_SMALL_PALACE:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_PALACE_1));
            break;
        case BUILDING_HOUSE_MEDIUM_PALACE:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_PALACE_1) + 1);
            break;
        case BUILDING_HOUSE_LARGE_PALACE:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_PALACE_2));
            break;
        case BUILDING_HOUSE_LUXURY_PALACE:
            add_building(b, image_group(GROUP_BUILDING_HOUSE_PALACE_2) + 1);
            break;
        // entertainment
        case BUILDING_AMPHITHEATER:
            add_building(b, image_group(GROUP_BUILDING_AMPHITHEATER));
            break;
        case BUILDING_THEATER:
            add_building(b, image_group(GROUP_BUILDING_THEATER));
            break;
        case BUILDING_COLOSSEUM:
            add_building(b, image_group(GROUP_BUILDING_COLOSSEUM));
            break;
        case BUILDING_GLADIATOR_SCHOOL:
            add_building(b, image_group(GROUP_BUILDING_GLADIATOR_SCHOOL));
            break;
        case BUILDING_LION_HOUSE:
            add_building(b, image_group(GROUP_BUILDING_LION_HOUSE));
            break;
        case BUILDING_ACTOR_COLONY:
            add_building(b, image_group(GROUP_BUILDING_ACTOR_COLONY));
            break;
        case BUILDING_CHARIOT_MAKER:
            add_building(b, image_group(GROUP_BUILDING_CHARIOT_MAKER));
            break;
        // statues
        case BUILDING_SMALL_STATUE:
            add_building(b, image_group(GROUP_BUILDING_STATUE));
            break;
        case BUILDING_MEDIUM_STATUE:
            add_building(b, image_group(GROUP_BUILDING_STATUE) + 1);
            break;
        case BUILDING_LARGE_STATUE:
            add_building(b, image_group(GROUP_BUILDING_STATUE) + 2);
            break;
        // health
        case BUILDING_DOCTOR:
            add_building(b, image_group(GROUP_BUILDING_DOCTOR));
            break;
        case BUILDING_HOSPITAL:
            add_building(b, image_group(GROUP_BUILDING_HOSPITAL));
            break;
        case BUILDING_BATHHOUSE:
            add_building(b, image_group(GROUP_BUILDING_BATHHOUSE_NO_WATER));
            break;
        case BUILDING_BARBER:
            add_building(b, image_group(GROUP_BUILDING_BARBER));
            break;
        // education
        case BUILDING_SCHOOL:
            add_building(b, image_group(GROUP_BUILDING_SCHOOL));
            break;
        case BUILDING_ACADEMY:
            add_building(b, image_group(GROUP_BUILDING_ACADEMY));
            break;
        case BUILDING_LIBRARY:
            add_building(b, image_group(GROUP_BUILDING_LIBRARY));
            break;
        // security
        case BUILDING_PREFECTURE:
            add_building(b, image_group(GROUP_BUILDING_PREFECTURE));
            break;
        // farms
        case BUILDING_WHEAT_FARM:
            TerrainGraphics_setBuildingFarm(b->id, b->x, b->y, image_group(GROUP_BUILDING_FARM_CROPS), 0);
            break;
        case BUILDING_VEGETABLE_FARM:
            TerrainGraphics_setBuildingFarm(b->id, b->x, b->y, image_group(GROUP_BUILDING_FARM_CROPS) + 5, 0);
            break;
        case BUILDING_FRUIT_FARM:
            TerrainGraphics_setBuildingFarm(b->id, b->x, b->y, image_group(GROUP_BUILDING_FARM_CROPS) + 10, 0);
            break;
        case BUILDING_OLIVE_FARM:
            TerrainGraphics_setBuildingFarm(b->id, b->x, b->y, image_group(GROUP_BUILDING_FARM_CROPS) + 15, 0);
            break;
        case BUILDING_VINES_FARM:
            TerrainGraphics_setBuildingFarm(b->id, b->x, b->y, image_group(GROUP_BUILDING_FARM_CROPS) + 20, 0);
            break;
        case BUILDING_PIG_FARM:
            TerrainGraphics_setBuildingFarm(b->id, b->x, b->y, image_group(GROUP_BUILDING_FARM_CROPS) + 25, 0);
            break;
        // industry
        case BUILDING_MARBLE_QUARRY:
            add_building(b, image_group(GROUP_BUILDING_MARBLE_QUARRY));
            break;
        case BUILDING_IRON_MINE:
            add_building(b, image_group(GROUP_BUILDING_IRON_MINE));
            break;
        case BUILDING_TIMBER_YARD:
            add_building(b, image_group(GROUP_BUILDING_TIMBER_YARD));
            break;
        case BUILDING_CLAY_PIT:
            add_building(b, image_group(GROUP_BUILDING_CLAY_PIT));
            break;
        // workshops
        case BUILDING_WINE_WORKSHOP:
            add_building(b, image_group(GROUP_BUILDING_WINE_WORKSHOP));
            break;
        case BUILDING_OIL_WORKSHOP:
            add_building(b, image_group(GROUP_BUILDING_OIL_WORKSHOP));
            break;
        case BUILDING_WEAPONS_WORKSHOP:
            add_building(b, image_group(GROUP_BUILDING_WEAPONS_WORKSHOP));
            break;
        case BUILDING_FURNITURE_WORKSHOP:
            add_building(b, image_group(GROUP_BUILDING_FURNITURE_WORKSHOP));
            break;
        case BUILDING_POTTERY_WORKSHOP:
            add_building(b, image_group(GROUP_BUILDING_POTTERY_WORKSHOP));
            break;
        // distribution
        case BUILDING_GRANARY:
            b->storage_id = building_storage_create();
            add_building(b, image_group(GROUP_BUILDING_GRANARY));
            break;
        case BUILDING_MARKET:
            add_building(b, image_group(GROUP_BUILDING_MARKET));
            break;
        // government
        case BUILDING_GOVERNORS_HOUSE:
            add_building(b, image_group(GROUP_BUILDING_GOVERNORS_HOUSE));
            break;
        case BUILDING_GOVERNORS_VILLA:
            add_building(b, image_group(GROUP_BUILDING_GOVERNORS_VILLA));
            break;
        case BUILDING_GOVERNORS_PALACE:
            add_building(b, image_group(GROUP_BUILDING_GOVERNORS_PALACE));
            break;
        case BUILDING_MISSION_POST:
            add_building(b, image_group(GROUP_BUILDING_MISSION_POST));
            break;
        case BUILDING_ENGINEERS_POST:
            add_building(b, image_group(GROUP_BUILDING_ENGINEERS_POST));
            break;
        case BUILDING_FORUM:
            add_building(b, image_group(GROUP_BUILDING_FORUM));
            break;
        // water
        case BUILDING_FOUNTAIN:
            add_building(b, image_group(GROUP_BUILDING_FOUNTAIN_1));
            break;
        case BUILDING_WELL:
            add_building(b, image_group(GROUP_BUILDING_WELL));
            break;
        // military
        case BUILDING_MILITARY_ACADEMY:
            add_building(b, image_group(GROUP_BUILDING_MILITARY_ACADEMY));
            break;
        // religion
        case BUILDING_SMALL_TEMPLE_CERES:
            add_building(b, image_group(GROUP_BUILDING_TEMPLE_CERES));
            break;
        case BUILDING_SMALL_TEMPLE_NEPTUNE:
            add_building(b, image_group(GROUP_BUILDING_TEMPLE_NEPTUNE));
            break;
        case BUILDING_SMALL_TEMPLE_MERCURY:
            add_building(b, image_group(GROUP_BUILDING_TEMPLE_MERCURY));
            break;
        case BUILDING_SMALL_TEMPLE_MARS:
            add_building(b, image_group(GROUP_BUILDING_TEMPLE_MARS));
            break;
        case BUILDING_SMALL_TEMPLE_VENUS:
            add_building(b, image_group(GROUP_BUILDING_TEMPLE_VENUS));
            break;
        case BUILDING_LARGE_TEMPLE_CERES:
            add_building(b, image_group(GROUP_BUILDING_TEMPLE_CERES) + 1);
            break;
        case BUILDING_LARGE_TEMPLE_NEPTUNE:
            add_building(b, image_group(GROUP_BUILDING_TEMPLE_NEPTUNE) + 1);
            break;
        case BUILDING_LARGE_TEMPLE_MERCURY:
            add_building(b, image_group(GROUP_BUILDING_TEMPLE_MERCURY) + 1);
            break;
        case BUILDING_LARGE_TEMPLE_MARS:
            add_building(b, image_group(GROUP_BUILDING_TEMPLE_MARS) + 1);
            break;
        case BUILDING_LARGE_TEMPLE_VENUS:
            add_building(b, image_group(GROUP_BUILDING_TEMPLE_VENUS) + 1);
            break;
        case BUILDING_ORACLE:
            add_building(b, image_group(GROUP_BUILDING_ORACLE));
            break;
        // ships
        case BUILDING_SHIPYARD:
            b->data.other.dockOrientation = waterside_orientation_abs;
            map_water_add_building(b->id, b->x, b->y, 2, image_group(GROUP_BUILDING_SHIPYARD) + waterside_orientation_rel);
            break;
        case BUILDING_WHARF:
            b->data.other.dockOrientation = waterside_orientation_abs;
            map_water_add_building(b->id, b->x, b->y, 2, image_group(GROUP_BUILDING_WHARF) + waterside_orientation_rel);
            break;
        case BUILDING_DOCK:
            Data_CityInfo.numWorkingDocks++;
            b->data.other.dockOrientation = waterside_orientation_abs;
            {
                int image_id;
                switch (waterside_orientation_rel) {
                    case 0: image_id = image_group(GROUP_BUILDING_DOCK_1); break;
                    case 1: image_id = image_group(GROUP_BUILDING_DOCK_2); break;
                    case 2: image_id = image_group(GROUP_BUILDING_DOCK_3); break;
                    default:image_id = image_group(GROUP_BUILDING_DOCK_4); break;
                }
                map_water_add_building(b->id, b->x, b->y, size, image_id);
            }
            break;
        // defense
        case BUILDING_TOWER:
            Terrain_clearWithRadius(b->x, b->y, 2, 0, TERRAIN_WALL);
            Terrain_addBuildingToGrids(b->id, b->x, b->y, size, image_group(GROUP_BUILDING_TOWER),
                TERRAIN_BUILDING | TERRAIN_GATEHOUSE);
            TerrainGraphics_updateAreaWalls(b->x, b->y, 5);
            break;
        case BUILDING_GATEHOUSE:
            Terrain_addBuildingToGrids(b->id, b->x, b->y, size,
                image_group(GROUP_BUILDING_TOWER) + orientation, TERRAIN_BUILDING | TERRAIN_GATEHOUSE);
            b->subtype.orientation = orientation;
            Building_determineGraphicIdsForOrientedBuildings();
            Terrain_addRoadsForGatehouse(b->x, b->y, orientation);
            TerrainGraphics_updateAreaRoads(b->x, b->y, 5);
            TerrainGraphics_updateRegionPlazas(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
            TerrainGraphics_updateAreaWalls(b->x, b->y, 5);
            break;
        case BUILDING_TRIUMPHAL_ARCH:
            add_building(b, image_group(GROUP_BUILDING_TRIUMPHAL_ARCH) + orientation - 1);
            b->subtype.orientation = orientation;
            Building_determineGraphicIdsForOrientedBuildings();
            Terrain_addRoadsForTriumphalArch(b->x, b->y, orientation);
            TerrainGraphics_updateAreaRoads(b->x, b->y, 5);
            TerrainGraphics_updateRegionPlazas(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
            Data_CityInfo.triumphalArchesPlaced++;
            SidebarMenu_enableBuildingMenuItems();
            Data_State.selectedBuilding.type = 0;
            break;
        case BUILDING_SENATE_UPGRADED:
            Data_CityInfo.buildingSenatePlaced = 1;
            add_building(b, image_group(GROUP_BUILDING_SENATE));
            if (!Data_CityInfo.buildingSenateGridOffset) {
                Data_CityInfo.buildingSenateBuildingId = b->id;
                Data_CityInfo.buildingSenateX = b->x;
                Data_CityInfo.buildingSenateY = b->y;
                Data_CityInfo.buildingSenateGridOffset = b->gridOffset;
            }
            break;
        case BUILDING_BARRACKS:
            add_building(b, image_group(GROUP_BUILDING_BARRACKS));
            if (!Data_CityInfo.buildingBarracksGridOffset) {
                Data_CityInfo.buildingBarracksBuildingId = b->id;
                Data_CityInfo.buildingBarracksX = b->x;
                Data_CityInfo.buildingBarracksY = b->y;
                Data_CityInfo.buildingBarracksGridOffset = b->gridOffset;
            }
            break;
        case BUILDING_WAREHOUSE:
            add_warehouse(b);
            break;
        case BUILDING_HIPPODROME:
            add_hippodrome(b);
            break;
        case BUILDING_FORT_LEGIONARIES:
        case BUILDING_FORT_JAVELIN:
        case BUILDING_FORT_MOUNTED:
            add_fort(type, b);
            break;
        // native buildings (unused, I think)
        case BUILDING_NATIVE_HUT:
            add_building(b, image_group(GROUP_BUILDING_NATIVE) + (random_byte() & 1));
            break;
        case BUILDING_NATIVE_MEETING:
            add_building(b, image_group(GROUP_BUILDING_NATIVE) + 2);
            break;
        case BUILDING_NATIVE_CROPS:
            add_building(b, image_group(GROUP_BUILDING_FARM_CROPS));
            break;
        // distribution center (also unused)
        case BUILDING_DISTRIBUTION_CENTER_UNUSED:
            Data_CityInfo.buildingDistributionCenterPlaced = 1;
            if (!Data_CityInfo.buildingDistributionCenterGridOffset) {
                Data_CityInfo.buildingDistributionCenterBuildingId = b->id;
                Data_CityInfo.buildingDistributionCenterX = b->x;
                Data_CityInfo.buildingDistributionCenterY = b->y;
                Data_CityInfo.buildingDistributionCenterGridOffset = b->gridOffset;
            }
            break;
    }
    map_routing_update_land();
    map_routing_update_walls();
}

int building_construction_place_building(building_type type, int x, int y)
{
    int terrain_mask = TERRAIN_ALL;
    if (type == BUILDING_GATEHOUSE || type == BUILDING_TRIUMPHAL_ARCH) {
        terrain_mask = ~TERRAIN_ROAD;
    } else if (type == BUILDING_TOWER) {
        terrain_mask = ~TERRAIN_WALL;
    }
    int size = building_properties_for_type(type)->size;
    if (type == BUILDING_WAREHOUSE) {
        size = 3;
    }
    int building_orientation = 0;
    if (type == BUILDING_GATEHOUSE) {
        building_orientation = Terrain_getOrientationGatehouse(x, y);
    } else if (type == BUILDING_TRIUMPHAL_ARCH) {
        building_orientation = Terrain_getOrientationTriumphalArch(x, y);
    }
    switch (Data_State.map.orientation) {
        case DIR_2_RIGHT: x = x - size + 1; break;
        case DIR_4_BOTTOM: x = x - size + 1; y = y - size + 1; break;
        case DIR_6_LEFT: y = y - size + 1; break;
    }
    // extra checks
    if (type == BUILDING_GATEHOUSE) {
        if (!Terrain_isClear(x, y, size, terrain_mask, 0)) {
            city_warning_show(WARNING_CLEAR_LAND_NEEDED);
            return 0;
        }
        if (!building_orientation) {
            if (Data_State.selectedBuilding.roadRequired == 1) {
                building_orientation = 1;
            } else {
                building_orientation = 2;
            }
        }
    }
    if (type == BUILDING_TRIUMPHAL_ARCH) {
        if (!Terrain_isClear(x, y, size, terrain_mask, 0)) {
            city_warning_show(WARNING_CLEAR_LAND_NEEDED);
            return 0;
        }
        if (!building_orientation) {
            if (Data_State.selectedBuilding.roadRequired == 1) {
                building_orientation = 1;
            } else {
                building_orientation = 3;
            }
        }
    }
    int waterside_orientation_abs, waterside_orientation_rel;
    if (type == BUILDING_SHIPYARD || type == BUILDING_WHARF) {
        if (map_water_determine_orientation_size2(
                x, y, 0, &waterside_orientation_abs, &waterside_orientation_rel)) {
            city_warning_show(WARNING_SHORE_NEEDED);
            return 0;
        }
    } else if (type == BUILDING_DOCK) {
        if (map_water_determine_orientation_size3(
                x, y, 0, &waterside_orientation_abs, &waterside_orientation_rel)) {
            city_warning_show(WARNING_SHORE_NEEDED);
            return 0;
        }
        if (!building_dock_is_connected_to_open_water(x, y)) {
            city_warning_show(WARNING_DOCK_OPEN_WATER_NEEDED);
            return 0;
        }
    } else {
        if (!Terrain_isClear(x, y, size, terrain_mask, 0)) {
            city_warning_show(WARNING_CLEAR_LAND_NEEDED);
            return 0;
        }
        if (Data_State.selectedBuilding.meadowRequired) {
            if (!Terrain_existsTileWithinRadiusWithType(x, y, 3, 1, TERRAIN_MEADOW)) {
                city_warning_show(WARNING_MEADOW_NEEDED);
                return 0;
            }
        } else if (Data_State.selectedBuilding.rockRequired) {
            if (!Terrain_existsTileWithinRadiusWithType(x, y, 2, 1, TERRAIN_ROCK)) {
                city_warning_show(WARNING_ROCK_NEEDED);
                return 0;
            }
        } else if (Data_State.selectedBuilding.treesRequired) {
            if (!Terrain_existsTileWithinRadiusWithType(x, y, 2, 1, TERRAIN_SCRUB | TERRAIN_TREE)) {
                city_warning_show(WARNING_TREE_NEEDED);
                return 0;
            }
        } else if (Data_State.selectedBuilding.waterRequired) {
            if (!Terrain_existsTileWithinRadiusWithType(x, y, 2, 3, TERRAIN_WATER)) {
                city_warning_show(WARNING_WATER_NEEDED);
                return 0;
            }
        } else if (Data_State.selectedBuilding.wallRequired) {
            if (!Terrain_allTilesWithinRadiusHaveType(x, y, 2, 0, TERRAIN_WALL)) {
                city_warning_show(WARNING_WALL_NEEDED);
                return 0;
            }
        }
    }
    if (type == BUILDING_FORT_LEGIONARIES || type == BUILDING_FORT_JAVELIN || type == BUILDING_FORT_MOUNTED) {
        if (!Terrain_isClear(x + 3, y - 1, 4, terrain_mask, 0)) {
            city_warning_show(WARNING_CLEAR_LAND_NEEDED);
            return 0;
        }
        if (formation_totals_get_num_legions() >= 6) {
            city_warning_show(WARNING_MAX_LEGIONS_REACHED);
            return 0;
        }
    }
    if (type == BUILDING_HIPPODROME) {
        if (Data_CityInfo.buildingHippodromePlaced) {
            city_warning_show(WARNING_ONE_BUILDING_OF_TYPE);
            return 0;
        }
        if (!Terrain_isClear(x + 5, y, 5, terrain_mask, 0) ||
            !Terrain_isClear(x + 10, y, 5, terrain_mask, 0)) {
            city_warning_show(WARNING_CLEAR_LAND_NEEDED);
            return 0;
        }
    }
    if (type == BUILDING_SENATE_UPGRADED && Data_CityInfo.buildingSenatePlaced) {
        city_warning_show(WARNING_ONE_BUILDING_OF_TYPE);
        return 0;
    }
    if (type == BUILDING_BARRACKS && building_count_total(BUILDING_BARRACKS) > 0) {
        city_warning_show(WARNING_ONE_BUILDING_OF_TYPE);
        return 0;
    }
    building_construction_warning_check_all(type, x, y, size);

    // phew, checks done!
    building *b;
    if (type == BUILDING_FORT_LEGIONARIES || type == BUILDING_FORT_JAVELIN || type == BUILDING_FORT_MOUNTED) {
        b = building_create(BUILDING_FORT, x, y);
    } else {
        b = building_create(type, x, y);
    }
    Undo_addBuildingToList(b->id);
    if (b->id <= 0) {
        return 0;
    }
    add_to_map(type, b, size, building_orientation, waterside_orientation_abs, waterside_orientation_rel);
    return 1;
}