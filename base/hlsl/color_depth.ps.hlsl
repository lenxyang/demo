// using row_major
#pragma pack_matrix(row_major)

struct VsOutput {
  float4 position:SV_POSITION;
};

float4 ps_main(VsOutput o):SV_TARGET {
  float depth = o.position.z;
  if (depth < 0.9f) {
      return float4(1.0, 0.0, 0.0, 1.0);
  } else if (depth > 0.925f) {
      return float4(0.0f, 1.0f, 0.0, 1.0);
  } else {
      return float4(0.0f, 0.0f, 1.0, 1.0);
  }
}
