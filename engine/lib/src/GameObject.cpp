#include "GameObject.h"
#include "Layer.h"
#include "World.h"

namespace Galaxy3D
{
	std::shared_ptr<GameObject> GameObject::Create(const std::string &name)
	{
		auto obj = std::shared_ptr<GameObject>(new GameObject(name));
		World::AddGameObject(obj);

		auto transform = std::shared_ptr<Transform>(new Transform());
		obj->m_transform = transform;
		obj->AddComponent(transform);

		return obj;
	}

	void GameObject::Destroy(std::shared_ptr<GameObject> &obj)
	{
		if(obj)
		{
			obj->Delete();
			obj->SetActive(false);
			obj.reset();
		}
	}

    void GameObject::DeepCopy(std::shared_ptr<Object> &source)
    {
        auto src_obj = std::dynamic_pointer_cast<GameObject>(source);

        for(auto i=m_components.begin(); i!=m_components.end(); i++)
        {
            
        }

        for(auto i=m_components_new.begin(); i!=m_components_new.end(); i++)
        {
            
        }

        m_active_in_hierarchy = src_obj->m_active_in_hierarchy;
        m_active_self = src_obj->m_active_self;
        m_layer = src_obj->m_layer;
        m_deleted = src_obj->m_deleted;
        m_static = src_obj->m_static;
    }

    std::shared_ptr<GameObject> GameObject::Instantiate(std::shared_ptr<GameObject> &source)
    {
        std::shared_ptr<GameObject> clone = GameObject::Create(source->GetName());
        
        clone->DeepCopy(std::dynamic_pointer_cast<Object>(source));

        return clone;
    }

	GameObject::GameObject(const std::string &name):
		m_active_in_hierarchy(true),
		m_active_self(true),
		m_layer(Layer::Default),
		m_deleted(false),
        m_static(false)
	{
		SetName(name);
	}

	GameObject::~GameObject()
 	{
	}

	void GameObject::Delete()
	{
		if(!m_deleted)
		{
			m_deleted = true;
		}

		auto transform = GetTransform();
		int child_count = transform->GetChildCount();
		for(int i=0; i<child_count; i++)
		{
			transform->GetChild(i)->GetGameObject()->Delete();
		}
	}

	void GameObject::Start()
	{
		std::list<std::shared_ptr<Component>> starts(m_components);
		do
		{
			for(auto &i : starts)
			{
				if(!IsActiveInHierarchy())
				{
					break;
				}

				if(i->IsEnable() && !i->m_started)
				{
					i->m_started = true;
					i->Start();
				}
			}
			starts.clear();

			starts = m_components_new;
			m_components.insert(m_components.end(), m_components_new.begin(), m_components_new.end());
			m_components_new.clear();
		}while(!starts.empty());
	}

	void GameObject::Update()
	{
		for(auto &i : m_components)
		{
			if(!IsActiveInHierarchy())
			{
				break;
			}

			if(i->IsEnable())
			{
				i->Update();
			}
		}
	}

	void GameObject::LateUpdate()
	{
		for(auto &i : m_components)
		{
			if(!IsActiveInHierarchy())
			{
				break;
			}

			if(i->IsEnable())
			{
				i->LateUpdate();
			}
		}

		//delete component
		auto it = m_components.begin();
		while(it != m_components.end())
		{
			if((*it)->m_deleted)
			{
				it = m_components.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	void GameObject::AddComponent(const std::shared_ptr<Component> &com)
	{
		m_components_new.push_back(com);

		auto obj = World::FindGameObject(this);
		com->m_gameobject = obj;
		com->m_transform = m_transform;
		com->SetName(GetName());
        com->Awake();
	}

	void GameObject::SetActive(bool active)
	{
		m_active_self = active;

		if(	(m_active_in_hierarchy != active) && 
			(m_transform.lock()->IsRoot() ||
			m_transform.lock()->GetParent().lock()->GetGameObject()->IsActiveInHierarchy()))
		{
			SetActiveInHierarchy(active);
		}
	}

	void GameObject::SetActiveInHierarchy(bool active)
	{
		if(m_active_in_hierarchy != active)
		{
			m_active_in_hierarchy = active;

			for(auto &i : m_components)
			{
				if(i->IsEnable())
				{
					if(m_active_in_hierarchy)
					{
						i->OnEnable();
					}
					else
					{
						i->OnDisable();
					}
				}
			}

			auto transform = m_transform.lock();
			int child_count = transform->GetChildCount();
			for(int i=0; i<child_count; i++)
			{
				auto child = transform->GetChild(i);
				if(child->GetGameObject()->IsActiveSelf())
				{
					child->GetGameObject()->SetActiveInHierarchy(active);
				}
			}
		}
	}

    void GameObject::SetLayerRecursively(int layer)
    {
        SetLayer(layer);

        auto transform = m_transform.lock();
        int child_count = transform->GetChildCount();
        for(int i=0; i<child_count; i++)
        {
            auto child = transform->GetChild(i);
            child->GetGameObject()->SetLayerRecursively(layer);
        }
    }

    void GameObject::SetStaticRecursively()
    {
        m_static = true;

        auto transform = m_transform.lock();
        int child_count = transform->GetChildCount();
        for(int i=0; i<child_count; i++)
        {
            auto child = transform->GetChild(i);
            child->GetGameObject()->SetStaticRecursively();
        }
    }

    std::shared_ptr<Component> GameObject::GetComponentPtr(const Component *com) const
    {
        for(auto i : m_components)
        {
            if(i.get() == com && !i->m_deleted)
            {
                return i;
            }
        }

        for(auto i : m_components_new)
        {
            if(i.get() == com && !i->m_deleted)
            {
                return i;
            }
        }

        return std::shared_ptr<Component>();
    }
}