#ifndef __Label_h__
#define __Label_h__

#include "Object.h"

namespace Galaxy3D
{
	class Label : public Object
	{
	public:
		static void InitFontLib();
		static void DoneFontLib();
		static std::shared_ptr<Label> Create(const std::string &text, bool rich = false);

		void SetText(const std::string &text);
		std::string GetText() const {return m_text;}

	protected:
		std::string m_text;
		bool m_rich;

		Label();
	};
}

#endif