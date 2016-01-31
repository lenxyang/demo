#pragma pack_matrix(row_major)

struct VsOutput {
  float4 position:SV_POSITION;
  float4 normal:NORMAL;
  float3 worldpos: WPOS;
  float3 viewin: VIEWIN;
};

struct VSInput {
  float3 position:POSITION;
  float3 normal:NORMAL;
};

cbuffer c_buffer {
   float4x4 pvw;
   float4x4 world;
   float4   camerapos;
};

VsOutput vs_main(VSInput input) {
  VsOutput o;
  float4 pos = float4(input.position, 1.0);
  float4 normal = float4(input.normal, 0.0);
  o.position = mul(pvw, pos);
  o.worldpos = mul(world, pos).xyz;
  o.viewin = normalize(camerapos - o.worldpos);
  o.normal = mul(world, normal);
  return o;
}
