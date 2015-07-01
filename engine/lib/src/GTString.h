#ifndef __GTString_h__
#define __GTString_h__

#include <string>
#include <vector>
#include <sstream>

namespace Galaxy3D
{
	class GTString
	{
	public:
		template<typename T>
		static GTString ToString(T value)
		{
			std::stringstream ss;
			ss << value;
		
			return GTString(ss.str());
		}
		static GTString Format(const char *format, ...);

		GTString(const std::string &str):str(str){}
		GTString Replace(const std::string &old, const std::string &replace) const;
		std::vector<GTString> Split(const std::string &split, bool remove_empty = false) const;
		GTString ToUpper() const;
		GTString ToLower() const;
		bool StartsWith(const std::string &value) const;
		GTString TrimSpace() const;

		std::string str;
	};
}

#endif