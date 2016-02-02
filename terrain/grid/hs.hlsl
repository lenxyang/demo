#pragma pack_matrix(row_major)
cbuffer c_buffer {
  float4 eyepos;
};
struct VsOutput {
  float4 position: SV_POSITION;
};
struct HSCOutput {
  float edge[4]: SV_TessFactor;
  float inside[2]:  SV_InsideTessFactor;
};
struct HsOutput {
  float4 position: POSITION;
};
float gMinDist = 0.0f;
float gMaxDist = 10.0f;
float gMinTess = 0.0f;
float gMaxTess = 2.0f;
float CalcTesFactor(float4 p) {
  float d = distance(p.xyz, float3(0, 0, 0));
  float s = 1.0 - saturate((d - gMinDist) / (gMaxDist - gMinDist));
  return 1;
  return pow(2, floor(lerp(gMinTess, gMaxTess, s)));
}
HSCOutput PatchConstantFunc(InputPatch<VsOutput, 4> patch, 
                            uint patchid : SV_PrimitiveID) {
  HSCOutput output;
  float4 e0 = 0.5f * (patch[0].position + patch[1].position);
  float4 e1 = 0.5f * (patch[1].position + patch[2].position);
  float4 e2 = 0.5f * (patch[2].position + patch[3].position);
  float4 e3 = 0.5f * (patch[3].position + patch[0].position);
  float4 c = 0.25f * (patch[0].position + patch[1].position + 
                      patch[2].position + patch[3].position);
  output.edge[0] = CalcTesFactor(e0);
  output.edge[1] = CalcTesFactor(e1);
  output.edge[2] = CalcTesFactor(e2);
  output.edge[3] = CalcTesFactor(e3);
  output.inside[0] = CalcTesFactor(c);
  output.inside[1] = output.inside[0];
  return output;
}
[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PatchConstantFunc")]
[maxtessfactor(64.0f)]
HsOutput hs_main(InputPatch<VsOutput, 4> patch, 
                 uint pointid: SV_OutputControlPointID, 
                 uint patchid: SV_PrimitiveID) {
  HsOutput output;
  output.position = patch[pointid].position;
  return output;
};
