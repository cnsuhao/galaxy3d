DeferredShading
{
    Tags
    {
        Queue Geometry
    }

    Pass 0
    {
        VS vs
        PS ps
        RenderStates rs
    }

    Pass 1
    {
        VS vs_point
        PS ps_point
        RenderStates rs_point
    }

    RenderStates rs
    {
        Cull Off
        ZWrite Off
        ZTest Always
        Blend Off
    }

    RenderStates rs_point
    {
        Cull Back
        ZWrite On
        ZTest LEqual
        Blend Off
    }

    HLVS vs
    {
        cbuffer cbuffer0 : register(b0)
        {
            float4 LightDirection;
        };

        cbuffer cbuffer1 : register(b1)
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
            float3 v_light_dir_world : TEXCOORD1;
            float3 v_eye_pos_world : TEXCOORD2;
        };

        PS_INPUT main(VS_INPUT input)
        {
            PS_INPUT output = (PS_INPUT) 0;

            output.v_pos = input.Position;
            output.v_uv = input.Texcoord0;
            output.v_light_dir_world = -LightDirection.xyz;
            output.v_eye_pos_world = EyePosition.xyz;

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

        cbuffer cbuffer2 : register(b2)
        {
            matrix InvViewProjection;
        };

        Texture2D _MainTex : register(t0);
        SamplerState _MainTex_Sampler : register(s0);
        Texture2D _GBuffer1 : register(t1);
        SamplerState _GBuffer1_Sampler : register(s1);
        Texture2D _GBuffer2 : register(t2);
        SamplerState _GBuffer2_Sampler : register(s2);
        Texture2D _CameraDepthTexture : register(t3);
        SamplerState _CameraDepthTexture_Sampler : register(s3);

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
            float3 v_light_dir_world : TEXCOORD1;
            float3 v_eye_pos_world : TEXCOORD2;
        };

        float4 main(PS_INPUT input) : SV_Target
        {
            float4 c = _MainTex.Sample(_MainTex_Sampler, input.v_uv);
            float2 normal_2 = _GBuffer1.Sample(_GBuffer1_Sampler, input.v_uv).rg;
            float2 specular = _GBuffer2.Sample(_GBuffer2_Sampler, input.v_uv).zw;
            float depth = _CameraDepthTexture.Sample(_CameraDepthTexture_Sampler, input.v_uv).r;

            float3 normal = 0;

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
            //

            // get world position from depth and uv
            float x = input.v_uv.x * 2 - 1;
            float y = (1 - input.v_uv.y) * 2 - 1;
            float z = depth;

            float4 pos_world = mul(float4(x, y, z, 1), InvViewProjection);
            pos_world /= pos_world.w;
            //

            float3 eye_dir = normalize(input.v_eye_pos_world - pos_world.xyz);
            float3 light_dir = normalize(input.v_light_dir_world);

            float diff = max(0, dot(normal, light_dir));
            float3 h = normalize(light_dir + eye_dir);
            float nh = max(0, dot(normal, h));
            float spec = pow(nh, 128 * specular.x) * specular.y;

            c.rgb = GlobalAmbient.rgb * c.rgb +
                diff * c.rgb * LightColor.rgb +
                spec * LightColor.rgb;

            return c;
        }
    }

    HLVS vs_point
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
        };

        PS_INPUT main(VS_INPUT input)
        {
            PS_INPUT output = (PS_INPUT) 0;

            output.v_pos = mul(input.Position, WorldViewProjection);

            return output;
        }
    }

    HLPS ps_point
    {
        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
        };

        float4 main(PS_INPUT input) : SV_Target
        {
            return float4(1, 1, 1, 1);
        }
    }
}