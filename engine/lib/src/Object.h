#ifndef __Object_h__
#define __Object_h__

#include <string>
#include <memory>

namespace Galaxy3D
{
	class Object
	{
    public:
        virtual ~Object() {}
		virtual void SetName(const std::string &value) {m_name = value;}
		std::string GetName() const {return m_name;}

	protected:
		std::string m_name;

		Object();
	};
}

#endif