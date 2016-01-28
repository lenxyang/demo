#pragma pack_matrix(row_major)

cbuffer c_buffer {
  float4x4 pvw;
  float4x4 world;
};
struct HSCOutput {
  float edge[3]: SV_TessFactor;
  float inside:  SV_InsideTessFactor;
};

struct VsInput {
  float3 position: POSITION;
  float3 normal  : NORMAL;
  float2 texcoord: TEXCOORD;
  float3 tangent : TANGENT;
};

struct DsOutput {
  float4 position: SV_POSITION;
  float3 normal  : NORMAL;
  float2 texcoord: TEXCOORD;
  float3 tangent : TANGENT;
  float3 binormal : BINORMAL;
};
[domain("tri")]
DsOutput ds_main(HSCOutput input, 
                 const OutputPatch<VsInput, 3> tri, 
                 float3 uvw : SV_DomainLocation) {
  DsOutput output;
  float3 v = uvw.x * tri[0].position.xyz 
           + uvw.y * tri[1].position.xyz
           + uvw.z * tri[2].position.xyz;
  float3 normal = uvw.x * tri[0].normal
           + uvw.y * tri[1].normal
           + uvw.z * tri[2].normal;
  float3 tangent = uvw.x * tri[0].tangent
           + uvw.y * tri[1].tangent
           + uvw.z * tri[2].tangent;
  float2 texcoord = uvw.x * tri[0].texcoord
           + uvw.y * tri[1].texcoord
           + uvw.z * tri[2].texcoord;
  output.position = mul(pvw, float4(v, 1.0f));
  output.normal = mul(world, float4(normal, 0.0)).xyz;
  output.texcoord = texcoord;
  output.tangent = mul(world, float4(tangent, 0.0)).xyz;
  return output;
};
