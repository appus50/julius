#ifndef MAP_BUILDING_TILES_H
#define MAP_BUILDING_TILES_H

void map_building_tiles_add(int building_id, int x, int y, int size, int image_id, int terrain);

void map_building_tiles_add_farm(int building_id, int x, int y, int crop_image_id, int progress);

void map_building_tiles_remove(int building_id, int x, int y);

void map_building_tiles_set_rubble(int building_id, int x, int y, int size);

#endif // MAP_BUILDING_TILES_H
