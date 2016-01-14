#pragma pack_matrix(row_major)

struct VsOutput {
  float4 position:SV_POSITION;
  float4 normal:NORMAL;
};

struct VSInput {
  float4 position:POSITION;
  float4 normal:NORMAL;
};

cbuffer c_buffer {
   float4x4 pvw;
   float4x4 world;
};

VsOutput vs_main(VSInput input) {
  VsOutput o;
  o.position = mul(pvw, input.position);
  o.normal = mul(world, input.normal);
  return o;
}
