#pragma pack_matrix(row_major)

cbuffer c_buffer {
  float4 edge;
};

struct VsOutput {
  float3 position: POSITION;
  float3 normal  : NORMAL;
  float2 texcoord: TEXCOORD;
  float3 tangent : TANGENT;
  float3 binormal : BINORMAL;
};

struct HSCOutput {
  float edge[3]: SV_TessFactor;
  float inside:  SV_InsideTessFactor;
};
struct HsOutput {
  float4 position: POSITION;
};

HSCOutput PatchConstantFunc(InputPatch<VsOutput, 3> input, 
                            uint patchid : SV_PrimitiveID) {
  HSCOutput output;
  output.edge[0] = edge.x;
  output.edge[1] = edge.y;
  output.edge[2] = edge.z;
  output.inside = edge.w;
  output.edge[0] = 3;
  output.edge[1] = 3;
  output.edge[2] = 3;
  output.inside = 1;
  return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchConstantFunc")]
[maxtessfactor(64.0f)]

VsOutput hs_main(InputPatch<VsOutput, 3> patch, 
                 uint pointid: SV_OutputControlPointID, 
                 uint patchid: SV_PrimitiveID) {
  return patch[pointid];
};
