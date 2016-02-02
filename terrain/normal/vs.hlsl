#pragma pack_matrix(row_major)

struct VsOutput {
  float4 position:SV_POSITION;
  float3 normal: NORMAL;
  float2 texcoord: TEXCOORD;
};

struct VSInput {
  float4 position:POSITION;
  float2 texcoord: TEXCOORD;
};

Texture2D heightmap;
SamplerState sam_heightmap {
  Filter = MIN_MAG_LINEAR_MIP_POINT;
  AddressU = CLAMP;
  AddressV = CLAMP;
};

VsOutput vs_main(VSInput input) {
  VsOutput o;
  o.position = input.position;
  o.texcoord = input.texcoord;
  o.position.y = heightmap.SampleLevel(sam_heightmap, input.texcoord, 0).r;

  float unit = 1.0f / 4096.0f;
  float2 negx = input.texcoord + float2(-unit, 0.0);
  float2 posx = input.texcoord + float2( unit, 0.0);
  float2 negz = input.texcoord + float2(0.0f, -unit);
  float2 posz = input.texcoord + float2(0.0f,  unit);

  float tang_detay =  heightmap.SampleLevel(sam_heightmap, posx, 0).r -
      heightmap.SampleLevel(sam_heightmap, negx, 0).r;
  float bitang_detay =  heightmap.SampleLevel(sam_heightmap, posz, 0).r -
      heightmap.SampleLevel(sam_heightmap, negz, 0).r;

  float3 tangent = float3(2.0f * unit, tang_detay, 0.0f);
  float3 bitan = float3(0.0f, bitang_detay, 2.0f * unit);
  o.normal = normalize(cross(tangent, bitan));
  return o;
};
