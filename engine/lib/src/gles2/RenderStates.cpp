#include "RenderStates.h"
#include "GTString.h"
#include "GraphicsDevice.h"

namespace Galaxy3D
{
	const std::string RenderStates::Keys[KeyCount] =
	{
		"Cull",
		"ZWrite",
		"ZTest",
		"Offset",
		"Blend",
		"BlendOp",
		"ColorMask",
		"Fog",
		"Stencil",
	};

	const std::string RenderStates::CullNames[CullNameCount] =
	{
		"Back",
		"Front",
		"Off",
	};

	static const int CullValues[RenderStates::CullNameCount] =
	{
		GL_BACK,
		GL_FRONT,
        0
	};

	const std::string RenderStates::ZWriteNames[ZWriteNameCount] =
	{
		"On",
		"Off",
	};

	static const int ZWriteValues[RenderStates::ZWriteNameCount] =
	{
		GL_TRUE,
		GL_FALSE,
	};

	const std::string RenderStates::ZTestNames[ZTestNameCount] =
	{
		"Less",
		"Greater",
		"LEqual",
		"GEqual",
		"Equal",
		"NotEqual",
		"Always",
	};

	static const int ZTestValues[RenderStates::ZTestNameCount] =
	{
		GL_LESS,
		GL_GREATER,
		GL_LEQUAL,
		GL_GEQUAL,
		GL_EQUAL,
		GL_NOTEQUAL,
		GL_ALWAYS,
	};

	const std::string RenderStates::BlendNames[BlendNameCount] =
	{
		"Off",
		"One",
		"Zero",
		"SrcColor",
		"SrcAlpha",
		"DstColor",
		"DstAlpha",
		"OneMinusSrcColor",
		"OneMinusSrcAlpha",
		"OneMinusDstColor",
		"OneMinusDstAlpha",
	};

	static const int BlendValues[RenderStates::BlendNameCount] = 
	{
		0,
		GL_ONE,
		GL_ZERO,
		GL_SRC_COLOR,
		GL_SRC_ALPHA,
		GL_DST_COLOR,
		GL_DST_ALPHA,
		GL_ONE_MINUS_SRC_COLOR,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_ONE_MINUS_DST_COLOR,
		GL_ONE_MINUS_DST_ALPHA,
	};

	const std::string RenderStates::BlendOpNames[BlendOpNameCount] =
	{
		"Add",
		"Sub",
		"RevSub",
		"Min",
		"Max",
	};

	static const int BlendOpValues[RenderStates::BlendOpNameCount] =
	{
		GL_FUNC_ADD,
		GL_FUNC_SUBTRACT,
		GL_FUNC_REVERSE_SUBTRACT,
		0,
		0,
	};

	struct StencilKey
    {
        enum Enum
        {
            Ref,
            ReadMask,
            WriteMask,
            Comp,
            Pass,
            Fail,
            ZFail,
        };
    };

	static const int StencilKeyNamesCount = 7;
    static const std::string StencilKeyNames[StencilKeyNamesCount] =
    {
        "Ref",
        "ReadMask",
        "WriteMask",
        "Comp",
        "Pass",
        "Fail",
        "ZFail",
    };

    const std::string RenderStates::StencilComparisonFunctionNames[StencilComparisonFunctionNameCount] =
    {
        "Greater",
        "GEqual",
        "Less",
        "LEqual",
        "Equal",
        "NotEqual",
        "Always",
        "Never",
    };

	static const int StencilComparisonFunctionValues[RenderStates::StencilComparisonFunctionNameCount] =
	{
		GL_GREATER,
		GL_GEQUAL,
		GL_LESS,
		GL_LEQUAL,
		GL_EQUAL,
		GL_NOTEQUAL,
		GL_ALWAYS,
		GL_NEVER,
	};

	const std::string RenderStates::StencilOperationNames[StencilOperationNameCount] =
    {
        "Keep",
        "Zero",
        "Replace",
        "IncrSat",
        "DecrSat",
        "Invert",
        "IncrWrap",
        "DecrWrap",
    };

	static const int StencilOperationValues[RenderStates::StencilOperationNameCount] =
	{
		GL_KEEP,
		0,
		GL_REPLACE,
		GL_INCR,
		GL_DECR,
		GL_INVERT,
		GL_INCR_WRAP,
		GL_DECR_WRAP,
	};

	std::shared_ptr<RenderStates::State> RenderStates::m_current_state;

	RenderStates::RenderStates()
	{
		m_values[0] = CullNames[0];
		m_values[1] = ZWriteNames[0];
		m_values[2] = ZTestNames[2];
		m_values[3] = "0, 0";
		m_values[4] = BlendNames[0];
		m_values[5] = BlendOpNames[0];
		m_values[6] = "RGBA";
		m_values[7] = "";
		m_values[8] = "";
	}

	void RenderStates::Parse(const std::string &s)
	{
		GTString str = s;
		str = str.Replace("\r\n", "\n");
		str = str.Replace("\t", "");
        str = str.Replace("    ", "");
		std::vector<GTString> lines = str.Split("\n", true);

		int stencil_index = -1;
		for(size_t i=0; i<lines.size();)
		{
			if(lines[i].StartsWith("Stencil"))
			{
				stencil_index = i;
				lines[stencil_index].str += " ";
			}
			else
			{
				if(stencil_index >= 0)
				{
					lines[stencil_index].str += "\n" + lines[i].str;
					if(lines[i].StartsWith("}"))
					{
						stencil_index = -1;
					}
					lines.erase(lines.begin() + i);

					continue;
				}
			}

			i++;
		}

		for(size_t i=0; i<lines.size(); i++)
		{
			for(int j=0; j<KeyCount; j++)
			{
				size_t key = lines[i].str.find(Keys[j]);
				if(key != std::string::npos && lines[i].str[key + Keys[j].length()] == ' ')
				{
 					std::string value = lines[i].str.substr(key + Keys[j].length());
					
					int left = 0;
					while(value[left] == ' ')
					{
						left++;
					}

					int right = (int) value.length() - 1;
					while(value[right] == ' ')
					{
						right--;
					}

					value = value.substr(left, right);

					m_values[j] = value;
					break;
				}
			}
		}
	}

	void RenderStates::Release()
	{
	}

	void RenderStates::Create()
	{
		int cull_index = -1;
		for(int i=0; i<RenderStates::CullNameCount; i++)
		{
			if(m_values[Key::Cull] == RenderStates::CullNames[i])
			{
				cull_index = i;
				break;
			}
		}

		int zwrite_index = -1;
		for(int i=0; i<RenderStates::ZWriteNameCount; i++)
		{
			if(m_values[Key::ZWrite] == RenderStates::ZWriteNames[i])
			{
				zwrite_index = i;
				break;
			}
		}

		int ztest_index = -1;
		for(int i=0; i<RenderStates::ZTestNameCount; i++)
		{
			if(m_values[Key::ZTest] == RenderStates::ZTestNames[i])
			{
				ztest_index = i;
				break;
			}
		}

		float zoffset_factor = 0;
		float zoffset_units = 0;
		GTString zoffset = m_values[Key::Offset];
		zoffset = zoffset.Replace(" ", "");
		std::vector<GTString> zoffset_values = zoffset.Split(",");
		if(zoffset_values.size() == 2)
		{
			std::stringstream ss_factor(zoffset_values[0].str);
			ss_factor >> zoffset_factor;

			std::stringstream ss_units(zoffset_values[1].str);
			ss_units >> zoffset_units;
		}

		bool blend_enable = false;
		int blends_index[4];
		blends_index[0] = -1;
		blends_index[1] = -1;
		blends_index[2] = -1;
		blends_index[3] = -1;
		GTString blend = m_values[Key::Blend];
		if(blend.str != RenderStates::BlendNames[0])
		{
			blend_enable = true;

			blend = blend.Replace(",", " ");
			std::vector<GTString> blends = blend.Split(" ");
			for(size_t i=0; i<blends.size(); )
			{
				if(blends[i].str == "")
				{
					blends.erase(blends.begin() + i);
					continue;
				}

				i++;
			}

			for(size_t j=0; j<4 && j<blends.size(); j++)
			{
				for(int i=0; i<RenderStates::BlendNameCount; i++)
				{
					if(blends[j].str == RenderStates::BlendNames[i])
					{
						blends_index[j] = i;
						break;
					}
				}
			}

			if(blends.size() == 2)
			{
				blends_index[2] = 4;
				blends_index[3] = 8;
			}
		}

		int blendop_index = -1;
		for(int i=0; i<RenderStates::BlendOpNameCount; i++)
		{
			if(m_values[Key::BlendOp] == RenderStates::BlendOpNames[i])
			{
				blendop_index = i;
				break;
			}
		}

		GLboolean red = GL_FALSE;
		GLboolean green = GL_FALSE;
		GLboolean blue = GL_FALSE;
		GLboolean alpha = GL_FALSE;
		std::string color_mask_str = m_values[Key::ColorMask];
		for(size_t i=0; i<color_mask_str.size(); i++)
		{
			if(color_mask_str[i] == 'R')
			{
				red = GL_TRUE;
			}
			else if(color_mask_str[i] == 'G')
			{
				green = GL_TRUE;
			}
			else if(color_mask_str[i] == 'B')
			{
				blue = GL_TRUE;
			}
			else if(color_mask_str[i] == 'A')
			{
				alpha = GL_TRUE;
			}
		}

        bool stencil_enable = false;
        std::string stencil_str = m_values[Key::Stencil];
        int stencil_values[StencilKeyNamesCount] =
        {
            0, 255, 255,
            GL_ALWAYS,
            GL_KEEP,
            GL_KEEP,
            GL_KEEP
        };
        if(!stencil_str.empty())
        {
            stencil_enable = true;

            auto values = GTString(stencil_str).Replace("\r\n", "\n").Split("\n", true);
            for(auto i : values)
            {
                auto pair = GTString(i).Split(" ", true);
                if(pair.size() == 2)
                {
                    auto key = pair[0].str;

                    if(key == "Comp")
                    {
                        for(int j=0; j<StencilComparisonFunctionNameCount; j++)
                        {
                            if(pair[1].str == StencilComparisonFunctionNames[j])
                            {
                                stencil_values[StencilKey::Comp] = StencilComparisonFunctionValues[j];
                                break;
                            }
                        }

                        continue;
                    }

                    for(int j=0; j<StencilKeyNamesCount; j++)
                    {
                        if((key == "Ref" ||
                            key == "ReadMask" ||
                            key == "WriteMask") && key == StencilKeyNames[j])
                        {
                            auto value = GTString::ToType<int>(pair[1].str);
                            stencil_values[j] = value;
                            break;
                        }
                        else if((key == "Pass" ||
                            key == "Fail" ||
                            key == "ZFail") && key == StencilKeyNames[j])
                        {
                            for(int k=0; k<StencilOperationNameCount; k++)
                            {
                                if(pair[1].str == StencilOperationNames[k])
                                {
                                    stencil_values[j] = StencilOperationValues[k];
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }

		m_state.cull_enable = CullValues[cull_index] != 0;
		m_state.cull_face = CullValues[cull_index];
		m_state.z_write = ZWriteValues[zwrite_index];
		m_state.z_test = ZTestValues[ztest_index];
		m_state.zoffset_factor = zoffset_factor;
		m_state.zoffset_units = zoffset_units;
		m_state.blend_enable = blend_enable;
		m_state.blend_src_rgb = BlendValues[blends_index[0]];
		m_state.blend_dst_rgb = BlendValues[blends_index[1]];
		m_state.blend_src_alpha = BlendValues[blends_index[2]];
		m_state.blend_dst_alpha = BlendValues[blends_index[3]];
		m_state.blend_op = BlendOpValues[blendop_index];
		m_state.color_mask_r = red;
		m_state.color_mask_g = green;
		m_state.color_mask_b = blue;
		m_state.color_mask_a = alpha;
		m_state.stencil_enable = stencil_enable;
		m_state.stencil_read_mask = stencil_values[StencilKey::ReadMask];
		m_state.stencil_write_mask = stencil_values[StencilKey::WriteMask];
		m_state.stencil_comp = stencil_values[StencilKey::Comp];
		m_state.stencil_pass = stencil_values[StencilKey::Pass];
		m_state.stencil_fail = stencil_values[StencilKey::Fail];
		m_state.stencil_zfail = stencil_values[StencilKey::ZFail];
		m_state.stencil_ref = stencil_values[StencilKey::Ref];
	}

	void RenderStates::Apply()
	{
		bool update = false;

		if(!m_current_state)
		{
			m_current_state = std::make_shared<State>();
			update = true;
		}
		else if(memcmp(m_current_state.get(), &m_state, sizeof(State)) != 0)
		{
			update = true;
		}

		*m_current_state = m_state;

		if(update)
		{
			if(m_state.cull_enable)
			{
				glEnable(GL_CULL_FACE);
				glCullFace(m_state.cull_face);
			}
			else
			{
				glDisable(GL_CULL_FACE);
			}
			glDepthMask(m_state.z_write);
			glDepthFunc(m_state.z_test);
			glPolygonOffset(m_state.zoffset_factor, m_state.zoffset_units);
			if(m_state.blend_enable)
			{
				glEnable(GL_BLEND);
				glBlendFuncSeparate(
					m_state.blend_src_rgb,
					m_state.blend_dst_rgb,
					m_state.blend_src_alpha,
					m_state.blend_dst_alpha);
				glBlendEquation(m_state.blend_op);
			}
			else
			{
				glDisable(GL_BLEND);
			}
			glColorMask(m_state.color_mask_r, m_state.color_mask_g, m_state.color_mask_b, m_state.color_mask_a);
			if(m_state.stencil_enable)
			{
				glEnable(GL_STENCIL_TEST);
				glStencilMask(m_state.stencil_write_mask);
				glStencilFunc(m_state.stencil_comp, m_state.stencil_ref, m_state.stencil_read_mask);
				glStencilOp(m_state.stencil_fail, m_state.stencil_zfail, m_state.stencil_pass);  
			}
			else
			{
				glDisable(GL_STENCIL_TEST);
			}
		}
	}
}