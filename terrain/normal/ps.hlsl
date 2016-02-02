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
   SpotLight  spotlight;
   float      ambient_scalar;
   float      specular_scalar;
   float      alpha;
};

float4 ps_main(DsOutput o):SV_TARGET {
  float4 color = float4(0.5, 0.5f, 0.5f, 1.0f);
  float3 normal = o.normal;
  Matrial mtrl;
  mtrl.ambient  = (ambient_scalar) * color;
  mtrl.specular = specular_scalar * color;
  mtrl.diffuse  = color;
  mtrl.emission = float4(0.0f, 0.0f, 0.0f, 0.0f);
  mtrl.power    = 4;
  mtrl.alpha    = alpha;

  float3 dir_color = CalcDirLightColor(dirlight, normal, o.viewin, mtrl);
  float3 spot_color = CalcSpotLightColor(spotlight, o.worldpos,
                                         normal, o.viewin, mtrl);
  return float4(spot_color + dir_color, mtrl.alpha);
};
