Terrain/Diffuse
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

    Pass deferred
    {
        VS vs_deferred
        PS ps_deferred
        RenderStates rs
    }

    RenderStates rs
    {
        Cull Back
        ZWrite On
        ZTest LEqual
        Offset 0, 0
        Blend Off
    }

    HLVS vs
    {
        cbuffer cbuffer0 : register(b0)
        {
            matrix WorldViewProjection;
        };

        cbuffer cbuffer1 : register(b1)
        {
            float4 TerrainSize;
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
            float2 v_uv_alpha : TEXCOORD1;
        };

        PS_INPUT main(VS_INPUT input)
        {
            PS_INPUT output = (PS_INPUT)0;

            output.v_pos = mul(input.Position, WorldViewProjection);
            output.v_uv = input.Texcoord0;
            output.v_uv_alpha = float2(input.Position.x, input.Position.z) * TerrainSize.x;
            output.v_uv_alpha.y = 1.0 - output.v_uv_alpha.y;

            return output;
        }
    }

    HLPS ps
    {
        Texture2D AlphaMap : register(t0);
        SamplerState AlphaMap_Sampler : register(s0);
        Texture2D Layer_0 : register(t1);
        SamplerState Layer_0_Sampler : register(s1);
        Texture2D Layer_1 : register(t2);
        SamplerState Layer_1_Sampler : register(s2);
        Texture2D Layer_2 : register(t3);
        SamplerState Layer_2_Sampler : register(s3);
        Texture2D Layer_3 : register(t4);
        SamplerState Layer_3_Sampler : register(s4);
        Texture2D Layer_4 : register(t5);
        SamplerState Layer_4_Sampler : register(s5);

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
            float2 v_uv_alpha : TEXCOORD1;
        };

        float4 main(PS_INPUT input) : SV_Target
        {
            float4 alpha = AlphaMap.Sample(AlphaMap_Sampler, input.v_uv_alpha);

            float4 color = float4(0.0, 0.0, 0.0, 0.0);

            color += Layer_0.Sample(Layer_0_Sampler, input.v_uv) * alpha.r;
            color += Layer_1.Sample(Layer_1_Sampler, input.v_uv) * alpha.g;
            color += Layer_2.Sample(Layer_2_Sampler, input.v_uv) * alpha.b;
            color += Layer_3.Sample(Layer_3_Sampler, input.v_uv) * alpha.a;

            float rest = 1.0 - alpha.r - alpha.g - alpha.b - alpha.a;
            if(rest > 0)
            {
                color += Layer_4.Sample(Layer_4_Sampler, input.v_uv) * rest;
            }

            return color;
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
            float4 TerrainSize;
        };

        cbuffer cbuffer2 : register(b2)
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
            float2 v_uv_alpha : TEXCOORD1;
            float4 v_pos_proj : TEXCOORD2;
            float3 v_normal_world : TEXCOORD3;
        };

        PS_INPUT main(VS_INPUT input)
        {
            PS_INPUT output = (PS_INPUT)0;

            output.v_pos = mul(input.Position, WorldViewProjection);
            output.v_uv = input.Texcoord0;
            output.v_uv_alpha = float2(input.Position.x, input.Position.z) * TerrainSize.x;
            output.v_uv_alpha.y = 1.0 - output.v_uv_alpha.y;
            output.v_pos_proj = output.v_pos;
            output.v_normal_world = mul(input.Normal, World);

            return output;
        }
    }

    HLPS ps_deferred
    {
        Texture2D AlphaMap : register(t0);
        SamplerState AlphaMap_Sampler : register(s0);
        Texture2D Layer_0 : register(t1);
        SamplerState Layer_0_Sampler : register(s1);
        Texture2D Layer_1 : register(t2);
        SamplerState Layer_1_Sampler : register(s2);
        Texture2D Layer_2 : register(t3);
        SamplerState Layer_2_Sampler : register(s3);
        Texture2D Layer_3 : register(t4);
        SamplerState Layer_3_Sampler : register(s4);
        Texture2D Layer_4 : register(t5);
        SamplerState Layer_4_Sampler : register(s5);

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
            float2 v_uv_alpha : TEXCOORD1;
            float4 v_pos_proj : TEXCOORD2;
            float3 v_normal_world : TEXCOORD3;
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

            float4 alpha = AlphaMap.Sample(AlphaMap_Sampler, input.v_uv_alpha);

            float4 color = float4(0.0, 0.0, 0.0, 0.0);

            color += Layer_0.Sample(Layer_0_Sampler, input.v_uv) * alpha.r;
            color += Layer_1.Sample(Layer_1_Sampler, input.v_uv) * alpha.g;
            color += Layer_2.Sample(Layer_2_Sampler, input.v_uv) * alpha.b;
            color += Layer_3.Sample(Layer_3_Sampler, input.v_uv) * alpha.a;

            float rest = 1.0 - alpha.r - alpha.g - alpha.b - alpha.a;
            if(rest > 0)
            {
                color += Layer_4.Sample(Layer_4_Sampler, input.v_uv) * rest;
            }

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

            float specular_power = 0;
            float specular_intensity = 0;

            output.o_color = color;
            output.o_normal.xy = normal_2;
            output.o_specular.z = specular_power;
            output.o_specular.w = specular_intensity;
            output.o_depth.r = input.v_pos_proj.z / input.v_pos_proj.w;

            return output;
        }
    }

    GLVS vs
    {
        #define mat4to3(m) mat3(m[0].xyz, m[1].xyz, m[2].xyz)

        uniform mat4 WorldViewProjection;
        uniform mat4 World;
        uniform vec4 TerrainSize;
        uniform vec4 LightDirection;
        uniform vec4 ClipPlane;
        uniform vec4 ClipEnable;

        attribute vec4 Position;
        attribute vec3 Normal;
        attribute vec2 Texcoord0;

        varying vec2 v_uv;
        varying vec2 v_uv_alpha;
        varying vec3 v_light_dir_world;
        varying vec3 v_normal_world;
        varying float v_clip;

        void main()
        {
            gl_Position = WorldViewProjection * Position;
            v_uv = Texcoord0;

            v_uv_alpha = vec2(Position.x, Position.z) * TerrainSize.x;
            v_uv_alpha.y = 1.0 - v_uv_alpha.y;

            mat3 world_3x3 = mat4to3(World);

            vec3 normal_world = world_3x3 * Normal;
            v_light_dir_world = -LightDirection.xyz;
            v_normal_world = normal_world;

            //clip
            if(int(ClipEnable.x) != 0)
            {
                vec3 pos_world = world_3x3 * Position.xyz;
                v_clip = dot(ClipPlane.xyz, pos_world) + ClipPlane.w;
            }
        }
    }

    GLPS ps
    {
        #define saturate(v) clamp(v, 0.0, 1.0)

        uniform vec4 _Color;
        uniform vec4 GlobalAmbient;
        uniform vec4 LightColor;
        uniform vec4 ClipEnablePS;

        uniform sampler2D AlphaMap;
        uniform sampler2D Layer_0;
        uniform sampler2D Layer_1;
        uniform sampler2D Layer_2;
        uniform sampler2D Layer_3;

        varying vec2 v_uv;
        varying vec2 v_uv_alpha;
        varying vec3 v_light_dir_world;
        varying vec3 v_normal_world;
        varying float v_clip;

        void main()
        {
            if(int(ClipEnablePS.x) != 0)
            {
                if(v_clip < 0.0)
                {
                    discard;
                }
            }

            vec4 alpha = texture2D(AlphaMap, v_uv_alpha);

            vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

            color += texture2D(Layer_0, v_uv) * alpha.r;
            color += texture2D(Layer_1, v_uv) * alpha.g;
            color += texture2D(Layer_2, v_uv) * alpha.b;
            color += texture2D(Layer_3, v_uv) * alpha.a;

            //light
            {
                color = color * _Color;

                vec3 normal = normalize(v_normal_world);
                vec3 light_dir = normalize(v_light_dir_world);

                float n_dot_l = saturate(dot(normal, light_dir));

                vec3 ambient = GlobalAmbient.rgb * color.rgb;
                vec3 diffuse = LightColor.rgb * n_dot_l * color.rgb * 2.0;

                color.rgb = ambient + diffuse;
            }

            gl_FragColor = color;
        }
    }
}