#include "Component.h"
#include "GameObject.h"

namespace Galaxy3D
{
	Component::Component():
		m_deleted(false),
		m_started(false),
		m_enable(true)
	{
	}

	void Component::Destroy(std::weak_ptr<Component> &obj)
	{
		if(!obj.expired())
		{
			obj.lock()->Delete();
			obj.reset();
		}
	}

	void Component::Delete()
	{
		if(!m_deleted)
		{
			m_deleted = true;
		}
	}

	void Component::Enable(bool enable)
	{
		if(m_enable != enable)
		{
			m_enable = enable;

			bool obj_active = GetGameObject()->IsActiveInHierarchy();
			if(obj_active)
			{
				if(m_enable)
				{
					OnEnable();
				}
				else
				{
					OnDisable();
				}
			}
		}
	}
}