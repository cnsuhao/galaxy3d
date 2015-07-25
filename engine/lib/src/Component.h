#ifndef __Component_h__
#define __Component_h__

#include "Object.h"

namespace Galaxy3D
{
	class Transform;

	class Component : public Object
	{
		friend class GameObject;

	public:
		static void Destroy(std::shared_ptr<Component> &obj);
		virtual ~Component() {}
		std::shared_ptr<GameObject> GetGameObject() const {return m_gameobject.lock();}
		std::shared_ptr<Transform> GetTransform() const {return m_transform.lock();}
		void Enable(bool enable);
		bool IsEnable() const {return m_enable;}

	protected:
		std::weak_ptr<GameObject> m_gameobject;
		std::weak_ptr<Transform> m_transform;

		Component();
		//没有OnDestroy，用析构函数
		virtual void Start() {}
		virtual void Update() {}
		virtual void LateUpdate() {}
		virtual void OnEnable() {}
		virtual void OnDisable() {}

	private:
		bool m_deleted;
		bool m_started;
		bool m_enable;

		void Delete();
	};
}

#endif