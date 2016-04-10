SkyBox
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
        Cull Front
        ZWrite Off
        ZTest LEqual
        Blend Off
    }

    HLVS vs
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
			float3 v_uv : POSITION;
		};

		PS_INPUT main(VS_INPUT input)
		{
			PS_INPUT output = (PS_INPUT) 0;

			output.v_pos = mul(input.Position, WorldViewProjection).xyww;
			output.v_uv = input.Position.xyz;

			return output;
		}
    }

    HLPS ps
    {
        TextureCube _MainTex : register(t0);
		SamplerState _MainTex_Sampler : register(s0);

		struct PS_INPUT
		{
			float4 v_pos : SV_POSITION;
			float3 v_uv : POSITION;
		};

		float4 main(PS_INPUT input) : SV_Target
		{
			float4 c = _MainTex.Sample(_MainTex_Sampler, input.v_uv);
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