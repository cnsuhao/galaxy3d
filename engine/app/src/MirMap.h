#ifndef __MirMap_h__
#define __MirMap_h__

#include "MirImage.h"
#include "SpriteRenderer.h"
#include <stdio.h>
#include <string>
#include <vector>

#pragma pack(1)
struct MapHeader//52
{
	static const int STRUCT_SIZE = 52;
	static const int FLAG_SIZE = 16;
	static const int RESERVED_SIZE = 16;

	short width;
	short height;
	char flag[16];
	long long update_time;
	char reserved[24];
};

struct MapTileInfo//12
{
    static const int STRUCT_SIZE = 12;
	static const int TILE_WIDTH = 48;
	static const int TILE_HEIGHT = 32;

    unsigned short back;//Tiles
    unsigned short middle;//SmTiles
    unsigned short front;//Objects
    char door_index;//Objects
    char door_state;//Objects
    char anim_frame;//Objects
    char anim_tick;//Objects
    char obj;//Objects
    char light;
};

struct MapTile
{
	MapTileInfo info;

	int x;
	int y;
	int back_index;
	int middle_index;
	int front_index;
	int front_count;
	bool can_walk;
	bool can_fly;
	bool door_open;

	std::shared_ptr<MirImage> back_image;
	std::shared_ptr<MirImage> middle_image;
	std::vector<std::shared_ptr<MirImage>> front_images;

	std::shared_ptr<SpriteRenderer> back_sprite;
	std::shared_ptr<SpriteRenderer> middle_sprite;
	std::shared_ptr<SpriteRenderer> front_sprite;
	std::vector<std::shared_ptr<Sprite>> front_sprites;
	int front_frame;
};
#pragma pack()

class MirMap
{
public:
	static void LoadTiles(const std::string &map_file, const std::vector<int> &coords, std::vector<MapTile> &tiles);
	static void UpdateTiles(std::vector<MapTile> &tiles);
};

#endif