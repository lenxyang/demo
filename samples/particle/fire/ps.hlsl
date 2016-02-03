#pragma pack_matrix(row_major)

struct GsOutput {
  float4 pos: SV_POSITION;
  float4 color:    COLOR;
  float2 tex: TEXCOORD;
};


Texture2D tex : register(t0);
SamplerState linear_sampler {
  Filter = MIN_MAG_MIP_LINEAR;
  AddressU = WRAP;
  AddressV = WRAP;
};

float4 ps_main(GsOutput o):SV_TARGET {
  float4 v = tex.SampleLevel(linear_sampler, o.tex, 0);
  v.a = o.color.a;
  return v;
};

