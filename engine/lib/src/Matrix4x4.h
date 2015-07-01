#ifndef __Matrix4x4_h__
#define __Matrix4x4_h__

#include "Vector3.h"
#include "Vector4.h"
#include "Quaternion.h"

namespace Galaxy3D
{
	//column major
	class Matrix4x4
	{
	public:
		Matrix4x4(){}
		Matrix4x4 operator *(const Matrix4x4 &quat) const;
		Vector4 operator *(const Vector4 &v) const;
		Vector3 MultiplyPoint(const Vector3 &v) const;
		Vector3 MultiplyPoint3x4(const Vector3 &v) const;
		Vector3 MultiplyDirection(const Vector3 &v) const;
		Matrix4x4 Inverse() const;
        Matrix4x4 Transpose() const;
		std::string ToString() const;
		void SetRow(int row, const Vector4 &v);
		Vector4 GetRow(int row);
		void SetColumn(int row, const Vector4 &v);
		Vector4 GetColumn(int row);

		inline static Matrix4x4 Identity() {return m_identity;}
		static Matrix4x4 Translation(const Vector3 &t);
		static Matrix4x4 Rotation(const Quaternion &r);
		static Matrix4x4 Scaling(const Vector3 &s);
		static Matrix4x4 TRS(const Vector3 &t, const Quaternion &r, const Vector3 &s);
		static Matrix4x4 Perspective(float fov, float aspect, float zNear, float zFar);
		static Matrix4x4 LookTo(const Vector3 &eye_position, const Vector3 &to_direction, const Vector3 &up_direction);
		static Matrix4x4 Ortho(float left, float right, float bottom, float top, float zNear, float zFar);

	private:
		Matrix4x4(const float *ms);

	public:
		float m00;
        float m01;
        float m02;
        float m03;
        float m10;
        float m11;
        float m12;
        float m13;
        float m20;
        float m21;
        float m22;
        float m23;
        float m30;
        float m31;
        float m32;
        float m33;

	private:
		static float m_identity[];
	};
}

#endif