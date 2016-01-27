#pragma pack_matrix(row_major)

struct VsOutput {
  float4 position : SV_POSITION;
};

struct VSInput {
  float3 position:POSITION;
  float3 normal:NORMAL;
};

cbuffer c_buffer {
   float4x4 pvw;
};

VsOutput vs_main(VSInput input) {
  VsOutput o;
  o.position = mul(pvw, float4(input.position, 1.0f));
  return o;
}
