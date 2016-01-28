#pragma pack_matrix(row_major)

cbuffer c_buffer {
  float4x4 pv;
  float4x4 world;
  float4 eyepos;
};
struct HSCOutput {
  float edge[3]: SV_TessFactor;
  float inside:  SV_InsideTessFactor;
};

struct VsOutput {
  float3 position: POSITION;
  float3 normal  : NORMAL;
  float2 texcoord: TEXCOORD;
  float3 tangent : TANGENT;
  float3 binormal : BINORMAL;
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
                 const OutputPatch<VsOutput, 3> tri, 
                 float3 uvw : SV_DomainLocation) {
  DsOutput o;
  float3 pos = uvw.x * tri[0].position
             + uvw.y * tri[1].position
             + uvw.z * tri[2].position;
  float4 worldpos = mul(world, float4(pos, 1.0));
  o.normal = uvw.x * tri[0].normal
           + uvw.y * tri[1].normal
           + uvw.z * tri[2].normal;
  o.tangent = uvw.x * tri[0].tangent
           + uvw.y * tri[1].tangent
           + uvw.z * tri[2].tangent;
  o.binormal = uvw.x * tri[0].binormal
           + uvw.y * tri[1].binormal
           + uvw.z * tri[2].binormal;
  o.texcoord = uvw.x * tri[0].texcoord
             + uvw.y * tri[1].texcoord
             + uvw.z * tri[2].texcoord;
  o.position = mul(pv, worldpos);
  o.normal = mul(world, float4(o.normal, 0.0)).xyz;
  o.tangent = mul(world, float4(o.tangent, 0.0)).xyz;
  return o;
};
