#ifndef __Transform_h__
#define __Transform_h__

#include "Component.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"
#include <vector>

namespace Galaxy3D
{
	class Transform : public Component
	{
        DECLARE_COM_CLASS(Transform, Component);

    private:
		friend class GameObject;

	public:
        virtual void DeepCopy(std::shared_ptr<Object> &source);
		std::weak_ptr<Transform> GetParent() const {return m_parent;}
		void SetParent(const std::weak_ptr<Transform> &parent);
		bool IsRoot() const {return m_parent.expired();}
		int GetChildCount() const {return m_children.size();}
		std::shared_ptr<Transform> GetChild(int index) const;
        std::shared_ptr<Transform> Find(const std::string &path) const;
		void SetLocalPosition(const Vector3 &pos);
		const Vector3 &GetLocalPosition() const {return m_local_position;}
		void SetLocalRotation(const Quaternion &rot);
		const Quaternion &GetLocalRotation() const {return m_local_rotation;}
		void SetLocalScale(const Vector3 &sca);
		const Vector3 &GetLocalScale() const {return m_local_scale;}
		void SetPosition(const Vector3 &pos);
		const Vector3 &GetPosition();
		void SetRotation(const Quaternion &rot);
		const Quaternion &GetRotation();
		void SetScale(const Vector3 &sca);
		const Vector3 &GetScale();
        void SetLocalPositionDirect(const Vector3 &pos) {m_local_position = pos;}
        void SetLocalRotationDirect(const Quaternion &rot) {m_local_rotation = rot;}
        void SetLocalScaleDirect(const Vector3 &sca) {m_local_scale = sca;}
		Vector3 TransformPoint(const Vector3 &point);
		Vector3 TransformDirection(const Vector3 &dir);
		Vector3 InverseTransformPoint(const Vector3 &point);
		Vector3 InverseTransformDirection(const Vector3 &dir);
		const Matrix4x4 &GetLocalToWorldMatrix();
		const Matrix4x4 &GetWorldToLocalMatrix();
		Vector3 GetRight();
		Vector3 GetUp();
		Vector3 GetForward();
        void SetForward(const Vector3 &forward);
        void Changed();

	private:
		std::weak_ptr<Transform> m_parent;
		std::vector<std::weak_ptr<Transform>> m_children;
		Vector3 m_local_position;
		Quaternion m_local_rotation;
		Vector3 m_local_scale;
		bool m_changed;
		Vector3 m_position;
		Quaternion m_rotation;
		Vector3 m_scale;
		Matrix4x4 m_local_to_world_matrix;
		Matrix4x4 m_world_to_local_matrix;

		Transform();
		void RemoveChild(std::weak_ptr<Transform> &child);
		void AddChild(std::weak_ptr<Transform> &child);
		void ApplyChange();
	};
}

#endif