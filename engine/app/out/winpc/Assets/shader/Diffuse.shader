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

	RenderStates rs
	{
        Cull Back
		ZWrite On
		ZTest LEqual
		Offset 0, 0
		Blend Off
		Stencil
		{
			Ref referenceValue
			ReadMask readMask
			WriteMask writeMask
			Comp comparisonFunction
			Pass stencilOperation
			Fail stencilOperation
			ZFail stencilOperation
		}
	}

	HLVS vs
	{
		cbuffer cbuffer0 : register(b0)
		{
			matrix WorldViewProjection;
		};

        cbuffer cbuffer1 : register(b1)
        {
            float4 _Color;
        };

        cbuffer cbuffer2 : register(b2)
        {
            matrix World;
        };

        cbuffer cbuffer3 : register(b3)
        {
            float4 LightDirection;
        };

        cbuffer cbuffer4 : register(b4)
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
            float4 v_color : COLOR;
            float3 v_light_dir_world : TEXCOORD1;
            float3 v_eye_dir_world : TEXCOORD2;
            float3 v_normal_world : TEXCOORD3;
		};

		PS_INPUT main(VS_INPUT input)
		{
			PS_INPUT output = (PS_INPUT) 0;

			output.v_pos = mul(input.Position, WorldViewProjection);
			output.v_uv = input.Texcoord0;
            output.v_color = _Color;

            float4 pos_world = mul(input.Position, World);
            float3 normal_world = mul(input.Normal, World);

            output.v_light_dir_world = - LightDirection.xyz;
            output.v_eye_dir_world = EyePosition.xyz - pos_world.xyz;
            output.v_normal_world = normal_world;

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

		Texture2D _MainTex : register(t0);
		SamplerState _MainTex_Sampler : register(s0);

		struct PS_INPUT
		{
			float4 v_pos : SV_POSITION;
			float2 v_uv : TEXCOORD0;
            float4 v_color : COLOR;
            float3 v_light_dir_world : TEXCOORD1;
            float3 v_eye_dir_world : TEXCOORD2;
            float3 v_normal_world : TEXCOORD3;
		};

		float4 main(PS_INPUT input) : SV_Target
		{
            float specular = 1.0;
            float3 spec_color = 0.5;

			float4 c = _MainTex.Sample(_MainTex_Sampler, input.v_uv) * input.v_color;

            float3 normal = normalize(input.v_normal_world);
            float3 light_dir = normalize(input.v_light_dir_world);
            float3 eye_dir = normalize(input.v_eye_dir_world);

            float diff = max(0, dot(normal, light_dir));
            float3 h = normalize(light_dir + eye_dir);
            float nh = max(0, dot(normal, h));
            float spec = pow(nh, 128 * specular) * c.a;

            c.rgb = GlobalAmbient.rgb * c.rgb + 
                diff * c.rgb * LightColor.rgb + 
                spec * spec_color * LightColor.rgb;
			return c;
		}
	}

	GLVS vs
	{
		uniform mat4 WorldViewProjection;

		attribute vec3 Position;
        attribute vec3 Normal;
        attribute vec4 Tangent;
        attribute vec4 Color;
		attribute vec2 Texcoord0;
        attribute vec2 Texcoord1;

		varying vec2 v_uv;
		varying vec4 v_color;

		void main()
		{
			gl_Position = WorldViewProjection * Position;
			v_uv = Texcoord0;
			v_color = Color;
		}
	}

	GLPS ps
	{
		uniform sampler2D _MainTex;

		varying vec2 v_uv;
		varying vec4 v_color;

		void main()
		{
			vec4 color = texture2D(_MainTex, v_uv) * v_color;

			gl_FragColor = color;
		}
	}
}