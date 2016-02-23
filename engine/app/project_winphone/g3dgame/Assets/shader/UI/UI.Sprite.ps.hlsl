cbuffer cbuffer0 : register(b0)
{
	float4 _Color;
};

cbuffer cbuffer1 : register(b1)
{
	float4 ClipRect;
};

cbuffer cbuffer2 : register(b2)
{
	float4 ClipSoft;
};

Texture2D _MainTex : register(t0);
SamplerState _MainTex_Sampler : register(s0);

struct PS_INPUT
{
	float4 v_pos : SV_POSITION;
	float2 v_uv : TEXCOORD0;
	float4 v_color : COLOR;
	float4 v_pos_proj : TEXCOORD1;
};

float4 main(PS_INPUT input) : SV_Target
{
	float4 c = _MainTex.Sample(_MainTex_Sampler, input.v_uv) * input.v_color * _Color;

	float2 pos = input.v_pos_proj.xy / input.v_pos_proj.w;

	float alpha = 1.0;
	if (pos.y > ClipSoft.y)
	{
		alpha = (ClipRect.y - pos.y) / (ClipRect.y - ClipSoft.y);
	}
	else if (pos.y < ClipSoft.w)
	{
		alpha = (ClipRect.w - pos.y) / (ClipRect.w - ClipSoft.w);
	}
	if (pos.x > ClipSoft.z)
	{
		alpha *= (ClipRect.z - pos.x) / (ClipRect.z - ClipSoft.z);
	}
	else if (pos.x < ClipSoft.x)
	{
		alpha *= (ClipRect.x - pos.x) / (ClipRect.x - ClipSoft.x);
	}
	c.a *= alpha;

	if (pos.x - ClipRect.x < 0 ||
		ClipRect.z - pos.x < 0 ||
		ClipRect.y - pos.y < 0 ||
		pos.y - ClipRect.w < 0)
	{
		clip(-1);
	}

	return c;
}