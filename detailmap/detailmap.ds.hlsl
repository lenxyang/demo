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
  float3 worldpos : WORLDPOS;
  float3 normal  : NORMAL;
  float2 texcoord: TEXCOORD;
  float3 viewin:   VIEWIN;
};

Texture2D normal_map : register(t0);
SamplerState texsampler {
  Filter = MIN_MAG_MIP_LINEAR;
  AddressU = Wrap;
  AddressV = Wrap;
};


float3 NormalSampleToWorldSpace(float3 normal, float3 T, float3 N, float3 B) {
  float3 n = float3(normal * 2.0 - 1.0f);
  float3 r = float3(0, 0, 0);
  r.x = dot(T, n);
  r.y = dot(B, n);
  r.z = dot(N, n);
  return r;
}	

[domain("tri")]
DsOutput ds_main(HSCOutput input, 
                 const OutputPatch<VsOutput, 3> tri,
                 float3 uvw : SV_DomainLocation) {
  DsOutput o;
  float3 pos = uvw.x * tri[0].position
             + uvw.y * tri[1].position
             + uvw.z * tri[2].position;

  float3 normal = uvw.x * tri[0].normal
                + uvw.y * tri[1].normal
                + uvw.z * tri[2].normal;

  float3 tangent = uvw.x * tri[0].tangent
                 + uvw.y * tri[1].tangent
                 + uvw.z * tri[2].tangent;

  float3 binormal = uvw.x * tri[0].binormal
           + uvw.y * tri[1].binormal
           + uvw.z * tri[2].binormal;

  o.texcoord = uvw.x * tri[0].texcoord
             + uvw.y * tri[1].texcoord
             + uvw.z * tri[2].texcoord;

  float4 nm = normal_map.SampleLevel(texsampler, o.texcoord, 0);
  o.normal = NormalSampleToWorldSpace(nm.xyz, tangent, normal, binormal);
  float4 worldpos = float4(pos.xyz + normal * (nm.w - 1.0f) * 2.0f, 1.0);
  o.normal = mul(world, float4(normal, 0.0)).xyz;
  worldpos = mul(world, worldpos);
  o.worldpos = worldpos.xyz;
  o.position = mul(pv, worldpos);
  o.viewin = normalize(eyepos.xyz - worldpos.xyz);
  return o;
};
