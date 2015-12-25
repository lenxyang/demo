#pragma pack_matrix(row_major)

struct VsOutput {
  float4 position:SV_POSITION;
  float4 orgpos : POSITION;
};

struct VSInput {
  float4 position:POSITION;
};

cbuffer c_buffer {
   float4x4 pvw;
};

VsOutput vs_main(VSInput input) {
  VsOutput o;
  o.position = mul(pvw, input.position);
  o.orgpos = o.position;
  return o;
}
