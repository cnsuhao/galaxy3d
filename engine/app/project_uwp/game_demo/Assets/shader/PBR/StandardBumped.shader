PBR/StandardBumped
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
			float4 v_tangent_world : TEXCOORD4;
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
			output.v_tangent_world = mul(input.Tangent, World);

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
			float4 _Metallic;
		};

		cbuffer cbuffer4 : register(b4)
		{
			float4 _Smoothness;
		};

		#define PI 3.14159265

		Texture2D _MainTex : register(t0);
		SamplerState _MainTex_Sampler : register(s0);
		TextureCube _ReflectionMap : register(t1);
		SamplerState _ReflectionMap_Sampler : register(s1);
		Texture2D _NormalMap : register(t2);
		SamplerState _NormalMap_Sampler : register(s2);

		struct PS_INPUT
		{
			float4 v_pos : SV_POSITION;
			float2 v_uv : TEXCOORD0;
			float3 v_light_dir_world : TEXCOORD1;
			float3 v_eye_dir_world : TEXCOORD2;
			float3 v_normal_world : TEXCOORD3;
			float4 v_tangent_world : TEXCOORD4;
		};

		float SmithVisibilityTerm(float NdotL, float NdotV, float k)
		{
			float gL = NdotL * (1-k) + k;
			float gV = NdotV * (1-k) + k;
			return 1.0 / (gL * gV + 1e-4f);
		}

		float SmithGGXVisibilityTerm(float NdotL, float NdotV, float roughness)
		{
			float k = (roughness * roughness) / 2;
			return SmithVisibilityTerm(NdotL, NdotV, k);
		}

		float GGXTerm(float NdotH, float roughness)
		{
			float a = roughness * roughness;
			float a2 = a * a;
			float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
			return a2 / (PI * d * d);
		}

		float Pow5(float x)
		{
			return x*x * x*x * x;
		}

		float3 FresnelTerm(float3 F0, float cosA)
		{
			float t = Pow5 (1 - cosA);
			return F0 + (1-F0) * t;
		}

		float3 FresnelLerp(float3 F0, float3 F90, float cosA)
		{
			float t = Pow5 (1 - cosA);
			return lerp(F0, F90, t);
		}

		float4 main(PS_INPUT input) : SV_Target
		{
			float4 c = _MainTex.Sample(_MainTex_Sampler, input.v_uv) * _Color;

			float3 spec_color = lerp(float3(0.22, 0.22, 0.22), c.rgb, _Metallic.x);
			float one_minus_dielectric_spec = 0.779;
			float one_minus_reflectivity = one_minus_dielectric_spec - _Metallic.x * one_minus_dielectric_spec;
			c.rgb = c.rgb * one_minus_reflectivity;

			float3 normal = _NormalMap.Sample(_NormalMap_Sampler, input.v_uv).rgb;
			normal = normal * 2.0 - 1.0;
				
			float3 N = normalize(input.v_normal_world);
			float3 T = normalize(input.v_tangent_world.xyz - N * input.v_tangent_world.xyz * N);
			float3 B = cross(N, T);
			float3x3 T2W = float3x3(T, B, N);
			normal = normalize(mul(normal, T2W));

			float3 light_dir = normalize(input.v_light_dir_world);
			float3 eye_dir = normalize(input.v_eye_dir_world);

			float3 h = normalize(light_dir + eye_dir);
			float nl = max(0, dot(normal, light_dir));
			float nh = max(0, dot(normal, h));
			float nv = max(0, dot(normal, eye_dir));
			float lh = max(0, dot(light_dir, h));

			float roughness = 1 - _Smoothness.x;
			float V = SmithGGXVisibilityTerm(nl, nv, roughness);
			float D = GGXTerm(nh, roughness);

			float nlPow5 = Pow5(1-nl);
			float nvPow5 = Pow5(1-nv);
			float Fd90 = 0.5 + 2 * lh * lh * roughness;
			float disneyDiffuse = (1 + (Fd90-1) * nlPow5) * (1 + (Fd90-1) * nvPow5);

			float diffuseTerm = disneyDiffuse * nl;
			float specularTerm = max(0, (V * D * nl) * (PI / 4));
			float grazingTerm = saturate(_Smoothness.x + (1-one_minus_reflectivity));

			const float SPECCUBE_LOD_STEPS = 7;
			float mip = roughness * SPECCUBE_LOD_STEPS;
			float3 uv_cube = (2.0 * (dot (normal, eye_dir) * normal)) - eye_dir;

#ifdef SM409
			float4 gi_specular = _ReflectionMap.Sample(_ReflectionMap_Sampler, uv_cube);
#else
			float4 gi_specular = _ReflectionMap.SampleLevel(_ReflectionMap_Sampler, uv_cube, mip);
#endif

			float3 ambient = c.rgb * GlobalAmbient.rgb;
			float3 diffuse = c.rgb * (LightColor.rgb * diffuseTerm);
			float3 spec = specularTerm * LightColor.rgb * FresnelTerm(spec_color, lh);
			float3 gi = gi_specular.rgb * FresnelLerp(spec_color, grazingTerm, max(nv, 0.25));

			spec = spec * roughness * 0.5;

			c.rgb =	ambient + diffuse + spec + gi;

			return c;
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
			float4 c = float4(0.0);
			c.r = input.v_pos_proj.z / input.v_pos_proj.w;
			return c;
		}
    }

	GLVS vs
    {#version 300 es
		uniform mat4 WorldViewProjection;
		uniform mat4 World;
		uniform vec4 LightDirection;
		uniform vec4 EyePosition;

        in vec4 Position;
        in vec3 Normal;
        in vec4 Tangent;
        in vec2 Texcoord0;
        in vec2 Texcoord1;

        out vec2 v_uv;
		out vec3 v_light_dir_world;
		out vec3 v_eye_dir_world;
		out vec3 v_normal_world;
		
        void main()
        {
            gl_Position = Position * WorldViewProjection;
            v_uv = Texcoord0;

			vec4 pos_world = Position * World;
			vec3 normal_world = (vec4(Normal, 0) * World).xyz;

			v_light_dir_world = - LightDirection.xyz;
			v_eye_dir_world = EyePosition.xyz - pos_world.xyz;
			v_normal_world = normal_world;
        }
    }

    GLPS ps
    {#version 300 es
		precision mediump float;
		uniform vec4 GlobalAmbient;
		uniform vec4 LightColor;
		uniform vec4 _Color;
		uniform vec4 _Metallic;
		uniform vec4 _Smoothness;
        uniform sampler2D _MainTex;
		uniform samplerCube _ReflectionMap;

        in vec2 v_uv;
		in vec3 v_light_dir_world;
		in vec3 v_eye_dir_world;
		in vec3 v_normal_world;
		out vec4 o_color;

#define PI 3.14159265
#define float3 vec3
#define float4 vec4
#define saturate(x) min(1.0, max(0.0, x))
#define lerp mix

		float SmithVisibilityTerm(float NdotL, float NdotV, float k)
		{
			float gL = NdotL * (1.0-k) + k;
			float gV = NdotV * (1.0-k) + k;
			return 1.0 / (gL * gV + 1e-4);
		}

		float SmithGGXVisibilityTerm(float NdotL, float NdotV, float roughness)
		{
			float k = (roughness * roughness) / 2.0;
			return SmithVisibilityTerm(NdotL, NdotV, k);
		}

		float GGXTerm(float NdotH, float roughness)
		{
			float a = roughness * roughness;
			float a2 = a * a;
			float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
			return a2 / (PI * d * d);
		}

		float Pow5(float x)
		{
			return x*x * x*x * x;
		}

		float3 FresnelTerm(float3 F0, float cosA)
		{
			float t = Pow5 (1.0 - cosA);
			return F0 + (1.0-F0) * t;
		}

		float3 FresnelLerp(float3 F0, float3 F90, float cosA)
		{
			float t = Pow5 (1.0 - cosA);
			return lerp(F0, F90, t);
		}

        void main()
        {
            vec4 c = texture(_MainTex, v_uv) * _Color;

			float3 spec_color = lerp(float3(0.22, 0.22, 0.22), c.rgb, _Metallic.x);
			float one_minus_dielectric_spec = 0.779;
			float one_minus_reflectivity = one_minus_dielectric_spec - _Metallic.x * one_minus_dielectric_spec;
			c.rgb = c.rgb * one_minus_reflectivity;

			float3 normal = normalize(v_normal_world);
			float3 light_dir = normalize(v_light_dir_world);
			float3 eye_dir = normalize(v_eye_dir_world);

			float3 h = normalize(light_dir + eye_dir);
			float nl = max(0.0, dot(normal, light_dir));
			float nh = max(0.0, dot(normal, h));
			float nv = max(0.0, dot(normal, eye_dir));
			float lh = max(0.0, dot(light_dir, h));

			float roughness = 1.0 - _Smoothness.x;
			float V = SmithGGXVisibilityTerm(nl, nv, roughness);
			float D = GGXTerm(nh, roughness);

			float nlPow5 = Pow5(1.0-nl);
			float nvPow5 = Pow5(1.0-nv);
			float Fd90 = 0.5 + 2.0 * lh * lh * roughness;
			float disneyDiffuse = (1.0 + (Fd90-1.0) * nlPow5) * (1.0 + (Fd90-1.0) * nvPow5);

			float diffuseTerm = disneyDiffuse * nl;
			float specularTerm = max(0.0, (V * D * nl) * (PI / 4.0));
			float grazingTerm = saturate(_Smoothness.x + (1.0-one_minus_reflectivity));

			const float SPECCUBE_LOD_STEPS = 7.0;
			float mip = roughness * SPECCUBE_LOD_STEPS;
			float3 uv_cube = (2.0 * (dot (normal, eye_dir) * normal)) - eye_dir;
			// gles 2
			//float4 gi_specular = textureCube(_ReflectionMap, uv_cube);
			// gles 3
			float4 gi_specular = textureLod(_ReflectionMap, uv_cube, mip);

			float3 ambient = c.rgb * GlobalAmbient.rgb;
			float3 diffuse = c.rgb * (LightColor.rgb * diffuseTerm);
			float3 spec = specularTerm * LightColor.rgb * FresnelTerm(spec_color, lh);
			float3 gi = gi_specular.rgb * FresnelLerp(spec_color, float3(grazingTerm), max(nv, 0.25));

			spec = spec * roughness * 0.5;

			c.rgb =	ambient + diffuse + spec + gi;

            o_color = c;
        }
    }
}