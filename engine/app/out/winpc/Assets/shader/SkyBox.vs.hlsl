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
    float3 v_uv : POSITION;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;

    output.v_pos = mul(input.Position, WorldViewProjection).xyww;
    output.v_uv = input.Position.xyz;

    return output;
}