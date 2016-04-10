TextureCube _MainTex : register(t0);
SamplerState _MainTex_Sampler : register(s0);

struct PS_INPUT
{
    float4 v_pos : SV_POSITION;
    float3 v_uv : POSITION;
};

float4 main(PS_INPUT input) : SV_Target
{
    float4 c = _MainTex.Sample(_MainTex_Sampler, input.v_uv);
    return c;
}