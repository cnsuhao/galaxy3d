#include "MirMap.h"
#include "GameObject.h"
#include "GTString.h"
#include "Application.h"

static const float MAP_UPDATE_DELTA_TIME = 0.1f;
static const int MAP_WIDTH = 1;// 23;
static const int MAP_HEIGHT_TOP = 1;// 10;
static const int MAP_HEIGHT_BOTTOM = 1;// 34;
static float g_map_update_time;
std::unordered_map<int, MapTile> g_map_tiles;
std::string g_map;
int g_map_x;
int g_map_y;

void MirMap::Load(const std::string &map, int x, int y)
{
	int w = MAP_WIDTH;
	int h = MAP_HEIGHT_TOP + 1 + MAP_HEIGHT_BOTTOM;

	std::vector<int> coords;
	
	for(int j=0; j<h; j++)
	{
		for(int i=0; i<w; i++)
		{
			coords.push_back(((i + x - w/2) << 16) | (j + y - MAP_HEIGHT_TOP));
		}
	}

	g_map = map;
	g_map_x = x;
	g_map_y = y;

	LoadTiles(Application::GetDataPath() + "/Assets/mir/Map/" + map + ".map", coords, g_map_tiles);
}

void MirMap::Unload()
{
	for(auto &i : g_map_tiles)
	{
		auto &t = i.second;

		if(t.back_sprite)
		{
			GameObject::Destroy(t.back_sprite->GetGameObject());
		}
		if(t.middle_sprite)
		{
			GameObject::Destroy(t.middle_sprite->GetGameObject());
		}
		if(t.front_sprite)
		{
			GameObject::Destroy(t.front_sprite->GetGameObject());
		}
	}

	g_map_tiles.clear();
}

void MirMap::Update()
{
	float now = GTTime::GetRealTimeSinceStartup();
	if(now - g_map_update_time > MAP_UPDATE_DELTA_TIME)
	{
		g_map_update_time = now;

		std::vector<int> deletes;

		for(auto &i : g_map_tiles)
		{
			auto &t = i.second;

			if(!t.front_images.empty())
			{
				t.front_frame++;
				if(t.front_frame >= (int) t.front_sprites.size())
				{
					t.front_frame = 0;
				}

				t.front_sprite->SetSprite(t.front_sprites[t.front_frame]);
			}

			if(	t.x < g_map_x - MAP_WIDTH / 2 ||
				t.x > g_map_x + MAP_WIDTH / 2 ||
				t.y < g_map_y - MAP_HEIGHT_TOP ||
				t.y > g_map_y + MAP_HEIGHT_BOTTOM
				)
			{
				if(t.back_sprite)
				{
					GameObject::Destroy(t.back_sprite->GetGameObject());
				}
				if(t.middle_sprite)
				{
					GameObject::Destroy(t.middle_sprite->GetGameObject());
				}
				if(t.front_sprite)
				{
					GameObject::Destroy(t.front_sprite->GetGameObject());
				}

				deletes.push_back(i.first);
			}
		}

		for(auto i : deletes)
		{
			g_map_tiles.erase(i);
		}
	}
}

void MirMap::Scroll(int dir_x, int dir_y, int dis)
{
	int w = MAP_WIDTH;
	int h = MAP_HEIGHT_TOP + 1 + MAP_HEIGHT_BOTTOM;

	std::vector<int> coords;

	int d = 0;
	while(d++ < dis)
	{
		if(dir_x != 0)
		{
			int x = g_map_x + (w / 2 + 1) * dir_x;
			int y_0;
			int y_1;

			y_0 = g_map_y - MAP_HEIGHT_TOP + dir_y;
			y_1 = g_map_y + MAP_HEIGHT_BOTTOM + dir_y;

			for(int i=y_0; i<=y_1; i++)
			{
				coords.push_back((x << 16) | i);
			}
		}

		if(dir_y != 0)
		{
			int y;
			if(dir_y > 0)
			{
				y = g_map_y + MAP_HEIGHT_BOTTOM + 1;
			}
			else
			{
				y = g_map_y - MAP_HEIGHT_TOP - 1;
			}

			int x_0;
			int x_1;

			x_0 = g_map_x - w / 2 + dir_x;
			x_1 = g_map_x + w / 2 + dir_x;

			if(dir_x > 0)
			{
				x_1--;
			}
			else if(dir_x < 0)
			{
				x_0++;
			}

			for(int i=x_0; i<=x_1; i++)
			{
				coords.push_back((i << 16) | y);
			}
		}

		g_map_x += dir_x;
		g_map_y += dir_y;
	}

	LoadTiles(Application::GetDataPath() + "/Assets/mir/Map/" + g_map + ".map", coords, g_map_tiles);
}

bool MirMap::CanMove(int dir_x, int dir_y, int dis)
{
	bool can_move = true;
	for(int i=1; i<=dis; i++)
	{
		int x = g_map_x + dir_x * i;
		int y = g_map_y + dir_y * i;

		can_move = can_move && g_map_tiles[(x << 16) | y].can_move;
	}

	return can_move;
}

void MirMap::LoadTiles(const std::string &map_file, const std::vector<int> &coords, std::unordered_map<int, MapTile> &tiles)
{
	std::unordered_map<int, MapTile> tiles_new;
	std::vector<int> back_indices(coords.size());
	std::vector<int> middle_indices(coords.size());
	std::unordered_map<int, std::vector<int>> front_indices;

	FILE *f = fopen(map_file.c_str(), "rb");
	if(f != nullptr)
	{
		MapHeader header;
		fread(&header, MapHeader::STRUCT_SIZE, 1, f);

		for(size_t i=0; i<coords.size(); i++)
		{
			if(tiles.count(coords[i]) > 0)
			{
				continue;
			}

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
            tile.can_move = ((tile.info.back & 0x8000) == 0) && ((tile.info.front & 0x8000) == 0);
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

			tiles_new[coords[i]] = tile;
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
	for(auto &i : tiles_new)
	{
		auto &t = i.second;

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

	for(auto &i : tiles_new)
	{
		auto &t = i.second;

		if(t.back_image)
		{
			auto sprite = Sprite::Create(t.back_image->texture);
			auto renderer = GameObject::Create("tile")->AddComponent<SpriteRenderer>();
			auto mat = Material::Create("UI/Mir Map");
			mat->SetTexture("ColorTable", MirImage::GetColorTable());
			renderer->SetSharedMaterial(mat);
			renderer->SetSprite(sprite);
			renderer->SetSortingOrder(0, 0);
			renderer->UpdateSprite();

			float x = (t.x + 1) * TILE_WIDTH * MIR_PIXEL_TO_UNIT;
			float y = (t.y + 1) * TILE_HEIGHT * MIR_PIXEL_TO_UNIT;

			renderer->GetTransform()->SetPosition(Vector3(x, -y, 0));

			t.back_sprite = renderer;
		}
			
		if(t.middle_image)
		{
			auto sprite = Sprite::Create(t.middle_image->texture);
			auto renderer = GameObject::Create("tile")->AddComponent<SpriteRenderer>();
			auto mat = Material::Create("UI/Mir Map");
			mat->SetTexture("ColorTable", MirImage::GetColorTable());
			renderer->SetSharedMaterial(mat);
			renderer->SetSprite(sprite);
			renderer->SetSortingOrder(1, 0);
			renderer->UpdateSprite();

			float x = (t.x + 0.5f) * TILE_WIDTH * MIR_PIXEL_TO_UNIT;
			float y = (t.y + 0.5f) * TILE_HEIGHT * MIR_PIXEL_TO_UNIT;

			renderer->GetTransform()->SetPosition(Vector3(x, -y, 0));

			t.middle_sprite = renderer;
		}

		if(!t.front_images.empty())
		{
			t.front_sprites.resize(t.front_images.size());
			for(size_t j=0; j<t.front_images.size(); j++)
			{
				t.front_sprites[j] = Sprite::Create(t.front_images[j]->texture);
			}
			t.front_frame = 0;

			float x = ((t.x) * TILE_WIDTH + t.front_images[0]->data.w/2) * MIR_PIXEL_TO_UNIT;
			float y = ((t.y + 1) * TILE_HEIGHT - t.front_images[0]->data.h/2) * MIR_PIXEL_TO_UNIT;

			auto renderer = GameObject::Create("tile")->AddComponent<SpriteRenderer>();
			auto mat = Material::Create("UI/Mir Sprite");
			mat->SetTexture("ColorTable", MirImage::GetColorTable());
			renderer->SetSharedMaterial(mat);
			renderer->SetSprite(t.front_sprites[t.front_frame]);
			renderer->SetSortingOrder(2, (t.y - 1) * 10);
			renderer->UpdateSprite();

			if(t.front_images.size() > 1)
			{
				x += (t.front_images[0]->data.x) * MIR_PIXEL_TO_UNIT;
				y += (t.front_images[0]->data.y) * MIR_PIXEL_TO_UNIT;

				mat->SetShader(Shader::Find("UI/Mir Effect"));
			}

			renderer->GetTransform()->SetPosition(Vector3(x, -y, 0));

			t.front_sprite = renderer;
		}
	}

	tiles.insert(tiles_new.begin(), tiles_new.end());

	g_map_update_time = GTTime::GetRealTimeSinceStartup();
}