#ifndef __Object_h__
#define __Object_h__

#include <string>
#include <memory>
#include <unordered_map>

namespace Galaxy3D
{
	class Object
	{
    public:
        static void ClearAllCachedObjects() {m_cached_objects.clear();}
        virtual ~Object() {}
		virtual void SetName(const std::string &value) {m_name = value;}
		std::string GetName() const {return m_name;}

	protected:
        static std::unordered_map<std::string, std::shared_ptr<Object>> m_cached_objects;
		std::string m_name;

        static bool ExistCachedObject(const std::string &key);
        static std::shared_ptr<Object> FindCachedObject(const std::string &key);
        static void CacheObject(const std::string &key, const std::shared_ptr<Object> &obj);

		Object();
	};
}

#endif