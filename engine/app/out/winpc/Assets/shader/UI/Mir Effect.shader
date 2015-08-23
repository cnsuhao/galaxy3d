UI/Shader Mir Effect
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
		Blend One OneMinusSrcColor
		BlendOp Add
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
		cbuffer cbuffer0 : register( b0 )
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
		};

		PS_INPUT main( VS_INPUT input )
		{
			PS_INPUT output = (PS_INPUT) 0;

			output.v_pos = mul( input.Position, WorldViewProjection );
			output.v_uv = input.Texcoord0;
			output.v_color = input.Color;

			output.v_uv.y = 1.0 - output.v_uv.y;
    
			return output;
		}
	}

	HLPS ps
	{
		Texture2D _MainTex : register( t0 );
		SamplerState _MainTex_Sampler : register( s0 );
		Texture2D ColorTable : register( t1 );
		SamplerState ColorTable_Sampler : register( s1 );

		struct PS_INPUT
		{
			float4 v_pos : SV_POSITION;
			float2 v_uv : TEXCOORD0;
			float4 v_color : COLOR;
		};

		float4 main( PS_INPUT input) : SV_Target
		{
			float x = _MainTex.Sample( _MainTex_Sampler, input.v_uv ).r;
			float4 c = ColorTable.Sample( ColorTable_Sampler, float2(x, 1.0/255) ) * input.v_color;

			return c;
		}
	}

	GLVS vs
	{
		uniform mat4 WorldViewProjection;

		attribute vec4 Position;
		attribute vec2 Texcoord0;
		attribute vec4 Color;

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