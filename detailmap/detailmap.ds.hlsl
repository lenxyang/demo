#pragma pack_matrix(row_major)

cbuffer c_buffer {
  float4x4 pvw;
  float4x4 world;
};
struct HSCOutput {
  float edge[4]: SV_TessFactor;
  float inside[2]:  SV_InsideTessFactor;
};
struct HsOutput {
  float4 position: POSITION;
};
struct DsOutput {
  float4 position: SV_POSITION;
};
[domain(quad)]
DsOutput ds_main(HSCOutput input, 
                 const OutputPatch<HsOutput, 4> quad, 
                 float2 uv : SV_DomainLocation) {
  DsOutput output;
  float3 v1 = lerp(quad[0].position.xyz, quad[1].position.xyz, uv.x);
  float3 v2 = lerp(quad[3].position.xyz, quad[2].position.xyz, uv.x);
  output.position = mul(pvw, float4(lerp(v1, v2, uv.y), 1.0f));
  return output;
};
