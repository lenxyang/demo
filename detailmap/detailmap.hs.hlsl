#pragma pack_matrix(row_major)
cbuffer c_buffer {
  float4 edge;
  float4 inside;
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
HSCOutput PatchConstantFunc(InputPatch<VsOutput, 4> input, 
                            uint patchid : SV_PrimitiveID) {
  HSCOutput output;
  output.edge[0] = edge.x;
  output.edge[1] = edge.y;
  output.edge[2] = edge.z;
  output.edge[3] = edge.w;
  output.inside[0] = inside.x;
  output.inside[1] = inside.y;
  return output;
}

[domain(quad)]
[partitioning(integer)]
[outputtopology(triangle_cw)]
[outputcontrolpoints(4)]
[patchconstantfunc(PatchConstantFunc)]
[maxtessfactor(64.0f)]
HsOutput hs_main(InputPatch<VsOutput, 4> patch, 
                 uint pointid: SV_OutputControlPointID, 
                 uint patchid: SV_PrimitiveID) {
  HsOutput output;
  output.position = patch[pointid].position;
  return output;
};
