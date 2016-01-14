// using row_major
#pragma pack_matrix(row_major)

#include "lordaeron/effect/lighting.hlsl"

struct VsOutput {
  float4 position:SV_POSITION;
  float4 normal:NORMAL;
  float3 worldpos: WPOS;
  float3 viewin: VIEWIN;
};

cbuffer c_buffer {
   float4 color;
   float4 emission;
   DirLight dirlight;
};

float4 ps_main(VsOutput o):SV_TARGET {
  float3 normal = o.normal;
  Matrial mtrl;
  mtrl.ambient  = 0.1 * color;
  mtrl.specular = 0.2 * color;
  mtrl.diffuse  = color;
  mtrl.emission = float4(0.0f, 0.0f, 0.0f, 0.0f);
  mtrl.power    = 4;
  mtrl.alpha    = 1.0f;
  float3 dir_color = CalcDirLightColor(dirlight, normal, o.viewin, mtrl);
  return float4(dir_color, mtrl.alpha);
}
