#include "Quaternion.h"
#include "Mathf.h"
#include <math.h>
#include <sstream>

namespace Galaxy3D
{
	Quaternion Quaternion::m_identity = Quaternion(0, 0, 0, 1);

	Quaternion::Quaternion(float x, float y, float z, float w):
		x(x), y(y), z(z), w(w)
	{
	}

	Vector3 Quaternion::ToEulerAngles() const
	{
		float rx = atan2(2 * (w * x + y * z), 1 - 2 * (x * x + y * y));
		float ry = asin(2 * (w * y - z * x));
		float rz = atan2(2 * (w * z + x * y), 1 - 2 * (z * z + y * y));

		return Vector3(rx, ry, rz) * Mathf::Rad2Deg;
	}

	std::string Quaternion::ToString() const
	{
		std::stringstream ss;
		ss << '(' << x << ',' << y << ',' << z << ',' << w << ')';

		return ss.str();
	}

	Quaternion Quaternion::operator *(const Quaternion &quat) const
	{
		float _x = w * quat.x + x * quat.w + y * quat.z - z * quat.y;
		float _y = w * quat.y + y * quat.w + z * quat.x - x * quat.z;
		float _z = w * quat.z + z * quat.w + x * quat.y - y * quat.x;
		float _w = w * quat.w - x * quat.x - y * quat.y - z * quat.z;

		return Quaternion(_x, _y, _z, _w);
	}

	Vector3 Quaternion::operator *(const Vector3 &p) const
	{
		Quaternion p_ = *this * Quaternion(p.x, p.y, p.z, 0) * Inverse(*this);

		return Vector3(p_.x, p_.y, p_.z);
	}

	bool Quaternion::operator !=(const Quaternion &v) const
	{
		return !(*this == v);
	}

	bool Quaternion::operator ==(const Quaternion &v) const
	{
		return Mathf::FloatEqual(v.x, x) &&
			Mathf::FloatEqual(v.y, y) &&
			Mathf::FloatEqual(v.z, z) &&
			Mathf::FloatEqual(v.w, w);
	}

	Quaternion Quaternion::Inverse(const Quaternion &q)
	{
		return Quaternion(-q.x, -q.y, -q.z, q.w);
	}

	Quaternion Quaternion::AngleAxis(float angle, const Vector3 &axis)
	{
		Vector3 v = Vector3::Normalize(axis);
		float cosv, sinv;

		cosv = cos(Mathf::Deg2Rad * angle * 0.5f);
		sinv = sin(Mathf::Deg2Rad * angle * 0.5f);

		float x = v.x * sinv;
		float y = v.y * sinv;
		float z = v.z * sinv;
		float w = cosv;

		return Quaternion(x, y, z, w);
	}

	Quaternion Quaternion::Euler(float x, float y, float z)
	{
		Quaternion around_x = AngleAxis(x, Vector3(1, 0, 0));
		Quaternion around_y = AngleAxis(y, Vector3(0, 1, 0));
		Quaternion around_z = AngleAxis(z, Vector3(0, 0, 1));

		return around_y * around_x * around_z;
	}

    Quaternion Quaternion::Lerp(const Quaternion &from, const Quaternion &to, float t)
    {
        Quaternion lerp = Quaternion(
            Mathf::Lerp(from.x, to.x, t, false),
            Mathf::Lerp(from.y, to.y, t, false),
            Mathf::Lerp(from.z, to.z, t, false),
            Mathf::Lerp(from.w, to.w, t, false));
        lerp.Normalize();

        return lerp;
    }

    void Quaternion::Normalize()
    {
        float sqr_magnitude = x*x + y*y + z*z + w*w;
        if(!Mathf::FloatEqual(sqr_magnitude, 0))
        {
            float sq = sqrt(sqr_magnitude);

            float inv = 1.0f / sq;
            x = x * inv;
            y = y * inv;
            z = z * inv;
            w = w * inv;
        }
    }
}