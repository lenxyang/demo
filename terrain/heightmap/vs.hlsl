#pragma pack_matrix(row_major)

struct VsOutput {
  float4 position:SV_POSITION;
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
  return o;
};
