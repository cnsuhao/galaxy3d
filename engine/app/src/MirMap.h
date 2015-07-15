#ifndef __MirMap_h__
#define __MirMap_h__

#include "MirImage.h"

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

	int x;
	int y;
	int back_index;
	int middle_index;
	int front_index;
	bool can_walk;
	bool can_fly;
	bool door_open;
};
#pragma pack()

class MirMap
{
public:
	static void LoadTiles(const std::string &map_file, const std::vector<int> &coords, std::vector<MapTileInfo> &tiles)
	{
		std::vector<int> back_indices(coords.size());
		std::vector<int> middle_indices(coords.size());
		std::vector<int> front_indices(coords.size());

		FILE *f = fopen(map_file.c_str(), "rb");
		if(f != nullptr)
		{
			MapHeader header;
			fread(&header, MapHeader::STRUCT_SIZE, 1, f);

			tiles.resize(coords.size());
			for(size_t i=0; i<coords.size(); i++)
			{
				int x = coords[i] >> 16;
				int y = coords[i] & 0xffff;

				if(x >= header.width || y >= header.height || x < 0 || y < 0)
				{
					continue;
				}

				int offset = MapHeader::STRUCT_SIZE + (x * header.height + y) * MapTileInfo::STRUCT_SIZE;

				fseek(f, offset, SEEK_SET);

				MapTileInfo tile;
				fread(&tile, MapTileInfo::STRUCT_SIZE, 1, f);

				int index = tile.back & 0x7fff;
                if(index == 0 || x % 2 == 1 || y % 2 == 1)
                {
                    index = -1;
                }
				tile.back_index = index - 1;

				index = tile.middle;
                if(index == 0)
                {
                    index = -1;
                }
				tile.middle_index = index - 1;

				index = tile.front & 0x7fff;
                if(index == 0)
                {
                    index = -1;
                }
				tile.front_index = index - 1;

				tile.x = x;
				tile.y = y;
                tile.can_walk = ((tile.back & 0x8000) == 0) && ((tile.front & 0x8000) == 0);
                tile.can_fly = ((tile.front & 0x8000) == 0);
                tile.door_open = (tile.door_state & 0x8000) != 0;

				tiles[i] = tile;

				back_indices[i] = tile.back_index;
				middle_indices[i] = tile.middle_index;
				front_indices[i] = tile.front_index;
			}

			fclose(f);
			f = nullptr;
		}

		auto back_images = MirImage::LoadImages("Tiles", back_indices);
	}
};

#endif