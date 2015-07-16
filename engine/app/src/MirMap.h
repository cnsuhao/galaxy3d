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
};
#pragma pack()

class MirMap
{
public:
	static void LoadTiles(const std::string &map_file, const std::vector<int> &coords, std::vector<MapTile> &tiles)
	{
		std::vector<int> back_indices(coords.size());
		std::vector<int> middle_indices(coords.size());
		std::unordered_map<int, std::vector<int>> front_indices;

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

				MapTile tile;
				fread(&tile.info, MapTileInfo::STRUCT_SIZE, 1, f);

				int index = tile.info.back & 0x7fff;
                if(index == 0 || x % 2 == 1 || y % 2 == 1)
                {
                    index = -1;
                }
				tile.back_index = index - 1;

				index = tile.info.middle;
                if(index == 0)
                {
                    index = -1;
                }
				tile.middle_index = index - 1;

				index = tile.info.front & 0x7fff;
                if(index == 0)
                {
                    index = -1;
                }
				tile.front_index = index - 1;

				tile.x = x;
				tile.y = y;
                tile.can_walk = ((tile.info.back & 0x8000) == 0) && ((tile.info.front & 0x8000) == 0);
                tile.can_fly = ((tile.info.front & 0x8000) == 0);
                tile.door_open = (tile.info.door_state & 0x8000) != 0;

				back_indices[i] = tile.back_index;
				middle_indices[i] = tile.middle_index;

				if(tile.front_index >= 0)
				{
					std::vector<int> *vec = nullptr;
					int obj = tile.info.obj;

					auto find_obj = front_indices.find(obj);
					if(find_obj != front_indices.end())
					{
						vec = &find_obj->second;
					}
					else
					{
						front_indices[obj] = std::vector<int>();
						vec = &front_indices[obj];
					}

					int frame_count = 1;
					int anim_frame = tile.info.anim_frame;
					if((anim_frame & 0x80) > 0)
					{
						frame_count = anim_frame & 0x7f;
					}
                
					for(int j=0; j<frame_count; j++)
					{
						vec->push_back(tile.front_index + j);
					}

					tile.front_count = frame_count;
				}
				else
				{
					tile.front_count = -1;
				}

				tiles[i] = tile;
			}

			fclose(f);
			f = nullptr;
		}

		//先将图片按文件批量加载
		auto back_images = MirImage::LoadImages("Tiles", back_indices);
		auto middle_images = MirImage::LoadImages("SmTiles", middle_indices);
		std::vector<std::shared_ptr<MirImage>> front_images;

		for(auto &i : front_indices)
		{
			std::string name = "Objects" + GTString::ToString(i.first + 1).str;
			auto fronts = MirImage::LoadImages(name, i.second);

			front_images.insert(front_images.end(), fronts.begin(), fronts.end());
		}

		//将图片绑定到tile
		for(size_t i=0; i<tiles.size(); i++)
		{
			auto &t = tiles[i];

			if(t.back_index >= 0)
			{
				t.back_image = MirImage::LoadImage("Tiles", t.back_index);
			}

			if(t.middle_index >= 0)
			{
				t.middle_image = MirImage::LoadImage("SmTiles", t.middle_index);
			}

			if(t.front_index >= 0)
			{
				t.front_images.resize(t.front_count);

				for(int j=0; j<t.front_count; j++)
				{
					std::string name = "Objects" + GTString::ToString(t.info.obj + 1).str;
					t.front_images[j] = MirImage::LoadImage(name, t.front_index + j);
				}
			}
		}

		for(size_t i=0; i<tiles.size(); i++)
		{
			auto &t = tiles[i];

			if(t.back_image)
			{
				auto sprite = Sprite::Create(t.back_image->texture);
				auto renderer = GameObject::Create("tile")->AddComponent<SpriteRenderer>();
				auto mat = Material::Create("Mir Sprite");
				mat->SetTexture("ColorTable", MirImage::GetColorTable());
				renderer->SetSharedMaterial(mat);
				renderer->SetSprite(sprite);
				renderer->SetSortingLayer(0);
				renderer->SetSortingOrder(0);
				renderer->UpdateSprite();

				float x = (t.x + 1) * MapTileInfo::TILE_WIDTH * 0.01f;
				float y = (t.y + 1) * MapTileInfo::TILE_HEIGHT * 0.01f;

				renderer->GetTransform()->SetPosition(Vector3(x, -y, 0));
			}
			
			if(t.middle_image)
			{
				auto sprite = Sprite::Create(t.middle_image->texture);
				auto renderer = GameObject::Create("tile")->AddComponent<SpriteRenderer>();
				auto mat = Material::Create("Mir Sprite");
				mat->SetTexture("ColorTable", MirImage::GetColorTable());
				renderer->SetSharedMaterial(mat);
				renderer->SetSprite(sprite);
				renderer->SetSortingLayer(1);
				renderer->SetSortingOrder(0);
				renderer->UpdateSprite();

				float x = (t.x + 0.5f) * MapTileInfo::TILE_WIDTH * 0.01f;
				float y = (t.y + 0.5f) * MapTileInfo::TILE_HEIGHT * 0.01f;

				renderer->GetTransform()->SetPosition(Vector3(x, -y, 0));
			}

			if(!t.front_images.empty())
			{
				float x = ((t.x) * MapTileInfo::TILE_WIDTH + t.front_images[0]->data.w/2) * 0.01f;
				float y = ((t.y + 1) * MapTileInfo::TILE_HEIGHT - t.front_images[0]->data.h/2) * 0.01f;

				auto sprite = Sprite::Create(t.front_images[0]->texture);
				auto renderer = GameObject::Create("tile")->AddComponent<SpriteRenderer>();
				auto mat = Material::Create("Mir Sprite");
				mat->SetTexture("ColorTable", MirImage::GetColorTable());
				renderer->SetSharedMaterial(mat);
				renderer->SetSprite(sprite);
				renderer->SetSortingLayer(2);
				renderer->SetSortingOrder(t.y);
				renderer->UpdateSprite();

				if(t.front_images.size() > 1)
				{
					x += (t.front_images[0]->data.x) * 0.01f;
					y += (t.front_images[0]->data.y) * 0.01f;
				}

				renderer->GetTransform()->SetPosition(Vector3(x, -y, 0));
			}
		}
	}
};

#endif