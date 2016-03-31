struct PS_INPUT
{
	float4 v_pos : SV_POSITION;
	float4 v_pos_proj : TEXCOORD1;
};

float4 main(PS_INPUT input) : SV_Target
{
	float4 c = 0.0;
	c.r = input.v_pos_proj.z / input.v_pos_proj.w;
	return c;
}