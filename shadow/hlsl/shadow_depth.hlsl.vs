#pragma pack_matrix(row_major)

struct VsOutput {
  float4 position:SV_POSITION;
  float4 pos: POS;
};

struct VSInput {
  float4 position:POSITION;
  float4 normal:NORMAL;
  float4 binormal:BINORMAL;
  float4 tangent:TANGENT;
  float2 texcoord: TEXCOORD0;
};

cbuffer c_buffer {
   float4x4 pvw;
   float4x4 world;
};

VsOutput vs_main(VSInput input) {
  VsOutput o;
  o.position = mul(pvw, input.position);
  o.pos = o.position;
  return o;
}
