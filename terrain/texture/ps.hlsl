#pragma pack_matrix(row_major)

#include "lordaeron/effect/lighting.hlsl"

struct DsOutput {
  float4 position: SV_POSITION;
  float3 worldpos: WORLDPOS;
  float3 normal: NORMAL;
  float2 texcoord: TEXCOORD;
  float3 viewin: VIEWIN;
};

cbuffer c_buffer {
   DirLight   dirlight;
   float      ambient_scalar;
   float      specular_scalar;
   float      alpha;
};

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);
Texture2D tex2 : register(t2);
Texture2D tex3 : register(t3);
Texture2D tex4 : register(t4);
Texture2D blendtex : register(t5);
SamplerState sam_tex {
  Filter = MIN_MAG_LINEAR;
  AddressU = CLAMP;
  AddressV = CLAMP;
};

float4 ps_main(DsOutput o):SV_TARGET {
  float4 c0 = tex0.Sample(sam_tex, o.texcoord);
  float4 c1 = tex1.Sample(sam_tex, o.texcoord);
  float4 c2 = tex2.Sample(sam_tex, o.texcoord);
  float4 c3 = tex3.Sample(sam_tex, o.texcoord);
  float4 c4 = tex4.Sample(sam_tex, o.texcoord);
  float4 blend = blendtex.Sample(sam_tex, o.texcoord);
  float4 color = c0;
  color = lerp(color, c1, blend.r);
  color = lerp(color, c2, blend.g);
  color = lerp(color, c3, blend.b);
  color = lerp(color, c4, blend.a);

  float3 normal = o.normal;
  Matrial mtrl;
  mtrl.ambient  = (ambient_scalar) * color;
  mtrl.specular = specular_scalar * color;
  mtrl.diffuse  = color;
  mtrl.emission = float4(0.0f, 0.0f, 0.0f, 0.0f);
  mtrl.power    = 4;
  mtrl.alpha    = alpha;

  float3 dir_color = CalcDirLightColor(dirlight, normal, o.viewin, mtrl);
  return float4(dir_color, mtrl.alpha);
};
