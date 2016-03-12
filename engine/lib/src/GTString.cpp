#include "GTString.h"
#include <algorithm>
#include <stdarg.h> 

namespace Galaxy3D
{
	GTString GTString::Replace(const std::string &old, const std::string &replace) const
	{
		GTString ret(str);

		size_t find;
		while((find = ret.str.find(old)) != std::string::npos)
		{
			ret.str.replace(find, old.length(), replace);
		}

		return ret;
	}

	std::vector<GTString> GTString::Split(const std::string &split, bool remove_empty) const
	{
		std::vector<GTString> ret;

		size_t pos = 0;
		size_t find;
		while((find = str.find(split, pos)) != std::string::npos)
		{
			auto s = str.substr(pos, find - pos);
			if(!s.empty() || !remove_empty)
			{
				ret.push_back(s);
			}

			pos = find + 1;
		}

		auto s = str.substr(pos);
		if(!s.empty() || !remove_empty)
		{
			ret.push_back(s);
		}

		return ret;
	}

	static char char_to_upper(char c)
	{
		if(c >= 'a' && c <= 'z')
		{
			c += 'A' - 'a';
		}

		return c;
	}

	GTString GTString::ToUpper() const
	{
		GTString upper(str);

		std::transform(str.begin(), str.end(), upper.str.begin(), char_to_upper);

		return upper;
	}

	static char char_to_lower(char c)
	{
		if(c >= 'A' && c <= 'Z')
		{
			c -= 'A' - 'a';
		}

		return c;
	}

	GTString GTString::ToLower() const
	{
		GTString lower(str);

		std::transform(str.begin(), str.end(), lower.str.begin(), char_to_lower);

		return lower;
	}

	bool GTString::StartsWith(const std::string &value) const
	{
		return str.find(value) == 0;
	}

	GTString GTString::TrimSpace() const
	{
		if(str.empty())
		{
			return str;
		}

		int begin = 0;
		int end = (int) str.size() - 1;

		while(begin < (int) str.size() && str[begin] == ' ')
		{
			begin++;
		}

		while(end >= 0 && str[end] == ' ')
		{
			end--;
		}

		return str.substr(begin, end - begin + 1);
	}

	GTString GTString::Format(const char *format, ...)
	{
		const static int BUFFER_SIZE = 1024;

		va_list plist;
		va_start(plist, format);
	
		char buffer[BUFFER_SIZE];
		vsnprintf(buffer, BUFFER_SIZE, format, plist);

		va_end(plist);

		return GTString(buffer);
	}
}