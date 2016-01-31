#pragma pack_matrix(row_major)

struct VsOutput {
  float4 position:SV_POSITION;
  float3 worldpos: WPOS;
  float3 normal:NORMAL;
  float3 viewin: VIEWIN;
  float2 texcoord: TEXCOORD0;
};

struct VSInput {
  float3 position:POSITION;
  float3 normal:NORMAL;
  float2 texcoord: TEXCOORD0;
  float3 tang: TANGENT;
};

VSInput vs_main(VSInput input) {
  return input;
}
