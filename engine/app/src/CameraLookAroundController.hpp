#ifndef __CameraLookAroundController_hpp__
#define __CameraLookAroundController_hpp__

#include "Component.h"

using namespace Galaxy3D;

class CameraLookAroundController : public Component
{
public:
	float cam_dis;
	Vector3 cam_rot;
	Vector3 cam_target;

private:
	bool m_mouse_down = false;
	Vector3 m_mouse_down_pos;

	Vector3 DragCameraRotate()
	{
		Vector3 mouse_pos = Input::GetMousePosition();
		Vector3 offset = mouse_pos - m_mouse_down_pos;
		Vector2 rot_scal(0.3f, 0.3f);
		Vector3 rot_offset = Vector3(-offset.y * rot_scal.x, offset.x * rot_scal.y, 0);
		Vector3 rot = cam_rot + rot_offset;
		rot.x = Mathf::Clamp(rot.x, -85.0f, 85.0f);
		rot_offset = rot - cam_rot;
		offset.y = - rot_offset.x / rot_scal.x;
		m_mouse_down_pos.y = mouse_pos.y - offset.y;

		GetTransform()->SetLocalRotation(Quaternion::Euler(rot));
		GetTransform()->SetLocalPosition(cam_target - GetTransform()->GetForward() * cam_dis);

		Vector3 cam_target = GetTransform()->GetPosition() + GetTransform()->GetForward() * cam_dis;

		return rot_offset;
	}

protected:
	virtual void Update()
	{
		if(Input::GetMouseButtonDown(0))
		{
			m_mouse_down = true;
			m_mouse_down_pos = Input::GetMousePosition();
		}

		if(Input::GetMouseButton(0))
		{
			DragCameraRotate();
		}

		if(Input::GetMouseButtonUp(0))
		{
			auto rot_offset = DragCameraRotate();

			cam_rot = cam_rot + rot_offset;
			m_mouse_down = false;
		}

		if(Input::GetKey(KeyCode::W))
		{
			auto move = GetTransform()->GetForward() * 0.5f;
			cam_target += move;
			GetTransform()->SetPosition(GetTransform()->GetPosition() + move);
		}

		if(Input::GetKey(KeyCode::S))
		{
			auto move = GetTransform()->GetForward() * -0.5f;
			cam_target += move;
			GetTransform()->SetPosition(GetTransform()->GetPosition() + move);
		}

		if(Input::GetKey(KeyCode::D))
		{
			auto move = GetTransform()->GetRight() * 0.5f;
			cam_target += move;
			GetTransform()->SetPosition(GetTransform()->GetPosition() + move);
		}

		if(Input::GetKey(KeyCode::A))
		{
			auto move = GetTransform()->GetRight() * -0.5f;
			cam_target += move;
			GetTransform()->SetPosition(GetTransform()->GetPosition() + move);
		}

		if(Input::GetKey(KeyCode::R))
		{
			auto move = Vector3(0, 1, 0) * 0.5f;
			cam_target += move;
			GetTransform()->SetPosition(GetTransform()->GetPosition() + move);
		}

		if(Input::GetKey(KeyCode::F))
		{
			auto move = Vector3(0, 1, 0) * -0.5f;
			cam_target += move;
			GetTransform()->SetPosition(GetTransform()->GetPosition() + move);
		}
	}
};

#endif