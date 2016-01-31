#pragma pack_matrix(row_major)

cbuffer c_buffer {
   float4x4 pvw;
};

struct VsOutput {
  float4 position : SV_POSITION;
};

struct VSInput {
  float4 position : POSITION;
};

VsOutput vs_main(VSInput v) {
  VsOutput o;
  o.position = mul(pvw, v.position);
  return o;
}
