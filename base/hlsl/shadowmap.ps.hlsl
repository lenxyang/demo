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
  Filter = MIN_MAG_MIP_LINEAR;
  AddressU = Clamp;
  AddressV = Clamp;
};

float4 ps_main(VsOutput o):SV_TARGET {
  o.projtex = o.projtex / o.projtex.w;
  o.projtex.x = ( o.projtex.x * 0.5f + 0.5f);
  o.projtex.y = (-o.projtex.y * 0.5f + 0.5f);
  float depth = shadowmap.Sample(smsampler, o.projtex.xy).r;
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
