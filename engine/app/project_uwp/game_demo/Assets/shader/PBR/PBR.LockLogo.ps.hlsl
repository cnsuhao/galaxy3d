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
	float4 _ColorBase;
};

cbuffer cbuffer4 : register(b4)
{
	float4 _ColorU;
};

cbuffer cbuffer5 : register(b5)
{
	float4 _ColorMetal;
};

cbuffer cbuffer6 : register(b6)
{
	float4 _FresnelExp;
};

#define PI 3.14159265

Texture2D _MaskMap : register(t0);
SamplerState _MaskMap_Sampler : register(s0);
TextureCube _ReflectionMap : register(t1);
SamplerState _ReflectionMap_Sampler : register(s1);

struct PS_INPUT
{
	float4 v_pos : SV_POSITION;
	float2 v_uv : TEXCOORD0;
	float3 v_light_dir_world : TEXCOORD1;
	float3 v_eye_dir_world : TEXCOORD2;
	float3 v_normal_world : TEXCOORD3;
};

float SmithVisibilityTerm(float NdotL, float NdotV, float k)
{
	float gL = NdotL * (1-k) + k;
	float gV = NdotV * (1-k) + k;
	return 1.0 / (gL * gV + 1e-4f);
}

float SmithGGXVisibilityTerm(float NdotL, float NdotV, float roughness)
{
	float k = (roughness * roughness) / 2;
	return SmithVisibilityTerm(NdotL, NdotV, k);
}

float GGXTerm(float NdotH, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
	return a2 / (PI * d * d);
}

float Pow5(float x)
{
	return x*x * x*x * x;
}

float3 FresnelTerm(float3 F0, float cosA)
{
	float t = Pow5 (1 - cosA);
	return F0 + (1-F0) * t;
}

float3 FresnelLerp(float3 F0, float3 F90, float cosA)
{
	float t = Pow5 (1 - cosA);
	return lerp(F0, F90, t);
}

float Fresnel(float exp, float frac, float cosA)
{
	half t = pow(abs(1 - cosA), exp);
	return lerp (1, frac, 1 - t);
}

float4 main(PS_INPUT input) : SV_Target
{
	float4 masks = _MaskMap.Sample(_MaskMap_Sampler, input.v_uv);
	float metallic = masks.g;
	float smoothness = 0.9;

	float4 c = lerp(_ColorBase, _ColorU, masks.r);
	c = lerp(c, masks.g, _ColorMetal) * clamp(lerp(-0.5, 1, masks.a), 0, 1);

	float3 spec_color = lerp(float3(0.22, 0.22, 0.22), c.rgb, metallic);
	float one_minus_dielectric_spec = 0.779;
	float one_minus_reflectivity = one_minus_dielectric_spec - metallic * one_minus_dielectric_spec;
	c.rgb = c.rgb * one_minus_reflectivity;

	float3 normal = normalize(input.v_normal_world);
	float3 light_dir = normalize(input.v_light_dir_world);
	float3 eye_dir = normalize(input.v_eye_dir_world);

	float3 h = normalize(light_dir + eye_dir);
	float nl = max(0, dot(normal, light_dir));
	float nh = max(0, dot(normal, h));
	float nv = max(0, dot(normal, eye_dir));
	float lh = max(0, dot(light_dir, h));

	float roughness = 1 - smoothness;
	float V = SmithGGXVisibilityTerm(nl, nv, roughness);
	float D = GGXTerm(nh, roughness);

	float nlPow5 = Pow5(1-nl);
	float nvPow5 = Pow5(1-nv);
	float Fd90 = 0.5 + 2 * lh * lh * roughness;
	float disneyDiffuse = (1 + (Fd90-1) * nlPow5) * (1 + (Fd90-1) * nvPow5);

	float diffuseTerm = disneyDiffuse * nl;
	float specularTerm = max(0, (V * D * nl) * (PI / 4));
	float grazingTerm = saturate(smoothness + (1-one_minus_reflectivity));

	const float SPECCUBE_LOD_STEPS = 7;
	float mip = roughness * SPECCUBE_LOD_STEPS;
	float3 uv_cube = (2.0 * (dot (normal, eye_dir) * normal)) - eye_dir;

#ifdef SM409
	float4 gi_specular = _ReflectionMap.Sample(_ReflectionMap_Sampler, uv_cube);
#else
	float4 gi_specular = _ReflectionMap.SampleLevel(_ReflectionMap_Sampler, uv_cube, mip);
#endif

	float3 ambient = c.rgb * GlobalAmbient.rgb;
	float3 diffuse = c.rgb * (LightColor.rgb * diffuseTerm);
	float3 spec = specularTerm * LightColor.rgb * FresnelTerm(spec_color, lh);
	float3 gi = gi_specular.rgb * FresnelLerp(spec_color, grazingTerm, max(nv, 0.25));

	spec = spec * roughness * 0.5;

	float3 emissive = masks.a * _Color.rgb * (1 - Fresnel(_FresnelExp.x, 0, nv));
	c.rgb =	ambient + diffuse + spec + gi + emissive;

	return c;
}