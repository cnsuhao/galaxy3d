#ifndef __RenderStates_h__
#define __RenderStates_h__

#include "GraphicsDevice.h"
#include <string>

namespace Galaxy3D
{
	class RenderStates
	{
	public:
		struct Key
		{
			enum Enum
			{
				Cull,
				ZWrite,
				ZTest,
				Offset,
				Blend,
				BlendOp,
				ColorMask,
				Fog,
				Stencil,
			};
		};

		static const int KeyCount = 9;
		static const std::string Keys[KeyCount];

		static const int CullNameCount = 3;
		static const std::string CullNames[CullNameCount];

		static const int ZWriteNameCount = 2;
		static const std::string ZWriteNames[ZWriteNameCount];

		static const int ZTestNameCount = 7;
		static const std::string ZTestNames[ZTestNameCount];

		static const int BlendNameCount = 11;
		static const std::string BlendNames[BlendNameCount];

		static const int BlendOpNameCount = 5;
		static const std::string BlendOpNames[BlendOpNameCount];

        static const int StencilComparisonFunctionNameCount = 8;
        static const std::string StencilComparisonFunctionNames[StencilComparisonFunctionNameCount];

        static const int StencilOperationNameCount = 8;
        static const std::string StencilOperationNames[StencilOperationNameCount];

		RenderStates();
		void Parse(const std::string &s);
		void Create();
		void Release();
		void Apply();

	private:
		struct State
		{
			bool cull_enable;
			GLenum cull_face;
			GLenum z_write;
			GLenum z_test;
			float zoffset_factor;
			float zoffset_units;
			bool blend_enable;
			GLenum blend_src_rgb;
			GLenum blend_dst_rgb;
			GLenum blend_src_alpha;
			GLenum blend_dst_alpha;
			GLenum blend_op;
			GLboolean color_mask_r;
			GLboolean color_mask_g;
			GLboolean color_mask_b;
			GLboolean color_mask_a;
			bool stencil_enable;
			int stencil_read_mask;
			int stencil_write_mask;
			GLenum stencil_comp;
			GLenum stencil_pass;
			GLenum stencil_fail;
			GLenum stencil_zfail;
			int stencil_ref;
		};
		
		static std::shared_ptr<State> m_current_state;
		std::string m_values[KeyCount];
        State m_state;
	};
}

#endif