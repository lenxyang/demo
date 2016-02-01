// using row_major
#pragma pack_matrix(row_major)

#include "lordaeron/effect/lighting.hlsl"

struct VsOutput {
  float4 position:SV_POSITION;
  float3 worldpos: WPOS;
  float3 normal:NORMAL;
  float3 viewin: VIEWIN;
  float2 texcoord: TEXCOORD0;
};

cbuffer c_buffer {
   DirLight   dirlight;
   PointLight pointlight;
   SpotLight  spotlight;
   float      ambient_scalar;
   float      specular_scalar;
   float      alpha;
};

Texture2D diffuse_map: register(t0);
SamplerState texsampler;

float4 ps_main(VsOutput o):SV_TARGET {
  float4 color = diffuse_map.Sample(texsampler, o.texcoord);
  float3 normal = o.normal;
  Matrial mtrl;
  mtrl.ambient  = (ambient_scalar) * color;
  mtrl.specular = specular_scalar * color;
  mtrl.diffuse  = color;
  mtrl.emission = float4(0.0f, 0.0f, 0.0f, 0.0f);
  mtrl.power    = 4;
  mtrl.alpha    = alpha;

  float3 dir_color = CalcDirLightColor(dirlight, normal, o.viewin, mtrl);
  float3 point_color = CalcPointLightColor(pointlight, o.worldpos,
                                           normal, o.viewin, mtrl);
  float3 spot_color = CalcSpotLightColor(spotlight, o.worldpos,
                                         normal, o.viewin, mtrl);
  return float4(spot_color + dir_color + point_color, mtrl.alpha);
}
