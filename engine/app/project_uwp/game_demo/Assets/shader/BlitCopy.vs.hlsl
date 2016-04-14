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