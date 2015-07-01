#include "GTStringUTF32.h"
#include "GTString.h"

namespace Galaxy3D
{
	GTStringUTF32::GTStringUTF32(const std::string &utf8)
	{
		if(!utf8.empty())
		{
			std::vector<char> utf32 = UTF8toUTF32(utf8);
			m_data.resize((utf32.size() / 4) - 1);
			memcpy(&m_data[0], &utf32[0], m_data.size() * 4);
		}
	}

	const int &GTStringUTF32::operator[](int index) const
	{
		return m_data[index];
	}

	int &GTStringUTF32::operator[](int index)
	{
		return m_data[index];
	}

	bool GTStringUTF32::operator==(const GTStringUTF32 &str) const
	{
		if(m_data.size() == str.m_data.size() && !m_data.empty())
		{
			return memcmp(&m_data[0], &str.m_data[0], m_data.size() * 4) == 0;
		}

		return false;
	}

	bool GTStringUTF32::operator==(const std::string &str) const
	{
		return *this == GTStringUTF32(str);
	}

	GTStringUTF32 &GTStringUTF32::operator +=(int c)
	{
		m_data.push_back(c);
		return *this;
	}

	GTStringUTF32 &GTStringUTF32::operator +=(const std::string &s)
	{
		GTStringUTF32 append(s);

		*this = *this + append;

		return *this;
	}

	GTStringUTF32 GTStringUTF32::operator +(const GTStringUTF32 &s) const
	{
		GTStringUTF32 append = *this;

		if(!s.Empty())
		{
			int old_size = append.m_data.size();
			append.m_data.resize(old_size + s.Size());
			memcpy(&append.m_data[old_size], &s.m_data[0], s.Size() * 4);
		}

		return append;
	}

	int GTStringUTF32::Size() const
	{
		return m_data.size();
	}

	bool GTStringUTF32::Empty() const
	{
		return m_data.empty();
	}

	GTStringUTF32::Iterator GTStringUTF32::Begin()
	{
		return m_data.begin();
	}

	GTStringUTF32::Iterator GTStringUTF32::End()
	{
		return m_data.end();
	}

	GTStringUTF32::Iterator GTStringUTF32::Erase(int index, int count)
	{
		return m_data.erase(m_data.begin() + index, m_data.begin() + index + count);
	}

	GTStringUTF32 GTStringUTF32::Substr(int start, int count) const
	{
		GTStringUTF32 str("");

		if(count < 0)
		{
			count = m_data.size() - start;
		}

		if(count > 0)
		{
			str.m_data.resize(count);
			memcpy(&str.m_data[0], &m_data[start], count * 4);
		}

		return str;
	}

	int GTStringUTF32::Find(int c, int start) const
	{
		for(int i=start; i<(int) m_data.size() && i>=0; i++)
		{
			if(m_data[i] == c)
			{
				return i;
			}
		}

		return -1;
	}

	bool GTStringUTF32::Contains(const GTStringUTF32 &value) const
	{
		int less = this->Size() - value.Size();
		if (less >= 0 && !this->Empty() && !value.Empty())
		{
			for (int i = 0; i<=less; i++)
			{
				if (memcmp(&m_data[i], &value.m_data[0], value.m_data.size() * 4) == 0)
				{
					return true;
				}
			}
		}

		return false;
	}

	bool GTStringUTF32::Contains(const std::string &value) const
	{
		return Contains(GTStringUTF32(value));
	}

	GTStringUTF32 GTStringUTF32::ToUpper() const
	{
		GTStringUTF32 str(*this);

		if(m_data.size() > 0)
		{
			std::vector<char> buffer((m_data.size() + 1) * 4, 0);
			memcpy(&buffer[0], &m_data[0], m_data.size() * 4);
			str = GTStringUTF32(GTString(UTF32toUTF8(buffer)).ToUpper().str);
		}

		return str;
	}

	std::string GTStringUTF32::Utf8() const
	{
		std::string str;

		if(m_data.size() > 0)
		{
			std::vector<char> buffer((m_data.size() + 1) * 4, 0);
			memcpy(&buffer[0], &m_data[0], m_data.size() * 4);
			str = UTF32toUTF8(buffer);
		}

		return str;
	}

	int GTStringUTF32::UTF8toUTF32(const char *utf8, unsigned int &utf32)
	{
		int byte_count = 0;

		for(int i=0; i<8; i++)
		{
			unsigned char c = utf8[0];

			if(((c << i) & 0x80) == 0)
			{
				if(i == 0)
				{
					byte_count = 1;
				}
				else
				{
					byte_count = i;
				}
				break;
			}
		}

		if(	byte_count >= 1 &&
			byte_count <= 6)
		{
			unsigned int code = 0;

			for(int j=0; j<byte_count; j++)
			{
				unsigned int c = utf8[j];
				unsigned char part;

				if(j == 0)
				{
					part = (c << (byte_count + 24)) >> (byte_count + 24);
				}
				else
				{
					part = c & 0x3f;
				}

				code = (code << 6) | part;
			}

			utf32 = code;
			return byte_count;
		}
		else
		{
			return 0;
		}
	}

	std::vector<char> GTStringUTF32::UTF8toUTF32(const std::string &src)
	{
		std::vector<char> buffer;
		int size = src.size();

		for(int i=0; i<size; i++)
		{
			unsigned int utf32 = 0;
			int byte_count = UTF8toUTF32(&src[i], utf32);

			if(byte_count > 0)
			{
				buffer.push_back(utf32 & 0xff);
				buffer.push_back((utf32 & 0xff00) >> 8);
				buffer.push_back((utf32 & 0xff0000) >> 16);
				buffer.push_back((utf32 & 0xff000000) >> 24);

				i += byte_count - 1;
			}
			else
			{
				break;
			}
		}

		buffer.push_back(0);
		buffer.push_back(0);
		buffer.push_back(0);
		buffer.push_back(0);

		return buffer;
	}

	std::vector<char> GTStringUTF32::UTF32toUTF8(unsigned int utf32)
	{
		std::vector<char> buffer;
		int byte_count = 0;

		if(utf32 <= 0x7f)
		{
			byte_count = 1;
		}
		else if(utf32 <= 0x7ff)
		{
			byte_count = 2;
		}
		else if(utf32 <= 0xffff)
		{
			byte_count = 3;
		}
		else if(utf32 <= 0x1fffff)
		{
			byte_count = 4;
		}
		else if(utf32 <= 0x3ffffff)
		{
			byte_count = 5;
		}
		else if(utf32 <= 0x7fffffff)
		{
			byte_count = 6;
		}

		std::vector<char> bytes;
		for(int j=0; j<byte_count-1; j++)
		{
			bytes.push_back((utf32 & 0x3f) | 0x80);
			utf32 >>= 6;
		}

		if(byte_count > 1)
		{
			bytes.push_back(utf32 | (0xffffff80 >> (byte_count - 1)));
		}
		else
		{
			bytes.push_back(utf32);
		}

		for(int j=0; j<byte_count; j++)
		{
			buffer.push_back(bytes[byte_count-1-j]);
		}

		return buffer;
	}

	std::string GTStringUTF32::UTF32toUTF8(const std::vector<char> &src)
	{
		std::vector<char> buffer;
		int word_count = src.size()/4;

		for(int i=0; i<word_count; i++)
		{
			unsigned int word = *(unsigned int *) &src[i*4];

			std::vector<char> utf8 = UTF32toUTF8(word);
			buffer.insert(buffer.end(), utf8.begin(), utf8.end());
		}

		return std::string(&buffer[0]);
	}
}