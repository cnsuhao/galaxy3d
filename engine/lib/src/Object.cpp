#include "Object.h"
#include "GameObject.h"
#include <memory>

namespace Galaxy3D
{
    std::unordered_map<std::string, std::shared_ptr<Object>> Object::m_cached_objects;

	Object::Object()
	{
	}

    void Object::DeepCopy(std::shared_ptr<Object> &source)
    {
        m_name = source->m_name;
    }

    bool Object::ExistCachedObject(const std::string &key)
    {
        return (bool) FindCachedObject(key);
    }

    std::shared_ptr<Object> Object::FindCachedObject(const std::string &key)
    {
        auto find = m_cached_objects.find(key);
        if(find != m_cached_objects.end())
        {
            return find->second;
        }

        return std::shared_ptr<Object>();
    }

    void Object::CacheObject(const std::string &key, const std::shared_ptr<Object> &obj)
    {
        if(!ExistCachedObject(key))
        {
            m_cached_objects[key] = obj;
        }
    }
}