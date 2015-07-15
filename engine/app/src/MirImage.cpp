#include "MirImage.h"
#include "Application.h"
#include "Debug.h"
#include "zlib.h"

std::unordered_map<std::string, std::unordered_map<int, std::weak_ptr<Texture2D>>> MirImage::m_cache;

std::vector<std::shared_ptr<Texture2D>> MirImage::LoadImages(const std::string &name, const std::vector<int> &indices)
{
	std::vector<std::shared_ptr<Texture2D>> textures(indices.size());

	std::unordered_map<int, std::weak_ptr<Texture2D>> *set = nullptr;
	auto find = m_cache.find(name);
	if(find != m_cache.end())
	{
		set = &find->second;
	}
	else
	{
		m_cache[name] = std::unordered_map<int, std::weak_ptr<Texture2D>>();
		set = &m_cache[name];
	}

	WZX wzx;
	std::string wzx_name = Application::GetDataPath() + "/Assets/mir/data/" + name + ".wzx";
	FILE *f_wzx = fopen(wzx_name.c_str(), "rb");
	if(f_wzx != nullptr)
	{
		fread(&wzx, WZX::STRUCT_SIZE, 1, f_wzx);
	}

	WZL wzl;
	std::string wzl_name = Application::GetDataPath() + "/Assets/mir/data/" + name + ".wzl";
	FILE *f_wzl = fopen(wzl_name.c_str(), "rb");
	if(f_wzl != nullptr)
	{
		fread(&wzl, WZL::INFO_SIZE, 1, f_wzl);
	}

	for(size_t i=0; i<indices.size(); i++)
	{
		int index = indices[i];

		auto find_image = set->find(index);
		if(find_image != set->end())
		{
			auto &tex = find_image->second;

			if(!tex.expired())
			{
				textures[i] = tex.lock();
				continue;
			}
			else
			{
				set->erase(index);
			}
		}

		if(f_wzx != nullptr && f_wzl != nullptr)
		{
			if(index >= 0 && index < wzx.image_count)
			{
				fseek(f_wzx, WZX::STRUCT_SIZE + index * 4, SEEK_SET);
                int offset;
				fread(&offset, 4, 1, f_wzx);

				textures[i] = LoadImage(f_wzl, offset);
				(*set)[index] = textures[i];
			}
		}
	}

	if(f_wzl != nullptr)
	{
		fclose(f_wzl);
		f_wzl = nullptr;
	}
	if(f_wzx != nullptr)
	{
		fclose(f_wzx);
		f_wzx = nullptr;
	}

	return textures;
}

std::shared_ptr<Texture2D> MirImage::LoadImage(FILE *f, int offset)
{
	std::shared_ptr<Texture2D> tex;

	if(f != nullptr)
	{
		MirImageData im;

		fseek(f, offset, SEEK_SET);
		fread(&im, MirImageData::INFO_SIZE, 1, f);

		int size = 0;
		if(im.format == MirImageFormat::Index8)
		{
			size = im.w * im.h;
			tex = Texture2D::Create(im.w, im.h, TextureFormat::Alpha8, FilterMode::Point, TextureWrapMode::Clamp);
		}
		else if(im.format == MirImageFormat::RGB565)
		{
			size = im.w * im.h * 2;
			tex = Texture2D::Create(im.w, im.h, TextureFormat::RGB565, FilterMode::Point, TextureWrapMode::Clamp);
		}

		if(im.zip_size > 0)
		{
			if(size > 0)
			{
				im.bmp.resize(size);

				char *buffer = (char *) malloc(im.zip_size);
				fread(buffer, im.zip_size, 1, f);
				
				uLongf dest_size = size;
				int err = uncompress((Bytef *) &im.bmp[0], &dest_size, (Bytef *) buffer, im.zip_size);
				if(err != Z_OK)
				{
					Debug::Log("unzip mir image data error");
				}

				free(buffer);

				tex->SetPixels(&im.bmp[0]);
			}
		}
		else
		{
			if(size > 0)
			{
				im.bmp.resize(size);

				fread(&im.bmp[0], size, 1, f);

				tex->SetPixels(&im.bmp[0]);
			}
		}
	}

	return tex;
}