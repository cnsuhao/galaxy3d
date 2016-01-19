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
        friend class Renderer;

	public:
		static std::shared_ptr<GameObject> Create(const std::string &name);
		static void Destroy(std::shared_ptr<GameObject> &obj);
        static std::shared_ptr<GameObject> Instantiate(std::shared_ptr<GameObject> &source);
		virtual ~GameObject();
        virtual void DeepCopy(std::shared_ptr<Object> &source);
		template<class T> std::shared_ptr<T> AddComponent();
		template<class T> std::shared_ptr<T> GetComponent() const;
        template<class T> std::vector<std::shared_ptr<T>> GetComponents() const;
        template<class T> std::vector<std::shared_ptr<T>> GetComponentsInChildren() const;
        template<class T> std::shared_ptr<T> GetComponentInParent() const;
        std::shared_ptr<Component> GetComponentPtr(const Component *com) const;
		std::shared_ptr<Transform> GetTransform() const {return m_transform.lock();}
		bool IsActiveInHierarchy() const {return m_active_in_hierarchy;}
		bool IsActiveSelf()const {return m_active_self;}
		void SetActive(bool active);
		int GetLayer() const {return m_layer;}
		void SetLayer(int layer) {m_layer = layer;}
        void SetLayerRecursively(int layer);
        bool IsStatic() const {return m_static;}
        void SetStaticRecursively();

	private:
		std::list<std::shared_ptr<Component>> m_components;
		std::list<std::shared_ptr<Component>> m_components_new;
		bool m_active_in_hierarchy;
		bool m_active_self;
		int m_layer;
		bool m_deleted;
		std::weak_ptr<Transform> m_transform;
        bool m_static;

		GameObject(const std::string &name);
		void Delete();
		void Start();
		void Update();
		void LateUpdate();
		void AddComponent(const std::shared_ptr<Component> &com);
		void SetActiveInHierarchy(bool active);
        void CopyComponent(std::shared_ptr<Component> &com);
        void OnTranformChanged();
        void OnWillRenderObject(int material_index);
        void OnDidRenderObject(int material_index);
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

	template<class T> std::shared_ptr<T> GameObject::GetComponent() const
	{
		for(auto i : m_components)
		{
			auto t = std::dynamic_pointer_cast<T>(i);
			if(t && !t->m_deleted)
			{
				return t;
			}
		}

		for(auto i : m_components_new)
		{
			auto t = std::dynamic_pointer_cast<T>(i);
			if(t && !t->m_deleted)
			{
				return t;
			}
		}

		return std::shared_ptr<T>();
	}

    template<class T> std::vector<std::shared_ptr<T>> GameObject::GetComponents() const
    {
        std::vector<std::shared_ptr<T>> coms;

        for(auto i : m_components)
        {
            auto t = std::dynamic_pointer_cast<T>(i);
            if(t && !t->m_deleted)
            {
                coms.push_back(t);
            }
        }

        for(auto i : m_components_new)
        {
            auto t = std::dynamic_pointer_cast<T>(i);
            if(t && !t->m_deleted)
            {
                coms.push_back(t);
            }
        }

        return coms;
    }

    template<class T> std::vector<std::shared_ptr<T>> GameObject::GetComponentsInChildren() const
    {
        std::vector<std::shared_ptr<T>> coms;

        for(auto i : m_components)
        {
            auto t = std::dynamic_pointer_cast<T>(i);
            if(t && !t->m_deleted)
            {
                coms.push_back(t);
            }
        }

        for(auto i : m_components_new)
        {
            auto t = std::dynamic_pointer_cast<T>(i);
            if(t && !t->m_deleted)
            {
                coms.push_back(t);
            }
        }

        auto transform = GetTransform();
        int child_count = transform->GetChildCount();
        for(int i=0; i<child_count; i++)
        {
            auto child = transform->GetChild(i);
            auto child_coms = child->GetGameObject()->GetComponentsInChildren<T>();
            
            coms.insert(coms.end(), child_coms.begin(), child_coms.end());
        }

        return coms;
    }

    template<class T> std::shared_ptr<T> GameObject::GetComponentInParent() const
    {
        std::shared_ptr<T> com;

        auto parent = GetTransform()->GetParent().lock();

        while(parent)
        {
            com = parent->GetGameObject()->GetComponent<T>();

            if(com)
            {
                break;
            }
            else
            {
                parent = parent->GetParent().lock();
            }
        }

        return com;
    }
}

#endif