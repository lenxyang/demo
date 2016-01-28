#pragma pack_matrix(row_major)

struct VsInput {
  float3 position: POSITION;
  float3 normal  : NORMAL;
  float2 texcoord: TEXCOORD;
  float3 tangent : TANGENT;
};

struct VsOutput {
  float3 position: POSITION;
  float3 normal  : NORMAL;
  float2 texcoord: TEXCOORD;
  float3 tangent : TANGENT;
  float3 binormal : BINORMAL;
};

VsOutput vs_main(VsInput input) {
  VsOutput o;
  o.position = input.position;
  o.normal = normalize(input.normal);
  o.texcoord = input.texcoord;
  o.tangent = normalize(input.tangent);
  o.binormal = normalize(cross(o.tangent, o.normal));
  return o;
};
