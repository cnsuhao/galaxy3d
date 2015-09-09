Lightmap/Transparent/Cutout/Diffuse
{
	Tags
	{
		Queue AlphaTest
	}

    Pass front
    {
        VS vs
        PS ps
        RenderStates rs_front
    }

	Pass back
	{
		VS vs
		PS ps
		RenderStates rs_back
	}

	RenderStates rs_front
	{
        Cull Back
	}

    RenderStates rs_back
    {
        Cull Front
        ZTest Less
    }

	HLVS vs
	{
		cbuffer cbuffer0 : register( b0 )
		{
			matrix WorldViewProjection;
		};

        cbuffer cbuffer1 : register( b1 )
        {
            float4 _Color;
        };

        cbuffer cbuffer2 : register( b2 )
        {
            float4 _LightmapST;
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
            float2 v_uv_2 : TEXCOORD1;
            float4 v_color : COLOR;
		};

		PS_INPUT main( VS_INPUT input )
		{
			PS_INPUT output = (PS_INPUT) 0;

			output.v_pos = mul( input.Position, WorldViewProjection );
			output.v_uv = input.Texcoord0;
            output.v_uv_2 = input.Texcoord1 * _LightmapST.xy + _LightmapST.zw;
            output.v_uv_2.y = 1.0 - output.v_uv_2.y;
            output.v_color = _Color;

			return output;
		}
	}

	HLPS ps
	{
        cbuffer cbuffer0 : register( b0 )
        {
            float4 _Cutoff;
        };
        
		Texture2D _MainTex : register( t0 );
		SamplerState _MainTex_Sampler : register( s0 );
        Texture2D _Lightmap : register(t1);
        SamplerState _Lightmap_Sampler : register(s1);

		struct PS_INPUT
		{
			float4 v_pos : SV_POSITION;
			float2 v_uv : TEXCOORD0;
            float2 v_uv_2 : TEXCOORD1;
            float4 v_color : COLOR;
		};

		float4 main( PS_INPUT input) : SV_Target
		{
			float4 c = _MainTex.Sample(_MainTex_Sampler, input.v_uv) * input.v_color;
            clip(c.a - _Cutoff);
            c.rgb = c.rgb * _Lightmap.Sample( _Lightmap_Sampler, input.v_uv_2 ).rgb * 2;
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