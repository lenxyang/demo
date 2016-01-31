#pragma pack_matrix(row_major)

cbuffer cbSetting {
  float weights[5] = {0.0545, 0.2442, 0.4026, 0.2442, 0.0545};
};

Texture2D<float4> input : register(t0);
RWTexture2D<float4> output : register(u0);
[numthreads(10, 1, 1)]
void cs_main(int3 dtid : SV_DispatchThreadID,
             int gidx :SV_GroupIndex) {
  const float w[5] = {0.0545, 0.2442, 0.4026, 0.2442, 0.0545};
  int2 xy = int2(dtid.x, dtid.y);
  float4 v;
  v  = input[int2(dtid.x, dtid.y - 2)] * w[0];
  v += input[int2(dtid.x, dtid.y - 1)] * w[1];
  v += input[int2(dtid.x, dtid.y + 0)] * w[2];
  v += input[int2(dtid.x, dtid.y + 1)] * w[3];
  v += input[int2(dtid.x, dtid.y + 2)] * w[4];
  output[xy] = v;
};
