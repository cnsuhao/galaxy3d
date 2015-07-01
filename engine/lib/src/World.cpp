#include "World.h"
#include "GTTime.h"

namespace Galaxy3D
{
	std::unordered_map<GameObject *, std::shared_ptr<GameObject>> World::m_gameobjects;
	std::unordered_map<GameObject *, std::shared_ptr<GameObject>> World::m_gameobjects_new;

	void World::AddGameObject(const std::shared_ptr<GameObject> &obj)
	{
		m_gameobjects_new[obj.get()] = obj;
	}

	std::weak_ptr<GameObject> World::FindGameObject(GameObject *obj)
	{
		auto find = m_gameobjects.find(obj);
		if(find != m_gameobjects.end())
		{
			return find->second;
		}

		find = m_gameobjects_new.find(obj);
		if(find != m_gameobjects_new.end())
		{
			return find->second;
		}

		return std::weak_ptr<GameObject>();
	}

	void World::Update()
	{
		float time = GTTime::GetRealTimeSinceStartup();

		m_gameobjects.insert(m_gameobjects_new.begin(), m_gameobjects_new.end());
		m_gameobjects_new.clear();

		for(auto i : m_gameobjects)
		{
			auto &obj = i.second;

			if(!obj->m_deleted)
			{
				i.second->Update();
			}
		}

		for(auto i : m_gameobjects)
		{
			auto &obj = i.second;

			if(!obj->m_deleted)
			{
				i.second->LateUpdate();
			}
		}

		//pick deleted
		std::list<GameObject *> deleted;
		for(auto i : m_gameobjects)
		{
			auto &obj = i.second;

			if(obj->m_deleted)
			{
				deleted.push_back(obj.get());
			}
		}

		//delete
		for(auto i : deleted)
		{
			m_gameobjects.erase(i);
		}

		GTTime::m_update_time = GTTime::GetRealTimeSinceStartup() - time;
	}

	void World::Destroy()
	{
		m_gameobjects_new.clear();
		m_gameobjects.clear();
	}
}