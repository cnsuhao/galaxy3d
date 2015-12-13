ImageEffect/ToneMapping
{
    Tags
    {
        Queue Geometry
    }

    Pass 0
    {
        VS vs
        PS ps_log
        RenderStates rs
    }

    Pass 1
    {
        VS vs
        PS ps_exp
        RenderStates rs
    }

    Pass 2
    {
        VS vs
        PS ps_exp
        RenderStates rs_blend
    }

    Pass 3
    {
        VS vs
        PS ps_adaptive
        RenderStates rs
    }

    RenderStates rs
    {
        Cull Off
        ZWrite Off
        ZTest Always
        Blend Off
    }

    RenderStates rs_blend
    {
        Cull Off
        ZWrite Off
        ZTest Always
        Blend SrcAlpha OneMinusSrcAlpha
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

    HLPS ps_log
    {
        cbuffer cbuffer0 : register(b0)
        {
            float4 _MainTex_TexelSize;
        };

        Texture2D _MainTex : register(t0);
        SamplerState _MainTex_Sampler : register(s0);

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
        };

        float luminance(float3 c)
        {
            return dot(c, float3(0.22, 0.707, 0.071));
        }

        float4 main(PS_INPUT input) : SV_Target
        {
            const float DELTA = 0.0001f;
            float fLogLumSum = 0.0f;

            fLogLumSum += log(luminance(_MainTex.Sample(_MainTex_Sampler, input.v_uv + _MainTex_TexelSize.xy * float2(-1,-1)).rgb) + DELTA);		
            fLogLumSum += log(luminance(_MainTex.Sample(_MainTex_Sampler, input.v_uv + _MainTex_TexelSize.xy * float2(1,1)).rgb) + DELTA);		
            fLogLumSum += log(luminance(_MainTex.Sample(_MainTex_Sampler, input.v_uv + _MainTex_TexelSize.xy * float2(-1,1)).rgb) + DELTA);		
            fLogLumSum += log(luminance(_MainTex.Sample(_MainTex_Sampler, input.v_uv + _MainTex_TexelSize.xy * float2(1,-1)).rgb) + DELTA);		

            float avg = fLogLumSum / 4.0;
            return float4(avg, avg, avg, avg);
        }
    }

    HLPS ps_exp
    {
        cbuffer cbuffer0 : register(b0)
        {
            float4 _MainTex_TexelSize;
        };

        cbuffer cbuffer1 : register(b1)
        {
            float4 _AdaptionSpeed;
        };

        Texture2D _MainTex : register(t0);
        SamplerState _MainTex_Sampler : register(s0);

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
        };

        float4 main(PS_INPUT input) : SV_Target
        {
            float2 lum = float2(0.0f, 0.0f);

            lum += _MainTex.Sample(_MainTex_Sampler, input.v_uv + _MainTex_TexelSize.xy * float2(-1,-1)).xy;	
            lum += _MainTex.Sample(_MainTex_Sampler, input.v_uv  + _MainTex_TexelSize.xy * float2(1,1)).xy;	
            lum += _MainTex.Sample(_MainTex_Sampler, input.v_uv + _MainTex_TexelSize.xy * float2(1,-1)).xy;	
            lum += _MainTex.Sample(_MainTex_Sampler, input.v_uv  + _MainTex_TexelSize.xy * float2(-1,1)).xy;	

            lum = exp(lum / 4.0f);

            return float4(lum.x, lum.x, lum.x, saturate(0.0125 * _AdaptionSpeed.x));
        }
    }

    HLPS ps_adaptive
    {
        cbuffer cbuffer0 : register(b0)
        {
            float4 _HdrParams;
        };

        Texture2D _MainTex : register(t0);
        SamplerState _MainTex_Sampler : register(s0);
        Texture2D _SmallTex : register(t1);
        SamplerState _SmallTex_Sampler : register(s1);

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
        };

        float luminance(float3 c)
        {
            return dot(c, float3(0.22, 0.707, 0.071));
        }

        float4 main(PS_INPUT input) : SV_Target
        {
            float avgLum = _SmallTex.Sample(_SmallTex_Sampler, input.v_uv).r;
            float4 color = _MainTex.Sample(_MainTex_Sampler, input.v_uv);

            float cieLum = max(0.000001, luminance(color.rgb));

            float lumScaled = cieLum * _HdrParams.z / (0.001 + avgLum);

            lumScaled = (lumScaled * (1.0f + lumScaled / (_HdrParams.w)))/(1.0f + lumScaled);
            color.rgb = color.rgb * (lumScaled / cieLum);

            return color;
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