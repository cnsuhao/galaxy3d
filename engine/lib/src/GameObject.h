#ifndef __GameObject_h__
#define __GameObject_h__

#include "Object.h"
#include "Component.h"
#include "Transform.h"
#include <list>
#include <memory>

namespace Galaxy3D
{
	class GameObject : public Object
	{
		friend class World;
		friend class Transform;

	public:
		static std::shared_ptr<GameObject> Create(const std::string &name);
		static void Destroy(std::weak_ptr<GameObject> &obj);
		virtual ~GameObject();
		template<class T> std::shared_ptr<T> AddComponent();
		template<class T> std::shared_ptr<T> GetComponent();
		std::shared_ptr<Transform> GetTransform() const {return m_transform.lock();}
		bool IsActiveInHierarchy() const {return m_active_in_hierarchy;}
		bool IsActiveSelf()const {return m_active_self;}
		void SetActive(bool active);
		int GetLayer() const {return m_layer;}
		void SetLayer(int layer) {m_layer = layer;}

	private:
		std::list<std::shared_ptr<Component>> m_components;
		std::list<std::shared_ptr<Component>> m_components_new;
		bool m_active_in_hierarchy;
		bool m_active_self;
		int m_layer;
		bool m_deleted;
		std::weak_ptr<Transform> m_transform;

		GameObject(const std::string &name);
		void Delete();
		void Start();
		void Update();
		void LateUpdate();
		void AddComponent(const std::shared_ptr<Component> &com);
		void SetActiveInHierarchy(bool active);
	};

	template<class T> std::shared_ptr<T> GameObject::AddComponent()
	{
		if(m_deleted)
		{
			return std::shared_ptr<T>();
		}

		auto t = std::make_shared<T>();
		AddComponent(std::dynamic_pointer_cast<Component>(t));

		return t;
	}

	template<class T> std::shared_ptr<T> GameObject::GetComponent()
	{
		for(auto i : m_components)
		{
			auto t = std::dynamic_pointer_cast<T>(i);
			if(t)
			{
				return t;
			}
		}

		for(auto i : m_components_new)
		{
			auto t = std::dynamic_pointer_cast<T>(i);
			if(t)
			{
				return t;
			}
		}

		return std::shared_ptr<T>();
	}
}

#endif