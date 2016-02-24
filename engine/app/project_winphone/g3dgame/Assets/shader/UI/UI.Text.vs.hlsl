cbuffer cbuffer0 : register(b0)
{
	matrix WorldViewProjection;
};

struct VS_INPUT
{
	float4 Position : POSITION;
	float4 Color : COLOR;
	float2 Texcoord0 : TEXCOORD0;
};

struct PS_INPUT
{
	float4 v_pos : SV_POSITION;
	float2 v_uv : TEXCOORD0;
	float4 v_color : COLOR;
    float4 v_pos_proj : TEXCOORD1;
};

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT) 0;

	output.v_pos = mul(input.Position, WorldViewProjection);
	output.v_uv = input.Texcoord0;
	output.v_color = input.Color;
    output.v_pos_proj = output.v_pos;

	return output;
}