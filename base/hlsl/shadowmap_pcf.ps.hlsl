// using row_major
#pragma pack_matrix(row_major)

#include "lordaeron/effect/lighting.hlsl"

struct VsOutput {
  float4 position:SV_POSITION;
  float3 worldpos: WPOS;
  float3 normal:NORMAL;
  float3 viewin: VIEWIN;
  float2 texcoord: TEXCOORD0;
  float4 projtex: TEXCOORD1;
};

cbuffer c_buffer {
   DirLight   dirlight;
   PointLight pointlight;
   SpotLight  spotlight;
   float      ambient_scalar;
   float      specular_scalar;
};

Texture2D texmap : register(t0);
Texture2D shadowmap : register(t1);
SamplerState texsampler;
SamplerState smsampler {
  Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
  AddressU = Border;
  AddressV = Border;
  AddressW = Border;
  BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
  ComparisonFunc = LESS_EQUAL;
};

float sample_depth(float2 tex, Texture2D shadowmap) {
  const int SMAP_SIZE = 1024;
  float delta = 1.0f / 1024.0f;
  float s1 = shadowmap.Sample(smsampler, tex.xy).r;
  float s2 = shadowmap.Sample(smsampler, tex.xy + float2(delta, 0.0)).r;
  float s3 = shadowmap.Sample(smsampler, tex.xy + float2(0.0, delta)).r;
  float s4 = shadowmap.Sample(smsampler, tex.xy + float2(delta, delta)).r;
  float2 texelpos = SMAP_SIZE * tex;
  float2 t = frac(texelpos);
  float v1 = lerp(s1, s2, t.x);
  float v2 = lerp(s3, s4, t.x);
  return lerp(v1, v2, t.y);
}

float4 ps_main(VsOutput o):SV_TARGET {
  o.projtex = o.projtex / o.projtex.w;
  o.projtex.x = ( o.projtex.x * 0.5f + 0.5f);
  o.projtex.y = (-o.projtex.y * 0.5f + 0.5f);
  float depth = sample_depth(o.projtex.xy, shadowmap);
  float spot_factor = 1.0f;
  if (saturate(o.projtex.x) == o.projtex.x && 
      saturate(o.projtex.y) == o.projtex.y) {
      float light_depth = o.projtex.z - 0.001;
      if (depth < light_depth) {
          spot_factor = 0.0f;
      }
  }
  
  float4 color = texmap.Sample(texsampler, o.texcoord);
  float3 normal = o.normal;
  Matrial mtrl;
  mtrl.ambient  = (ambient_scalar) * color;
  mtrl.specular = specular_scalar * color;
  mtrl.diffuse  = color;
  mtrl.emission = float4(0.0f, 0.0f, 0.0f, 0.0f);
  mtrl.power    = 4;
  mtrl.alpha    = 1.0f;

  float3 dir_color = CalcDirLightColor(dirlight, normal, o.viewin, mtrl);
  float3 point_color = CalcPointLightColor(pointlight, o.worldpos,
                                           normal, o.viewin, mtrl);
  float3 spot_color = CalcSpotLightColor(spotlight, o.worldpos,
                                         normal, o.viewin, mtrl);
  return float4(spot_color * spot_factor + dir_color + point_color, mtrl.alpha);
}
