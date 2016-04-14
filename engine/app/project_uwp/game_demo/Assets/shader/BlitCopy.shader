BlitCopy
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
        Cull Off
        ZWrite Off
        ZTest Always
        Blend Off
    }

    HLVS vs
    {
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

            output.v_pos = input.Position;
            output.v_uv = input.Texcoord0;

            return output;
        }
    }

    HLPS ps
    {
        Texture2D _MainTex : register(t0);
        SamplerState _MainTex_Sampler : register(s0);

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
        };

        float4 main(PS_INPUT input) : SV_Target
        {
            float4 c = _MainTex.Sample(_MainTex_Sampler, input.v_uv);
            return c;
        }
    }

    GLVS vs
    {
        attribute vec4 Position;
        attribute vec3 Normal;
        attribute vec4 Tangent;
        attribute vec2 Texcoord0;
        attribute vec2 Texcoord1;

        varying vec2 v_uv;

        void main()
        {
            gl_Position = Position;
            v_uv = Texcoord0;
			v_uv.y = 1.0 - v_uv.y;
        }
    }

    GLPS ps
    {
		precision mediump float;
        uniform sampler2D _MainTex;

        varying vec2 v_uv;

        void main()
        {
            vec4 c = texture2D(_MainTex, v_uv);

            gl_FragColor = c;
        }
    }
}