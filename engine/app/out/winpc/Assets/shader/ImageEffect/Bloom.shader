ImageEffect/Bloom
{
    Tags
    {
        Queue Geometry
    }

    Pass 0
    {
        VS vs_multi_tap
        PS ps_multi_tap_blur
        RenderStates rs
    }

    Pass 1
    {
        VS vs
        PS ps_bright
        RenderStates rs
    }

    Pass 2
    {
        VS vs_blur
        PS ps_blur
        RenderStates rs
    }

    Pass 3
    {
        VS vs
        PS ps_add
        RenderStates rs
    }

    RenderStates rs
    {
        Cull Off
        ZWrite Off
        ZTest Always
        Blend Off
    }

    HLVS vs_multi_tap
    {
        cbuffer cbuffer0 : register(b0)
        {
            float4 _MainTex_TexelSize;
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
            float2 v_uv[5] : TEXCOORD0;
        };

        PS_INPUT main(VS_INPUT input)
        {
            PS_INPUT output = (PS_INPUT) 0;

            output.v_pos = input.Position;
            output.v_uv[4] = input.Texcoord0;
            output.v_uv[0] = input.Texcoord0 + _MainTex_TexelSize.xy * 0.5;
            output.v_uv[1] = input.Texcoord0 - _MainTex_TexelSize.xy * 0.5;	
            output.v_uv[2] = input.Texcoord0 - _MainTex_TexelSize.xy * half2(1,-1) * 0.5;	
            output.v_uv[3] = input.Texcoord0 + _MainTex_TexelSize.xy * half2(1,-1) * 0.5;	

            return output;
        }
    }

    HLPS ps_multi_tap_blur
    {
        Texture2D _MainTex : register(t0);
        SamplerState _MainTex_Sampler : register(s0);

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv[5] : TEXCOORD0;
        };

        float4 main(PS_INPUT input) : SV_Target
        {
            half4 outColor = 0;
            outColor += _MainTex.Sample(_MainTex_Sampler, input.v_uv[0]);
            outColor += _MainTex.Sample(_MainTex_Sampler, input.v_uv[1]);
            outColor += _MainTex.Sample(_MainTex_Sampler, input.v_uv[2]);
            outColor += _MainTex.Sample(_MainTex_Sampler, input.v_uv[3]);
            return outColor/4;
        }
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

    HLPS ps_bright
    {
        cbuffer cbuffer0 : register(b0)
        {
            float4 _Threshhold;
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
            float4 color = _MainTex.Sample(_MainTex_Sampler, input.v_uv);
            color.rgb = max(float3(0,0,0), color.rgb - _Threshhold.xxx);
            return color;
        }
    }

    HLVS vs_blur
    {
        cbuffer cbuffer0 : register(b0)
        {
            float4 _Offsets;
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
            float4 v_uv01 : TEXCOORD1;
            float4 v_uv23 : TEXCOORD2;
            float4 v_uv45 : TEXCOORD3;
            float4 v_uv67 : TEXCOORD4;
        };

        PS_INPUT main(VS_INPUT input)
        {
            PS_INPUT output = (PS_INPUT) 0;

            output.v_pos = input.Position;
            output.v_uv = input.Texcoord0;
            output.v_uv01 =  input.Texcoord0.xyxy + _Offsets.xyxy * float4(1, 1, -1, -1);
            output.v_uv23 =  input.Texcoord0.xyxy + _Offsets.xyxy * float4(1, 1, -1, -1) * 2.0;
            output.v_uv45 =  input.Texcoord0.xyxy + _Offsets.xyxy * float4(1, 1, -1, -1) * 3.0;
            output.v_uv67 =  input.Texcoord0.xyxy + _Offsets.xyxy * float4(1, 1, -1, -1) * 4.0;

            return output;
        }
    }

    HLPS ps_blur
    {
        cbuffer cbuffer0 : register(b0)
        {
            float4 _Threshhold;
        };

        Texture2D _MainTex : register(t0);
        SamplerState _MainTex_Sampler : register(s0);

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
            float4 v_uv01 : TEXCOORD1;
            float4 v_uv23 : TEXCOORD2;
            float4 v_uv45 : TEXCOORD3;
            float4 v_uv67 : TEXCOORD4;
        };

        float4 main(PS_INPUT input) : SV_Target
        {
            float4 color = float4(0, 0, 0, 0);
            color += 0.225 * _MainTex.Sample(_MainTex_Sampler, input.v_uv);
            color += 0.150 * _MainTex.Sample(_MainTex_Sampler, input.v_uv01.xy);
            color += 0.150 * _MainTex.Sample(_MainTex_Sampler, input.v_uv01.zw);
            color += 0.110 * _MainTex.Sample(_MainTex_Sampler, input.v_uv23.xy);
            color += 0.110 * _MainTex.Sample(_MainTex_Sampler, input.v_uv23.zw);
            color += 0.075 * _MainTex.Sample(_MainTex_Sampler, input.v_uv45.xy);
            color += 0.075 * _MainTex.Sample(_MainTex_Sampler, input.v_uv45.zw);	
            color += 0.0525 * _MainTex.Sample(_MainTex_Sampler, input.v_uv67.xy);
            color += 0.0525 * _MainTex.Sample(_MainTex_Sampler, input.v_uv67.zw);
            return color;
        }
    }

    HLPS ps_add
    {
        cbuffer cbuffer0 : register(b0)
        {
            float4 _Intensity;
        };

        Texture2D _MainTex : register(t0);
        SamplerState _MainTex_Sampler : register(s0);
        Texture2D _ColorBuffer : register(t1);
        SamplerState _ColorBuffer_Sampler : register(s1);
        
        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
        };

        float4 main(PS_INPUT input) : SV_Target
        {
            float4 addedbloom = _MainTex.Sample(_MainTex_Sampler, input.v_uv);
            float4 screencolor = _ColorBuffer.Sample(_ColorBuffer_Sampler, input.v_uv);
            return _Intensity.x * addedbloom + screencolor;
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