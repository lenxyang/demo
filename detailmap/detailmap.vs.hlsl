#pragma pack_matrix(row_major)

struct VsInput {
  float3 position: POSITION;
  float3 normal  : NORMAL;
  float2 texcoord: TEXCOORD;
  float3 tangent : TANGENT;
};

VsInput vs_main(VsInput input) {
  return input;
};
