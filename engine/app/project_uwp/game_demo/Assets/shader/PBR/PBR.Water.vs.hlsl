cbuffer cbuffer0 : register(b0)
{
	matrix WorldViewProjection;
};

cbuffer cbuffer1 : register(b1)
{
    matrix World;
};

cbuffer cbuffer2 : register(b2)
{
    float4 LightDirection;
};

cbuffer cbuffer3 : register(b3)
{
    float4 EyePosition;
};

cbuffer cbuffer4 : register(b4)
{
	float4 _Time;
};

cbuffer cbuffer5 : register(b5)
{
	float4 _Speed;
};

cbuffer cbuffer6 : register(b6)
{
	float4 _Scale;
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
	float2 v_uv0 : TEXCOORD0;
	float2 v_uv1 : TEXCOORD5;
    float3 v_light_dir_world : TEXCOORD1;
    float3 v_eye_dir_world : TEXCOORD2;
    float3 v_normal_world : TEXCOORD3;
	float4 v_tangent_world : TEXCOORD4;
};

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT) 0;

	output.v_pos = mul(input.Position, WorldViewProjection);

    float4 pos_world = mul(input.Position, World);
    float3 normal_world = mul(float4(input.Normal, 0), World).xyz;

    output.v_light_dir_world = - LightDirection.xyz;
    output.v_eye_dir_world = EyePosition.xyz - pos_world.xyz;
    output.v_normal_world = normal_world;
	output.v_tangent_world = mul(input.Tangent, World);

	output.v_uv0 = input.Texcoord0 * _Scale.x + _Time.x * _Speed.x * float2(0.3131, 0.123);
	output.v_uv1 = input.Texcoord0 * _Scale.y + _Time.x * _Speed.y * float2(0.114, 0.2342);

	return output;
}