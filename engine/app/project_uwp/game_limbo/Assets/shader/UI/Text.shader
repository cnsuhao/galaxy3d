UI/Text
{
	Tags
	{
		Queue Transparent
	}

	Pass
	{
		VS vs
		PS ps
		RenderStates rs
	}

	RenderStates rs
	{
		Cull Off
		ZWrite Off
		ZTest Always
		Offset -1, -1
		Blend SrcAlpha OneMinusSrcAlpha
		BlendOp Add
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
			float4 Color : COLOR;
			float2 Texcoord0 : TEXCOORD0;
		};

		struct PS_INPUT
		{
			float4 v_pos : SV_POSITION;
			float2 v_uv : TEXCOORD0;
			float4 v_color : COLOR;
            float4 v_pos_proj : TEXCOORD1;
		};

		PS_INPUT main(VS_INPUT input)
		{
			PS_INPUT output = (PS_INPUT) 0;

			output.v_pos = mul(input.Position, WorldViewProjection);
			output.v_uv = input.Texcoord0;
			output.v_color = input.Color;
            output.v_pos_proj = output.v_pos;

			return output;
		}
	}

	HLPS ps
	{
        cbuffer cbuffer0 : register(b0)
        {
            float4 _Color;
        };

        cbuffer cbuffer1 : register(b1)
        {
            float4 ClipRect;
        };

        cbuffer cbuffer2 : register(b2)
        {
            float4 ClipSoft;
        };

		Texture2D _MainTex : register(t0);
		SamplerState _MainTex_Sampler : register(s0);

		struct PS_INPUT
		{
			float4 v_pos : SV_POSITION;
			float2 v_uv : TEXCOORD0;
			float4 v_color : COLOR;
            float4 v_pos_proj : TEXCOORD1;
		};

		float4 main(PS_INPUT input) : SV_Target
		{
			float4 c = input.v_color * _Color;
			c.a *= _MainTex.Sample( _MainTex_Sampler, input.v_uv ).r;

            float2 pos = input.v_pos_proj.xy / input.v_pos_proj.w;

            float alpha = 1.0;
            if(pos.y > ClipSoft.y)
            {
                alpha = (ClipRect.y - pos.y) / (ClipRect.y - ClipSoft.y);
            }
            else if(pos.y < ClipSoft.w)
            {
                alpha = (ClipRect.w - pos.y) / (ClipRect.w - ClipSoft.w);
            }
            if(pos.x > ClipSoft.z)
            {
                alpha *= (ClipRect.z - pos.x) / (ClipRect.z - ClipSoft.z);
            }
            else if(pos.x < ClipSoft.x)
            {
                alpha *= (ClipRect.x - pos.x) / (ClipRect.x - ClipSoft.x);
            }
            c.a *= alpha;

            if( pos.x - ClipRect.x < 0 ||
                ClipRect.z - pos.x < 0 ||
                ClipRect.y - pos.y < 0 ||
                pos.y - ClipRect.w < 0)
            {
                clip(-1);
            }

			return c;
		}
	}

	GLVS vs
	{
		uniform mat4 WorldViewProjection;

		attribute vec4 Position;
		attribute vec4 Color;
		attribute vec2 Texcoord0;

		varying vec2 v_uv;
		varying vec4 v_color;
		varying vec4 v_pos_proj;

		void main()
		{
			gl_Position = Position * WorldViewProjection;
			v_uv = Texcoord0;
			v_color = Color;
			v_pos_proj = gl_Position;
		}
	}

	GLPS ps
	{
		precision mediump float;
		uniform vec4 _Color;
		uniform vec4 ClipRect;
		uniform vec4 ClipSoft;
		uniform sampler2D _MainTex;

		varying vec2 v_uv;
		varying vec4 v_color;
		varying vec4 v_pos_proj;

		void main()
		{
			vec4 c = v_color * _Color;
			c.a *= texture2D(_MainTex, v_uv).a;

			gl_FragColor = c;
		}
	}
}