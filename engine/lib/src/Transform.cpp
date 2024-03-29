#include "Transform.h"
#include "GameObject.h"
#include "GTString.h"

namespace Galaxy3D
{
    DEFINE_COM_CLASS(Transform);

	Transform::Transform():
		m_local_position(0, 0, 0),
		m_local_rotation(0, 0, 0, 1),
		m_local_scale(1, 1, 1),
		m_changed(true),
		m_position(m_local_position),
		m_rotation(m_local_rotation),
		m_scale(m_local_scale),
        m_change_notifying(false)
	{
	}

    void Transform::DeepCopy(const std::shared_ptr<Object> &source)
    {
        auto src_tran = std::dynamic_pointer_cast<Transform>(source);

        Component::DeepCopy(source);

        m_changed = src_tran->m_changed;
        m_position = src_tran->m_position;
        m_rotation = src_tran->m_rotation;
        m_scale = src_tran->m_scale;
        m_local_to_world_matrix = src_tran->m_local_to_world_matrix;
        m_world_to_local_matrix = src_tran->m_world_to_local_matrix;
        SetLocalPosition(m_position);
        SetLocalRotation(m_rotation);
        SetLocalScale(m_scale);

        for(size_t i=0; i<src_tran->m_children.size(); i++)
        {
            auto src_child = src_tran->m_children[i].lock();
            auto child = GameObject::Instantiate(src_child->GetGameObject());
            
            child->GetTransform()->SetParent(std::dynamic_pointer_cast<Transform>(GetComponentPtr()));
        }
    }

	void Transform::RemoveChild(std::weak_ptr<Transform> &child)
	{
		for(auto i=m_children.begin(); i!=m_children.end(); i++)
		{
			if(i->lock() == child.lock())
			{
				m_children.erase(i);
				break;
			}
		}
	}

	void Transform::AddChild(std::weak_ptr<Transform> &child)
	{
		m_children.push_back(child);
	}

	void Transform::SetParent(const std::weak_ptr<Transform> &parent)
	{
		auto obj = GetGameObject();

		ApplyChange();

		if(!m_parent.expired())
		{
			m_parent.lock()->RemoveChild(m_transform);
			m_parent.reset();

			//become root
			if(parent.expired())
			{
				m_local_position = m_position;
				m_local_rotation = m_rotation;
				m_local_scale = m_scale;
				Changed();

				obj->SetActiveInHierarchy(obj->IsActiveSelf());
			}
		}

		m_parent = parent;

		if(!m_parent.expired())
		{
			auto p = m_parent.lock();
			p->AddChild(m_transform);

			//become child
			{
				m_local_position = p->InverseTransformPoint(m_position);
				m_local_rotation = Quaternion::Inverse(p->GetRotation()) * m_rotation;
				const Vector3 &parent_scale = p->GetScale();
				float x = m_scale.x / parent_scale.x;
				float y = m_scale.y / parent_scale.y;
				float z = m_scale.z / parent_scale.z;
				m_local_scale = Vector3(x, y, z);
				Changed();

				obj->SetActiveInHierarchy(p->GetGameObject()->IsActiveInHierarchy() && obj->IsActiveSelf());
			}
		}
	}

	std::shared_ptr<Transform> Transform::GetChild(int index) const
	{
		return m_children[index].lock();
	}

    std::shared_ptr<Transform> Transform::Find(const std::string &path) const
    {
        std::shared_ptr<Transform> find;

        GTString p = path;
        auto names = p.Split("/");
        
        for(auto &c : m_children)
        {
            auto child = c.lock();
            auto name = names[0].str;

            if(child->GetName() == name)
            {
                if(names.size() > 1)
                {
                    find = child->Find(path.substr(name.length() + 1));
                }
                else
                {
                    find = child;
                }

                break;
            }
        }

        return find;
    }

    void Transform::NotifyChange()
    {
        m_change_notifying = true;

        GetGameObject()->OnTranformChanged();

        for(auto &i : m_children)
        {
            i.lock()->NotifyChange();
        }

        m_change_notifying = false;
    }

	void Transform::SetLocalPosition(const Vector3 &pos)
	{
		if(m_local_position != pos)
		{
			m_local_position = pos;
			Changed();
            NotifyChange();
		}
	}

	void Transform::SetLocalRotation(const Quaternion &rot)
	{
        Quaternion r = rot;
        r.Normalize();

		if(m_local_rotation != r)
		{
			m_local_rotation = r;
			Changed();
            NotifyChange();
		}
	}

	void Transform::SetLocalScale(const Vector3 &sca)
	{
		if(m_local_scale != sca)
		{
			m_local_scale = sca;
			Changed();
            NotifyChange();
		}
	}

	void Transform::SetPosition(const Vector3 &pos)
	{
		if(!m_changed && m_position == pos)
		{
			return;
		}

		if(IsRoot())
		{
			SetLocalPosition(pos);
		}
		else
		{
			Vector3 local = m_parent.lock()->InverseTransformPoint(pos);
			SetLocalPosition(local);
		}
	}

	const Vector3 &Transform::GetPosition()
	{
		ApplyChange();

		return m_position;
	}

	void Transform::SetRotation(const Quaternion &rot)
	{
		if(!m_changed && m_rotation == rot)
		{
			return;
		}

		if(IsRoot())
		{
			SetLocalRotation(rot);
		}
		else
		{
			Quaternion local = Quaternion::Inverse(m_parent.lock()->GetRotation()) * rot;
			SetLocalRotation(local);
		}
	}

	const Quaternion &Transform::GetRotation()
	{
		ApplyChange();

		return m_rotation;
	}

	void Transform::SetScale(const Vector3 &sca)
	{
		if(!m_changed && m_scale == sca)
		{
			return;
		}

		if(IsRoot())
		{
			SetLocalScale(sca);
		}
		else
		{
			const Vector3 &parent_scale = m_parent.lock()->GetScale();
			float x = sca.x / parent_scale.x;
			float y = sca.y / parent_scale.y;
			float z = sca.z / parent_scale.z;
			SetLocalScale(Vector3(x, y, z));
		}
	}

	const Vector3 &Transform::GetScale()
	{
		ApplyChange();

		return m_scale;
	}

	void Transform::Changed()
	{
		m_changed = true;
		for(auto &i : m_children)
		{
			i.lock()->Changed();
		}
	}

	void Transform::ApplyChange()
	{
		if(m_changed)
		{
			m_changed = false;

			if(IsRoot())
			{
				m_position = m_local_position;
				m_rotation = m_local_rotation;
				m_scale = m_local_scale;
			}
			else
			{
				auto parent = m_parent.lock();

				m_position = parent->TransformPoint(m_local_position);
				m_rotation = parent->GetRotation() * m_local_rotation;

				const Vector3 &ps = parent->GetScale();
				float x = m_local_scale.x * ps.x;
				float y = m_local_scale.y * ps.y;
				float z = m_local_scale.z * ps.z;
				m_scale = Vector3(x, y, z);
			}

			m_local_to_world_matrix = Matrix4x4::TRS(m_position, m_rotation, m_scale);
		}
	}

	Vector3 Transform::TransformPoint(const Vector3 &point)
	{
		return GetLocalToWorldMatrix().MultiplyPoint3x4(point);
	}

	Vector3 Transform::TransformDirection(const Vector3 &dir)
	{
		return GetLocalToWorldMatrix().MultiplyDirection(dir);
	}

	Vector3 Transform::InverseTransformPoint(const Vector3 &point)
	{
		return GetWorldToLocalMatrix().MultiplyPoint3x4(point);
	}

	Vector3 Transform::InverseTransformDirection(const Vector3 &dir)
	{
		return GetWorldToLocalMatrix().MultiplyDirection(dir);
	}

	const Matrix4x4 &Transform::GetLocalToWorldMatrix()
	{
		ApplyChange();

		return m_local_to_world_matrix;
	}
	
	const Matrix4x4 &Transform::GetWorldToLocalMatrix()
	{
		m_world_to_local_matrix = GetLocalToWorldMatrix().Inverse();

		return m_world_to_local_matrix;
	}

	Vector3 Transform::GetRight()
	{
		return GetRotation() * Vector3(1, 0, 0);
	}

	Vector3 Transform::GetUp()
	{
		return GetRotation() * Vector3(0, 1, 0);
	}

	Vector3 Transform::GetForward()
	{
		return GetRotation() * Vector3(0, 0, 1);
	}

    void Transform::SetForward(const Vector3 &forward)
    {
        Vector3 origin = Vector3(0, 0, 1);
        Vector3 fn = Vector3::Normalize(forward);

        if(fn != origin)
        {
            if(!Mathf::FloatEqual(fn.SqrMagnitude(), 0))
            {
                float deg = Vector3::Angle(origin, fn);
                Vector3 axis = origin * fn;

                if(axis == Vector3(0, 0, 0))
                {
                    SetRotation(Quaternion::AngleAxis(deg, GetUp()));
                }
                else
                {
                    SetRotation(Quaternion::AngleAxis(deg, axis));
                }
            }
        }
        else
        {
            SetRotation(Quaternion::Identity());
        }
    }
}