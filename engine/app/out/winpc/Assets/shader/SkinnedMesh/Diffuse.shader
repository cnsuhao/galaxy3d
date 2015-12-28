SkinnedMesh/Diffuse
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

    Pass depth
    {
        VS vs_depth
        RenderStates rs
    }

	RenderStates rs
	{
        Cull Back
	}

	HLVS vs
	{
        cbuffer cbuffer0 : register(b0)
        {
            matrix ViewProjection;
        };

        cbuffer cbuffer1 : register(b1)
        {
            float4 _Color;
        };

        cbuffer cbuffer2 : register(b2)
        {
            float4 Bones[216];
        };

        cbuffer cbuffer3 : register(b3)
        {
            float4 LightDirection;
        };

        cbuffer cbuffer4 : register(b4)
        {
            float4 EyePosition;
        };

		struct VS_INPUT
		{
			float4 Position : POSITION;
            float3 Normal : NORMAL;
            float4 Tangent : TANGENT;
            float4 BlendWeight : BLENDWEIGHT;
            float4 BlendIndices : BLENDINDICES;
			float2 Texcoord0 : TEXCOORD0;
		};

		struct PS_INPUT
		{
			float4 v_pos : SV_POSITION;
			float2 v_uv : TEXCOORD0;
            float4 v_color : COLOR;
            float3 v_light_dir_world : TEXCOORD1;
            float3 v_eye_dir_world : TEXCOORD2;
            float3 v_normal_world : TEXCOORD3;
		};

		PS_INPUT main( VS_INPUT input )
		{
			PS_INPUT output = (PS_INPUT) 0;

            float4 skinned_pos_world = 0;
            float3 skinned_normal_world = 0;
            for(int i=0; i<4; i++)
            {
                float weight = input.BlendWeight[i];
                int index = (int) input.BlendIndices[i];

                if(weight > 0 && index >= 0)
                {
                    matrix bone = {
                        Bones[index*3].x, Bones[index*3+1].x, Bones[index*3+2].x, 0,
                        Bones[index*3].y, Bones[index*3+1].y, Bones[index*3+2].y, 0,
                        Bones[index*3].z, Bones[index*3+1].z, Bones[index*3+2].z, 0,
                        Bones[index*3].w, Bones[index*3+1].w, Bones[index*3+2].w, 1
                    };
                    float4 world = mul(input.Position, bone);
                    skinned_pos_world += world * weight;

                    float3 normal_world = mul(input.Normal, (float3x3) bone);
                    skinned_normal_world += normal_world * weight;
                }
            }

            output.v_pos = mul(skinned_pos_world, ViewProjection);
			output.v_uv = input.Texcoord0;
            output.v_color = _Color;

            float4 pos_world = skinned_pos_world;
            float3 normal_world = skinned_normal_world;

            output.v_light_dir_world = - LightDirection.xyz;
            output.v_eye_dir_world = EyePosition.xyz - pos_world.xyz;
            output.v_normal_world = normal_world;
    
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

        Texture2D _MainTex : register(t0);
        SamplerState _MainTex_Sampler : register(s0);

		struct PS_INPUT
		{
			float4 v_pos : SV_POSITION;
			float2 v_uv : TEXCOORD0;
            float4 v_color : COLOR;
            float3 v_light_dir_world : TEXCOORD1;
            float3 v_eye_dir_world : TEXCOORD2;
            float3 v_normal_world : TEXCOORD3;
		};

		float4 main( PS_INPUT input) : SV_Target
		{
            float specular_power = 1.0;
            float specular_intensity = 0.5;

			float4 c = _MainTex.Sample(_MainTex_Sampler, input.v_uv) * input.v_color;
            
            float3 normal = normalize(input.v_normal_world);
            float3 light_dir = normalize(input.v_light_dir_world);
            float3 eye_dir = normalize(input.v_eye_dir_world);
            
            float diff = max(0, dot(normal, light_dir));
            float3 h = normalize(light_dir + eye_dir);
            float nh = max(0, dot(normal, h));
            float spec = pow(nh, 128 * specular_power) * specular_intensity;

            c.rgb = GlobalAmbient.rgb * c.rgb + 
                diff * c.rgb * LightColor.rgb + 
                spec * LightColor.rgb;
            
			return c;
		}
	}

    HLVS vs_deferred
    {
        cbuffer cbuffer0 : register(b0)
        {
            matrix ViewProjection;
        };

        cbuffer cbuffer1 : register(b1)
        {
            float4 _Color;
        };

        cbuffer cbuffer2 : register(b2)
        {
            float4 Bones[216];
        };

        struct VS_INPUT
        {
            float4 Position : POSITION;
            float3 Normal : NORMAL;
            float4 Tangent : TANGENT;
            float4 BlendWeight : BLENDWEIGHT;
            float4 BlendIndices : BLENDINDICES;
            float2 Texcoord0 : TEXCOORD0;
        };

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
            float4 v_color : COLOR;
            float4 v_pos_proj : TEXCOORD1;
            float3 v_normal_world : TEXCOORD2;
        };

        PS_INPUT main(VS_INPUT input)
        {
            PS_INPUT output = (PS_INPUT) 0;

            float4 skinned_pos_world = 0;
            float3 skinned_normal_world = 0;
            for(int i=0; i<4; i++)
            {
                float weight = input.BlendWeight[i];
                int index = (int) input.BlendIndices[i];

                if(weight > 0 && index >= 0)
                {
                    matrix bone = {
                        Bones[index*3].x, Bones[index*3+1].x, Bones[index*3+2].x, 0,
                        Bones[index*3].y, Bones[index*3+1].y, Bones[index*3+2].y, 0,
                        Bones[index*3].z, Bones[index*3+1].z, Bones[index*3+2].z, 0,
                        Bones[index*3].w, Bones[index*3+1].w, Bones[index*3+2].w, 1
                    };
                    float4 world = mul(input.Position, bone);
                    skinned_pos_world += world * weight;

                    float3 normal_world = mul(input.Normal, (float3x3) bone);
                    skinned_normal_world += normal_world * weight;
                }
            }

            output.v_pos = mul(skinned_pos_world, ViewProjection);
            output.v_uv = input.Texcoord0;
            output.v_color = _Color;
            output.v_pos_proj = output.v_pos;
            output.v_normal_world = skinned_normal_world;

            return output;
        }
    }

    HLPS ps_deferred
    {
        Texture2D _MainTex : register(t0);
        SamplerState _MainTex_Sampler : register(s0);

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
            float2 v_uv : TEXCOORD0;
            float4 v_color : COLOR;
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

            float4 color = _MainTex.Sample(_MainTex_Sampler, input.v_uv) * input.v_color;
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

            float specular_power = 1.0;
            float specular_intensity = 0.5;

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
            matrix ViewProjection;
        };

        cbuffer cbuffer1 : register(b1)
        {
            float4 Bones[216];
        };

        struct VS_INPUT
        {
            float4 Position : POSITION;
            float3 Normal : NORMAL;
            float4 Tangent : TANGENT;
            float4 BlendWeight : BLENDWEIGHT;
            float4 BlendIndices : BLENDINDICES;
            float2 Texcoord0 : TEXCOORD0;
        };

        struct PS_INPUT
        {
            float4 v_pos : SV_POSITION;
        };

        PS_INPUT main(VS_INPUT input)
        {
            PS_INPUT output = (PS_INPUT) 0;

            float4 skinned_pos_world = 0;
            for(int i=0; i<4; i++)
            {
                float weight = input.BlendWeight[i];
                int index = (int) input.BlendIndices[i];

                if(weight > 0 && index >= 0)
                {
                    matrix bone = {
                        Bones[index*3].x, Bones[index*3+1].x, Bones[index*3+2].x, 0,
                        Bones[index*3].y, Bones[index*3+1].y, Bones[index*3+2].y, 0,
                        Bones[index*3].z, Bones[index*3+1].z, Bones[index*3+2].z, 0,
                        Bones[index*3].w, Bones[index*3+1].w, Bones[index*3+2].w, 1
                    };
                    float4 world = mul(input.Position, bone);
                    skinned_pos_world += world * weight;
                }
            }

            output.v_pos = mul(skinned_pos_world, ViewProjection);

            return output;
        }
    }

	GLVS vs
	{
		uniform mat4 ViewProjection;
        uniform vec4 Bones[108];
        uniform vec4 _MainColor;

		attribute vec3 Position;
        attribute vec3 Normal;
        attribute vec4 Tangent;
        attribute vec4 BlendWeight;
        attribute vec4 BlendIndices;
		attribute vec2 Texcoord0;

		varying vec2 v_uv;
		varying vec4 v_color;

		void main()
		{
            vec4 skinned_pos_world = vec4(0.0);
            int indices[4];
            indices[0] = int(BlendIndices.x);
            indices[1] = int(BlendIndices.y);
            indices[2] = int(BlendIndices.z);
            indices[3] = int(BlendIndices.w);
            float weights[4];
            weights[0] = BlendWeight.x;
            weights[1] = BlendWeight.y;
            weights[2] = BlendWeight.z;
            weights[3] = BlendWeight.w;
            for(int i=0; i<4; i++) {
                float weight = weights[i];
                int index = indices[i];
                if(weight > 0.0) {
                    mat4 bone = mat4(Bones[index*3], Bones[index*3+1], Bones[index*3+2], vec4(0, 0, 0, 1));
                    vec4 world = Position * bone;
                    skinned_pos_world += world * weight;
                }
            }

            gl_Position = skinned_pos_world * ViewProjection;
            v_uv = Texcoord0;
			v_color = _MainColor;
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