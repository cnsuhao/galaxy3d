#ifndef __GTStringUTF32_h__
#define __GTStringUTF32_h__

#include <string>
#include <vector>

namespace Galaxy3D
{
	class GTStringUTF32
	{
	public:
		typedef std::vector<int>::iterator Iterator;

		static int UTF8toUTF32(const char *utf8, unsigned int &utf32);
		static std::vector<char> UTF8toUTF32(const std::string &src);
		static std::vector<char> UTF32toUTF8(unsigned int utf32);
		static std::string UTF32toUTF8(const std::vector<char> &src);

		GTStringUTF32() {}
		explicit GTStringUTF32(const std::string &utf8);
		const int &operator[](int index) const;
		int &operator[](int index);
		bool operator==(const GTStringUTF32 &str) const;
		bool operator!=(const GTStringUTF32 &str) const { return !(*this == str); }
		bool operator==(const std::string &str) const;
		GTStringUTF32 &operator +=(int c);
		GTStringUTF32 &operator +=(const std::string &s);
		GTStringUTF32 operator +(const GTStringUTF32 &s) const;
		int Size() const;
		bool Empty() const;
		Iterator Begin();
		Iterator End();
		Iterator Erase(int index, int count = 1);
		GTStringUTF32 Substr(int start, int count = -1) const;
		int Find(int c, int start) const;
		GTStringUTF32 ToUpper() const;
		std::string Utf8() const;
		bool Contains(const GTStringUTF32 &value) const;
		bool Contains(const std::string &value) const;

	private:
		std::vector<int> m_data;
	};
};

#endif