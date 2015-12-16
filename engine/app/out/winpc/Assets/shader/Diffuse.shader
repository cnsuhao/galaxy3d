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
            float specular_power = 1.0;
            float specular_intensity = 0.5;

			float4 c = _MainTex.Sample(_MainTex_Sampler, input.v_uv) * input.v_color;

            float3 normal = normalize(input.v_normal_world);
            float3 light_dir = normalize(input.v_light_dir_world);
            float3 eye_dir = normalize(input.v_eye_dir_world);

            float diff = max(0, dot(normal, light_dir));
            float3 h = normalize(light_dir + eye_dir);
            float nh = max(0, dot(normal, h));
            float spec = pow(nh, 128 * specular_power) * specular_intensity;

            c.rgb = GlobalAmbient.rgb * c.rgb + 
                diff * c.rgb * LightColor.rgb + 
                spec * LightColor.rgb;
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
            float4 _Color;
        };

        cbuffer cbuffer2 : register(b2)
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
            float4 v_color : COLOR;
            float3 v_pos_world : TEXCOORD1;
            float3 v_normal_world : TEXCOORD2;
        };

        PS_INPUT main(VS_INPUT input)
        {
            PS_INPUT output = (PS_INPUT) 0;

            output.v_pos = mul(input.Position, WorldViewProjection);
            output.v_uv = input.Texcoord0;
            output.v_color = _Color;

            float3 pos_world = mul(input.Position, World).xyz;
            float3 normal_world = mul(input.Normal, World);

            output.v_pos_world = pos_world;
            output.v_normal_world = normal_world;

            return output;
        }
    }

    HLPS ps_deferred
    {
        cbuffer cbuffer0 : register(b0)
        {
            float4 GlobalAmbient;
        };

        Texture2D _MainTex : register(t0);
        SamplerState _MainTex_Sampler : register(s0);

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
            float4 v_color : COLOR;
            float3 v_pos_world : TEXCOORD1;
            float3 v_normal_world : TEXCOORD2;
        };

        struct PS_OUTPUT
        {
            float4 o_color : SV_Target0;
            float4 o_normal : SV_Target1;
            float4 o_specular : SV_Target2;
        };

        PS_OUTPUT main(PS_INPUT input)
        {
            PS_OUTPUT output = (PS_OUTPUT) 0;

            float4 color = _MainTex.Sample(_MainTex_Sampler, input.v_uv) * input.v_color;
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

            float specular_power = 1.0;
            float specular_intensity = 0.5;

            output.o_color = color;
            output.o_normal.xy = normal_2;
            output.o_specular.z = specular_power;
            output.o_specular.w = specular_intensity;

            return output;
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