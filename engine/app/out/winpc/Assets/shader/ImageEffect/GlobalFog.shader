ImageEffect/GlobalFog
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
            float4 v_pos_proj : TEXCOORD1;
        };

        PS_INPUT main(VS_INPUT input)
        {
            PS_INPUT output = (PS_INPUT) 0;

            output.v_pos = input.Position;
            output.v_uv = input.Texcoord0;
            output.v_pos_proj = output.v_pos;

            return output;
        }
    }

    HLPS ps
    {
        cbuffer cbuffer0 : register(b0)
        {
            float4 _ZBufferParams;
        };

        cbuffer cbuffer1 : register(b1)
        {
            float4 _ProjectionParams;
        };

        cbuffer cbuffer2 : register(b2)
        {
            float4 _SceneFogColor;
        };

        cbuffer cbuffer3 : register(b3)
        {
            float4 _SceneFogMode;
        };

        cbuffer cbuffer4 : register(b4)
        {
            float4 _CameraWS;
        };

        cbuffer cbuffer5 : register(b5)
        {
            float4 _HeightParams;
        };

        cbuffer cbuffer6 : register(b6)
        {
            float4 _SceneFogParams;
        };

        cbuffer cbuffer7 : register(b7)
        {
            float4 _DistanceParams;
        };

        cbuffer cbuffer8 : register(b8)
        {
            matrix InvViewProjection;
        };

        Texture2D _MainTex : register(t0);
        SamplerState _MainTex_Sampler : register(s0);
        Texture2D _CameraDepthTexture : register(t1);
        SamplerState _CameraDepthTexture_Sampler : register(s1);

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
            float4 v_pos_proj : TEXCOORD1;
        };

        float linear_depth_01(float z)
        {
            return 1.0 / (_ZBufferParams.x * z + _ZBufferParams.y);
        }

        // Distance-based fog
        float ComputeDistance(float3 camDir, float zdepth)
        {
            float dist; 
            if ((int) _SceneFogMode.y == 1)
                dist = length(camDir);
            else
                dist = zdepth * _ProjectionParams.z;
            // Built-in fog starts at near plane, so match that by
            // subtracting the near value. Not a perfect approximation
            // if near plane is very large, but good enough.
            dist -= _ProjectionParams.y;
            return dist;
        }

        float ComputeHalfSpace(float3 wsDir)
        {
            float3 C = _CameraWS.xyz;
            float3 wpos = C + wsDir;
            float FH = _HeightParams.x;
            float3 V = wsDir;
            float3 P = wpos;
            float3 aV = _HeightParams.w * V;
            float FdotC = _HeightParams.y;
            float k = _HeightParams.z;
            float FdotP = P.y-FH;
            float FdotV = wsDir.y;
            float c1 = k * (FdotP + FdotC);
            float c2 = (1-2*k) * FdotP;
            float g = min(c2, 0.0);
            g = -length(aV) * (c1 - g * g / abs(FdotV + 1.0e-5f));
            return g;
        }

        float ComputeFogFactor(float coord)
        {
            float fogFac = 0.0;
            if ((int) _SceneFogMode.x == 1) // linear
            {
                // factor = (end-z)/(end-start) = z * (-1/(end-start)) + (end/(end-start))
                fogFac = coord * _SceneFogParams.z + _SceneFogParams.w;
            }
            if ((int) _SceneFogMode.x == 2) // exp
            {
                // factor = exp(-density*z)
                fogFac = _SceneFogParams.y * coord;
                fogFac = exp2(-fogFac);
            }
            if ((int) _SceneFogMode.x == 3) // exp2
            {
                // factor = exp(-(density*z)^2)
                fogFac = _SceneFogParams.x * coord;
                fogFac = exp2(-fogFac*fogFac);
            }
            return saturate(fogFac);
        }

        float4 main(PS_INPUT input) : SV_Target
        {
            float4 sceneColor = _MainTex.Sample(_MainTex_Sampler, input.v_uv);

            // Reconstruct world space position & direction
            // towards this screen pixel.
            float depth = _CameraDepthTexture.Sample(_CameraDepthTexture_Sampler, input.v_uv).r;
            float depth_linear = linear_depth_01(depth);

            float4 pos_world = mul(float4(input.v_pos_proj.xy / input.v_pos_proj.w, depth, 1), InvViewProjection);
            pos_world /= pos_world.w;
            float3 dir_world = pos_world.xyz - _CameraWS.xyz;

            // Compute fog distance
            bool distance = (int) _SceneFogMode.z == 1;
            bool height = (int) _SceneFogMode.w == 1;
            float g = _DistanceParams.x;
            if(distance)
                g += ComputeDistance(dir_world, depth_linear);
            if(height)
                g += ComputeHalfSpace(dir_world);

            // Compute fog amount
            half fogFac = ComputeFogFactor(max(0.0,g));
            // Do not fog skybox
            if (depth == _DistanceParams.y)
                fogFac = 1.0;
            //return fogFac; // for debugging

            return lerp(_SceneFogColor, sceneColor, fogFac);
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