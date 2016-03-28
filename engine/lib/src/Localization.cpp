#include "Localization.h"
#include "GTFile.h"
#include "json.h"

#ifdef WINPHONE
#include <windows.h>
#endif

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

					m_map.clear();
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

	std::string Localization::GetLanguage()
	{
		std::string lan;

#ifdef WINPHONE
		auto lans = Windows::System::UserProfile::GlobalizationPreferences::Languages;
		if(lans->Size > 0)
		{
			auto ref_str = lans->GetAt(0);
			char buffer[MAX_PATH] = {0};
			WideCharToMultiByte(CP_ACP, 0, ref_str->Data(), ref_str->Length(), buffer, MAX_PATH, NULL, NULL);
			lan = buffer;
		}
#endif

		return lan;
	}
}