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
	float4 v_pos_proj : TEXCOORD1;
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
	output.v_pos_proj = output.v_pos;

    return output;
}