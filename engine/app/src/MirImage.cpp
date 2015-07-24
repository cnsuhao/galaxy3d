#include "MirImage.h"
#include "Application.h"
#include "GTFile.h"
#include "Debug.h"
#include "zlib.h"

std::unordered_map<std::string, std::unordered_map<int, std::weak_ptr<MirImage>>> MirImage::m_cache;
std::shared_ptr<Texture2D> MirImage::m_color_table;

std::shared_ptr<Texture2D> MirImage::GetColorTable()
{
	if(!m_color_table)
	{
		m_color_table = Texture2D::Create(256, 1, TextureFormat::RGBA32, FilterMode::Point, TextureWrapMode::Clamp);
		auto bytes = GTFile::ReadAllBytes(Application::GetDataPath() + "/Assets/mir/ColorTable.bytes");
		for(int i=0; i<1024; i+=4)
		{
			char b = bytes[i+0];
			char g = bytes[i+1];
			char r = bytes[i+2];
			char a = bytes[i+3];

			bytes[i+0] = r;
			bytes[i+1] = g;
			bytes[i+2] = b;
			bytes[i+3] = -1;
		}
		bytes[3] = 0;

		m_color_table->SetPixels(&bytes[0]);
		m_color_table->Apply();
	}

	return m_color_table;
}

std::shared_ptr<MirImage> MirImage::LoadImage(const std::string &name, int index)
{
	std::shared_ptr<MirImage> image;

	auto find = m_cache.find(name);
	if(find != m_cache.end())
	{
		auto set = &find->second;
		auto find_image = set->find(index);

		if(find_image != set->end())
		{
			auto &img = find_image->second;

			if(!img.expired())
			{
				image = img.lock();
			}
			else
			{
				set->erase(index);
			}
		}
	}

	return image;
}

std::vector<std::shared_ptr<MirImage>> MirImage::LoadImages(const std::string &name, const std::vector<int> &indices)
{
	std::vector<std::shared_ptr<MirImage>> images(indices.size());

	std::unordered_map<int, std::weak_ptr<MirImage>> *set = nullptr;
	auto find = m_cache.find(name);
	if(find != m_cache.end())
	{
		set = &find->second;
	}
	else
	{
		m_cache[name] = std::unordered_map<int, std::weak_ptr<MirImage>>();
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
			auto &img = find_image->second;

			if(!img.expired())
			{
				images[i] = img.lock();
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

				images[i] = LoadImage(f_wzl, offset, name, index);
				(*set)[index] = images[i];
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

	return images;
}

std::shared_ptr<MirImage> MirImage::LoadImage(FILE *f, int offset, const std::string &name, int index)
{
	std::shared_ptr<MirImage> image;

	if(f != nullptr)
	{
		std::shared_ptr<Texture2D> tex;
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
		else
		{
			Debug::Log("unknow MirImageFormat:%x %s %d", im.format, name.c_str(), index);
			return image;
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
				im.bmp.clear();
				tex->Apply();
			}
		}
		else
		{
			if(size > 0)
			{
				im.bmp.resize(size);

				fread(&im.bmp[0], size, 1, f);

				tex->SetPixels(&im.bmp[0]);
				im.bmp.clear();
				tex->Apply();
			}
		}

		image = std::shared_ptr<MirImage>(new MirImage());
		image->data = im;
		image->texture = tex;
	}

	return image;
}