#include "Localization.h"
#include "GTFile.h"
#include "json.h"

namespace Galaxy3D
{
	std::unordered_map<std::string, std::string> Localization::m_map;

	void Localization::LoadStrings(const std::string &file)
	{
		if(GTFile::Exist(file))
		{
			std::string str;
            GTFile::ReadAllText(file, str);

            if(!str.empty())
            {
                Json::Reader reader;
                Json::Value root;

                if(reader.parse(str, root))
				{
					auto keys = root.getMemberNames();

					for(auto &i : keys)
					{
						m_map[i] = root[i].asString();
					}
				}
			}
		}
	}

	std::string Localization::GetString(const std::string &key)
	{
		auto find = m_map.find(key);
		if(find != m_map.end())
		{
			return find->second;
		}

		return key;
	}
}