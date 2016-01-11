ImageEffect/Highlighting
{
    Tags
    {
        Queue Geometry
    }

    Pass 0
    {
        VS vs_blur
        PS ps_blur
        RenderStates rs
    }

    Pass 1
    {
        VS vs_compose
        PS ps_compose
        RenderStates rs
    }

    RenderStates rs
    {
        Cull Off
        ZWrite Off
        ZTest Always
        Blend Off
    }

    HLVS vs_blur
    {
        cbuffer cbuffer0 : register(b0)
        {
            float4 OffsetScale;
        };

        cbuffer cbuffer1 : register(b1)
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
            float2 v_uv[4] : TEXCOORD0;
        };

        PS_INPUT main(VS_INPUT input)
        {
            PS_INPUT output = (PS_INPUT) 0;

            output.v_pos = input.Position;

            float2 offs = _MainTex_TexelSize.xy * OffsetScale.x;

            output.v_uv[0] = input.Texcoord0 + offs * float2(-1, -1);
            output.v_uv[1] = input.Texcoord0 + offs * float2(1, -1);
            output.v_uv[2] = input.Texcoord0 + offs * float2(1, 1);
            output.v_uv[3] = input.Texcoord0 + offs * float2(-1, 1);

            return output;
        }
    }

    HLPS ps_blur
    {
        cbuffer cbuffer0 : register(b0)
        {
            float4 Intensity;
        };

        Texture2D _MainTex : register(t0);
        SamplerState _MainTex_Sampler : register(s0);

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv[4] : TEXCOORD0;
        };

        float4 main(PS_INPUT input) : SV_Target
        {
            float4 color = _MainTex.Sample(_MainTex_Sampler, input.v_uv[0]);
            float4 color1 = _MainTex.Sample(_MainTex_Sampler, input.v_uv[1]);
            float4 color2 = _MainTex.Sample(_MainTex_Sampler, input.v_uv[2]);
            float4 color3 = _MainTex.Sample(_MainTex_Sampler, input.v_uv[3]);

            color.rgb = max(color.rgb, color1.rgb);
            color.rgb = max(color.rgb, color2.rgb);
            color.rgb = max(color.rgb, color3.rgb);
            color.a = (color.a + color1.a + color2.a + color3.a) * Intensity.x;
            
            return color;
        }
    }

    HLVS vs_compose
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

    HLPS ps_compose
    {
        Texture2D _MainTex : register(t0);
        SamplerState _MainTex_Sampler : register(s0);
        Texture2D TexHighlight : register(t1);
        SamplerState TexHighlight_Sampler : register(s1);
        Texture2D TexBlur : register(t2);
        SamplerState TexBlur_Sampler : register(s2);
        Texture2D _CameraDepthTexture : register(t3);
        SamplerState _CameraDepthTexture_Sampler : register(s3);
        Texture2D _HighlightingDepthTexture : register(t4);
        SamplerState _HighlightingDepthTexture_Sampler : register(s4);

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
        };

        float4 main(PS_INPUT input) : SV_Target
        {
            float4 color = _MainTex.Sample(_MainTex_Sampler, input.v_uv);
            float4 highlight = TexHighlight.Sample(TexHighlight_Sampler, input.v_uv);
            float depth_color = _CameraDepthTexture.Sample(_CameraDepthTexture_Sampler, input.v_uv).r;
            float depth_highlight = _HighlightingDepthTexture.Sample(_HighlightingDepthTexture_Sampler, input.v_uv).r;

            if(!any(highlight))
            {
                float4 blur = TexBlur.Sample(TexBlur_Sampler, input.v_uv);
                color.rgb = lerp(color.rgb, blur.rgb, saturate(blur.a - highlight.a));
            }

            return color;
        }
    }
}