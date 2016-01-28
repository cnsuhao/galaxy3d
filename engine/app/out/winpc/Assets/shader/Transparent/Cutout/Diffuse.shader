Transparent/Cutout/Diffuse
{
	Tags
	{
		Queue AlphaTest
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
		cbuffer cbuffer0 : register( b0 )
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
			float2 v_uv : TEXCOORD0;
		};

		PS_INPUT main( VS_INPUT input )
		{
			PS_INPUT output = (PS_INPUT) 0;

			output.v_pos = mul( input.Position, WorldViewProjection );
			output.v_uv = input.Texcoord0;

			return output;
		}
	}

	HLPS ps
	{
        cbuffer cbuffer0 : register( b0 )
        {
            float4 _Cutoff;
        };

        cbuffer cbuffer1 : register( b1 )
        {
            float4 _Color;
        };
        
		Texture2D _MainTex : register( t0 );
		SamplerState _MainTex_Sampler : register( s0 );

		struct PS_INPUT
		{
			float4 v_pos : SV_POSITION;
			float2 v_uv : TEXCOORD0;
		};

		float4 main( PS_INPUT input) : SV_Target
		{
			float4 c = _MainTex.Sample(_MainTex_Sampler, input.v_uv) * _Color;
            clip(c.a - _Cutoff);
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
        cbuffer cbuffer0 : register( b0 )
        {
            float4 _Cutoff;
        };

        cbuffer cbuffer1 : register( b1 )
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
            clip(color.a - _Cutoff);

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
            float2 v_uv : TEXCOORD0;
        };

        PS_INPUT main(VS_INPUT input)
        {
            PS_INPUT output = (PS_INPUT) 0;

            output.v_pos = mul(input.Position, WorldViewProjection);
            output.v_uv = input.Texcoord0;

            return output;
        }
    }

    HLPS ps_depth
    {
        cbuffer cbuffer0 : register( b0 )
        {
            float4 _Cutoff;
        };

        cbuffer cbuffer1 : register( b1 )
        {
            float4 _Color;
        };

        Texture2D _MainTex : register( t0 );
        SamplerState _MainTex_Sampler : register( s0 );

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
        };

        float4 main( PS_INPUT input) : SV_Target
        {
            float4 c = _MainTex.Sample(_MainTex_Sampler, input.v_uv) * _Color;
            clip(c.a - _Cutoff);
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