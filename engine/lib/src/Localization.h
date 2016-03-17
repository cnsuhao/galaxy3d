#ifndef __Localization_h__
#define __Localization_h__

#include <string>
#include <unordered_map>

namespace Galaxy3D
{
	class Localization
	{
	public:
		static void LoadStrings(const std::string &file);
		static std::string GetString(const std::string &key);
		static std::string GetLanguage();

	private:
		static std::unordered_map<std::string, std::string> m_map;
	};
}

#endif