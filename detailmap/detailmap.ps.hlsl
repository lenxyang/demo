#pragma pack_matrix(row_major)

#include "lordaeron/effect/lighting.hlsl"

struct DsOutput {
  float4 position: SV_POSITION;
  float3 normal  : NORMAL;
  float2 texcoord: TEXCOORD;
};

cbuffer c_buffer {
  DirLight   dirlight;
  SpotLight  spotlight;
};

Texture2D diffuse_map:  register(t0);
SamplerState texsampler;

float4 ps_main(DsOutput o):SV_TARGET {
  float4 color = diffuse_map.Sample(texsampler, o.texcoord);
  return color;
  return float4(o.normal, 1.0f);
  return color;
};
