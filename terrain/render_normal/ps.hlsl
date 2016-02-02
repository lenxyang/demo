#pragma pack_matrix(row_major)


struct GsOutput {
  float4 position:SV_POSITION;
};

float4 ps_main(GsOutput o):SV_TARGET {
  float4 color = float4(0.0, 1.0f, 0.0f, 1.0f);
  return color;
};
