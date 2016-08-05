Diffuse
{
	Tags
	{
		Queue Geometry
	}

	Pass
	{
		VS vs
		PS ps
		RenderStates rs
	}

    Pass deferred
    {
        VS vs_deferred
        PS ps_deferred
        RenderStates rs
    }

    Pass depth
    {
        VS vs_depth
		PS ps_depth
        RenderStates rs
    }

	RenderStates rs
	{
        Cull Back
	}

	HLVS vs
	{
		cbuffer cbuffer0 : register(b0)
		{
			matrix WorldViewProjection;
		};

        cbuffer cbuffer1 : register(b1)
        {
            matrix World;
        };

        cbuffer cbuffer2 : register(b2)
        {
            float4 LightDirection;
        };

        cbuffer cbuffer3 : register(b3)
        {
            float4 EyePosition;
        };

		struct VS_INPUT
		{
			float4 Position : POSITION;
            float3 Normal : NORMAL;
            float4 Tangent : TANGENT;
			float2 Texcoord0 : TEXCOORD0;
            float2 Texcoord1 : TEXCOORD1;
		};

		struct PS_INPUT
		{
			float4 v_pos : SV_POSITION;
			float2 v_uv : TEXCOORD0;
            float3 v_light_dir_world : TEXCOORD1;
            float3 v_eye_dir_world : TEXCOORD2;
            float3 v_normal_world : TEXCOORD3;

			float4 v_pos_proj : TEXCOORD4;
			float4 v_pos_world : TEXCOORD5;
		};

		PS_INPUT main(VS_INPUT input)
		{
			PS_INPUT output = (PS_INPUT) 0;

			output.v_pos = mul(input.Position, WorldViewProjection);
			output.v_uv = input.Texcoord0;

            float4 pos_world = mul(input.Position, World);
            float3 normal_world = mul(float4(input.Normal, 0), World).xyz;

            output.v_light_dir_world = - LightDirection.xyz;
            output.v_eye_dir_world = EyePosition.xyz - pos_world.xyz;
            output.v_normal_world = normal_world;

			output.v_pos_proj = output.v_pos;
			output.v_pos_world = pos_world;

			return output;
		}
	}

	HLPS ps
	{
		cbuffer cbuffer0 : register(b0)
		{
			float4 GlobalAmbient;
		};

		cbuffer cbuffer1 : register(b1)
		{
			float4 LightColor;
		};

		cbuffer cbuffer2 : register(b2)
		{
			float4 _Color;
		};

		cbuffer cbuffer3 : register(b3)
		{
			matrix ViewProjectionLight[3];
		};

		cbuffer cbuffer4 : register(b4)
		{
			float4 ShadowParam;
		};

		cbuffer cbuffer5 : register(b5)
		{
			float4 _ZBufferParams;
		};

		cbuffer cbuffer6 : register(b6)
		{
			float4 ShadowMapTexel;
		};

		cbuffer cbuffer7 : register(b7)
		{
			float4 CascadeSplits;
		};

		Texture2D _MainTex : register(t0);
		SamplerState _MainTex_Sampler : register(s0);

		Texture2D _ShadowMapTexture : register(t1);
		SamplerState _ShadowMapTexture_Sampler : register(s1);

		struct PS_INPUT
		{
			float4 v_pos : SV_POSITION;
			float2 v_uv : TEXCOORD0;
			float3 v_light_dir_world : TEXCOORD1;
			float3 v_eye_dir_world : TEXCOORD2;
			float3 v_normal_world : TEXCOORD3;

			float4 v_pos_proj : TEXCOORD4;
			float4 v_pos_world : TEXCOORD5;
		};

        float texture2DCompare(float2 uv, float compare)
		{
			return step(compare, _ShadowMapTexture.Sample(_ShadowMapTexture_Sampler, uv).r);
		}

		float PCF(float2 uv, float2 size, float z)
		{
			float bias = ShadowParam.x;
			float strength = ShadowParam.y;
			float shadow_weak = clamp(1 - strength, 0, 1);
			float shadow = 0;
			int pcf_size = 1;

			for(int i=-pcf_size;i<=pcf_size;i++)
			{
				for(int j=-pcf_size;j<=pcf_size;j++)
				{
					float2 off = float2(i, j) * size;
					float compare = texture2DCompare(uv + off, z - bias);

					if(compare < 1)
					{
						shadow += shadow_weak;
					}
					else
					{
						shadow += 1;
					}
				}
			}
			return shadow / ((pcf_size*2+1) * (pcf_size*2+1));
		}

		float sample_shadow(float depth, float4 pos_world)
		{
			// shadow
			bool cascade = ((int) ShadowParam.z) == 1;

			int index = -1;

			if(cascade)
			{
				float linear_depth = 1.0 / (_ZBufferParams.x * depth + _ZBufferParams.y);
				if(linear_depth < CascadeSplits.x)
				{
					index = 0;
				}
				else if(linear_depth < CascadeSplits.y)
				{
					index = 1;
				}
				else if(linear_depth < CascadeSplits.z)
				{
					index = 2;
				}
			}
			else
			{
				index = 0;
			}

			float shadow = 1.0;

			if(index >= 0)
			{
				float4 pos_light_4 = mul(pos_world, ViewProjectionLight[index]);
				float3 pos_light = pos_light_4.xyz / pos_light_4.w;
				pos_light.z = min(1, pos_light.z);

				float2 uv_shadow = 0;
				uv_shadow.x = 0.5 + pos_light.x * 0.5;
				uv_shadow.y = 0.5 - pos_light.y * 0.5;

				float tex_witdh = 1.0;
				float tex_height = 1.0;
				if(cascade)
				{
					tex_witdh = 1.0 / 3;
					uv_shadow.x = index * tex_witdh + uv_shadow.x * tex_witdh;
				}

				float2 size = ShadowMapTexel.xy * float2(tex_witdh, tex_height);

				if(uv_shadow.x > 0.0 && uv_shadow.x < 1.0 && uv_shadow.y > 0.0 && uv_shadow.y < 1.0)
				{
					shadow = PCF(uv_shadow, size, pos_light.z);
				}
			}

			return shadow;
		}

		float4 main(PS_INPUT input) : SV_Target
		{
			float specular_power = 1.0;
			float specular_intensity = 0.5;

			float4 c = _MainTex.Sample(_MainTex_Sampler, input.v_uv) * _Color;

			float3 normal = normalize(input.v_normal_world);
			float3 light_dir = normalize(input.v_light_dir_world);
			float3 eye_dir = normalize(input.v_eye_dir_world);

			float diff = max(0, dot(normal, light_dir));
			float3 h = normalize(light_dir + eye_dir);
			float nh = max(0, dot(normal, h));
			float spec = pow(nh, 128 * specular_power) * specular_intensity;

			float intensity = 1;
	
			bool shadow_enable = ((int) ShadowParam.w) == 1;
			if(shadow_enable)
			{
				float depth = input.v_pos_proj.z / input.v_pos_proj.w;
				float shadow = sample_shadow(depth, input.v_pos_world);
				intensity *= shadow;
			}

			c.rgb = GlobalAmbient.rgb * c.rgb + 
				(diff * c.rgb * LightColor.rgb + 
				spec * LightColor.rgb) * intensity;
			return c;
		}
	}

    HLVS vs_deferred
    {
        cbuffer cbuffer0 : register(b0)
        {
            matrix WorldViewProjection;
        };

        cbuffer cbuffer1 : register(b1)
        {
            matrix World;
        };

        struct VS_INPUT
        {
            float4 Position : POSITION;
            float3 Normal : NORMAL;
            float4 Tangent : TANGENT;
            float2 Texcoord0 : TEXCOORD0;
            float2 Texcoord1 : TEXCOORD1;
        };

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
            float4 v_pos_proj : TEXCOORD1;
            float3 v_normal_world : TEXCOORD2;
        };

        PS_INPUT main(VS_INPUT input)
        {
            PS_INPUT output = (PS_INPUT) 0;

            output.v_pos = mul(input.Position, WorldViewProjection);
            output.v_uv = input.Texcoord0;
            output.v_pos_proj = output.v_pos;
            output.v_normal_world = mul(input.Normal, World);

            return output;
        }
    }

    HLPS ps_deferred
    {
        cbuffer cbuffer0 : register(b0)
        {
            float4 _Color;
        };

        Texture2D _MainTex : register(t0);
        SamplerState _MainTex_Sampler : register(s0);

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
            float4 v_pos_proj : TEXCOORD1;
            float3 v_normal_world : TEXCOORD2;
        };

        struct PS_OUTPUT
        {
            float4 o_color : SV_Target0;
            float4 o_normal : SV_Target1;
            float4 o_specular : SV_Target2;
            float4 o_depth : SV_Target3;
        };

        PS_OUTPUT main(PS_INPUT input)
        {
            PS_OUTPUT output = (PS_OUTPUT) 0;

            float4 color = _MainTex.Sample(_MainTex_Sampler, input.v_uv) * _Color;
            float3 normal = normalize(input.v_normal_world);

            // encode normal3 to normal2
            float2 normal_2 = 0;
            if(length(normal.xy) <= 0)
            {
                normal_2 = float2(normal.z * 10, 0);
            }
            else
            {
                normal_2 = normalize(normal.xy)*sqrt(normal.z*0.5+0.5);
            }

            /*
            // decode normal2 to normal3
            if(length(normal_2) > 1)
            {
                normal = float3(0, 0, sign(normal_2.x));
            }
            else
            {
                float len = length(normal_2);
                normal.z = len*len*2-1;
                normal.xy = normalize(normal_2.xy)*sqrt(1-normal.z*normal.z);
            }
            */

            float specular_power = 0;//1.0;
            float specular_intensity = 0;//0.5;

            output.o_color = color;
            output.o_normal.xy = normal_2;
            output.o_specular.z = specular_power;
            output.o_specular.w = specular_intensity;
            output.o_depth.r = input.v_pos_proj.z / input.v_pos_proj.w;

            return output;
        }
    }

    HLVS vs_depth
    {
        cbuffer cbuffer0 : register(b0)
        {
            matrix WorldViewProjection;
        };

        struct VS_INPUT
        {
            float4 Position : POSITION;
            float3 Normal : NORMAL;
            float4 Tangent : TANGENT;
            float2 Texcoord0 : TEXCOORD0;
            float2 Texcoord1 : TEXCOORD1;
        };

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
			float4 v_pos_proj : TEXCOORD1;
        };

        PS_INPUT main(VS_INPUT input)
        {
            PS_INPUT output = (PS_INPUT) 0;

            output.v_pos = mul(input.Position, WorldViewProjection);
			output.v_pos_proj = output.v_pos;

            return output;
        }
    }

	HLPS ps_depth
    {
		struct PS_INPUT
		{
			float4 v_pos : SV_POSITION;
			float4 v_pos_proj : TEXCOORD1;
		};

		float4 main(PS_INPUT input) : SV_Target
		{
			float4 c = 0.0;
			c.r = input.v_pos_proj.z / input.v_pos_proj.w;
			return c;
		}
    }

	GLVS vs
	{
		uniform mat4 WorldViewProjection;
		uniform mat4 World;
		uniform vec4 LightDirection;
		uniform vec4 EyePosition;

		attribute vec4 Position;
        attribute vec3 Normal;
        attribute vec4 Tangent;
		attribute vec2 Texcoord0;
        attribute vec2 Texcoord1;

		varying vec2 v_uv;
		varying vec3 v_light_dir_world;
		varying vec3 v_eye_dir_world;
		varying vec3 v_normal_world;
		varying vec4 v_pos_proj;
		varying vec4 v_pos_world;

		void main()
		{
			gl_Position = Position * WorldViewProjection;

			v_uv = Texcoord0;

            vec4 pos_world = Position * World;
            vec3 normal_world = (vec4(Normal, 0.0) * World).xyz;

            v_light_dir_world = - LightDirection.xyz;
            v_eye_dir_world = EyePosition.xyz - pos_world.xyz;
            v_normal_world = normal_world;
			v_pos_proj = gl_Position;
			v_pos_world = pos_world;
		}
	}

	GLPS ps
	{
		precision highp float;
		uniform vec4 GlobalAmbient;
		uniform vec4 LightColor;
		uniform vec4 _Color;
		uniform mat4 ViewProjectionLight[3];
		uniform vec4 ShadowParam;
		uniform vec4 _ZBufferParams;
		uniform vec4 ShadowMapTexel;
		uniform vec4 CascadeSplits;

		uniform sampler2D _MainTex;
		uniform sampler2D _ShadowMapTexture;

		varying vec2 v_uv;
		varying vec3 v_light_dir_world;
		varying vec3 v_eye_dir_world;
		varying vec3 v_normal_world;
		varying vec4 v_pos_proj;
		varying vec4 v_pos_world;

		float texture2DCompare(vec2 uv, float compare)
		{
			return step(compare * 0.5 + 0.5, texture2D(_ShadowMapTexture, uv).r);
		}

		float PCF(vec2 uv, vec2 size, float z)
		{
			float bias = ShadowParam.x;
			float strength = ShadowParam.y;
			float shadow_weak = clamp(1.0 - strength, 0.0, 1.0);
			float shadow = 0.0;
			int pcf_size = 1;

			for(int i=-pcf_size;i<=pcf_size;i++)
			{
				for(int j=-pcf_size;j<=pcf_size;j++)
				{
					vec2 off = vec2(float(i), float(j)) * size;
					float compare = texture2DCompare(uv + off, z - bias);

					if(compare < 1.0)
					{
						shadow += shadow_weak;
					}
					else
					{
						shadow += 1.0;
					}
				}
			}
			return shadow / float((pcf_size*2+1) * (pcf_size*2+1));
		}

		float sample_shadow(float depth, vec4 pos_world)
		{
			// shadow
			int cascade = int(ShadowParam.z);

			int index = -1;

			if(cascade == 1)
			{
				float linear_depth = 1.0 / (_ZBufferParams.x * depth + _ZBufferParams.y);
				if(linear_depth < CascadeSplits.x)
				{
					index = 0;
				}
				else if(linear_depth < CascadeSplits.y)
				{
					index = 1;
				}
				else if(linear_depth < CascadeSplits.z)
				{
					index = 2;
				}
			}
			else
			{
				index = 0;
			}

			float shadow = 1.0;

			if(index >= 0)
			{
				vec4 pos_light_4 = pos_world * ViewProjectionLight[index];
				vec3 pos_light = pos_light_4.xyz / pos_light_4.w;
				pos_light.z = min(1.0, pos_light.z);

				vec2 uv_shadow = vec2(0.0, 0.0);
				uv_shadow.x = 0.5 + pos_light.x * 0.5;
				uv_shadow.y = 0.5 + pos_light.y * 0.5;

				float tex_witdh = 1.0;
				float tex_height = 1.0;
				if(cascade == 1)
				{
					tex_witdh = 1.0 / 3.0;
					uv_shadow.x = float(index) * tex_witdh + uv_shadow.x * tex_witdh;
				}

				vec2 size = ShadowMapTexel.xy * vec2(tex_witdh, tex_height);

				if(uv_shadow.x > 0.0 && uv_shadow.x < 1.0 && uv_shadow.y > 0.0 && uv_shadow.y < 1.0)
				{
					shadow = PCF(uv_shadow, size, pos_light.z);
				}
			}

			return shadow;
		}

		void main()
		{
			float specular_power = 1.0;
            float specular_intensity = 0.5;

			vec4 c = texture2D(_MainTex, v_uv) * _Color;

			vec3 normal = normalize(v_normal_world);
            vec3 light_dir = normalize(v_light_dir_world);
            vec3 eye_dir = normalize(v_eye_dir_world);
            
            float diff = max(0.0, dot(normal, light_dir));
            vec3 h = normalize(light_dir + eye_dir);
            float nh = max(0.0, dot(normal, h));
            float spec = pow(nh, 128.0 * specular_power) * specular_intensity;

            float intensity = 1.0;
	
			int shadow_enable = int(ShadowParam.w);
			if(shadow_enable == 1)
			{
				float depth = v_pos_proj.z / v_pos_proj.w;
				depth = depth * 0.5 + 0.5;
				float shadow = sample_shadow(depth, v_pos_world);
				intensity *= shadow;
			}

			c.rgb = GlobalAmbient.rgb * c.rgb + 
				(diff * c.rgb * LightColor.rgb + 
				spec * LightColor.rgb) * intensity;
			
			gl_FragColor = c;
		}
	}

	GLVS vs_depth
	{
		uniform mat4 WorldViewProjection;

		attribute vec4 Position;
        attribute vec3 Normal;
        attribute vec4 Tangent;
		attribute vec2 Texcoord0;
        attribute vec2 Texcoord1;

		void main()
		{
			gl_Position = Position * WorldViewProjection;
		}
	}

	GLPS ps_depth
	{
		void main()
		{
		}
	}
}