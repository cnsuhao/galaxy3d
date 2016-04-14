cbuffer cbuffer0 : register(b0)
{
	float4 GlobalAmbient;
};

cbuffer cbuffer1 : register(b1)
{
	float4 LightColor;
};

cbuffer cbuffer2 : register(b2)
{
	float4 _Color;
};

cbuffer cbuffer3 : register(b3)
{
	matrix ViewProjectionLight[3];
};

cbuffer cbuffer4 : register(b4)
{
	float4 ShadowParam;
};

cbuffer cbuffer5 : register(b5)
{
	float4 _ZBufferParams;
};

cbuffer cbuffer6 : register(b6)
{
	float4 ShadowMapTexel;
};

cbuffer cbuffer7 : register(b7)
{
	float4 CascadeSplits;
};

Texture2D _MainTex : register(t0);
SamplerState _MainTex_Sampler : register(s0);

Texture2D _ShadowMapTexture : register(t1);
SamplerState _ShadowMapTexture_Sampler : register(s1);

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

float texture2DCompare(float2 uv, float compare)
{
	return step(compare, _ShadowMapTexture.Sample(_ShadowMapTexture_Sampler, uv).r);
}

float PCF(float2 uv, float2 size, float z)
{
	float bias = ShadowParam.x;
	float strength = ShadowParam.y;
	float shadow_weak = clamp(1 - strength, 0, 1);
	float shadow = 0;
	int pcf_size = 1;

	for(int i=-pcf_size;i<=pcf_size;i++)
	{
		for(int j=-pcf_size;j<=pcf_size;j++)
		{
			float2 off = float2(i, j) * size;
			float compare = texture2DCompare(uv + off, z - bias);

			if(compare < 1)
			{
				shadow += shadow_weak;
			}
			else
			{
				shadow += 1;
			}
		}
	}
	return shadow / ((pcf_size*2+1) * (pcf_size*2+1));
}

float sample_shadow(float depth, float4 pos_world)
{
	// shadow
	bool cascade = ((int) ShadowParam.z) == 1;

	int index = -1;

	if(cascade)
	{
		float linear_depth = 1.0 / (_ZBufferParams.x * depth + _ZBufferParams.y);
		if(linear_depth < CascadeSplits.x)
		{
			index = 0;
		}
		else if(linear_depth < CascadeSplits.y)
		{
			index = 1;
		}
		else if(linear_depth < CascadeSplits.z)
		{
			index = 2;
		}
	}
	else
	{
		index = 0;
	}

	float shadow = 1.0;

	if(index >= 0)
	{
		float4 pos_light_4 = mul(pos_world, ViewProjectionLight[index]);
		float3 pos_light = pos_light_4.xyz / pos_light_4.w;
		pos_light.z = min(1, pos_light.z);

		float2 uv_shadow = 0;
		uv_shadow.x = 0.5 + pos_light.x * 0.5;
		uv_shadow.y = 0.5 - pos_light.y * 0.5;

		float tex_witdh = 1.0;
		float tex_height = 1.0;
		if(cascade)
		{
			tex_witdh = 1.0 / 3;
			uv_shadow.x = index * tex_witdh + uv_shadow.x * tex_witdh;
		}

		float2 size = ShadowMapTexel.xy * float2(tex_witdh, tex_height);

		if(uv_shadow.x > 0.0 && uv_shadow.x < 1.0 && uv_shadow.y > 0.0 && uv_shadow.y < 1.0)
		{
			shadow = PCF(uv_shadow, size, pos_light.z);
		}
	}

	return shadow;
}

float4 main(PS_INPUT input) : SV_Target
{
	float specular_power = 1.0;
	float specular_intensity = 0.5;

	float4 c = _MainTex.Sample(_MainTex_Sampler, input.v_uv) * _Color;

	float3 normal = normalize(input.v_normal_world);
	float3 light_dir = normalize(input.v_light_dir_world);
	float3 eye_dir = normalize(input.v_eye_dir_world);

	float diff = max(0, dot(normal, light_dir));
	float3 h = normalize(light_dir + eye_dir);
	float nh = max(0, dot(normal, h));
	float spec = pow(nh, 128 * specular_power) * specular_intensity;

	float intensity = 1.0;
	
	bool shadow_enable = ((int) ShadowParam.w) == 1;
	if(shadow_enable)
	{
		float depth = input.v_pos_proj.z / input.v_pos_proj.w;
		float shadow = sample_shadow(depth, input.v_pos_world);
		intensity *= shadow;
	}

	c.rgb = GlobalAmbient.rgb * c.rgb + 
		(diff * c.rgb * LightColor.rgb + 
		spec * LightColor.rgb) * intensity;
	return c;
}