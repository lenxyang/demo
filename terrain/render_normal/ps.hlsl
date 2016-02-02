#pragma pack_matrix(row_major)


struct VsOutput {
  float4 position:SV_POSITION;
  float3 normal: NORMAL;
  float2 texcoord: TEXCOORD;
};

float4 ps_main(DsOutput o):SV_TARGET {
  float4 color = float4(0.8, 0.8f, 0.8f, 1.0f);
};
