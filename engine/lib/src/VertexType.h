#ifndef __VertexType_h__
#define __VertexType_h__

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Color.h"

namespace Galaxy3D
{
	struct VertexSkinned
	{
		Vector3 POSITION;
		Vector3 NORMAL;
		Vector4 TANGENT;
		Vector4 BLENDWEIGHT;
		Vector4 BLENDINDICES;
		Vector2 TEXCOORD0;
	};

	struct VertexMesh
	{
		Vector3 POSITION;
		Vector3 NORMAL;
		Vector4 TANGENT;
		Vector2 TEXCOORD0;
		Vector2 TEXCOORD1;
	};

	struct VertexUI
	{
		Vector3 POSITION;
		Color COLOR;
		Vector2 TEXCOORD0;
	};
}

#endif