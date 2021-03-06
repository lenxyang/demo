#pragma pack_matrix(row_major)

struct DsOutput {
  float4 position:SV_POSITION;
  float2 texcoord: TEXCOORD;
};

cbuffer c_buffer {
  float4 color;
};

float4 ps_main(DsOutput o):SV_TARGET {
  return color;
};
