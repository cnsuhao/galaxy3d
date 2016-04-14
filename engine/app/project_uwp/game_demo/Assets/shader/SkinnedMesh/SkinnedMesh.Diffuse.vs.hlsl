cbuffer cbuffer0 : register(b0)
{
    matrix ViewProjection;
};

cbuffer cbuffer1 : register(b1)
{
    float4 Bones[216];
};

cbuffer cbuffer2 : register(b2)
{
    float4 LightDirection;
};

cbuffer cbuffer3 : register(b3)
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
    float3 v_light_dir_world : TEXCOORD1;
    float3 v_eye_dir_world : TEXCOORD2;
    float3 v_normal_world : TEXCOORD3;

	float4 v_pos_proj : TEXCOORD4;
	float4 v_pos_world : TEXCOORD5;
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

    float4 pos_world = skinned_pos_world;
    float3 normal_world = skinned_normal_world;

    output.v_light_dir_world = - LightDirection.xyz;
    output.v_eye_dir_world = EyePosition.xyz - pos_world.xyz;
    output.v_normal_world = normal_world;

	output.v_pos_proj = output.v_pos;
	output.v_pos_world = pos_world;
    
	return output;
}