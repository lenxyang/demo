#pragma pack_matrix(row_major)

struct VsOutput {
  float4 position:SV_POSITION;
  float4 normal:NORMAL;
  float3 worldpos: WPOS;
  float3 viewin: VIEWIN;
};

struct VSInput {
  float4 position:POSITION;
  float4 normal:NORMAL;
};

cbuffer c_buffer {
   float4x4 pvw;
   float4x4 world;
   float4   camerapos;
};

VsOutput vs_main(VSInput input) {
  VsOutput o;
  o.position = mul(pvw, input.position);
  o.worldpos = mul(world, input.position).xyz;
  o.viewin = normalize(camerapos - o.worldpos);
  o.normal = mul(world, input.normal);
  return o;
}